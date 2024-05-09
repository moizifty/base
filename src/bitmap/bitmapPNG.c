#include "bitmap\bitmapPNG.h"
#include "log\log.h"
#include "s.h"

u8 gBitmapPNGColorTypeComponentsTable[PNG_COLOR_TYPE_COUNT] = 
{
    [PNG_COLOR_TYPE_GREYSCALE] = 1,
    [PNG_COLOR_TYPE_TRUECOLOR] = 3,
    [PNG_COLOR_TYPE_INDEXED] = 3,
    [PNG_COLOR_TYPE_GREYSCALE_ALPHA] = 2,
    [PNG_COLOR_TYPE_TRUECOLOR_ALPHA] = 4,
};

PNGChunk bitmapPNGParseNextChunk(BaseArena *arena, u8 *currPtr)
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

PNGChunkIHDR bitmapPNGParseIHDRInfo(BaseArena *arena, PNGChunk ihdrChunk)
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
        currChunk = bitmapPNGParseNextChunk(arena, currBytePtr);

        switch(currChunk.chunkType)
        {
            //IHDR
            case PNG_CHUNK_TYPE_IHDR:
            {
                hdrInfo = bitmapPNGParseIHDRInfo(arena, currChunk);

            }break;

            //IDAT
            case PNG_CHUNK_TYPE_IDAT:
            {
                PNGChunk *idatNode = baseArenaPush(arena, sizeof(PNGChunk));
                *idatNode = currChunk;

                BaseDllNodePushLast(list->first, list->last, idatNode);
                list->len++;
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

    ret.colorComponents = gBitmapPNGColorTypeComponentsTable[ret.hdr.colType];

    return ret;
}

ArrayView bitmapPNGCombineIDATDataBlocks(BaseArena *arena, PNGChunkList *list)
{
    u64 combinedDataBlockLen = 0;
    BASE_PTR_LIST_FOREACH(PNGChunk, chunk, list)
    {
        // https://www.w3.org/TR/png-3/#10CompressionCM0
        // https://www.rfc-editor.org/rfc/rfc1950
        combinedDataBlockLen += chunk->length - 1 - 1 - 4;
    }

    ArrayView view = 
    {
        .data = baseArenaPush(arena, combinedDataBlockLen),
    };

    BASE_PTR_LIST_FOREACH(PNGChunk, chunk, list)
    {
        // https://www.w3.org/TR/png-3/#10CompressionCM0
        // https://www.rfc-editor.org/rfc/rfc1950
        u8 *dataBlock = chunk->chunkData + 2;
        u64 dataBlockLen = chunk->length - 1 - 1 - 4;

        BASE_MEMCPY(((u8 *) view.data) + view.len, dataBlock, dataBlockLen);
        view.len += dataBlockLen;
    }

    return view;
}

void bitmapPNGUncompress(BaseArena *arena, PNGCompressedData input)
{
    BaseBitstream stream = {.bytes = (U8Array){.data = input.compressedStream.data, .len = input.compressedStream.len}};
    u64 imageBufferLength = input.w * input.h * (input.bitDepth / 8);

    BaseArenaTemp temp = baseTempBegin(&arena, 1);
    {

        ArrayView decompressedBuffer = {0};
        decompressedBuffer.data = baseArenaPushNoZero(temp.arena, imageBufferLength);
        decompressedBuffer.len = imageBufferLength;

        // when decompressing the filtered data will be stored in decompressedBuffer which is why u create it from temp arena
        // the actuall unfiltereed data will be created with the arena passed in
        compressionDeflateUncompress(&stream, &decompressedBuffer);

        //todo unfilter png
    }
    baseTempEnd(temp);
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

            ArrayView compressedStream = bitmapPNGCombineIDATDataBlocks(arena, &idatChunks);
            PNGCompressedData input = 
            {
                .w = processedData.hdr.width,
                .h = processedData.hdr.height,
                .colorComponents = processedData.colorComponents,
                .compressedStream = compressedStream,
                .bitDepth = processedData.hdr.bitDepth,
            };

            bitmapPNGUncompress(arena, input);

            int a = 90;
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
        u64 fileSize = 0;
        u8 *fileBytes = OSReadFileAll(temp.arena, file, &fileSize);

        if(fileBytes != null)
        {
            bm = bitmapFromPNGRaw(arena, fileBytes, fileSize);
            if (bm.pixels == null)
            {
                logProgErrorFmt("Failed to parse '%S'", file);
            }
        }
    }
    baseTempEnd(temp);

    return bm;
}