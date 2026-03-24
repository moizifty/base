#include "compressionLZ.h"
#include "base/baseMath.h"
#include "base/baseHash.h"

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

CompressBackReference compressionLZ4NaiveLinearBackRefMatch2(Arena *arena, CompressionLZ4MSubByteDict *dict, U8Array backRefWindow, U8ArrayList lookaheadsList)
{
    CompressBackReference br = {0};

    for(u64 i = 0; i < backRefWindow.len; i++)
    {
        U8Array subbytes = {.data = backRefWindow.data + i, .len = backRefWindow.len - i};
        if(lookaheadsList.len != 0 && backRefWindow.data[i] == lookaheadsList.first->val.data[0])
        {
            BASE_LIST_FOREACH(U8ArrayListNode, node, lookaheadsList)
            {
                u64 len = baseLesser(node->val.len, subbytes.len);
                if(!BASE_MEMCMP(backRefWindow.data + i, node->val.data, len))
                {
                    br.found = true;
                    br.length = len;
                    br.distance = backRefWindow.len - i;
                    return br;
                }
            }
        }


        // add to to dict
        {
            u64 index = baseHashDJB2(subbytes.data, subbytes.len) % dict->slots.len;

            CompressionLZ4MSubByteDictEntry *entry = arenaPush(arena, sizeof(CompressionLZ4MSubByteDictEntry));
            entry->subbytes = subbytes;
            BaseListNodePushLast(dict->slots.data[index], entry);
        }
    }

    return br;
}


CompressBackReference compressionLZ4DictBackRefMatch(Arena *arena, CompressionLZ4MSubByteDict *dict, U8Array backRefWindow, U8Array lookaheadWindow)
{
    CompressBackReference br = {0};

    ArenaTemp temp = baseTempBegin(&arena, 1);
    {
        U8ArrayList lookaheadsList = {0};

        for(u64 i = lookaheadWindow.len; i > 0; --i)
        {
            U8ArrayListPushLast(temp.arena, &lookaheadsList, (U8Array){.data = lookaheadWindow.data, .len = i});
        }
        
        BASE_LIST_FOREACH(U8ArrayListNode, node, lookaheadsList)
        {
            u64 index = baseHashDJB2(node->val.data, node->val.len) % dict->slots.len;

            CompressionLZ4MSubByteDictSlot slot = dict->slots.data[index];
            BASE_LIST_FOREACH(CompressionLZ4MSubByteDictEntry, slotEntry, slot)
            {
                if(slotEntry->subbytes.len == node->val.len)
                {
                    if(!BASE_MEMCMP(slotEntry->subbytes.data, node->val.data, node->val.len))
                    {
                        // found, use
                        br.found = true;
                        br.distance = (backRefWindow.data + backRefWindow.len) - slotEntry->subbytes.data;
                        br.length = node->val.len;

                        baseTempEnd(temp); 
                        return br;
                    }
                }


            }
        }

        if(br.found != true)
        {
            br = compressionLZ4NaiveLinearBackRefMatch2(arena, dict, backRefWindow, lookaheadsList);
        }
    }
    baseTempEnd(temp);

    return br;
}

CompressionLZ4MBlockCompoundList compressionLZ4MCollectBlockCompounds(Arena *arena, U8Array input)
{
    CompressionLZ4MBlockCompoundList compList = {0};

    u64 backRefWindowMaxLen = BASE_KILOBYTES(64);
    u64 lookaheadWindowMaxLen = BASE_BYTES(128);

    U8Array backRefWindow = {.data = null, .len = 0};
    U8Array lookaheadWindow = {.data = null, .len = 0};

    u64 bytesRead = 0;

    CompressionLZ4MSubByteDict backrefDict = {0};
    backrefDict.slots.len = 569;
    backrefDict.slots.data = arenaPush(arena, sizeof(CompressionLZ4MSubByteDictSlot) * backrefDict.slots.len);

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
            node = arenaPush(arena, sizeof(CompressionLZ4MBlockCompound));
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
                node = arenaPush(arena, sizeof(CompressionLZ4MBlockCompound));
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

u64 compressionLZ4MCalculateNumOfExtraMatchlenBytes(CompressionLZ4MBlockCompound block)
{
    u64 size = 0;
    switch(block.kind)
    {
        case COMPRESSION_LZM4_BLOCK_COMPOUND_LITERAL:
        {
            if(block.next != null)
            {
                size = compressionLZ4MCalculateNumOfExtraMatchlenBytes(*block.next);
            }
        }break;

        case COMPRESSION_LZM4_BLOCK_COMPOUND_BACKREF:
        {
            i64 l = (i64)block.backref.length - 15 - COMPRESSION_LZ4M_MINIMUM_MATCHLEN;
            while(l >= 0)
            {
                size += 1;

                l -= 255;
            }
        }break;
    }

    return size;
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

                {
                    i64 l = (i64) node->literals.len - 15;
                    while(l >= 0)
                    {
                        size += 1;

                        l -= 255;
                    }
                }

                size += node->literals.len; // bytes for literal
                size += 2;
                size += compressionLZ4MCalculateNumOfExtraMatchlenBytes(*node);

                // the node after a literal cmpound should always be a backref,
                // you want to skip processing tht one in the next iteration
                //since we just did it above
                if(node->next != null)
                {
                    node = node->next;
                }
            }break;

            case COMPRESSION_LZM4_BLOCK_COMPOUND_BACKREF:
            {
                size += 1; //for token byte
                size += 2; // offset byte

                size += compressionLZ4MCalculateNumOfExtraMatchlenBytes(*node);
            }break;
        }
    }
    
    return size;
}

U8Array compressionLZ4MCompress(Arena *arena, U8Array input, CompressOptions *options)
{
    BASE_UNUSED_PARAM(options);
    // CompressOptions compressOpt = {0};
    // if(options != null) compressOpt = *options;

    U8Array retCompressed = {0};
    ArenaTemp temp = baseTempBegin(null, 0);
    {
        CompressionLZ4MBlockCompoundList compList = compressionLZ4MCollectBlockCompounds(temp.arena, input);

        u64 outputCalculatedSize = compressionLZ4MCalculateCompressedOutputSize(compList);
        U8Array output = {.data = arenaPushNoZero(arena, outputCalculatedSize), .len = outputCalculatedSize};

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
                        i64 l = (i64) tokLitLen - 15;
                        while(l >= 0)
                        {
                            i64 val = baseLesser(255, l);
                            output.data[bytesWritten++] = (u8)val;

                            l -= 255;
                        }
                    }

                    for(u64 i = 0; i < node->literals.len; i++)
                    {
                        output.data[bytesWritten++] = node->literals.data[i];
                    }

                    output.data[bytesWritten++] = ((u8*)&offset)[0];
                    output.data[bytesWritten++] = ((u8*)&offset)[1];

                    if (tokMatchLen == 15)
                    {
                        tokMatchLen = node->next->backref.length;
                        i64 l = (i64) tokMatchLen - 15 - COMPRESSION_LZ4M_MINIMUM_MATCHLEN;
                        while(l >= 0)
                        {
                            i64 val = baseLesser(255, l);
                            output.data[bytesWritten++] = (u8)val;

                            l -= 255;
                        }
                    }

                    // the node after a literal cmpound should always be a backref,
                    // you want to skip processing tht one in the next iteration
                    //since we just did it above
                    if(node->next != null)
                    {
                        node = node->next;
                    }
                }break;

                case COMPRESSION_LZM4_BLOCK_COMPOUND_BACKREF:
                {
                    tokLitLen = 0;
                    offset = node->backref.distance;
                    tokMatchLen = (node->backref.length - COMPRESSION_LZ4M_MINIMUM_MATCHLEN >= 15) ? 15 : (u8)node->backref.length - COMPRESSION_LZ4M_MINIMUM_MATCHLEN;

                    output.data[bytesWritten++] = (u8)(tokLitLen << 4) | (u8)tokMatchLen;
                    output.data[bytesWritten++] = ((u8*)&offset)[0];
                    output.data[bytesWritten++] = ((u8*)&offset)[1];

                    if (tokMatchLen == 15)
                    {
                        tokMatchLen = node->backref.length;
                        i64 l = (i64) tokMatchLen - 15 - COMPRESSION_LZ4M_MINIMUM_MATCHLEN;
                        while(l >= 0)
                        {
                            i64 val = baseLesser(255, l);
                            output.data[bytesWritten++] = (u8)val;

                            l -= 255;
                        }
                    }
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
            while(input.data[bytesRead] == 255)
            {
                len += 255;
                bytesRead++;
            };

            len += input.data[bytesRead++];
        }

        for(u64 i = 0; i < len; i++)
        {
            output.data[bytesWritten++] = input.data[bytesRead++];
        }

        if(bytesRead >= input.len) break;

        u16 offset = input.data[bytesRead++];  
        offset |= (input.data[bytesRead++] << 8);
        u64 matchlen = (tokByte & 0b00001111) + COMPRESSION_LZ4M_MINIMUM_MATCHLEN;
        if(matchlen == (15 + COMPRESSION_LZ4M_MINIMUM_MATCHLEN))
        {
            while(input.data[bytesRead] == 255)
            {
                matchlen += 255;
                bytesRead++;
            };

            matchlen += input.data[bytesRead++];
        }

        for(u64 i = 0; i < matchlen; i++)
        {
            output.data[bytesWritten + i] = output.data[(bytesWritten - offset) + i];
        }

        bytesWritten += matchlen;
    }

    return true;
}