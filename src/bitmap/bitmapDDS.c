#include "bitmap\bitmapDDS.h"

// uncompress the raw byte data,
vec3u8 bitmapDDSCalculateColorFromU16(u16 col)
{
    vec3u8 col0;

    col0.r = (col >> 11);
    col0.r = (col0.r << 3) | (col0.r >> 2);

    col0.g = (col & 0b0000011111100000) >> 5;
    col0.g = (col0.g << 2) | (col0.g >> 4);

    col0.b = (col &  0b0000000000011111);
    col0.b = (col0.b << 3) | (col0.b >> 2);

    return col0;
}
void bitmapDDSCalculateColorsFromDXT1Block(DDSDXT1Block block, vec3u8 colTable[4])
{
    vec3u8 col0 = {0};
    vec3u8 col1 = {0};
    vec3u8 col2 = {0};
    vec3u8 col3 = {0};

    col0 = bitmapDDSCalculateColorFromU16(block.c0);
    col1 = bitmapDDSCalculateColorFromU16(block.c1);

    col2 = (block.c0 > block.c1) ? Vec3u8(((2 * (u16)col0.r) + (u16)col1.r) / 3,
                                            ((2 * (u16)col0.g) + (u16)col1.g) / 3,
                                            ((2 * (u16)col0.b) + (u16)col1.b) / 3) :

                                    Vec3u8(((u16)col0.r + (u16)col1.r) / 2,
                                            ((u16)col0.g + (u16)col1.g) / 2,
                                            ((u16)col0.b + (u16)col1.b) / 2);

    col3 = (block.c0 > block.c1) ? Vec3u8(((u16)col0.r + (2 * (u16)col1.r)) / 3,
                                            ((u16)col0.g + (2 * (u16)col1.g)) / 3,
                                            ((u16)col0.b + (2 * (u16)col1.b)) / 3) : col3;

    colTable[0] = col0;
    colTable[1] = col1;
    colTable[2] = col2;
    colTable[3] = col3;
}
void bitmapDDSCalculateColorsFromDXT3Block(DDSDXT3Block block, vec3u8 colTable[4])
{
    vec3u8 col0 = {0};
    vec3u8 col1 = {0};
    vec3u8 col2 = {0};
    vec3u8 col3 = {0};

    col0 = bitmapDDSCalculateColorFromU16(block.c0);
    col1 = bitmapDDSCalculateColorFromU16(block.c1);

    col2 = Vec3u8(((2 * (u16)col0.r) + (u16)col1.r) / 3,
                ((2 * (u16)col0.g) + (u16)col1.g) / 3,
                ((2 * (u16)col0.b) + (u16)col1.b) / 3);

    col3 = Vec3u8(((u16)col0.r + (2 * (u16)col1.r)) / 3,
                ((u16)col0.g + (2 * (u16)col1.g)) / 3,
                ((u16)col0.b + (2 * (u16)col1.b)) / 3);

    colTable[0] = col0;
    colTable[1] = col1;
    colTable[2] = col2;
    colTable[3] = col3;
}
void bitmapDDSCalculateColorsFromDXT5Block(DDSDXT5Block block, vec3u8 colTable[4])
{
    vec3u8 col0 = {0};
    vec3u8 col1 = {0};
    vec3u8 col2 = {0};
    vec3u8 col3 = {0};

    col0 = bitmapDDSCalculateColorFromU16(block.c0);
    col1 = bitmapDDSCalculateColorFromU16(block.c1);

    col2 = Vec3u8(((2 * (u16)col0.r) + (u16)col1.r) / 3,
                ((2 * (u16)col0.g) + (u16)col1.g) / 3,
                ((2 * (u16)col0.b) + (u16)col1.b) / 3);

    col3 = Vec3u8(((u16)col0.r + (2 * (u16)col1.r)) / 3,
                ((u16)col0.g + (2 * (u16)col1.g)) / 3,
                ((u16)col0.b + (2 * (u16)col1.b)) / 3);

    colTable[0] = col0;
    colTable[1] = col1;
    colTable[2] = col2;
    colTable[3] = col3;
}

DDSUncompressedData bitmapDDSUncompress(Arena *arena, DDSCompressedData input)
{
    DDSUncompressedData uncompressed = {0};

    u8 *data = input.bytes;
    switch(input.compressionType)
    {
        default:
        {
            logThreadErrorFmt("The dds file contains an unsupported compression method.");
            return uncompressed;
        }break;

        case DXT1:
        case DXT3:
        case DXT5:
        {
            // intentionally empty
        }break;
    }

    uncompressed.fmt = BITMAP_FORMAT_RGBA_8;
    uncompressed.bytesPerPixel = 4; //rgba
    uncompressed.pixels = arenaPush(arena, input.w * input.h * uncompressed.bytesPerPixel);
    
    for(u64 bY = 0; bY < input.h; bY += 4)
    {
        for(u64 bX = 0; bX < input.w; bX += 4)
        {
            u32 indices = 0;
            u64 alphas = 0;

            vec3u8 colTable[4] = {0};
            u8 alphaTable[16] = {0};

            switch(input.compressionType)
            {
                case DXT1:
                {
                    DDSDXT1Block block = {0};
                    BASE_MEMCPY(&block, data, sizeof(block));
                    data += sizeof(block);

                    indices = block.r0 | (block.r1 << 8) | (block.r2 << 16) | (block.r3 << 24);
                    bitmapDDSCalculateColorsFromDXT1Block(block, colTable);
                }break;
                case DXT3:
                {
                    DDSDXT3Block block = {0};
                    BASE_MEMCPY(&block, data, sizeof(block));
                    data += sizeof(block);

                    indices = block.r0 | (block.r1 << 8) | (block.r2 << 16) | (block.r3 << 24);
                    alphas =  (u64) block.alphaP01 | 
                                ((u64) block.alphaP23 << 8) | 
                                ((u64) block.alphaP45 << 16) | 
                                ((u64) block.alphaP67 << 24) |
                                ((u64) block.alphaP89 << 32) |
                                ((u64) block.alphaP1011 << 40) |
                                ((u64) block.alphaP1213 << 48) |
                                ((u64) block.alphaP1415 << 56);

                    bitmapDDSCalculateColorsFromDXT3Block(block, colTable);
                }break;
                case DXT5:
                {
                    DDSDXT5Block block = {0};
                    BASE_MEMCPY(&block, data, sizeof(block));
                    data += sizeof(block);

                    indices = block.r0 | (block.r1 << 8) | (block.r2 << 16) | (block.r3 << 24);
                    alphas = (u64) block.a0 | 
                            ((u64) block.a1 << 8) | 
                            ((u64) block.a2 << 16) | 
                            ((u64) block.a3 << 24) |
                            ((u64) block.a4 << 32) |
                            ((u64) block.a5 << 40);

                    alphaTable[0] = block.alpha0;
                    alphaTable[1] = block.alpha1;
                    alphaTable[2] = 0;
                    alphaTable[3] = 0;
                    alphaTable[4] = 0;
                    alphaTable[5] = 0;
                    alphaTable[6] = 0;
                    alphaTable[7] = 0;

                    if (alphaTable[0] > alphaTable[1])
                    {
                        alphaTable[2] = (6 * (u64)alphaTable[0] + (u64)alphaTable[1]) / 7;
                        alphaTable[3] = (5 * (u64)alphaTable[0] + 2 * (u64)alphaTable[1]) / 7;
                        alphaTable[4] = (4 * (u64)alphaTable[0] + 3 * (u64)alphaTable[1]) / 7;
                        alphaTable[5] = (3 * (u64)alphaTable[0] + 4 * (u64)alphaTable[1]) / 7;
                        alphaTable[6] = (2 * (u64)alphaTable[0] + 5 * (u64)alphaTable[1]) / 7;
                        alphaTable[7] = ((u64)alphaTable[0] + 6 * (u64)alphaTable[1]) / 7;
                    }
                    else
                    {
                        alphaTable[2] = (4 * (u64)alphaTable[0] + (u64)alphaTable[1]) / 5;
                        alphaTable[3] = (3 * (u64)alphaTable[0] + 2 * (u64)alphaTable[1]) / 5;
                        alphaTable[4] = (2 * (u64)alphaTable[0] + 3 * (u64)alphaTable[1]) / 5;
                        alphaTable[5] = ((u64)alphaTable[0] + 4 * (u64)alphaTable[1]) / 5;
                        alphaTable[6] = 0;
                        alphaTable[7] = 255;
                    }

                    bitmapDDSCalculateColorsFromDXT5Block(block, colTable);
                }break;
            }

            for(u64 pY = 0; pY < 4 && ((pY + bY) < input.h); pY++)
            {
                for(u64 pX = 0; pX < 4 && ((pX + bX) < input.w); pX++)
                {
                    u64 y = (bY + pY) * input.w * uncompressed.bytesPerPixel;
                    u64 x = (bX + pX) * uncompressed.bytesPerPixel;

                    u8 index = (indices >> (2 * (pY * 4 + pX))) & 0x03;
                    vec3u8 col = colTable[index];

                    u8 alphaIndex = 0;
                    u8 alpha = 0;
                    switch (input.compressionType)
                    {
                        case DXT1: alpha = 255; break;
                        case DXT3: 
                        {
                            alpha = (alphas >> (4 * (pY * 4 + pX))) & 0b1111;
                            // extend the alpha
                            alpha = (alpha << 4) | alpha;
                        }break;
                        case DXT5:
                        {
                            alphaIndex = (alphas >> (3 * (pY * 4 + pX))) & 0b111;
                            alpha = alphaTable[alphaIndex];
                        }break;
                    }

                    uncompressed.pixels[y + x] = col.r;
                    uncompressed.pixels[y + x + 1] = col.g;
                    uncompressed.pixels[y + x + 2] = col.b;
                    uncompressed.pixels[y + x + 3] = alpha;
                }
            }
        }
    }

    return uncompressed;
}

Bitmap bitmapFromDDSRaw(Arena *arena, u8 *rawBytes, u64 byteLen)
{
    Bitmap bm = {0};

    u8 *currBytePtr = rawBytes;
    DDSHeader header;

    if (byteLen > gBitmapFileKindsTable[BITMAP_FILE_KIND_DDS].numOfMagicBytes)
    {
        if (rawBytes[0] == 0x44 && rawBytes[1] == 0x44 && rawBytes[2] == 0x53 && rawBytes[3] == 0x20)
        {
            currBytePtr += 4;

            BASE_MEMCPY(&header, currBytePtr, sizeof(DDSHeader));
            currBytePtr += sizeof(DDSHeader);
            
            bm.size.w = header.dwWidth;
            bm.size.h = header.dwHeight;

            bool containsAlpha = header.ddspf.dwFlags & DDPF_ALPHAPIXELS;
            bool containsAlphaOnly = header.ddspf.dwFlags & DDPF_ALPHA;
            bool isCompressed = header.ddspf.dwFlags & DDPF_FOURCC;
            bool containsUncompressedRGB = header.ddspf.dwFlags & DDPF_RGB;
            bool yuv = header.ddspf.dwFlags & DDPF_YUV;
            bool luminanceOnly = header.ddspf.dwFlags & DDPF_LUMINANCE;
            u32 dataFormat = header.ddspf.dwFourCC;
            u32 bitCount = 0;

            if(containsAlpha || containsAlphaOnly || containsUncompressedRGB || luminanceOnly || yuv)
            {
                bitCount = header.ddspf.dwRGBBitCount;

                 // you want the opposite because little endian
                if (header.ddspf.dwRBitMask == 0x000000ff &&
                    header.ddspf.dwGBitMask == 0x0000ff00 &&
                    header.ddspf.dwBBitMask == 0x00ff0000 &&
                    header.ddspf.dwABitMask == 0xff000000)
                {
                    bm.fmt = BITMAP_FORMAT_R8G8B8A8;
                }
                else if (header.ddspf.dwRBitMask == 0x00ff0000 &&
                        header.ddspf.dwGBitMask == 0x0000ff00 &&
                        header.ddspf.dwBBitMask == 0x000000ff &&
                        header.ddspf.dwABitMask == 0xff000000)
                {
                    bm.fmt = BITMAP_FORMAT_B8G8R8A8;
                }

                if(bm.fmt == BITMAP_FORMAT_UNKNOWN)
                {
                    logThreadErrorFmt("bitmap format is unknown, something went wrong");
                    return (Bitmap){0};
                }

                bm.bytesPerPixel = bitCount / 8;

                u64 dataSize = bm.size.w * bm.size.h * bm.bytesPerPixel;
                bm.pixels = arenaPushNoZero(arena, dataSize);
                BASE_MEMCPY(bm.pixels, currBytePtr, dataSize);
            }
            else if(isCompressed)
            {
                if(header.dwFlags & DDSD_LINEARSIZE)
                {
                    u32 compressedDataByteLength = header.dwPitchOrLinearSize;
                    DDSCompressedData c = 
                    {
                        .byteCount = compressedDataByteLength,
                        .bytes = currBytePtr,
                        .w = header.dwWidth,
                        .h = header.dwHeight,
                        .compressionType = dataFormat,    
                    };

                    DDSUncompressedData uncompressed = bitmapDDSUncompress(arena, c);

                    bm.bytesPerPixel = uncompressed.bytesPerPixel;
                    bm.fmt = uncompressed.fmt;
                    bm.pixels = uncompressed.pixels;
                }
                else
                {
                    logThreadErrorFmt("The dds file is compressed but is missing the flag DDSD_LINEARSIZE, something is wrong with its contents.");
                    return (Bitmap){0};
                }
            }
            else
            {
                logThreadErrorFmt("Unable to determine bitcount from dds file. Maybe it is set up incorrectly.");
                return (Bitmap){0};
            }
        }
    }

    bm.srcFile = BITMAP_FILE_KIND_DDS;
    return bm;
}
Bitmap bitmapFromDDSPath(Arena *arena, str8 file)
{
    Bitmap bm = {0};
    ArenaTemp temp = baseTempBegin(&arena, 1);
    {
        U8Array fileBytes = OSFileReadAll(temp.arena, file);

        if(fileBytes.data != null)
        {
            bm = bitmapFromDDSRaw(arena, fileBytes.data, fileBytes.len);
            if (bm.pixels == null)
            {
                logThreadErrorFmt("Failed to parse '%S'", file);
            }
        }
    }
    baseTempEnd(temp);

    return bm;
}