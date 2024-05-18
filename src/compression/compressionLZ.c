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

bool compressionLZ4Compress(U8Array input, U8Array output, CompressOptions *options)
{
    BaseArenaTemp temp = baseTempBegin(null, 0);
    {
        u64 backRefWindowMaxLen = BASE_KILOBYTES(32);
        u64 lookaheadWindowMaxLen = BASE_BYTES(255);

        U8Array backRefWindow = {.data = null, .len = 0};
        U8Array lookaheadWindow = {.data = null, .len = 0};

        u64 bytesRead = 0;
        u64 bytesWritten = 0;
        while(bytesRead < input.len)
        {
            u64 backRefWindowLen = (baseLesser(bytesRead, backRefWindowMaxLen));
            u64 lookaheadWindowLen = (baseLesser((input.len - bytesRead), backRefWindowMaxLen));

            backRefWindow.data = (input.data + bytesRead) - backRefWindowLen;
            backRefWindow.len = backRefWindowLen;

            lookaheadWindow.data = input.data + bytesRead;
            lookaheadWindow.len = lookaheadWindowLen;

            CompressBackReference br = compressionLZ4NaiveLinearBackRefMatch(backRefWindow, lookaheadWindow);
            if(br.found && br.length > 1)
            {
                printf("<%llu, %llu>", br.distance, br.length);
                bytesRead += br.length;
            }
            else
            {
                printf("%c", input.data[bytesRead]);
                bytesRead++;
            }
        }
    }
    baseTempEnd(temp);
}

bool compressionLZ4Uncompress(U8Array input, U8Array output)
{
    u64 bytesRead = 0;
    u64 bytesWritten = 0;
    while(bytesWritten < output.len)
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
        offset |= (input.data[bytesRead++] << 8);

        u64 matchlen = ((tokByte & 0b1111'0000) >> 4) + 4;
        if(matchlen == 19)
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