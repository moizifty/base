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

Bitmap bitmapFromPath(Arena *arena, str8 file);

Bitmap bitmapFromBMPRaw(Arena *arena, u8 *rawBytes, u64 byteLen);
Bitmap bitmapFromBMPPath(Arena *arena, str8 file);

Bitmap bitmapFromTGARaw(Arena *arena, u8 *rawBytes, u64 byteLen);
Bitmap bitmapFromTGAPath(Arena *arena, str8 file);

#endif