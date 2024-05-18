#include "compressionLZ.h"
#include "base\baseMath.h"

CompressBackReference compressionLZ4NaiveLinearBackRefMatch(U8Array backRefWindow, U8Array lookaheadWindow)
{
    CompressBackReference br = {0};

    for(u64 i = 0; i < backRefWindow.len; i++)
    {
        if(lookaheadWindow.len != 0 && backRefWindow.data[i] == lookaheadWindow.data[0])
        {
            for(u64 j = lookaheadWindow.len; j > 0; --j)
            {
                u64 len = baseLesser(j, backRefWindow.len - i);
                if(!BASE_MEMCMP(backRefWindow.data + i, lookaheadWindow.data, len))
                {
                    br.found = true;
                    br.length = len;
                    br.distance = backRefWindow.len - i;
                    return br;
                }
            }
        }
    }

    return br;
}

CompressionLZ4MBlockCompoundList compressionLZ4MCollectBlockCompounds(BaseArena *arena, U8Array input)
{
    CompressionLZ4MBlockCompoundList compList = {0};

    u64 backRefWindowMaxLen = BASE_KILOBYTES(32);
    u64 lookaheadWindowMaxLen = BASE_BYTES(255);

    U8Array backRefWindow = {.data = null, .len = 0};
    U8Array lookaheadWindow = {.data = null, .len = 0};

    u64 bytesRead = 0;
    while(bytesRead < input.len)
    {
        u64 backRefWindowLen = (baseLesser(bytesRead, backRefWindowMaxLen));
        u64 lookaheadWindowLen = (baseLesser((input.len - bytesRead), lookaheadWindowMaxLen));

        backRefWindow.data = (input.data + bytesRead) - backRefWindowLen;
        backRefWindow.len = backRefWindowLen;

        lookaheadWindow.data = input.data + bytesRead;
        lookaheadWindow.len = lookaheadWindowLen;

        CompressBackReference br = compressionLZ4NaiveLinearBackRefMatch(backRefWindow, lookaheadWindow);
        if(br.found && br.length > COMPRESSION_LZ4M_MINIMUM_MATCHLEN)
        {
            CompressionLZ4MBlockCompound *node = null;
            node = baseArenaPush(arena, sizeof(CompressionLZ4MBlockCompound));
            node->kind = COMPRESSION_LZM4_BLOCK_COMPOUND_BACKREF;
            node->backref = br;

            BaseListNodePushLast(compList, node);

            bytesRead += br.length;
        }
        else
        {
            CompressionLZ4MBlockCompound *node = null;
            if(BASE_ANY(compList))
            {
                if(compList.last->kind == COMPRESSION_LZM4_BLOCK_COMPOUND_LITERAL)
                {
                    node = compList.last;
                }
            }

            if(node == null)
            {
                node = baseArenaPush(arena, sizeof(CompressionLZ4MBlockCompound));
                node->kind = COMPRESSION_LZM4_BLOCK_COMPOUND_LITERAL;
                node->literals.data = input.data + bytesRead;

                BaseListNodePushLast(compList, node);
            }

            bytesRead++;
            node->literals.len++;
        }
    }

    return compList;
}

u64 compressionLZ4MCalculateCompressedOutputSize(CompressionLZ4MBlockCompoundList compList)
{
    u64 size = 0;
    BASE_LIST_FOREACH(CompressionLZ4MBlockCompound, node, compList)
    {
        switch(node->kind)
        {
            case COMPRESSION_LZM4_BLOCK_COMPOUND_LITERAL:
            {
                size += 1; //for token byte
                size += (node->literals.len >= 15) ? (u64)ceil((f64)(node->literals.len - 15) / 255) : 0; // for extra len bytes
                size += node->literals.len; // bytes for literal
                size += (node->next != null) ? 2 : 1; // offset byte

                u64 matchlenNumBytes = 0;
                if(node->next != null)
                {
                    matchlenNumBytes = (node->next->backref.length >= 15) ? 
                    (u64)ceil((f64)((i64)node->next->backref.length - 15 - COMPRESSION_LZ4M_MINIMUM_MATCHLEN) / 255) : 0;
                }

                size += matchlenNumBytes;

            }break;

            case COMPRESSION_LZM4_BLOCK_COMPOUND_BACKREF:
            {
                continue;
            }break;
        }
    }
    
    return size;
}

//TODO: important, i cant just skip the COMPRESSION_LZM4_BLOCK_COMPOUND_BACKREF in the switch,
//sine there might be multiple BACKREF in a row back to back
U8Array compressionLZ4MCompress(BaseArena *arena, U8Array input, CompressOptions *options)
{
    CompressOptions compressOpt = {0};
    if(options != null) compressOpt = *options;

    U8Array retCompressed = {0};
    BaseArenaTemp temp = baseTempBegin(null, 0);
    {
        CompressionLZ4MBlockCompoundList compList = compressionLZ4MCollectBlockCompounds(temp.arena, input);

        u64 outputCalculatedSize = compressionLZ4MCalculateCompressedOutputSize(compList);
        U8Array output = {.data = baseArenaPushNoZero(arena, outputCalculatedSize), .len = outputCalculatedSize};

        u64 bytesWritten = 0;
        BASE_LIST_FOREACH(CompressionLZ4MBlockCompound, node, compList)
        {
            u64 tokLitLen = 0;
            u64 tokMatchLen = 0;
            u64 offset = 0;
            switch(node->kind)
            {
                case COMPRESSION_LZM4_BLOCK_COMPOUND_LITERAL:
                {
                    tokLitLen = (node->literals.len >= 15) ? 15 : (u8)node->literals.len;
                    if(node->next != null)
                    {
                        offset = node->next->backref.distance;
                        tokMatchLen = (node->next->backref.length - COMPRESSION_LZ4M_MINIMUM_MATCHLEN >= 15) ? 15 : (u8)node->next->backref.length - COMPRESSION_LZ4M_MINIMUM_MATCHLEN;
                    }

                    output.data[bytesWritten++] = (u8)(tokLitLen << 4) | (u8)tokMatchLen;
                    if (tokLitLen == 15)
                    {
                        tokLitLen = node->literals.len;
                        for(u64 i = 0; i < (u64)ceil((f64)(tokLitLen - 15) / 255); i++)
                        {
                            u64 val = (tokLitLen - 15 - (i * 255));
                            output.data[bytesWritten++] = (val > 255) ? 255 : (u8)val;
                        }
                    }

                    for(u64 i = 0; i < node->literals.len; i++)
                    {
                        output.data[bytesWritten++] = node->literals.data[i];
                    }

                    output.data[bytesWritten++] = ((u8*)&offset)[0];
                    if(offset > 0)
                    {
                        output.data[bytesWritten++] = ((u8*)&offset)[1];
                    }

                    if (tokMatchLen == 15)
                    {
                        tokMatchLen = node->next->backref.length;
                        for(u64 i = 0; i < (u64)ceil((f64)(tokMatchLen - 15 - COMPRESSION_LZ4M_MINIMUM_MATCHLEN) / 255); i++)
                        {
                            u64 val = ((i64)tokMatchLen - 15 - COMPRESSION_LZ4M_MINIMUM_MATCHLEN - (i * 255));
                            output.data[bytesWritten++] = (val > 255) ? 255 : (u8)val;
                        }
                    }
                }break;

                case COMPRESSION_LZM4_BLOCK_COMPOUND_BACKREF:
                {
                    continue;
                }break;
            }
        }

        retCompressed = output;
    }
    baseTempEnd(temp);

    return retCompressed;
}

bool compressionLZ4MUncompress(U8Array input, U8Array output)
{
    u64 bytesRead = 0;
    u64 bytesWritten = 0;
    while(bytesRead < input.len && bytesWritten < output.len)
    {
        u8 tokByte = input.data[bytesRead++];

        u64 len = tokByte >> 4;

        if(len == 15)
        {
            do 
            {
                len += input.data[bytesRead++];
            }while(input.data[bytesRead] == 255);
        }

        for(u64 i = 0; i < len; i++)
        {
            output.data[bytesWritten++] = input.data[bytesRead++];
        }

        if(bytesRead >= input.len) break;

        u16 offset = input.data[bytesRead++];  
        if(offset == 0)
        {
            continue;
        }

        offset |= (input.data[bytesRead++] << 8);
        u64 matchlen = (tokByte & 0b0000'1111) + COMPRESSION_LZ4M_MINIMUM_MATCHLEN;
        if(matchlen == (15 + COMPRESSION_LZ4M_MINIMUM_MATCHLEN))
        {
            do 
            {
                matchlen += input.data[bytesRead++];
            }while(input.data[bytesRead] == 255);
        }

        for(u64 i = 0; i < matchlen; i++)
        {
            output.data[bytesWritten + i] = output.data[(bytesWritten - offset) + i];
        }

        bytesWritten += matchlen;
    }

    return true;
}