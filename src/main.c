#include "base/base.h"
#include "os/core/osCore.h"
#include "bitmap/bitmapCore.h"
#include "fontCore.h"

#include "base/base.c"
#include "os/core/osCore.c"
#include "bitmap/bitmapCore.c"
#include "fontCore.c"

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

    FontAtlas atlas = fontAtlasFromCodepointRanges(arena, font, ARRAY_VIEW(RangeI64Array, rangei, {32, 128}), 32, true);
    basePrintf("Finished\n");

    Bitmap bm = bitmapPush(arena, atlas.bitmap.size, BITMAP_FORMAT_R8G8B8);
    str8 testString = STR8("ABCD123455\nmoiz is a maniac!!\"hi\"\n#$100%£");

    u64 x = 0;
    u64 y = 0;
    u64 yOffset = 0;
    u64 bytesDone = 0;
    while(*testString.data)
    {
        DecodeCodePointInfo cp = DecodeCodepointFromUtf8(testString.data, testString.len - bytesDone);
        testString.data += cp.advance;
        bytesDone += cp.advance;

        if (cp.codepoint != '\n')
        {
            FontAtlasGlyph glyph = {0};
            fontAtlasTryGetGlyphFromCodepoint(atlas, cp.codepoint, &glyph);
            
            x += glyph.bearingLeft;
            y = yOffset + glyph.bearingTop;

            range2i srcRange = 
            {
                .min = glyph.pos,
                .max = vec2iAdd(glyph.pos, glyph.size),
            };

            range2i destRange = 
            {
                .min = Vec2i(x, y),
                .max = vec2iAdd(Vec2i(x, y), glyph.size),
            };

            bitmapBlitToBitmap(&atlas.bitmap, &bm, srcRange, destRange, BitmapSamplerDefault, BitmapSamplerDefault);

            x += glyph.size.w + glyph.bearingRight;
        }
        else
        {
            x = 0;
            yOffset += (atlas.metrics.ascent - atlas.metrics.descent) + atlas.metrics.lineGap;
        }
        
    }

    OSHandle file = OSFileOpen(STR8("test2.ppm"), false, OS_FILEACCESS_WRITE, OS_FILECREATION_CREATE_OVERRITE);

    OSFileWriteFmt(file, "P3\n%lld %lld\n255\n", bm.size.w, bm.size.h);

    for (u64 i = 0; i < bm.bytesPerPixel * bm.size.w * bm.size.h; i++)
    {
        OSFileWriteFmt(file, "%lld\n", bm.pixels[i]);
    }

    OSFileClose(file);
    
    basePrintf("Finished\n");
}