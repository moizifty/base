#ifndef BITMAP_CORE_H
#define BITMAP_CORE_H

#include "base/baseCore.h"
#include "base/baseStrings.h"
#include "base/baseMemory.h"
#include "base/baseThreads.h"
#include "base/baseMath.h"
#include "base/baseLog.h"

#include "bitmap/bitmapCoreTypes.h"
#include "bitmap/bitmapDDS.h"
#include "bitmap/bitmapPNG.h"
#include "bitmap/bitmapQOI.h"

BitmapFileKind bitmapFileKindFromPath(str8 path);

Bitmap bitmapPush(Arena *arena, vec2i size, BitmapFormatKind fmt);
u8 *bitmapGetPtrToPixel(Bitmap *bitmap, vec2i point);
void bitmapFastBlitToBitmap(Bitmap *src, Bitmap *dest, range2i srcRangeToCopy, range2i destRangeToPasteInto);
void bitmapBlitToBitmap(Bitmap *src, Bitmap *dest, range2i srcRangeToCopy, range2i destRangeToPasteInto);

void bitmapBlitToBitmap(Bitmap *src, Bitmap *dest, range2i srcRangeToCopy, range2i destRangeToPasteInto);
vec4u8 bitmapGetPixelColor4u8(Bitmap *bitmap, vec2i point);
void bitmapDrawPixel(Bitmap *bitmap, vec2i point, vec4u8 color);
void bitmapDrawLine(Bitmap *bitmap, vec2i start, vec2i end, vec4u8 color);

#endif