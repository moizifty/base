#ifndef BITMAP_DDS_H
#define BITMAP_DDS_H

#include "base\baseCore.h"
#include "base\baseStrings.h"
#include "base\baseMemory.h"
#include "base\baseThreads.h"
#include "base\baseMath.h"

#include "bitmap\bitmapCoreTypes.h"

#define DDSD_CAPS 0x1
#define DDSD_HEIGHT	0x2
#define DDSD_WIDTH 0x4
#define DDSD_PITCH 0x8
#define DDSD_PIXELFORMAT 0x1000
#define DDSD_MIPMAPCOUNT 0x20000
#define DDSD_LINEARSIZE 0x80000
#define DDSD_DEPTH 0x800000

#define DDPF_ALPHAPIXELS 0x1
#define DDPF_ALPHA 0x2
#define DDPF_FOURCC 0x4
#define DDPF_RGB 0x40
#define DDPF_YUV 0x200
#define DDPF_LUMINANCE 0x20000

#define DXT1 0x31545844
#define DXT2 0x32545844
#define DXT3 0x33545844
#define DXT4 0x34545844
#define DXT5 0x35545844
#define DX10 0x30315844

typedef struct
{
    u32 dwSize;
    u32 dwFlags;
    u32 dwFourCC;
    u32 dwRGBBitCount;
    u32 dwRBitMask;
    u32 dwGBitMask;
    u32 dwBBitMask;
    u32 dwABitMask;
}DDSPixelFormat;

typedef struct
{
    u32           dwSize;
    u32           dwFlags;
    u32           dwHeight;
    u32           dwWidth;
    u32           dwPitchOrLinearSize;
    u32           dwDepth;
    u32           dwMipMapCount;
    u32           dwReserved1[11];
    DDSPixelFormat ddspf;
    u32           dwCaps;
    u32           dwCaps2;
    u32           dwCaps3;
    u32           dwCaps4;
    u32           dwReserved2;
}DDSHeader;

typedef struct DDSCompressedData
{
    u8 *bytes;
    u64 byteCount;
    u32 w;
    u32 h;
    u32 compressionType;
}DDSCompressedData;

typedef struct DDSUncompressedData
{
    u8 *pixels;
    u32 bytesPerPixel;
    BitmapFormatKind fmt;
}DDSUncompressedData;

typedef struct DDSDXT1Block
{
    u16 c0;
    u16 c1;
    u8 r0;
    u8 r1;
    u8 r2;
    u8 r3;
}DDSDXT1Block;

typedef struct DDSDXT3Block
{
    //alpha is stored 4bit per pixel
    //so one byte off alpha stores alpha for 2 pixels
    u8 alphaP01;
    u8 alphaP23;
    u8 alphaP45;
    u8 alphaP67;
    u8 alphaP89;
    u8 alphaP1011;
    u8 alphaP1213;
    u8 alphaP1415;

    u16 c0;
    u16 c1;
    u8 r0;
    u8 r1;
    u8 r2;
    u8 r3;
}DDSDXT3Block;

typedef struct DDSDXT5Block
{
    u8 alpha0;
    u8 alpha1;
    u8 a0;
    u8 a1;
    u8 a2;
    u8 a3;
    u8 a4;
    u8 a5;

    u16 c0;
    u16 c1;
    u8 r0;
    u8 r1;
    u8 r2;
    u8 r3;
}DDSDXT5Block;

vec3i8 bitmapDDSCalculateColorFromU16(u16 col);
void bitmapDDSCalculateColorsFromDXT1Block(DDSDXT1Block block, vec3i8 colTable[4]);
void bitmapDDSCalculateColorsFromDXT3Block(DDSDXT3Block block, vec3i8 colTable[4]);
void bitmapDDSCalculateColorsFromDXT5Block(DDSDXT5Block block, vec3i8 colTable[4]);
DDSUncompressedData bitmapDDSUncompress(BaseArena *arena, DDSCompressedData input);

Bitmap bitmapFromDDSRaw(BaseArena *arena, u8 *rawBytes, u64 byteLen);
Bitmap bitmapFromDDSPath(BaseArena *arena, str8 file);

#endif