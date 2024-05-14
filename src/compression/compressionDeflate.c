#include "compressionDeflate.h"
#include "log\log.h"

u8 gDeflateLiteralSymsLengths[COMPRESSION_DEFLATE_NUM_LITERAL_CODES] =
{
   8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8, 8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
   8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8, 8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
   8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8, 8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
   8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8, 8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
   8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8, 9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,
   9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9, 9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,
   9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9, 9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,
   9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9, 9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,
   7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7, 7,7,7,7,7,7,7,7,8,8,8,8,8,8,8,8
};

u8 gDeflateDistancesSymsLengths[COMPRESSION_DEFLATE_NUM_DISTANCE_CODES] =
{
   5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 
   5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
   5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
};

u8 gDeflateLengthExtraBits[COMPRESSION_DEFLATE_NUM_LITERAL_CODES] =
{
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,1,1,1,1,2,2,2,2,3,3,3,3,4,4,
    4,4,5,5,5,5,0
};

u16 gDeflateLengthBase[COMPRESSION_DEFLATE_NUM_LITERAL_CODES] =
{
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,3,4,5,6,7,8,  9,10,11,13,15,17,19,23,27,31,35,
    43,51,59,67,83,99,115,131,163,  195,227,258,
};

u8 gDeflateDistancesExtraBits[COMPRESSION_DEFLATE_NUM_DISTANCE_CODES] =
{
    0,0,0,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7,8,8,9,9,10,10,11,11,12,12,
    13,13
};

u16 gDeflateDistancesBase[] =
{
    1,2,3,4,5,7,9,13,17,25,33,49,65,97,129,193,257,385,513,769,
    1025,1537,2049,3073,4097,6145,8193,12289,16385,24577,
};

void compressionDeflateGenerateHuffmanCodes(U8Array codeLengths, u32 *outCodes)
{
    u32 nextCode[COMPRESSION_DEFLATE_MAX_CODE_LENGTH] = {0};
    u32 sizes[COMPRESSION_DEFLATE_MAX_CODE_LENGTH] = {0};

    for(u64 i = 0; i < codeLengths.len; i++)
    {
        sizes[codeLengths.data[i]]++;
    }

    u32 code = 0;
    sizes[0] = 0;
    for(u64 i = 1; i <= COMPRESSION_DEFLATE_MAX_CODE_LENGTH; i++)
    {
        code = (code + sizes[i - 1]) << 1;
        nextCode[i] = code;
    }

    for (u64 n = 0; n < codeLengths.len; n++) {
        u8 len = codeLengths.data[n];
        if (len != 0) {
            outCodes[n] = nextCode[len];
            nextCode[len]++;
        }
    }
}
u64 compressionDeflateDecodeHuffmanCode(BaseBitstream *stream, U8Array symbolLens, u32 *symbolCodes)
{
    for(u64 i = 0; i < symbolLens.len; i++)
    {
        u64 code = 0;
        if(symbolLens.data[i] != 0 && baseBitstreamPeekBitsReversedAsU64(stream, symbolLens.data[i], &code))
        {
            if(symbolCodes[i] == code)
            {
                baseBitstreamConsumeBits(stream, symbolLens.data[i]);
                return i;
            }
        }
    }

    return 0;
}
u64 compressionDeflateDecodeHuffmanBlock(BaseBitstream *stream, CompressionDeflateDecodeHuffmanBlockInput input)
{
    u64 bytesWritten = 0;
    while(true)
    {
        u64 c = compressionDeflateDecodeHuffmanCode(stream, input.literalSymsLens, input.literalSymsCodes);
        if(c == 256)
        {
            break;
        }

        if(c < 256)
        {
            input.outBuf.data[input.writeOffset + bytesWritten] = (u8)c;
            bytesWritten++;
        }
        else if(c >= 257 && c <= 285)
        {
            u8 extraLengthBits = gDeflateLengthExtraBits[c];
            u64 extraLength = 0;
            baseBitstreamPopBitsAsU64(stream, extraLengthBits, &extraLength);
            u64 length = gDeflateLengthBase[c] + extraLength;

            u64 d = compressionDeflateDecodeHuffmanCode(stream, input.distSymsLens, input.distSymsCodes);
            u16 extraDistBits = gDeflateDistancesExtraBits[d];
            u64 extraDist = 0;
            baseBitstreamPopBitsAsU64(stream, extraDistBits, &extraDist);
            u64 dist = gDeflateDistancesBase[d] + extraDist;
            u64 bytesWrittenBefore = input.writeOffset + bytesWritten;
            for(u64 i = 0; i < length; i++)
            {
                u64 readIndex = (bytesWrittenBefore - dist) + i;

                input.outBuf.data[+input.writeOffset + bytesWritten] = input.outBuf.data[readIndex];
                bytesWritten++;
            }
        }
        else
        {
            logProgErrorFmt("Encountered invalid decoded huffman code '%llu'", c);
        }
    }

    return bytesWritten;
}

CompressionDeflateUncompressedOutput compressionDeflateUncompress(BaseBitstream *inputStream, U8Array *outBuffer)
{
    CompressionDeflateUncompressedOutput output = {0};

    u64 bytesWritten = 0;
    bool finalBlock;
    u8 compressionType;
    do
    {
        baseBitstreamPopBitsAsU8(inputStream, 1, &finalBlock);
        baseBitstreamPopBitsAsU8(inputStream, 2, &compressionType);
        
        switch(compressionType)
        {
            // no compression
            case 0:
            {
                baseBitstreamPopTillNextByte(inputStream);

                u16 blockLen;
                baseBitstreamPopU16LE(inputStream, &blockLen);

                u16 blockLenComplement;
                baseBitstreamPopU16LE(inputStream, &blockLenComplement);

                u64 byteIndex = inputStream->bitIndex / 8;
                BASE_MEMCPY(outBuffer->data + bytesWritten, inputStream->bytes.data + byteIndex, blockLen);
                bytesWritten += blockLen;
                
                inputStream->bitIndex += (u64)blockLen * 8;

            }break;

            // fixed with huffman
            case 1:
            {
                u32 literalCodes[COMPRESSION_DEFLATE_NUM_LITERAL_CODES];
                u32 distCodes[COMPRESSION_DEFLATE_NUM_DISTANCE_CODES];

                compressionDeflateGenerateHuffmanCodes((U8Array){.data = gDeflateLiteralSymsLengths, COMPRESSION_DEFLATE_NUM_LITERAL_CODES}, literalCodes);
                compressionDeflateGenerateHuffmanCodes((U8Array){.data = gDeflateDistancesSymsLengths, COMPRESSION_DEFLATE_NUM_DISTANCE_CODES}, distCodes);

                CompressionDeflateDecodeHuffmanBlockInput input = 
                {
                    .outBuf = *outBuffer,
                    .writeOffset = bytesWritten,
                    .distSymsCodes = distCodes,
                    .distSymsLens = (U8Array){.data = gDeflateDistancesSymsLengths, COMPRESSION_DEFLATE_NUM_DISTANCE_CODES},
                    .literalSymsCodes = literalCodes,
                    .literalSymsLens = (U8Array){.data = gDeflateLiteralSymsLengths, COMPRESSION_DEFLATE_NUM_LITERAL_CODES},
                };

                bytesWritten += compressionDeflateDecodeHuffmanBlock(inputStream, input);
                
            }break;

            // dynamic with huffman
            case 2:
            {
                u8 hlit, hdist, uclen;
                baseBitstreamPopBitsAsU8(inputStream, 5, &hlit);
                baseBitstreamPopBitsAsU8(inputStream, 5, (&hdist));
                baseBitstreamPopBitsAsU8(inputStream, 4, (&uclen));

                u32 numLiteralCodes = (u32)hlit + 257;
                u32 numDistCodes = (u32)hdist + 1;
                u32 numCLCodes = (u32)uclen + 4;

                u8 clLengthsIndexRedirect[19] =
                {
                    16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15
                };

                u8 clSymLengths[BASE_ARRAY_SIZE(clLengthsIndexRedirect)] = {0};
                u32 clSymCodes[BASE_ARRAY_SIZE(clLengthsIndexRedirect)] = {0};
                for(u64 i = 0; i < numCLCodes; i++)
                {
                    u8 c = 0;
                    baseBitstreamPopBitsAsU8(inputStream, 3, &c);
                    clSymLengths[clLengthsIndexRedirect[i]] = c;
                }

                compressionDeflateGenerateHuffmanCodes((U8Array){.data = clSymLengths, .len = BASE_ARRAY_SIZE(clSymLengths)}, clSymCodes);

                u8 literalDistSymsLengths[COMPRESSION_DEFLATE_NUM_LITERAL_CODES + COMPRESSION_DEFLATE_NUM_DISTANCE_CODES] = {0};
                U8Array literalDistSymsLengthsArray = {.data = literalDistSymsLengths};

                while(literalDistSymsLengthsArray.len < numLiteralCodes + numDistCodes)
                {
                    u64 c = compressionDeflateDecodeHuffmanCode(inputStream, (U8Array){.data = clSymLengths, .len = BASE_ARRAY_SIZE(clSymLengths)}, clSymCodes);
                    u8 repeatCount = 0;
                    u8 repeatVal = 0;

                    if(c <= 15)
                    {
                        repeatCount = 1;
                        repeatVal = (u8)c;
                    }
                    else if(c == 16)
                    {
                        repeatVal = literalDistSymsLengthsArray.data[literalDistSymsLengthsArray.len - 1];
                        baseBitstreamPopBitsAsU8(inputStream, 2, &repeatCount);
                        repeatCount += 3;
                    }
                    else if(c == 17)
                    {
                        repeatVal = 0;
                        baseBitstreamPopBitsAsU8(inputStream, 3, &repeatCount);
                        repeatCount += 3;
                    }
                    else if(c == 18)
                    {
                        repeatVal = 0;
                        baseBitstreamPopBitsAsU8(inputStream, 7, &repeatCount);
                        repeatCount += 11;
                    }

                    while(repeatCount--)
                    {
                        literalDistSymsLengthsArray.data[literalDistSymsLengthsArray.len++] = repeatVal;
                    }
                }

                u32 literalCodes[COMPRESSION_DEFLATE_NUM_LITERAL_CODES];
                u32 distCodes[COMPRESSION_DEFLATE_NUM_DISTANCE_CODES];

                compressionDeflateGenerateHuffmanCodes((U8Array){.data = literalDistSymsLengths, numLiteralCodes}, literalCodes);
                compressionDeflateGenerateHuffmanCodes((U8Array){.data = literalDistSymsLengths + numLiteralCodes, numDistCodes}, distCodes);

                CompressionDeflateDecodeHuffmanBlockInput input = 
                {
                    .outBuf = *outBuffer,
                    .writeOffset = bytesWritten,
                    .distSymsCodes = distCodes,
                    .distSymsLens = {.data = literalDistSymsLengths + numLiteralCodes, .len = numDistCodes},

                    .literalSymsCodes = literalCodes,
                    .literalSymsLens = {.data = literalDistSymsLengths, .len = numLiteralCodes},
                };

                bytesWritten += compressionDeflateDecodeHuffmanBlock(inputStream, input);
            }break;

            default:
            {
                logProgErrorFmt("Error, deflate, unregognised compression type '%d'", compressionType);
            }break;
        }
    }while(!finalBlock);


    return output;
}