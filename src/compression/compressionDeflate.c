#include "compressionDeflate.h"

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

void compressionDeflateGenerateHuffmanCodes(U8Array codeLengths, u32 *outCodes)
{
    u32 nextCode[COMPRESSION_DEFLATE_MAX_CODE_LENGTH] = {0};
    uint_fast32_t sizes[COMPRESSION_DEFLATE_MAX_CODE_LENGTH] = {0};

    for(u64 i = 0; i < codeLengths.len; i++)
    {
        sizes[codeLengths.data[i]]++;
    }

    u32 code = 0;
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
        if(baseBitstreamPeekBitsReversedAsU64Impl(stream, symbolLens.data[i], &code))
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

CompressionDeflateUncompressedOutput compressionDeflateUncompress(BaseBitstream *inputStream, ArrayView *outBuffer)
{
    CompressionDeflateUncompressedOutput output = {0};

    bool finalBlock;
    u8 compressionType;
    baseBitstreamPopBitsAsU8(inputStream, 1, &finalBlock);
    baseBitstreamPopBitsAsU8(inputStream, 2, &compressionType);
    
    switch(compressionType)
    {
        // no compression
        case 0:
        {
            u16 blockLen;
            baseBitstreamPopU16LE(inputStream, &blockLen);

            u16 blockLenComplement;
            baseBitstreamPopU16LE(inputStream, &blockLenComplement);

        }break;

        // fixed with huffman
        case 1:
        {
            u32 literalCodes[COMPRESSION_DEFLATE_NUM_LITERAL_CODES];
            u32 distCodes[COMPRESSION_DEFLATE_NUM_DISTANCE_CODES];

            compressionDeflateGenerateHuffmanCodes((U8Array){.data = gDeflateLiteralSymsLengths, COMPRESSION_DEFLATE_NUM_LITERAL_CODES}, literalCodes);
            compressionDeflateGenerateHuffmanCodes((U8Array){.data = gDeflateDistancesSymsLengths, COMPRESSION_DEFLATE_NUM_DISTANCE_CODES}, distCodes);

            while(true)
            {
                u64 c = compressionDeflateDecodeHuffmanCode(inputStream, (U8Array){.data = gDeflateLiteralSymsLengths, COMPRESSION_DEFLATE_NUM_LITERAL_CODES}, literalCodes);
                if(c == 256)
                {
                    break;
                }

            }
            
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
        }break;

        default:
        {
            logProgErrorFmt("Error, deflate, unregognised compression type '%d'", compressionType);
        }break;
    }

    return output;
}