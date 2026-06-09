#ifndef BITMAP_CORE_TYPES
#define BITMAP_CORE_TYPES

#include "base/baseCore.h"
#include "base/baseStrings.h"
#include "base/baseMath.h"

#define BITMAP_MAX_MAGIC_BYTES 10

#define BitmapSamplerDefault ((BitmapSampler){0})
typedef enum BitmapFileKind
{
    BITMAP_FILE_KIND_UNKNOWN,
    BITMAP_FILE_KIND_DDS,
    BITMAP_FILE_KIND_BMP,
    BITMAP_FILE_KIND_TGA,
    BITMAP_FILE_KIND_PNG,
    BITMAP_FILE_KIND_QOI,

    BITMAP_FILE_KIND_COUNT,
}BitmapFileKind;

typedef enum BitmapFormatKind
{
    BITMAP_FORMAT_UNKNOWN,
    
    BITMAP_FORMAT_R8_GRAYSCALE,
    BITMAP_FORMAT_A8,
    BITMAP_FORMAT_R8G8B8,
    BITMAP_FORMAT_R8G8B8A8,
    BITMAP_FORMAT_A8R8G8B8,
    BITMAP_FORMAT_B8G8R8A8,

    BITMAP_FORMAT_COUNT,

    BITMAP_FORMAT_RGB_8 = BITMAP_FORMAT_R8G8B8,
    BITMAP_FORMAT_RGBA_8 = BITMAP_FORMAT_R8G8B8A8,
    BITMAP_FORMAT_BGRA_8 = BITMAP_FORMAT_B8G8R8A8,
}BitmapFormatKind;

typedef enum BitmapAddressModeKind
{
    BITMAP_ADDRESS_MODE_WRAP,
    BITMAP_ADDRESS_MODE_CLAMP,
    BITMAP_ADDRESS_MODE_DISCARD,
}BitmapAddressModeKind;

typedef struct BitmapFileKindTableEntry
{
    BitmapFileKind kind;
    str8 ext;
    u8 magicBytes[BITMAP_MAX_MAGIC_BYTES];
    u8 numOfMagicBytes;
}BitmapFileKindTableEntry;

typedef struct BitmapSampler
{
    BitmapAddressModeKind addrModeX;
    BitmapAddressModeKind addrModeY;
}BitmapSampler;

typedef struct Bitmap
{
    u8 *pixels;

    vec2i size;
    BitmapFormatKind fmt;
    u64 bytesPerPixel;
}Bitmap;

global BitmapFileKindTableEntry gBitmapFileKindsTable[BITMAP_FILE_KIND_COUNT];

#endif