#ifndef COMPRESSION_DEFLATE_H
#define COMPRESSION_DEFLATE_H

#include "base\baseCore.h"
#include "base\baseMemory.h"
#include "base\baseBitstream.h"

#define COMPRESSION_DEFLATE_WINDOW_SIZE 32768 // Maximum size of the sliding window
#define COMPRESSION_DEFLATE_MAX_CODE_LENGTH 15 // Maximum length of a Huffman code
#define COMPRESSION_DEFLATE_NUM_LITERAL_CODES 288 // Number of literal/length codes
#define COMPRESSION_DEFLATE_NUM_DISTANCE_CODES 32 // Number of distance codes

typedef struct CompressionDeflateUncompressedOutput
{
    ArrayView uncompressedStream;
    u64 bytesMoved;
}CompressionDeflateUncompressedOutput;

void compressionDeflateGenerateHuffmanCodes(U8Array codeLengths, u32 *outCodes);
u64 compressionDeflateDecodeHuffmanCode(BaseBitstream *stream, U8Array symbolLens, u32 *symbolCodes);
CompressionDeflateUncompressedOutput compressionDeflateUncompress(BaseBitstream *inputStream, ArrayView *outBuffer);

#endif