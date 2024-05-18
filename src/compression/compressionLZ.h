#ifndef COMPRESSION_LZ_H
#define COMPRESSION_LZ_H

#include "base\baseCore.h"
#include "base\baseStrings.h"
#include "base\baseMemory.h"
#include "base\baseThreads.h"

typedef struct CompressOptions
{
    u8 placeholder;
}CompressOptions;

typedef struct CompressBackReference
{
    u64 distance;
    u64 length;
    bool found;
}CompressBackReference;

bool compressionLZ4Compress(U8Array input, U8Array output, CompressOptions *options);
bool compressionLZ4Uncompress(U8Array input, U8Array output);

#endif