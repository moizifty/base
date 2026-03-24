#ifndef BITMAP_QOI_H
#define BITMAP_QOI_H

#include "base/baseCore.h"
#include "base/baseMath.h"
#include "base/baseMemory.h"
#include "base/baseStrings.h"
#include "base/baseThreads.h"
#include "datastructures/bitstream.h"
#include "os/core/osCore.h"
#include "bitmapCoreTypes.h"

typedef struct QOIHeader
{
    u8 magic[4];
    u32 width;
    u32 height;
    u8 channels;
    u8 colorspace;
}QOIHeader;

typedef enum QOITagKind
{
    QOI_TAG_RGB = 0b11111110,
    QOI_TAG_RGBA = 0b11111111,
    QOI_TAG_INDEX = 0b00,
    QOI_TAG_DIFF = 0b01,
    QOI_TAG_LUMA = 0b10,
    QOI_TAG_RUN = 0b11,
}QOITagKind;

Bitmap bitmapFromQOIRaw(Arena *arena, u8 *rawBytes, u64 byteLen);
Bitmap bitmapFromQOIPath(Arena *arena, str8 file);

#endif