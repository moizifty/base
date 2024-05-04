#include "bitmap\bitmap.h"

// uncompress the raw byte data,
vec3i8 bitmapDDSCalculateColorFromU16(u16 col)
{
    vec3i8 col0;

    col0.r = (col >> 11);
    col0.r = (col0.r << 3) | (col0.r >> 2);

    col0.g = (col & 0b00000'111111'00000) >> 5;
    col0.g = (col0.g << 2) | (col0.g >> 4);

    col0.b = (col &  0b00000'000000'11111);
    col0.b = (col0.b << 3) | (col0.b >> 2);

    return col0;
}

DDSUncompressedData bitmapDDSUncompress(BaseArena *arena, DDSCompressedData input)
{
    DDSUncompressedData uncompressed = {0};

    u8 *data = input.bytes;
    switch(input.compressionType)
    {
        case DX10: logProgErrorFmt("The dds uses a dds_header_dxt10 which is not supported."); break;

        case DXT1:
        {
            uncompressed.fmt = BITMAP_FORMAT_RGBA_8;
            uncompressed.bytesPerPixel = 4; //rgba
            uncompressed.pixels = baseArenaPush(arena, input.w * input.h * uncompressed.bytesPerPixel);
            
            u64 numXBlocks = baseGreater(input.w / 4, 1);
            numXBlocks += (input.w > 4 && input.w % 4 != 0) ? 1 : 0;

            u64 numYBlocks = baseGreater(input.h / 4, 1);
            numYBlocks += (input.h > 4 && input.h % 4 != 0) ? 1 : 0;

            for(u64 yBlock = 0; yBlock < numYBlocks; yBlock++)
            {
                for(u64 xBlock = 0; xBlock < numXBlocks; xBlock++)
                {
                    DDSDXT1Block block = {0};
                    BASE_MEMCPY(&block, data, sizeof(block));
                    data += sizeof(block);

                    u32 indices = block.r0 | (block.r1 << 8) | (block.r2 << 16) | (block.r3 << 24);

                    vec3i8 col0 = {0};
                    vec3i8 col1 = {0};
                    vec3i8 col2 = {0};
                    vec3i8 col3 = {0};

                    col0 = bitmapDDSCalculateColorFromU16(block.c0);
                    col1 = bitmapDDSCalculateColorFromU16(block.c1);

                    col2 = (block.c0 > block.c1) ? Vec3i8(((2 * (u16)col0.r) + (u16)col1.r) / 3,
                                                          ((2 * (u16)col0.g) + (u16)col1.g) / 3,
                                                          ((2 * (u16)col0.b) + (u16)col1.b) / 3) :

                                                   Vec3i8(((u16)col0.r + (u16)col1.r) / 2,
                                                          ((u16)col0.g + (u16)col1.g) / 2,
                                                          ((u16)col0.b + (u16)col1.b) / 2);

                    col3 = (block.c0 > block.c1) ? Vec3i8(((u16)col0.r + (2 * (u16)col1.r)) / 3,
                                                          ((u16)col0.g + (2 * (u16)col1.g)) / 3,
                                                          ((u16)col0.b + (2 * (u16)col1.b)) / 3) : col3;

                    u64 yOffset = 4 * yBlock;
                    u64 xOffset = 4 * xBlock;

                    for(u64 pY = 0; pY < 4; pY++)
                    {
                        if((pY + yOffset) >= input.h)
                        {
                            break;
                        }

                        for(u64 pX = 0; pX < 4; pX++)
                        {
                            if ((pX + xOffset) >= input.w)
                            {
                                break;
                            }

                            u64 y = (yOffset + pY) * input.w * uncompressed.bytesPerPixel;
                            u64 x = (xOffset + pX) * uncompressed.bytesPerPixel;

                            u8 index = (indices >> (2 * (pY * 4 + pX))) & 0x03;
                            vec3i8 t = (index == 0) ? col0 : ((index == 1) ? col1 : ((index == 2) ? col2 : col3));

                            uncompressed.pixels[y + x] = t.r;
                            uncompressed.pixels[y + x + 1] = t.g;
                            uncompressed.pixels[y + x + 2] = t.b;
                            uncompressed.pixels[y + x + 3] = 255;
                        }
                    }
                }
            }
        }break;

        case DXT3:
        {
            uncompressed.fmt = BITMAP_FORMAT_RGBA_8;
            uncompressed.bytesPerPixel = 4; //rgba
            uncompressed.pixels = baseArenaPush(arena, input.w * input.h * uncompressed.bytesPerPixel);
            
            u64 numXBlocks = baseGreater(input.w / 4, 1);
            numXBlocks += (input.w > 4 && input.w % 4 != 0) ? 1 : 0;

            u64 numYBlocks = baseGreater(input.h / 4, 1);
            numYBlocks += (input.h > 4 && input.h % 4 != 0) ? 1 : 0;

            for(u64 yBlock = 0; yBlock < numYBlocks; yBlock++)
            {
                for(u64 xBlock = 0; xBlock < numXBlocks; xBlock++)
                {
                    DDSDXT3Block block = {0};
                    BASE_MEMCPY(&block, data, sizeof(block));
                    data += sizeof(block);

                    u32 indices = block.r0 | (block.r1 << 8) | (block.r2 << 16) | (block.r3 << 24);
                    u64 alphas =  (u64) block.alphaP01 | 
                                 ((u64) block.alphaP23 << 8) | 
                                 ((u64) block.alphaP45 << 16) | 
                                 ((u64) block.alphaP67 << 24) |
                                 ((u64) block.alphaP89 << 32) |
                                 ((u64) block.alphaP1011 << 40) |
                                 ((u64) block.alphaP1213 << 48) |
                                 ((u64) block.alphaP1415 << 56);
                    

                    vec3i8 col0 = {0};
                    vec3i8 col1 = {0};
                    vec3i8 col2 = {0};
                    vec3i8 col3 = {0};

                    col0 = bitmapDDSCalculateColorFromU16(block.c0);
                    col1 = bitmapDDSCalculateColorFromU16(block.c1);

                    col2 = (block.c0 > block.c1) ? Vec3i8(((2 * (u16)col0.r) + (u16)col1.r) / 3,
                                                          ((2 * (u16)col0.g) + (u16)col1.g) / 3,
                                                          ((2 * (u16)col0.b) + (u16)col1.b) / 3) :

                                                   Vec3i8(((u16)col0.r + (u16)col1.r) / 2,
                                                          ((u16)col0.g + (u16)col1.g) / 2,
                                                          ((u16)col0.b + (u16)col1.b) / 2);

                    col3 = (block.c0 > block.c1) ? Vec3i8(((u16)col0.r + (2 * (u16)col1.r)) / 3,
                                                          ((u16)col0.g + (2 * (u16)col1.g)) / 3,
                                                          ((u16)col0.b + (2 * (u16)col1.b)) / 3) : col3;

                    u64 yOffset = 4 * yBlock;
                    u64 xOffset = 4 * xBlock;

                    for(u64 pY = 0; pY < 4; pY++)
                    {
                        if((pY + yOffset) >= input.h)
                        {
                            break;
                        }

                        for(u64 pX = 0; pX < 4; pX++)
                        {
                            if ((pX + xOffset) >= input.w)
                            {
                                break;
                            }

                            u64 y = (yOffset + pY) * input.w * uncompressed.bytesPerPixel;
                            u64 x = (xOffset + pX) * uncompressed.bytesPerPixel;

                            u8 index = (indices >> (2 * (pY * 4 + pX))) & 0b11;
                            u8 alpha = (alphas >> (4 * (pY * 4 + pX))) & 0b1111;

                            // extend the alpha
                            alpha = (alpha << 4) | alpha;

                            vec3i8 t = (index == 0) ? col0 : ((index == 1) ? col1 : ((index == 2) ? col2 : col3));

                            uncompressed.pixels[y + x] = t.r;
                            uncompressed.pixels[y + x + 1] = t.g;
                            uncompressed.pixels[y + x + 2] = t.b;
                            uncompressed.pixels[y + x + 3] = alpha;
                        }
                    }
                }
            }
        }break;

        case DXT5:
        {
            uncompressed.fmt = BITMAP_FORMAT_RGBA_8;
            uncompressed.bytesPerPixel = 4; //rgba
            uncompressed.pixels = baseArenaPush(arena, input.w * input.h * uncompressed.bytesPerPixel);
            
            u64 numXBlocks = baseGreater(input.w / 4, 1);
            numXBlocks += (input.w > 4 && input.w % 4 != 0) ? 1 : 0;

            u64 numYBlocks = baseGreater(input.h / 4, 1);
            numYBlocks += (input.h > 4 && input.h % 4 != 0) ? 1 : 0;

            for(u64 yBlock = 0; yBlock < numYBlocks; yBlock++)
            {
                for(u64 xBlock = 0; xBlock < numXBlocks; xBlock++)
                {
                    DDSDXT5Block block = {0};
                    BASE_MEMCPY(&block, data, sizeof(block));
                    data += sizeof(block);

                    u32 indices = block.r0 | (block.r1 << 8) | (block.r2 << 16) | (block.r3 << 24);
                    u64 alphas = (u64) block.a0 | 
                                ((u64) block.a1 << 8) | 
                                ((u64) block.a2 << 16) | 
                                ((u64) block.a3 << 24) |
                                ((u64) block.a4 << 32) |
                                ((u64) block.a5 << 40);

                    u8 alpha0 = block.alpha0;
                    u8 alpha1 = block.alpha1;
                    u8 alpha2 = 0;
                    u8 alpha3 = 0;
                    u8 alpha4 = 0;
                    u8 alpha5 = 0;
                    u8 alpha6 = 0;
                    u8 alpha7 = 0;

                    if (alpha0 > alpha1)
                    {
                        alpha2 = (6 * (u64)alpha0 + (u64)alpha1) / 7;
                        alpha3 = (5 * (u64)alpha0 + 2 * (u64)alpha1) / 7;
                        alpha4 = (4 * (u64)alpha0 + 3 * (u64)alpha1) / 7;
                        alpha5 = (3 * (u64)alpha0 + 4 * (u64)alpha1) / 7;
                        alpha6 = (2 * (u64)alpha0 + 5 * (u64)alpha1) / 7;
                        alpha7 = ((u64)alpha0 + 6 * (u64)alpha1) / 7;
                    }
                    else
                    {
                        alpha2 = (4 * (u64)alpha0 + (u64)alpha1) / 5;
                        alpha3 = (3 * (u64)alpha0 + 2 * (u64)alpha1) / 5;
                        alpha4 = (2 * (u64)alpha0 + 3 * (u64)alpha1) / 5;
                        alpha5 = ((u64)alpha0 + 4 * (u64)alpha1) / 5;
                        alpha6 = 0;
                        alpha7 = 255;
                    }

                    vec3i8 col0 = {0};
                    vec3i8 col1 = {0};
                    vec3i8 col2 = {0};
                    vec3i8 col3 = {0};

                    col0 = bitmapDDSCalculateColorFromU16(block.c0);
                    col1 = bitmapDDSCalculateColorFromU16(block.c1);

                    col2 = (block.c0 > block.c1) ? Vec3i8(((2 * (u16)col0.r) + (u16)col1.r) / 3,
                                                          ((2 * (u16)col0.g) + (u16)col1.g) / 3,
                                                          ((2 * (u16)col0.b) + (u16)col1.b) / 3) :

                                                   Vec3i8(((u16)col0.r + (u16)col1.r) / 2,
                                                          ((u16)col0.g + (u16)col1.g) / 2,
                                                          ((u16)col0.b + (u16)col1.b) / 2);

                    col3 = (block.c0 > block.c1) ? Vec3i8(((u16)col0.r + (2 * (u16)col1.r)) / 3,
                                                          ((u16)col0.g + (2 * (u16)col1.g)) / 3,
                                                          ((u16)col0.b + (2 * (u16)col1.b)) / 3) : col3;

                    u64 yOffset = 4 * yBlock;
                    u64 xOffset = 4 * xBlock;

                    for(u64 pY = 0; pY < 4; pY++)
                    {
                        if((pY + yOffset) >= input.h)
                        {
                            break;
                        }

                        for(u64 pX = 0; pX < 4; pX++)
                        {
                            if ((pX + xOffset) >= input.w)
                            {
                                break;
                            }

                            u64 y = (yOffset + pY) * input.w * uncompressed.bytesPerPixel;
                            u64 x = (xOffset + pX) * uncompressed.bytesPerPixel;

                            u8 index = (indices >> (2 * (pY * 4 + pX))) & 0b11;
                            u8 alphaIndex = (alphas >> (3 * (pY * 4 + pX))) & 0b111;
                            u8 alpha = 0;

                            switch(alphaIndex)
                            {
                                case 0: alpha = alpha0; break;
                                case 1: alpha = alpha1; break;
                                case 2: alpha = alpha2; break;
                                case 3: alpha = alpha3; break;
                                case 4: alpha = alpha4; break;
                                case 5: alpha = alpha5; break;
                                case 6: alpha = alpha6; break;
                                case 7: alpha = alpha7; break;
                            }

                            vec3i8 t = (index == 0) ? col0 : ((index == 1) ? col1 : ((index == 2) ? col2 : col3));

                            uncompressed.pixels[y + x] = t.r;
                            uncompressed.pixels[y + x + 1] = t.g;
                            uncompressed.pixels[y + x + 2] = t.b;
                            uncompressed.pixels[y + x + 3] = alpha;
                        }
                    }
                }
            }
        }break;

        default:
        {
            logProgErrorFmt("The dds file contains an unregognised compression method.");
        }break;
    }
    
    return uncompressed;
}

Bitmap bitmapFromDDSRaw(BaseArena *arena, u8 *rawBytes, u64 byteLen)
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
                    logProgErrorFmt("bitmap format is unknown, something went wrong");
                    return (Bitmap){0};
                }

                bm.bytesPerPixel = bitCount / 8;

                u64 dataSize = bm.size.w * bm.size.h * bm.bytesPerPixel;
                bm.pixels = baseArenaPushNoZero(arena, dataSize);
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
                    logProgErrorFmt("The dds file is compressed but is missing the flag DDSD_LINEARSIZE, something is wrong with its contents.");
                    return (Bitmap){0};
                }
            }
            else
            {
                logProgErrorFmt("Unable to determine bitcount from dds file. Maybe it is set up incorrectly.");
                return (Bitmap){0};
            }
        }
    }

    bm.srcFile = BITMAP_FILE_KIND_DDS;
    return bm;
}
Bitmap bitmapFromDDSPath(BaseArena *arena, str8 file)
{
    Bitmap bm = {0};
    BaseArenaTemp temp = baseTempBegin(&arena, 1);
    {
        u64 fileSize = 0;
        u8 *fileBytes = OSReadFileAll(temp.arena, file, &fileSize);

        if(fileBytes != null)
        {
            bm = bitmapFromDDSRaw(arena, fileBytes, fileSize);
            if (bm.pixels == null)
            {
                logProgErrorFmt("Failed to parse '%S'", file);
            }
        }
    }
    baseTempEnd(temp);

    return bm;
}