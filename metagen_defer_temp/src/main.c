#include "base/base.h"
#include "os/core/osCore.h"
#include "bitmap/bitmapCore.h"
#include "font.h"

#include "base/base.c"
#include "os/core/osCore.c"
#include "bitmap/bitmapCore.c"
#include "font.c"

#include "os/core/osEntryPoint.c"

void ProgramMain(Str8List *args)
{
    Str8List *trailing = cmdlineTrailing(STR8("fontfile"));

    if (!cmdlineParse(*args) || !BASE_ANY_PTR(trailing))
    {
        cmdlineUsage();
        return;
    }

    Arena *arena = arenaAllocDefault();
    Font font = fontTTFParseFromFile(arena, trailing->first->val);

    if (font.error != FONT_ERROR_NONE)
    {
        baseEPrintf("{r}Failed to load font\n");
        return;
    }

    // for (u64 i = 65; i < font.parsed.parsedGlyf.len; i++)
    // {
    //     termClear();
    //     fontRasteriseGlyphShapeTerminal(font.parsed.parsedGlyf.data[i], 70, 40, true);
    //     Sleep(250);
    // }

    FontAtlas atlas = fontAtlasFromCodepointRanges(arena, font, ARRAY_VIEW(RangeI64Array, rangei, {32, 127}), 48, true);
    basePrintf("Finished\n");

    OSHandle file = OSFileOpen(STR8("test.ppm"), false, OS_FILEACCESS_WRITE, OS_FILECREATION_CREATE_OVERRITE);

    OSFileWriteFmt(file, "P3\n%lld %lld\n255\n", atlas.bitmap.size.w, atlas.bitmap.size.h);

    for (u64 i = 0; i < atlas.bitmap.bytesPerPixel * atlas.bitmap.size.w * atlas.bitmap.size.h; i++)
    {
        OSFileWriteFmt(file, "%lld\n", atlas.bitmap.pixels[i]);
    }

    OSFileClose(file);
    
    basePrintf("Finished\n");
}