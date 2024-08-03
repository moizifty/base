#include "bitmap\bitmapPNG.h"
#include "base\baseLog.h"

u8 gBitmapPNGColorTypeComponentsTable[PNG_COLOR_TYPE_COUNT] = 
{
    [PNG_COLOR_TYPE_GREYSCALE] = 1,
    [PNG_COLOR_TYPE_TRUECOLOR] = 3,
    [PNG_COLOR_TYPE_INDEXED] = 3,
    [PNG_COLOR_TYPE_GREYSCALE_ALPHA] = 2,
    [PNG_COLOR_TYPE_TRUECOLOR_ALPHA] = 4,
};

PNGChunk bitmapPNGParseNextChunk(u8 *currPtr)
{
    PNGChunk chunk = {0};

    //chunk length is big endian whereas my machine is little endian
    BASE_MEMCPY_BE(&chunk.length, currPtr, sizeof(u32));
    currPtr += sizeof(u32);

    BASE_MEMCPY(chunk.chunkName, currPtr, sizeof(u8[4]));
    currPtr += sizeof(u8[4]);

    chunk.chunkData = currPtr;
    currPtr += chunk.length;
    
    BASE_MEMCPY(&chunk.crc, currPtr, sizeof(u32));
    currPtr += sizeof(u32);

    return chunk;
}
PNGChunkIHDR bitmapPNGParseIHDRInfo(PNGChunk ihdrChunk)
{
    PNGChunkIHDR hdr = {0};
    BASE_MEMCPY_BE(&hdr.width, ihdrChunk.chunkData, sizeof(u32));
    ihdrChunk.chunkData += sizeof(u32);

    BASE_MEMCPY_BE(&hdr.height, ihdrChunk.chunkData, sizeof(u32));
    ihdrChunk.chunkData += sizeof(u32);

    BASE_MEMCPY(&hdr.bitDepth, ihdrChunk.chunkData, sizeof(u8));
    ihdrChunk.chunkData += sizeof(u8);

    BASE_MEMCPY(&hdr.colType, ihdrChunk.chunkData, sizeof(u8));
    ihdrChunk.chunkData += sizeof(u8);

    BASE_MEMCPY(&hdr.compressMethod, ihdrChunk.chunkData, sizeof(u8));
    ihdrChunk.chunkData += sizeof(u8);

    BASE_MEMCPY(&hdr.filterMethod, ihdrChunk.chunkData, sizeof(u8));
    ihdrChunk.chunkData += sizeof(u8);

    BASE_MEMCPY(&hdr.interlaceMethod, ihdrChunk.chunkData, sizeof(u8));
    ihdrChunk.chunkData += sizeof(u8);

    return hdr;
}
PNGCollectIDATChunksData bitmapPNGCollectIDATChunks(BaseArena *arena, u8 *currBytePtr, PNGChunkList *list)
{
    PNGChunkIHDR hdrInfo = {0};
    PNGChunk currChunk = {0};

    u8 *originalBytePtr = currBytePtr;

    do
    {
        currChunk = bitmapPNGParseNextChunk(currBytePtr);

        switch(currChunk.chunkType)
        {
            //IHDR
            case PNG_CHUNK_TYPE_IHDR:
            {
                hdrInfo = bitmapPNGParseIHDRInfo(currChunk);

            }break;

            //IDAT
            case PNG_CHUNK_TYPE_IDAT:
            {
                PNGChunk *idatNode = baseArenaPush(arena, sizeof(PNGChunk));
                *idatNode = currChunk;

                BasePtrListNodePushLast(list, idatNode);
            }break;

            // PLTE
            case PNG_CHUNK_TYPE_PLTE:
            {

            }break;
            
            // IEND
            case PNG_CHUNK_TYPE_IEND: break;

            default:
            {
                if (!(currChunk.chunkName[0] & 0b100000))
                {
                    logProgErrorFmt("Decoder does not handler chunk type '%S'", baseStr8(currChunk.chunkName, 4));
                    return (PNGCollectIDATChunksData){0};
                }
            }break;
        }

        currBytePtr += sizeof(u32) + sizeof(u8[4]) + currChunk.length + sizeof(u32);

    }while(currChunk.chunkType != PNG_CHUNK_TYPE_IEND);

    PNGCollectIDATChunksData ret = 
    {
        .hdr = hdrInfo,
        .bytesMoved = (currBytePtr - originalBytePtr),
    };

    ret.pngInfo.colorComponents = gBitmapPNGColorTypeComponentsTable[ret.hdr.colType];
    ret.pngInfo.bitDepth = ret.hdr.bitDepth;
    ret.pngInfo.w = ret.hdr.width;
    ret.pngInfo.h = ret.hdr.height;

    return ret;
}
U8Array bitmapPNGCombineIDATDataBlocks(BaseArena *arena, PNGChunkList *list)
{
    u64 combinedDataBlockLen = 0;
    // https://www.w3.org/TR/png-3/#10CompressionCM0
    // https://www.rfc-editor.org/rfc/rfc1950
    // its hella confusing but the size of the compressed data block is size of chunk minus 1 minus 1
    // not minus 1 minus 1 minus 4 (- 1 - 1 - 4), since the adlar sum thing is at the end of the entire data combined
    // abit dumb.
    BASE_PTR_LIST_FOREACH(PNGChunk, chunk, list)
    {
        combinedDataBlockLen += chunk->length;
    }

    combinedDataBlockLen = combinedDataBlockLen - 1 - 1;
    U8Array view = 
    {
        .data = baseArenaPushNoZero(arena, combinedDataBlockLen),
    };

    BASE_PTR_LIST_FOREACH(PNGChunk, chunk, list)
    {
        // https://www.w3.org/TR/png-3/#10CompressionCM0
        // https://www.rfc-editor.org/rfc/rfc1950
        u8 *dataBlock = chunk->chunkData;
        u64 dataBlockLen = chunk->length;
        if(chunk == list->first)
        {
            dataBlock = chunk->chunkData + 2;
            dataBlockLen = chunk->length - 1 - 1;
        }

        BASE_MEMCPY(((u8 *) view.data) + view.len, dataBlock, dataBlockLen);
        view.len += dataBlockLen;
    }

    return view;
}
PNGUncompressedData bitmapPNGUncompress(BaseArena *arena, PNGCompressedData input)
{
    PNGUncompressedData uncompressedRet = {0};
    uncompressedRet.pngInfo = input.pngInfo;

    BaseBitstream stream = {.bytes = input.compressedStream };
    u64 imageBufferLength = input.pngInfo.w * input.pngInfo.h * ((input.pngInfo.bitDepth * input.pngInfo.colorComponents) / 8);

    // the filter is stored as one byte per scanline
    // so the size of the decompressed buffer is
    // image buffer size + (image height * 1);
    U8Array decompressedBuffer = {0};
    u64 filteredBufferSize = imageBufferLength + input.pngInfo.h * 1;
    decompressedBuffer.data = baseArenaPushNoZero(arena, filteredBufferSize);
    decompressedBuffer.len = filteredBufferSize;

    // when decompressing the filtered data will be stored in decompressedBuffer which is why u create it from temp arena
    // the actuall unfiltereed data will be created with the arena passed in
    compressionDeflateUncompress(&stream, &decompressedBuffer);
    uncompressedRet.uncompressedStream = decompressedBuffer;

    return uncompressedRet;
}
u32 bitmapPNGFilterPaethPredictor(u32 a, u32 b, u32 c)
{
    u32 p = a + b - c;
    u32 pa = abs(p - a);
    u32 pb = abs(p - b);
    u32 pc = abs(p - c);
    if (pa <= pb && pa <= pc)
    {
        return a;
    }
    else if (pb <= pc)
    {
        return b;
    }
    else
    {
        return c;
    }
}
PNGUnfilteredData bitmapPNGUnfilter(BaseArena *arena, PNGUncompressedData uncompressedInput)
{
    u64 pixelWidth = (uncompressedInput.pngInfo.colorComponents * (uncompressedInput.pngInfo.bitDepth / 8));
    u64 scanlineByteWidthIncludingFilter = uncompressedInput.pngInfo.w * pixelWidth + 1; //+1 for filter type byte at start

    U8Array defilteredBuffer = {0};
    u64 imageBufferLength = uncompressedInput.pngInfo.w * uncompressedInput.pngInfo.h * (pixelWidth);
    defilteredBuffer.data = baseArenaPushNoZero(arena, imageBufferLength);
    defilteredBuffer.len = imageBufferLength;

    u8 *data = uncompressedInput.uncompressedStream.data;
    for(u64 scanline = 0; scanline < uncompressedInput.pngInfo.h; scanline++)
    {
        u64 scanlineBeginByteIndex = scanline * (scanlineByteWidthIncludingFilter);
        u8 filterType = data[scanlineBeginByteIndex];

        for(u64 x = 0; x < scanlineByteWidthIncludingFilter - 1; x++)
        {
            u64 loadIndex = (scanlineBeginByteIndex + 1) + x;
            u64 storeIndex = ((scanlineBeginByteIndex + 1) + x) - (1 * (scanline + 1));

            // todo: for a, b and c i dont account for pixel bit depth, if its 16 i want to jump 2 byts each for component
            i64 aIndex = (x < uncompressedInput.pngInfo.colorComponents) ? -1 : storeIndex - uncompressedInput.pngInfo.colorComponents;
            i64 bIndex = (scanline <= 0) ? -1 : storeIndex - (scanlineByteWidthIncludingFilter - 1);
            i64 cIndex = (x < uncompressedInput.pngInfo.colorComponents || scanline <= 0) ? -1 : bIndex - uncompressedInput.pngInfo.colorComponents;

            u8 a = (aIndex < 0) ? 0 : defilteredBuffer.data[aIndex];
            u8 b = (bIndex < 0) ? 0 : defilteredBuffer.data[bIndex];
            u8 c = (cIndex < 0) ? 0 : defilteredBuffer.data[cIndex];
            
            switch(filterType)
            {
                case 0: defilteredBuffer.data[storeIndex] = data[loadIndex]; break;
                case 1: defilteredBuffer.data[storeIndex] = data[loadIndex] + a; break;
                case 2: defilteredBuffer.data[storeIndex] = data[loadIndex] + b; break;
                case 3: defilteredBuffer.data[storeIndex] = data[loadIndex] + (a + b) / 2; break;
                case 4: defilteredBuffer.data[storeIndex] = (u8)((u32)data[loadIndex] + bitmapPNGFilterPaethPredictor(a, b, c)); break;
                default: break;
            }
        }
    }

    return (PNGUnfilteredData){.pngInfo = uncompressedInput.pngInfo, .output = defilteredBuffer};
}
Bitmap bitmapFromPNGRaw(BaseArena *arena, u8 *rawBytes, u64 byteLen)
{
    Bitmap bm = {0};

    u8 *currBytePtr = rawBytes;

    if (byteLen > gBitmapFileKindsTable[BITMAP_FILE_KIND_PNG].numOfMagicBytes) 
    {
        u8 *magicBytes = gBitmapFileKindsTable[BITMAP_FILE_KIND_PNG].magicBytes;
        u32 numMagicBytes = gBitmapFileKindsTable[BITMAP_FILE_KIND_PNG].numOfMagicBytes;

        if (!BASE_MEMCMP(magicBytes, currBytePtr, numMagicBytes))
        {
            currBytePtr += numMagicBytes;
            
            PNGChunkList idatChunks = {0};

            PNGCollectIDATChunksData processedData = bitmapPNGCollectIDATChunks(arena, currBytePtr, &idatChunks);
            currBytePtr += processedData.bytesMoved;

            U8Array compressedStream = bitmapPNGCombineIDATDataBlocks(arena, &idatChunks);
            PNGCompressedData input = 
            {
                .pngInfo = processedData.pngInfo,
                .compressedStream = compressedStream,
            };

            BaseArenaTemp temp = baseTempBegin(&arena, 1);
            {
                PNGUncompressedData uncompressedData = bitmapPNGUncompress(temp.arena, input);
                PNGUnfilteredData unfiltered = bitmapPNGUnfilter(arena, uncompressedData);

                bm.pixels = unfiltered.output.data;
                bm.size = Vec2i(unfiltered.pngInfo.w, unfiltered.pngInfo.h);

                // todo: more robust, this is fixed
                bm.bytesPerPixel = unfiltered.pngInfo.colorComponents * (unfiltered.pngInfo.bitDepth / 8);
                bm.fmt = BITMAP_FORMAT_RGBA_8;
            }

            baseTempEnd(temp);
        }
    }

    bm.srcFile = BITMAP_FILE_KIND_PNG;
    return bm;
}

Bitmap bitmapFromPNGPath(BaseArena *arena, str8 file)
{
    Bitmap bm = {0};
    BaseArenaTemp temp = baseTempBegin(&arena, 1);
    {
        U8Array fileBytes = OSFileReadAll(temp.arena, file);

        if(fileBytes.data != null)
        {
            bm = bitmapFromPNGRaw(arena, fileBytes.data, fileBytes.len);
            if (bm.pixels == null)
            {
                logProgErrorFmt("Failed to parse '%S'", file);
            }
        }
    }
    baseTempEnd(temp);

    return bm;
}