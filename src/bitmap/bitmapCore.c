#include "bitmap\bitmapCore.h"

// assumes little endian
BitmapFileKindTableEntry gBitmapFileKindsTable[BITMAP_FILE_KIND_COUNT] = 
{
    [BITMAP_FILE_KIND_UNKNOWN] = {.ext = STR8_LIT_COMP_CONST("unknown"), .kind = BITMAP_FILE_KIND_UNKNOWN, .numOfMagicBytes = 0},
    [BITMAP_FILE_KIND_DDS]     = {.ext = STR8_LIT_COMP_CONST(".dds"), .kind = BITMAP_FILE_KIND_DDS, .magicBytes = {0x44, 0x44, 0x53, 0x20} , .numOfMagicBytes = 4},
    [BITMAP_FILE_KIND_BMP]     = {.ext = STR8_LIT_COMP_CONST(".bmp"), .kind = BITMAP_FILE_KIND_BMP, .magicBytes = {0x42, 0x4d}, .numOfMagicBytes = 2},
    [BITMAP_FILE_KIND_TGA]     = {.ext = STR8_LIT_COMP_CONST(".tga"), .kind = BITMAP_FILE_KIND_TGA, .magicBytes = {0}, .numOfMagicBytes = 0},
    [BITMAP_FILE_KIND_PNG]     = {.ext = STR8_LIT_COMP_CONST(".png"), .kind = BITMAP_FILE_KIND_PNG, .magicBytes = {0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a}, .numOfMagicBytes = 8},
};

BitmapFileKind bitmapFileKindFromPath(str8 path)
{
    for(u64 i = 0; i < BASE_ARRAY_SIZE(gBitmapFileKindsTable); i++)
    {
        BitmapFileKindTableEntry entry = gBitmapFileKindsTable[i];
        
        if (baseStringsStrEndsWith(path, entry.ext, STR_MATCHFLAGS_CASE_INSENSITIVE))
        {
            return entry.kind;
        }
    }
    
    // if we couldnt find it based on path extension, 
    // open the file and check the magic number

    BitmapFileKind fallBackKind = BITMAP_FILE_KIND_UNKNOWN;
    BaseArenaTemp temp = baseTempBegin(null, 0);
    {
        u64 fileSize = 0;
        u8 *fileBytes = OSReadFileAll(temp.arena, path, &fileSize);

        if (fileBytes != null && fileSize >= BITMAP_MAX_MAGIC_BYTES)
        {
            for(u64 i = 0; i < BASE_ARRAY_SIZE(gBitmapFileKindsTable); i++)
            {
                BitmapFileKindTableEntry entry = gBitmapFileKindsTable[i];
                
                // assumes little endian
                u32 eq = 0;
                for(u64 mb = 0; mb < entry.numOfMagicBytes; mb++)
                {
                    if(entry.magicBytes[mb] == fileBytes[mb]) eq++;
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

Bitmap bitmapFromPath(BaseArena *arena, str8 file)
{
    Bitmap bm = {0};
    BaseArenaTemp temp = baseTempBegin(&arena, 1);
    {
        BitmapFileKind kind = bitmapFileKindFromPath(file);
        switch(kind)
        {
            case BITMAP_FILE_KIND_DDS: return bitmapFromDDSPath(arena, file);
            case BITMAP_FILE_KIND_PNG: return bitmapFromPNGPath(arena, file);
            case BITMAP_FILE_KIND_BMP: return bitmapFromDDSPath(arena, file);
            case BITMAP_FILE_KIND_TGA: return bitmapFromDDSPath(arena, file);

            case BITMAP_FILE_KIND_UNKNOWN:
            case BITMAP_FILE_KIND_COUNT:
            default:
            {
                logProgErrorFmt("Unregognised image file format '%S'", file);
            }break;
        }
    }
    baseTempEnd(temp);

    return bm;
}

Bitmap bitmapFromBMPRaw(BaseArena *arena, u8 *rawBytes, u64 byteLen);
Bitmap bitmapFromBMPPath(BaseArena *arena, str8 file);

Bitmap bitmapFromTGARaw(BaseArena *arena, u8 *rawBytes, u64 byteLen);
Bitmap bitmapFromTGAPath(BaseArena *arena, str8 file);