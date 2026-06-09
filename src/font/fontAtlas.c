#include "fontAtlas.h"

i64 fontAtlasGetGlyphArea(FontAtlasGlyph g)
{
    return g.size.x * g.size.y;
}
i32 fontAtlasGlyphDescendingCompare(const void *g1, const void *g2)
{
    FontAtlasGlyph *a = (FontAtlasGlyph*)g1;
    FontAtlasGlyph *b = (FontAtlasGlyph*)g2;

    return fontAtlasGetGlyphArea(*b) - fontAtlasGetGlyphArea(*a);
}

u64 fontAtlasEstimateNumberOfGlyphsThatFitInBitmap(Font font, vec2i bitmapSize, RangeI64Array ranges, f64 pixelSize, bool allowNullGlyph)
{
    u64 numGlyphs = 0;
    u64 currX = 0;
    u64 currY = 0;
    u64 maxGlyphHeightInRow = 0;
    bool blittedNullGlyph = false;

    if (allowNullGlyph)
    {
        // do the null glyph first
        FontGlyphShape glyphShape = font.parsed.parsedGlyf.data[0];
        vec2i d = fontGetGlyphShapeBitmapDimensions(font, glyphShape, pixelSize);

        if (currX + d.w >= bitmapSize.w)
        {
            currY += maxGlyphHeightInRow;
            currX = 0;
            maxGlyphHeightInRow = 0;
        }

        currX += d.w;
        maxGlyphHeightInRow = BASE_MAX(maxGlyphHeightInRow, d.h);

        blittedNullGlyph = true;
    }

    for (u64 i = 0; i < ranges.len; i++)
    {
        rangei r = ranges.data[i];

        for (i64 ri = r.start; ri <= r.end; ri++)
        {
            FontGlyph glyph = fontGetGlyphFromCodepoint(font, ri);
            if (glyph.glyphIndex == 0 && (!allowNullGlyph || blittedNullGlyph))
            {
                continue;
            }

            vec2i d = fontGetGlyphShapeBitmapDimensions(font, glyph.shape, pixelSize);

            if (currX + d.w > bitmapSize.w)
            {
                currY += maxGlyphHeightInRow;
                currX = 0;
                maxGlyphHeightInRow = 0;
            }

            if (currY + d.height > bitmapSize.h)
            {
                break;
            }

            numGlyphs += 1;
            currX += d.w;
            maxGlyphHeightInRow = BASE_MAX(maxGlyphHeightInRow, d.h);
        }   
    }

    return numGlyphs;
}

void fontAtlasFillAtlasGlyphs(Font font, FontAtlas *atlas, RangeI64Array ranges, f64 pixelSize, bool allowNullGlyph)
{
    f64 scale = fontGetEmToPixelScale(font, pixelSize);

    u64 currX = 0;
    u64 currY = 0;
    u64 maxGlyphHeightInRow = 0;
    bool blittedNullGlyph = false;

    if (allowNullGlyph)
    {
        // do the null glyph first
        FontGlyphShape glyphShape = font.parsed.parsedGlyf.data[0];
        vec2i d = fontGetGlyphShapeBitmapDimensions(font, glyphShape, pixelSize);
        if (currX + d.w > atlas->bitmap.size.w)
        {
            currY += maxGlyphHeightInRow;
            currX = 0;
            maxGlyphHeightInRow = 0;
        }

        FontAtlasGlyph atlasGlyph = {0};
        atlasGlyph.codepoint = 0;
        atlasGlyph.pos = Vec2i(currX, currY);
        atlasGlyph.size = d;
        atlasGlyph.advanceWidth = (u64)ceil(scale * (f64)glyphShape.advanceWidth);
        atlasGlyph.bearingLeft = (i64)ceil(scale * (f64)glyphShape.leftSideBearing);
        atlasGlyph.bearingRight = (i64)ceil(scale* (f64)(glyphShape.advanceWidth - glyphShape.bounds.max.x));
        atlasGlyph.bearingTop = (i64)((i64)atlas->metrics.ascent - (i64)ceil(scale * ((f64)glyphShape.bounds.max.y)));
        atlasGlyph.isInvalidGlyph = true;

        atlas->glyphs.data[0] = atlasGlyph;

        currX += d.w;
        maxGlyphHeightInRow = BASE_MAX(maxGlyphHeightInRow, d.h);


        blittedNullGlyph = true;
    }

    for (u64 i = 0; i < ranges.len; i++)
    {
        rangei r = ranges.data[i];

        for (i64 ri = r.start; ri <= r.end; ri++)
        {
            FontGlyph glyph = fontGetGlyphFromCodepoint(font, ri);
            if (glyph.glyphIndex == 0 && (!allowNullGlyph || blittedNullGlyph))
            {
                continue;
            }

            vec2i d = fontGetGlyphShapeBitmapDimensions(font, glyph.shape, pixelSize);
            if (currX + d.w > atlas->bitmap.size.w)
            {
                currY += maxGlyphHeightInRow;
                currX = 0;
                maxGlyphHeightInRow = 0;
            }

            if (currY + d.height > atlas->bitmap.size.h)
            {
                break;
            }

            FontAtlasGlyph atlasGlyph = {0};
            atlasGlyph.codepoint = ri;
            atlasGlyph.pos = Vec2i(currX, currY);
            atlasGlyph.size = d;
            atlasGlyph.advanceWidth = (u64)ceil(scale * (f64)glyph.shape.advanceWidth);
            atlasGlyph.bearingLeft = (i64)ceil(scale * (f64)glyph.shape.leftSideBearing);
            atlasGlyph.bearingRight = (i64)ceil(scale* (f64)(glyph.shape.advanceWidth - glyph.shape.bounds.max.x));
            atlasGlyph.bearingTop = (i64)((i64)atlas->metrics.ascent - (i64)ceil(scale * ((f64)glyph.shape.bounds.max.y)));
            atlasGlyph.isInvalidGlyph = false;

            if (glyph.glyphIndex == 0)
            {
                atlasGlyph.isInvalidGlyph = true;
            }

            if (ri >= 0 && ri <= 127)
            {
                atlas->glyphs.data[ri] = atlasGlyph;
            }
            else
            {
                atlas->glyphs.data[atlas->glyphs.len] = atlasGlyph;
                atlas->glyphs.len++;
            }
            
            currX += d.w;
            maxGlyphHeightInRow = BASE_MAX(maxGlyphHeightInRow, d.h);
        }   
    }
}

FontAtlas fontAtlasFromCodepointRanges(Arena *arena, Font font, RangeI64Array ranges, f64 pixelSize, bool allowNullGlyph)
{
    FontAtlas atlas = {0};
    f64 scale = fontGetEmToPixelScale(font, pixelSize);

    atlas.metrics.ascent = (i16)ceil(scale * (f64)font.metrics.ascent);
    atlas.metrics.descent= (i16)ceil(scale * (f64)font.metrics.descent);
    atlas.metrics.lineGap = (i16)ceil(scale * (f64)font.metrics.lineGap);
    atlas.metrics.bounds = fontEmBoundsToPixelBounds(font, font.metrics.bounds, pixelSize);
    atlas.metrics.unitsPerEm = font.metrics.unitsPerEm;

    i64 bitmapW = 512;
    i64 bitmapH = 512;
    atlas.bitmap = bitmapPush(arena, Vec2i(bitmapW, bitmapH), BITMAP_FORMAT_R8_GRAYSCALE);

    u64 numGlyphs = fontAtlasEstimateNumberOfGlyphsThatFitInBitmap(font, atlas.bitmap.size, ranges, pixelSize, allowNullGlyph);
    u64 currX = 0;
    u64 currY = 0;
    u64 maxGlyphHeightInRow = 0;
    bool blittedNullGlyph = false;
    
    atlas.numFastAccessGlyphs = 128;
    atlas.glyphs.data = arenaPushArray(arena, FontAtlasGlyph, numGlyphs + atlas.numFastAccessGlyphs);
    atlas.glyphs.len = atlas.numFastAccessGlyphs;

    fontAtlasFillAtlasGlyphs(font, &atlas, ranges, pixelSize, allowNullGlyph);

    // qsort(atlas.glyphs.data + atlas.numFastAccessGlyphs, atlas.glyphs.len - atlas.numFastAccessGlyphs, sizeof(FontAtlasGlyph), fontAtlasGlyphDescendingCompare);

    for (u64 i = 0; i < atlas.glyphs.len; i++)
    {
        FontGlyph glyph = {0};
        if (i == 0 && allowNullGlyph)
        {
            glyph.codepoint = 0;
            glyph.glyphIndex = 0;
            glyph.shape = font.parsed.parsedGlyf.data[0];
        }
        else
        {
            glyph = fontGetGlyphFromCodepoint(font, atlas.glyphs.data[i].codepoint);

            if (glyph.codepoint == 0 || glyph.glyphIndex == 0)
            {
                continue;
            }
        }
        

        vec2i d = fontGetGlyphShapeBitmapDimensions(font, glyph.shape, pixelSize);
        Bitmap slice = atlas.bitmap;
        slice.size = d;

        if (currX + d.w > atlas.bitmap.size.w)
        {
            currY += maxGlyphHeightInRow;
            currX = 0;
            maxGlyphHeightInRow = 0;
        }

        if (currY + d.height > atlas.bitmap.size.h)
        {
            break;
        }

        slice.pixels += currY * atlas.bitmap.size.w * atlas.bitmap.bytesPerPixel + atlas.bitmap.bytesPerPixel * currX; 
        fontRasteriseGlyphShapeToBitmap(font, glyph.shape, &slice, atlas.bitmap.size.w, pixelSize, FONT_RASTERISE_ANTI_ALIASING_COVERAGE_ACCUMULATION);

        currX += d.w;
        maxGlyphHeightInRow = BASE_MAX(maxGlyphHeightInRow, d.h);
    }

    return atlas;
}
bool fontAtlasTryGetGlyphFromCodepoint(FontAtlas atlas, i32 codepoint, FontAtlasGlyph *out)
{
    *out = (FontAtlasGlyph){.isInvalidGlyph = true};

    if (codepoint >= 0 && codepoint <= atlas.numFastAccessGlyphs)
    {
        *out = atlas.glyphs.data[codepoint];
        if (out->codepoint != codepoint)
        {
            *out = atlas.glyphs.data[0];
        }

        return true;
    }

    FontAtlasGlyph *nullGlyph = null;
    for (u64 i = atlas.numFastAccessGlyphs; i < atlas.glyphs.len; i++)
    {
        if (atlas.glyphs.data[i].codepoint == codepoint)
        {
            *out = atlas.glyphs.data[i];
            return true;
        }

        if (atlas.glyphs.data[i].isInvalidGlyph)
        {
            nullGlyph = atlas.glyphs.data + i;
        }
    }

    if (nullGlyph != null)
    {
        *out = *nullGlyph;
    }
    else
    {
        *out = atlas.glyphs.data[0];
    }
    return false;
}
