#ifndef BITMAP_PNG_H
#define BITMAP_PNG_H

#include "base\baseCore.h"
#include "base\baseStrings.h"
#include "base\baseMemory.h"
#include "base\baseThreads.h"
#include "base\baseMath.h"
#include "compression\compressionDeflate.h"

#include "bitmap\bitmapCoreTypes.h"

typedef u32 PNGChunkType;
    //LITLE ENDIAN
#define PNG_CHUNK_TYPE_IHDR ((u32)'RDHI')
#define PNG_CHUNK_TYPE_IDAT ((u32)'TADI')
#define PNG_CHUNK_TYPE_PLTE ((u32)'ETLP')
#define PNG_CHUNK_TYPE_IEND ((u32)'DNEI')

typedef enum PNGColorType
{
    PNG_COLOR_TYPE_GREYSCALE = 0,
    PNG_COLOR_TYPE_TRUECOLOR = 2,
    PNG_COLOR_TYPE_INDEXED = 3,
    PNG_COLOR_TYPE_GREYSCALE_ALPHA = 4,
    PNG_COLOR_TYPE_TRUECOLOR_ALPHA = 6,

    PNG_COLOR_TYPE_COUNT
}PNGColorType;


typedef struct PNGChunk
{
    struct PNGChunk *next;
    struct PNGChunk *prev;

    u32 length;
    union
    {
        u8 chunkName[4];
        PNGChunkType chunkType;
    };

    u8 *chunkData;
    u32 crc;
}PNGChunk;

typedef struct PNGChunkIHDR
{
    u32 width;
    u32 height;
    u8 bitDepth;
    u8 colType;
    u8 compressMethod;
    u8 filterMethod;
    u8 interlaceMethod;
}PNGChunkIHDR;

typedef struct PNGChunkList
{
    PNGChunk *first;
    PNGChunk *last;

    u64 len;
}PNGChunkList;

typedef struct PNGInfo
{
    u32 colorComponents;
    u64 bitDepth;
    u32 w;
    u32 h;
}PNGInfo;

typedef struct PNGCollectIDATChunksData
{
    PNGChunkIHDR hdr;
    u64 bytesMoved;

    PNGInfo pngInfo;
}PNGCollectIDATChunksData;

typedef struct PNGCompressedData
{
    U8Array compressedStream;
    PNGInfo pngInfo;
}PNGCompressedData;

typedef struct PNGUncompressedData
{
    U8Array uncompressedStream;
    PNGInfo pngInfo;
}PNGUncompressedData;

typedef struct PNGUnfilteredData
{
    U8Array output;
    PNGInfo pngInfo;
}PNGUnfilteredData;

Bitmap bitmapFromPNGRaw(BaseArena *arena, u8 *rawBytes, u64 byteLen);
Bitmap bitmapFromPNGPath(BaseArena *arena, str8 file);

#endif