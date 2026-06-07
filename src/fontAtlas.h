#ifndef FONT_ATLAS_H
#define FONT_ATLAS_H

#include "fontCore.h"
#include "fontTTF.h"

typedef struct FontAtlasGlyph
{
    vec2i pos;
    vec2i size;

    // lsb, y bearing from baseline
    i64 bearingTop;
    i64 bearingLeft;
    i64 bearingRight;
    u64 advanceWidth;

    u32 codepoint;

    bool isInvalidGlyph;
}FontAtlasGlyph;

BASE_CREATE_ARRAY_VIEW_DECLS_DEFS(FontAtlasGlyphArray, FontAtlasGlyph)

typedef struct FontAtlas
{
    Bitmap bitmap;
    FontMetrics metrics;

    // essentially adcii characters are stored first for fast access with ascii codes
    u64 numFastAccessGlyphs;
    FontAtlasGlyphArray glyphs;
}FontAtlas;

FontAtlas fontAtlasFromCodepointRanges(Arena *arena, Font font, RangeI64Array ranges, f64 pixelSize, bool allowNullGlyph);
bool fontAtlasTryGetGlyphFromCodepoint(FontAtlas atlas, i32 codepoint, FontAtlasGlyph *out);

#endif