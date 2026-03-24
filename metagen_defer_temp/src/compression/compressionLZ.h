#ifndef COMPRESSION_LZ_H
#define COMPRESSION_LZ_H

#include "base/baseCore.h"
#include "base/baseStrings.h"
#include "base/baseMemory.h"
#include "base/baseThreads.h"

#define COMPRESSION_LZ4M_MINIMUM_MATCHLEN 4

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

typedef enum CompressionLZ4MBlockCompoundKind
{
    COMPRESSION_LZM4_BLOCK_COMPOUND_LITERAL,
    COMPRESSION_LZM4_BLOCK_COMPOUND_BACKREF,
}CompressionLZ4MBlockCompoundKind;
typedef struct CompressionLZ4MBlockCompound
{
    struct CompressionLZ4MBlockCompound *next;
    struct CompressionLZ4MBlockCompound *prev;

    CompressionLZ4MBlockCompoundKind kind;
    union
    {
        U8Array literals;
        CompressBackReference backref;
    };
}CompressionLZ4MBlockCompound;

typedef struct CompressionLZ4MBlockCompoundList
{
    CompressionLZ4MBlockCompound *first;
    CompressionLZ4MBlockCompound *last;
    u64 len;
}CompressionLZ4MBlockCompoundList;

typedef struct CompressionLZ4MSubByteDictEntry
{
    struct CompressionLZ4MSubByteDictEntry *next;
    struct CompressionLZ4MSubByteDictEntry *prev;

    U8Array subbytes;
}CompressionLZ4MSubByteDictEntry;

typedef struct CompressionLZ4MSubByteDictSlot
{
    CompressionLZ4MSubByteDictEntry *first;
    CompressionLZ4MSubByteDictEntry *last;

    u64 len;
}CompressionLZ4MSubByteDictSlot;

BASE_CREATE_ARRAY_VIEW_DECLS_DEFS(CompressionLZ4MSubByteDictSlotArray, CompressionLZ4MSubByteDictSlot)

typedef struct CompressionLZ4MSubByteDict
{
    CompressionLZ4MSubByteDictSlotArray slots;
}CompressionLZ4MSubByteDict;

/** 
 * LZ4M is my own take on LZ4
 * Its very similar, each compressed block begins with a 1 byte token
 * the high 4 bits of the token represent the length of the number of literals that follow
 * you can write them directly to the output buffer, lets call the high 4 bits "len",
 * the lower 4 bits represent the "match length", which is used when there is a backreference.
 * 
 * If "len" is 15, you want to read in more bytes after the token byte, and add these bytes to the "len",
 * this lets you specifiy a higher length then 15. If the additional bytes you read in are 255, that means 
 * there is another byte that follows that should be read and added to "len" etc.
 * 
 * Note: if the length of the raw literals is 15, meaning len will be written as 15, there will be another 0 byte after
 * 
 * after this is done, you read in "len" number of bytes (for the raw literal data) and write to the  output buffer
 * after that, you have 2 byte which represents "offset" value, in little endian. 
 * "offset" value here just means the number of bytes to go backwards from the current point, to begin outputting the backreference
 * after the offset value is read. If "match length" was 15, you want to do a similar thing you did with "len", read in additional bytes and add
 * and carry on reading aslong as the previous byte was 255. This gives the length of the backreference (how many bytes to output for the backreference)
 * 
 * each blocks need a token at the start and 2 byte for offset at the end
*/
U8Array compressionLZ4MCompress(Arena *arena, U8Array input, CompressOptions *options);
bool compressionLZ4MUncompress(U8Array input, U8Array output);

#endif