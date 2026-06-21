#include "bitmap/bitmapCore.h"

// assumes little endian
BitmapFileKindTableEntry gBitmapFileKindsTable[BITMAP_FILE_KIND_COUNT] = 
{
    [BITMAP_FILE_KIND_UNKNOWN] = {.ext = STR8_LIT_COMP_CONST("unknown"), .kind = BITMAP_FILE_KIND_UNKNOWN, .numOfMagicBytes = 0},
    [BITMAP_FILE_KIND_DDS]     = {.ext = STR8_LIT_COMP_CONST(".dds"), .kind = BITMAP_FILE_KIND_DDS, .magicBytes = {0x44, 0x44, 0x53, 0x20} , .numOfMagicBytes = 4},
    [BITMAP_FILE_KIND_BMP]     = {.ext = STR8_LIT_COMP_CONST(".bmp"), .kind = BITMAP_FILE_KIND_BMP, .magicBytes = {0x42, 0x4d}, .numOfMagicBytes = 2},
    [BITMAP_FILE_KIND_TGA]     = {.ext = STR8_LIT_COMP_CONST(".tga"), .kind = BITMAP_FILE_KIND_TGA, .magicBytes = {0}, .numOfMagicBytes = 0},
    [BITMAP_FILE_KIND_PNG]     = {.ext = STR8_LIT_COMP_CONST(".png"), .kind = BITMAP_FILE_KIND_PNG, .magicBytes = {0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a}, .numOfMagicBytes = 8},
    [BITMAP_FILE_KIND_QOI]     = {.ext = STR8_LIT_COMP_CONST(".qoi"), .kind = BITMAP_FILE_KIND_QOI, .magicBytes = {'q', 'o', 'i', 'f'}, .numOfMagicBytes = 4},
};

Bitmap bitmapPush(Arena *arena, vec2i size, BitmapFormatKind fmt)
{
    Bitmap bitmap = {0};
    bitmap.fmt = fmt;
    bitmap.size = size;

    switch (bitmap.fmt)
    {
        case BITMAP_FORMAT_R8_GRAYSCALE: bitmap.bytesPerPixel = 1; break;
        case BITMAP_FORMAT_A8: bitmap.bytesPerPixel = 1; break;
        case BITMAP_FORMAT_R8G8B8: bitmap.bytesPerPixel = 3; break;

        case BITMAP_FORMAT_R8G8B8A8:
        case BITMAP_FORMAT_A8R8G8B8:
        case BITMAP_FORMAT_B8G8R8A8: bitmap.bytesPerPixel = 4; break;

        default:
        {
            baseEPrintf("Unexpected bitmap format.\n");
        }break;
    }

    bitmap.pixels = arenaPushArray(arena, u8, bitmap.size.w * bitmap.size.h * bitmap.bytesPerPixel);

    return bitmap;
}

i64 bitmapApplyAddressMode(i64 coordComponent, i64 size, BitmapAddressModeKind kind)
{
    switch (kind)
    {
        case BITMAP_ADDRESS_MODE_WRAP:
        {
            return ((coordComponent % size) + size) % size;
        }break;
        
        case BITMAP_ADDRESS_MODE_CLAMP:
        {
            return BASE_CLAMP(coordComponent, 0, size - 1);
        }break;

        case BITMAP_ADDRESS_MODE_DISCARD:
        {
            return coordComponent >= 0 && coordComponent < size ? coordComponent : -1;
        }break;

        default:
        {
            baseEPrintf("Unhandled address mode.\n");
            return -1;
        }break;
    }
}
u8 *bitmapGetPtrToPixel(Bitmap *bitmap, vec2i point, BitmapSampler sampler)
{
    point.x = bitmapApplyAddressMode(point.x, bitmap->size.w, sampler.addrModeX);
    point.y = bitmapApplyAddressMode(point.y, bitmap->size.h, sampler.addrModeY);

    if (point.x == -1 || point.y == -1)
    {
        return null;
    }

    u64 index = point.y * bitmap->size.w * bitmap->bytesPerPixel + point.x * bitmap->bytesPerPixel;

    return bitmap->pixels + index;
}
// faster then bitmapBlitToBitmap if the bitmap has same format
// of cource the format could also not be the same which can be useful at time
// say if dest has format bgr and src has rgb and for whatever reason u want red to be blue and bice versa
// also assumes that the ranges are set up correctly and dont go over
// this function doesnt take samplers, as it memcpys rows over,
// it would defeat the purpose to calculated the sampled pixel coords, as they can wrap around etc, so ud have to check for signs
void bitmapFastBlitToBitmap(Bitmap *src, Bitmap *dest, range2i srcRangeToCopy, range2i destRangeToPasteInto)
{
    BitmapSampler sampler = {.addrModeX = BITMAP_ADDRESS_MODE_DISCARD, .addrModeY = BITMAP_ADDRESS_MODE_DISCARD};
    u8 *srcStart = bitmapGetPtrToPixel(src, srcRangeToCopy.min, sampler);

    vec2i srcRangeStart = 
    {
        .x = bitmapApplyAddressMode(srcRangeToCopy.min.x, src->size.w, sampler.addrModeX),
    };

    vec2i srcRangeEnd = 
    {
        .x = bitmapApplyAddressMode(srcRangeToCopy.max.x, src->size.x, sampler.addrModeX),
    };

    vec2i destRangeStart = 
    {
        .x = bitmapApplyAddressMode(destRangeToPasteInto.min.x, dest->size.w, sampler.addrModeX),
    };

    vec2i destRangeEnd = 
    {
        .x = bitmapApplyAddressMode(destRangeToPasteInto.max.x, dest->size.x, sampler.addrModeX),
    };

    if ((destRangeEnd.x == -1 && destRangeStart.x == -1) ||
        (srcRangeEnd.x == -1 && srcRangeStart.x == -1))
    {
        return;
    }
    
    srcRangeStart.x = srcRangeStart.x == -1 ? src->size.w : baseClamp(srcRangeStart.x, 0, src->size.w);
    srcRangeEnd.x = srcRangeEnd.x == -1 ? src->size.w : baseClamp(srcRangeEnd.x, 0, src->size.w);

    destRangeStart.x = destRangeStart.x == -1 ? dest->size.w : baseClamp(destRangeStart.x, 0, dest->size.w);
    destRangeEnd.x = destRangeEnd.x == -1 ? dest->size.w : baseClamp(destRangeEnd.x, 0, dest->size.w);

    for (i64 srcYOffset = 0; srcYOffset < Range2iDim(srcRangeToCopy).h && srcStart; srcYOffset++)
    {
        srcRangeStart.y = bitmapApplyAddressMode(srcRangeToCopy.min.y + srcYOffset, src->size.h, sampler.addrModeY);
        destRangeStart.y = bitmapApplyAddressMode(destRangeToPasteInto.min.y + srcYOffset, dest->size.h, sampler.addrModeY);

        srcRangeEnd.y = bitmapApplyAddressMode(srcRangeToCopy.max.y + srcYOffset, src->size.h, sampler.addrModeY);
        destRangeEnd.y = bitmapApplyAddressMode(destRangeToPasteInto.max.y + srcYOffset, dest->size.h, sampler.addrModeY);

        if ((destRangeEnd.y == -1 && destRangeStart.y == -1) ||
            (srcRangeEnd.y == -1 && srcRangeStart.y == -1))
        {
            break;
        }
        
        srcRangeStart.y = srcRangeStart.y == -1 ? src->size.h : baseClamp(srcRangeStart.y, 0, src->size.h);
        srcRangeEnd.y = srcRangeEnd.y == -1 ? src->size.h : baseClamp(srcRangeEnd.y, 0, src->size.h);

        destRangeStart.y = destRangeStart.y == -1 ? dest->size.h : baseClamp(destRangeStart.y, 0, dest->size.h);
        destRangeEnd.y = destRangeEnd.y == -1 ? dest->size.h :  baseClamp(destRangeEnd.y, 0, dest->size.h);

        u8 *srcRow = bitmapGetPtrToPixel(src, Vec2i(srcRangeStart.x, srcRangeStart.y), sampler);
        u8 *destRow = bitmapGetPtrToPixel(dest, Vec2i(destRangeStart.x, destRangeStart.y), sampler);

        u64 widthToCopy = min(Range2iDim(Range2iFromVec2i(srcRangeStart, srcRangeEnd)).w, Range2iDim(Range2iFromVec2i(destRangeStart, destRangeEnd)).w);

        if (srcRow && destRow)
        {
            BASE_MEMCPY(destRow, srcRow, src->bytesPerPixel * widthToCopy);
        }
    }
}

void bitmapBlitToBitmap(Bitmap *src, Bitmap *dest, range2i srcRangeToCopy, range2i destRangeToPasteInto, BitmapSampler srcSampler, BitmapSampler destSampler)
{
    vec2i deltas = Vec2i(Range2iDim(srcRangeToCopy).w, Range2iDim(srcRangeToCopy).h);

    for (i64 srcYOffset = 0; srcYOffset < deltas.h; srcYOffset++)
    {
        for (i64 srcXOffset = 0; srcXOffset < deltas.w; srcXOffset++)
        {
            i64 srcX = srcRangeToCopy.min.x + srcXOffset;
            i64 srcY = srcRangeToCopy.min.y + srcYOffset;

            i64 destX = destRangeToPasteInto.min.x + srcXOffset;
            i64 destY = destRangeToPasteInto.min.y + srcYOffset;

            vec4u8 srcColor = bitmapGetPixelColor4u8(src, Vec2i(srcX, srcY), srcSampler);

            bitmapDrawPixel(dest, Vec2i(destX, destY), srcColor, destSampler);
        }
    }
}

vec4u8 bitmapGetPixelColor4u8(Bitmap *bitmap, vec2i point, BitmapSampler sampler)
{
    vec4u8 color = {0};

    u8 *start = bitmapGetPtrToPixel(bitmap, point, sampler);

    if (start)
    {
        switch (bitmap->fmt)
        {
            case BITMAP_FORMAT_R8_GRAYSCALE:
            {
                color.r = *(start + 0);
                color.g = *(start + 0);
                color.b = *(start + 0);
                color.a = 255;
            }break;

            case BITMAP_FORMAT_A8:
            {
                color.a = *(start + 0);
            }break;

            case BITMAP_FORMAT_R8G8B8:
            {
                color.r = *(start + 0);
                color.g = *(start + 1);
                color.b = *(start + 2);
                color.a = 255;
            }break;

            case BITMAP_FORMAT_R8G8B8A8:
            {
                color.r = *(start + 0);
                color.g = *(start + 1);
                color.b = *(start + 2);
                color.a = *(start + 3);
            }break;
        }
    }
    
    return color;
}

void bitmapSetPixelColor4u8(Bitmap *bitmap, vec2i point, vec4u8 color, BitmapSampler sampler)
{
    u8 *start = bitmapGetPtrToPixel(bitmap, point, sampler);

    if (start)
    {
        switch (bitmap->fmt)
        {
            case BITMAP_FORMAT_R8_GRAYSCALE:
            {
                *(start + 0) = color.r;
            }break;

            case BITMAP_FORMAT_A8:
            {
                *(start + 0) = color.a;
            }break;

            case BITMAP_FORMAT_R8G8B8:
            {
                *(start + 0) = color.r;
                *(start + 1) = color.g;
                *(start + 2) = color.b;
            }break;

            case BITMAP_FORMAT_R8G8B8A8:
            {
                *(start + 0) = color.r;
                *(start + 1) = color.g;
                *(start + 2) = color.b;
                *(start + 3) = color.a;
            }break;
        }
    }
}
void bitmapClear(Bitmap *bitmap, vec4u8 color)
{
    for (u64 y = 0; y < bitmap->size.height; y++)
    {
        for (u64 x = 0; x < bitmap->size.width; x++)
        {
            bitmapDrawPixel(bitmap, Vec2i(x, y), color, BitmapSamplerDefault);
        }
    }
}
void bitmapDrawPixel(Bitmap *bitmap, vec2i point, vec4u8 color, BitmapSampler sampler)
{
    bitmapSetPixelColor4u8(bitmap, point, color, sampler);
}

void bitmapDrawRect(Bitmap *bitmap, vec2i pos, vec2i dim, vec4u8 color, BitmapSampler sampler)
{
    for (u64 row = 0; row < dim.height; row++)
    {
        for (u64 col = 0; col < dim.width; col++)
        {
            bitmapSetPixelColor4u8(bitmap, Vec2i(pos.x + col, pos.y + row), color, sampler);
        }
    }
}

void bitmapDrawLine(Bitmap *bitmap, vec2i start, vec2i end, vec4u8 color, BitmapSampler sampler)
{
    i64 dx = end.x - start.x;
    i64 dy = end.y - start.y;

    i64 step = max(llabs(dx), llabs(dy));
    f64 stepX = (f64)dx / (f64)step;
    f64 stepY = (f64)dy / (f64)step;

    f64 x = (f64)start.x;
    f64 y = (f64)start.y;
    for (i64 i = 0; i <= step; i++)
    {
        bitmapDrawPixel(bitmap, Vec2i(round(x), round(y)), color, sampler);

        x += stepX;
        y += stepY;
    }
}