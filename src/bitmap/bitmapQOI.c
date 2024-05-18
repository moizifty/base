#include "bitmapQOI.h"

u64 bitmapQOIHashColor(vec4u8 color)
{
    u8 r = color.r;
    u8 g = color.g;
    u8 b = color.b;
    u8 a = color.a;

    return ((r * 3) + (g * 5) + (b * 7) + (a * 11)) % 64;
}

QOITagKind bitmapQOIGetTagType(BaseBitstream *stream)
{
    u8 tag = 0;
    if(!baseBitstreamPeekBitsAsU8(stream, 8, &tag))
    {
        return -1;
    }

    switch(tag)
    {
        case QOI_TAG_RGB:
        case QOI_TAG_RGBA:
        {
            baseBitstreamConsumeBits(stream, 8);
            return tag;
        }break;
        default:
        {
            tag = tag >> 6;
            switch(tag)
            {
                case QOI_TAG_DIFF:
                case QOI_TAG_INDEX:
                case QOI_TAG_LUMA:
                case QOI_TAG_RUN:
                {
                    return tag;
                }break;

                default:
                {
                    // unregognised
                    //error
                    //todo
                    return -1;
                }break;
            }
        }break;
    }
}

u64 bitmapQOIProcessNextChunk(BaseBitstream *stream, vec4u8 prevPixels[64], vec4u8 prevPixel, U8Array pixels, u64 currOffset)
{
    QOITagKind tag = bitmapQOIGetTagType(stream);

    u64 bytesWritten = 0;
    switch(tag)
    {
        case QOI_TAG_RGB:
        {
            u8 r, g, b;
            baseBitstreamPopU8(stream, &r);
            baseBitstreamPopU8(stream, &g);
            baseBitstreamPopU8(stream, &b);

            pixels.data[currOffset] = r;
            pixels.data[currOffset + 1] = g;
            pixels.data[currOffset + 2] = b;
            pixels.data[currOffset + 3] = prevPixel.a;

            bytesWritten += 4;
        }break;
        case QOI_TAG_RGBA:
        {
            u8 r, g, b, a;
            baseBitstreamPopU8(stream, &r);
            baseBitstreamPopU8(stream, &g);
            baseBitstreamPopU8(stream, &b);
            baseBitstreamPopU8(stream, &a);

            pixels.data[currOffset] = r;
            pixels.data[currOffset + 1] = g;
            pixels.data[currOffset + 2] = b;
            pixels.data[currOffset + 3] = a;

            bytesWritten += 4;
        }break;
        case QOI_TAG_RUN:
        {
            u8 run;
            baseBitstreamPopBitsAsU8(stream, 6, &run);
            run += 1;

            while(run--)
            {
                pixels.data[currOffset + bytesWritten] = prevPixel.r;
                pixels.data[currOffset + bytesWritten + 1] = prevPixel.g;
                pixels.data[currOffset + bytesWritten + 2] = prevPixel.b;
                pixels.data[currOffset + bytesWritten + 3] = prevPixel.a;

                bytesWritten += 4;
            }

            baseBitstreamConsumeBits(stream, 2);
        }break;
        case QOI_TAG_INDEX:
        {
            u8 index;
            baseBitstreamPopBitsAsU8(stream, 6, &index);

            pixels.data[currOffset] = prevPixels[index].r;
            pixels.data[currOffset + 1] = prevPixels[index].g;
            pixels.data[currOffset + 2] = prevPixels[index].b;
            pixels.data[currOffset + 3] = prevPixels[index].a;

            bytesWritten += 4;
            baseBitstreamConsumeBits(stream, 2);
        }break;
        case QOI_TAG_DIFF:
        {
            u8 dr, dg, db;
            baseBitstreamPopBitsAsU8(stream, 2, &db);
            baseBitstreamPopBitsAsU8(stream, 2, &dg);
            baseBitstreamPopBitsAsU8(stream, 2, &dr);

            i8 udr = ((i8)dr - 2);
            i8 udg = ((i8)dg - 2);
            i8 udb = ((i8)db - 2);

            pixels.data[currOffset] = prevPixel.r + udr;
            pixels.data[currOffset + 1] = prevPixel.g + udg;
            pixels.data[currOffset + 2] = prevPixel.b + udb;
            pixels.data[currOffset + 3] = prevPixel.a;

            bytesWritten += 4;
            baseBitstreamConsumeBits(stream, 2);
        }break;
        case QOI_TAG_LUMA:
        {
            u8 dg, drdg, dbdg;
            baseBitstreamPopBitsAsU8(stream, 6, &dg);
            baseBitstreamConsumeBits(stream, 2);

            baseBitstreamPopBitsAsU8(stream, 4, &dbdg);
            baseBitstreamPopBitsAsU8(stream, 4, &drdg);

            i8 udg = ((i8)dg - 32);
            i8 udrdg = ((i8)drdg - 8);
            i8 udbdg = ((i8)dbdg - 8);

            pixels.data[currOffset + 1] = prevPixel.g + udg;

            pixels.data[currOffset] = prevPixel.r + udg + udrdg;
            pixels.data[currOffset + 2] = prevPixel.b + udg + udbdg;
            pixels.data[currOffset + 3] = prevPixel.a;

            bytesWritten += 4;
        }break;
        default:
        {
            // error;
            return 0;
        };
    }

    return bytesWritten;
}

Bitmap bitmapFromQOIRaw(BaseArena *arena, u8 *rawBytes, u64 byteLen)
{
    Bitmap bm = {0};

    u8 *currBytePtr = rawBytes;

    if (byteLen > gBitmapFileKindsTable[BITMAP_FILE_KIND_QOI].numOfMagicBytes) 
    {
        u8 *magicBytes = gBitmapFileKindsTable[BITMAP_FILE_KIND_QOI].magicBytes;
        u32 numMagicBytes = gBitmapFileKindsTable[BITMAP_FILE_KIND_QOI].numOfMagicBytes;

        if (!BASE_MEMCMP(magicBytes, currBytePtr, numMagicBytes))
        {
            QOIHeader header;

            BaseBitstream inputStream = {.bytes = (U8Array){.data = currBytePtr, .len = byteLen}};
            baseBitstreamPopU32LE(&inputStream, (u32*)header.magic);
            baseBitstreamPopU32BE(&inputStream, &header.width);
            baseBitstreamPopU32BE(&inputStream, &header.height);
            baseBitstreamPopU8(&inputStream, &header.channels);
            baseBitstreamPopU8(&inputStream, &header.colorspace);

            u64 imgByteSizeIncludingAlpha = header.width * header.height * 4;
            U8Array imgPixels = {.data = baseArenaPushNoZero(arena, imgByteSizeIncludingAlpha), .len = imgByteSizeIncludingAlpha};

            vec4u8 prevPixels[64] = {0};
            vec4u8 prevPixel = Vec4u8(0, 0, 0, 255);

            u64 bytesWritten = 0;
            while(bytesWritten < imgByteSizeIncludingAlpha)
            {
                bytesWritten += bitmapQOIProcessNextChunk(&inputStream, prevPixels, prevPixel, imgPixels, bytesWritten);
                prevPixel.r = imgPixels.data[(bytesWritten - 1) - 3];
                prevPixel.g = imgPixels.data[(bytesWritten - 1) - 2];
                prevPixel.b = imgPixels.data[(bytesWritten - 1) - 1];
                prevPixel.a = imgPixels.data[(bytesWritten - 1)];

                prevPixels[bitmapQOIHashColor(prevPixel)] = prevPixel;
            }

            bm.bytesPerPixel = 1;
            bm.fmt = BITMAP_FORMAT_RGBA_8,
            bm.pixels = imgPixels.data;
            bm.size = Vec2i(header.width, header.height);
        }
    }

    bm.srcFile = BITMAP_FILE_KIND_QOI;
    return bm;
}

Bitmap bitmapFromQOIPath(BaseArena *arena, str8 file)
{
    Bitmap bm = {0};

    BaseArenaTemp temp = baseTempBegin(&arena, 1);
    {
        U8Array fileBytes = OSFileReadAll(temp.arena, file);

        if(fileBytes.data != null)
        {
            bm = bitmapFromQOIRaw(arena, fileBytes.data, fileBytes.len);
            if (bm.pixels == null)
            {
                logProgErrorFmt("Failed to parse '%S'", file);
            }
        }
    }
    baseTempEnd(temp);

    return bm;
}