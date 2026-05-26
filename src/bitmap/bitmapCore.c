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

BitmapFileKind bitmapFileKindFromPath(str8 path)
{
    for(u64 i = 0; i < BASE_ARRAY_SIZE(gBitmapFileKindsTable); i++)
    {
        BitmapFileKindTableEntry entry = gBitmapFileKindsTable[i];
        
        if (Str8EndsWith(path, entry.ext, STR_MATCHFLAGS_CASE_INSENSITIVE))
        {
            return entry.kind;
        }
    }
    
    // if we couldnt find it based on path extension, 
    // open the file and check the magic number

    BitmapFileKind fallBackKind = BITMAP_FILE_KIND_UNKNOWN;
    ArenaTemp temp = baseTempBegin(null, 0);
    {
        U8Array fileBytes = OSFileReadAll(temp.arena, path);

        if (fileBytes.data != null && fileBytes.len >= BITMAP_MAX_MAGIC_BYTES)
        {
            for(u64 i = 0; i < BASE_ARRAY_SIZE(gBitmapFileKindsTable); i++)
            {
                BitmapFileKindTableEntry entry = gBitmapFileKindsTable[i];
                
                // assumes little endian
                u32 eq = 0;
                for(u64 mb = 0; mb < entry.numOfMagicBytes; mb++)
                {
                    if(entry.magicBytes[mb] == fileBytes.data[mb]) eq++;
                    else continue;
                }

                if(eq != 0)
                {
                    fallBackKind = entry.kind;
                }
            }
        }

    }
    baseTempEnd(temp);

    return fallBackKind;
}

Bitmap bitmapPush(Arena *arena, vec2i size, BitmapFormatKind fmt)
{
    Bitmap bitmap = {0};
    bitmap.fmt = fmt;
    bitmap.size = size;

    switch (bitmap.fmt)
    {
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

u8 *bitmapGetPtrToPixel(Bitmap *bitmap, vec2i point)
{
    u64 index = point.y * bitmap->size.w * bitmap->bytesPerPixel + point.x * bitmap->bytesPerPixel;

    return bitmap->pixels + index;
}
// faster then bitmapBlitToBitmap if the bitmap has same format
// of cource the format could also not be the same which can be useful at time
// say if dest has format bgr and src has rgb and for whatever reason u want red to be blue and bice versa
// also assumes that the ranges are set up correctly and dont go over
void bitmapFastBlitToBitmap(Bitmap *src, Bitmap *dest, range2i srcRangeToCopy, range2i destRangeToPasteInto)
{
    if (dest->size.w < src->size.w || 
        dest->size.h < src->size.h)
    {
        return;
    }

    u8 *srcStart = bitmapGetPtrToPixel(src, srcRangeToCopy.min);
    u8 *srcEnd = bitmapGetPtrToPixel(src, srcRangeToCopy.max);

    for (u64 srcYOffset = 0; srcYOffset < Range2iDim(srcRangeToCopy).h; srcYOffset++)
    {
        u64 srcX = srcRangeToCopy.min.x; 
        u64 srcY = srcRangeToCopy.min.y + srcYOffset;

        u64 destX = destRangeToPasteInto.min.x;
        u64 destY = destRangeToPasteInto.min.y + srcYOffset;

        u8 *srcRow = bitmapGetPtrToPixel(src, Vec2i(srcX, srcY));
        u8 *destRow = bitmapGetPtrToPixel(dest, Vec2i(destX, destY));

        BASE_MEMCPY(destRow, srcRow, src->bytesPerPixel * Range2iDim(srcRangeToCopy).w);
    }
}

void bitmapBlitToBitmap(Bitmap *src, Bitmap *dest, range2i srcRangeToCopy, range2i destRangeToPasteInto)
{
    if (dest->size.w < src->size.w || 
        dest->size.h < src->size.h)
    {
        return;
    }

    if (destRangeToPasteInto.min.x >= dest->size.w ||
        destRangeToPasteInto.min.y >= dest->size.h)
    {
        return;
    }

    if (srcRangeToCopy.min.x >= src->size.w ||
        srcRangeToCopy.min.y >= src->size.h)
    {
        return;
    }

    vec2i clampedSrcRangeEnd = Vec2i(baseClamp(srcRangeToCopy.max.x, srcRangeToCopy.min.x, src->size.w), 
                                      baseClamp(srcRangeToCopy.max.y, srcRangeToCopy.min.y, src->size.h));

    vec2i clampedDestRangeEnd = Vec2i(baseClamp(destRangeToPasteInto.max.x, destRangeToPasteInto.min.x, dest->size.w), 
                                      baseClamp(destRangeToPasteInto.max.y, destRangeToPasteInto.min.y, dest->size.h));
    
    for (u64 srcYOffset = 0; srcYOffset < clampedSrcRangeEnd.y - srcRangeToCopy.min.y; srcYOffset++)
    {
        for (u64 srcXOffset = 0; srcXOffset < clampedSrcRangeEnd.x - srcRangeToCopy.min.x; srcXOffset++)
        {
            u64 srcX = srcRangeToCopy.min.x + srcXOffset;
            u64 srcY = srcRangeToCopy.min.y + srcYOffset;

            u64 destX = destRangeToPasteInto.min.x + srcXOffset;
            u64 destY = destRangeToPasteInto.min.y + srcYOffset;

            vec4u8 srcColor = bitmapGetPixelColor4u8(src, Vec2i(srcX, srcY));

            bitmapDrawPixel(dest, Vec2i(destX, destY), srcColor);
        }
    }
}
// Bitmap bitmapFromPath(Arena *arena, str8 file)
// {
//     Bitmap bm = {0};
//     ArenaTemp temp = baseTempBegin(&arena, 1);
//     {
//         BitmapFileKind kind = bitmapFileKindFromPath(file);
//         switch(kind)
//         {
//             case BITMAP_FILE_KIND_DDS: return bitmapFromDDSPath(arena, file);
//             case BITMAP_FILE_KIND_PNG: return bitmapFromPNGPath(arena, file);
//             case BITMAP_FILE_KIND_QOI: return bitmapFromQOIPath(arena, file);
//             case BITMAP_FILE_KIND_BMP: return bitmapFromDDSPath(arena, file);
//             case BITMAP_FILE_KIND_TGA: return bitmapFromDDSPath(arena, file);

//             case BITMAP_FILE_KIND_UNKNOWN:
//             case BITMAP_FILE_KIND_COUNT:
//             default:
//             {
//                 logThreadErrorFmt("Unregognised image file format '%S'", file);
//             }break;
//         }
//     }
//     baseTempEnd(temp);

//     return bm;
// }

vec4u8 bitmapGetPixelColor4u8(Bitmap *bitmap, vec2i point)
{
    vec4u8 color = {0};
    u64 index = (((u64)point.y % bitmap->size.height) * bitmap->size.width * bitmap->bytesPerPixel) + 
                ((u64)point.x % bitmap->size.width) * bitmap->bytesPerPixel;

    switch (bitmap->fmt)
    {
        case BITMAP_FORMAT_R8G8B8:
        {
            color.r = bitmap->pixels[index + 0];
            color.g = bitmap->pixels[index + 1];
            color.b = bitmap->pixels[index + 2];
        }break;

        case BITMAP_FORMAT_R8G8B8A8:
        {
            color.r = bitmap->pixels[index + 0];
            color.g = bitmap->pixels[index + 1];
            color.b = bitmap->pixels[index + 2];
            color.a = bitmap->pixels[index + 3];
        }break;
    }

    return color;
}

void bitmapDrawPixel(Bitmap *bitmap, vec2i point, vec4u8 color)
{
    u64 index = (((u64)point.y % bitmap->size.height) * bitmap->size.width * bitmap->bytesPerPixel) + 
                ((u64)point.x % bitmap->size.width) * bitmap->bytesPerPixel;

    switch (bitmap->fmt)
    {
        case BITMAP_FORMAT_R8G8B8:
        {
            bitmap->pixels[index + 0] = color.r;
            bitmap->pixels[index + 1] = color.g;
            bitmap->pixels[index + 2] = color.b;
        }break;

        case BITMAP_FORMAT_R8G8B8A8:
        {
            bitmap->pixels[index + 0] = color.r;
            bitmap->pixels[index + 1] = color.g;
            bitmap->pixels[index + 2] = color.b;
            bitmap->pixels[index + 3] = color.a;
        }break;
    }
}

void bitmapDrawLine(Bitmap *bitmap, vec2i start, vec2i end, vec4u8 color)
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
        bitmapDrawPixel(bitmap, Vec2i(round(x), round(y)), color);

        x += stepX;
        y += stepY;
    }
}