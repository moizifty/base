#ifndef FONT_CORE_H
#define FONT_CORE_H

#include "base/baseCore.h"
#include "base/baseStrings.h"
#include "base/baseMemory.h"
#include "base/baseThreads.h"
#include "base/baseMath.h"

#include "bitmap/bitmapCore.h"

#define ARG_1_AND_2_ARE_WORDS 0x0001
#define ARGS_ARE_XY_VALUES 0x0002
#define WE_HAVE_A_SCALE 0x0008
#define MORE_COMPONENTS 0x0020
#define WE_HAVE_AN_X_AND_Y_SCALE 0x0040
#define WE_HAVE_A_TWO_BY_TWO 0x0080
#define WE_HAVE_INSTRUCTIONS 0x0100

typedef i16 FontF2Dot14;

typedef struct FontGlyphShapePoint
{
    vec2f64 point;
    bool isControlPoint;
}FontGlyphShapePoint;

BASE_CREATE_ARRAY_VIEW_DECLS_DEFS(FontGlyphShapePointArray, FontGlyphShapePoint)

typedef struct FontGlyphShapeEdge
{
    vec2f64 start;
    vec2f64 end;

    i8 winding;
    vec2f64 intersection;
}FontGlyphShapeEdge;

BASE_CREATE_ARRAY_VIEW_DECLS_DEFS(FontGlyphShapeEdgeArray, FontGlyphShapeEdge)

typedef struct FontGlyphShapeContour
{
    FontGlyphShapePointArray points;
}FontGlyphShapeContour;

BASE_CREATE_ARRAY_VIEW_DECLS_DEFS(FontGlyphShapeContourArray, FontGlyphShapeContour)

typedef struct FontGlyphShape
{
    FontGlyphShapePointArray points;
    FontGlyphShapeContourArray contours;

    range2i bounds;
    u16 advanceWidth;
    i16 leftSideBearing;
}FontGlyphShape;
BASE_CREATE_ARRAY_VIEW_DECLS_DEFS(FontGlyphShapeArray, FontGlyphShape)

typedef struct FontGlyphOffset
{
    u32 offset;
    bool parsed;
}FontGlyphOffset;
BASE_CREATE_ARRAY_VIEW_DECLS_DEFS(FontGlyphOffsetArray, FontGlyphOffset)

typedef enum FontParseErrorKind
{
    FONT_ERROR_NONE = 0,
    FONT_ERROR_EMPTY_OR_FILE_DOESNT_EXIST,
    FONT_ERROR_UNSUPPORTED_CMAP,
    FONT_ERROR_UNSUPPORTED_HEAD,
    FONT_ERROR_UNSUPPORTED_CMAP_SUBTABLE_FORMAT,
    
    FONT_ERROR_MISSING_CMAP,
    FONT_ERROR_MISSING_GLYF,
    FONT_ERROR_MISSING_HEAD,
    FONT_ERROR_MISSING_HHEA,
    FONT_ERROR_MISSING_HMTX,
    FONT_ERROR_MISSING_LOCA,
    FONT_ERROR_MISSING_MAXP,
}FontParseErrorKind;

// references, read both, they both are utterly shit as each other
// but shit can sometimes compliment shit, they both provide information the other docs dont have
// microsoft one is a little better
// https://learn.microsoft.com/en-us/typography/opentype/spec/
// https://developer.apple.com/fonts/TrueType-Reference-Manual

// https://learn.microsoft.com/en-us/typography/opentype/spec/cmap#unicode-platform-platform-id--0
typedef enum FontTTFPlatformID
{
    FONT_TTF_PLATFORMID_UNICODE = 0,
}FontTTFPlatformID;
typedef enum FontTTFUnicodeEncodingKind
{
    FONT_TTF_UNICODE_ENCODING_INVALID = -1,
    FONT_TTF_UNICODE_ENCODING_BMP = 3,
    FONT_TTF_UNICODE_ENCODING_FULL = 4,
}FontTTFUnicodeEncodingKind;

typedef struct FontTTFParsedCmapFormat4
{
    u64 segCount;
    u16 *startCodes;
    u16 *endCodes;
    i16 *idDeltas;
    u16 *idRangeOffsets;

    u64 numGlyphIds;
    u16 *glphIds;
}FontTTFParsedCmapFormat4;

typedef struct FontTTFParsedCmapFormat12Group
{
    u32 startCharCode;
    u32 endCharCode;
    u32 startGlyphId;
}FontTTFParsedCmapFormat12Group;

BASE_CREATE_ARRAY_VIEW_DECLS_DEFS(FontTTFParsedCmapFormat12GroupArray, FontTTFParsedCmapFormat12Group)

typedef struct FontTTFParsedCmapFormat12
{
    u32 numGroups;
    FontTTFParsedCmapFormat12GroupArray groups;
}FontTTFParsedCmapFormat12;

typedef struct FontTTFParsedCmap
{
    bool isFormat12;
    union
    {
        FontTTFParsedCmapFormat4 format4;
        FontTTFParsedCmapFormat12 format12;
    };
}FontTTFParsedCmap;

typedef struct FontTTFParsedHead
{
    u16 unitsPerEm;
    vec2i min;
    vec2i max;
    i16 indexToLocFormat;
}FontTTFParsedHead;

typedef struct FontTTFParsedHhea
{
    i16 ascent;
    i16 descent;
    i16 lineGap;
    u16 numOfLongHorMetrics;
}FontTTFParsedHhea;

typedef struct FontTTFParsedFont
{
    U8Array fontData;
    FontTTFParsedCmap parsedCMAP;
    FontTTFParsedHead parsedHead;
    FontTTFParsedHhea parsedHhea;
    FontGlyphOffsetArray parsedLoca;
    FontGlyphShapeArray parsedGlyf;

    u32 maxGlyphs;
    u64 cmapOffset;
    u64 cmapSubtableOffset;
    u64 iroOffset;
    u64 glyfOffset;
    u64 headOffset;
    u64 hheaOffset;
    u64 hmtxOffset;
    u64 locaOffset;
    u64 maxpOffset;
    FontParseErrorKind error;
}FontTTFParsedFont;

typedef struct FontMetrics
{
    u16 unitsPerEm;
    range2i bounds;
    i16 ascent;
    i16 descent;
    i16 lineGap;
}FontMetrics;

typedef struct FontGlyph
{
    FontGlyphShape shape;

    u32 codepoint;
    u32 glyphIndex;
}FontGlyph;

BASE_CREATE_ARRAY_VIEW_DECLS_DEFS(FontGlyphArray, FontGlyph)

typedef struct Font
{
    FontTTFParsedFont parsed;
    FontParseErrorKind error;

    FontMetrics metrics;
}Font;

f32 F32FromFontF2Dot14(FontF2Dot14 val);

f64 fontGetEmToPixelScale(Font font, f64 pixelSize);
range2i fontEmBoundsToPixelBounds(Font font, range2i emBounds, f64 pixelSize);
vec2f64 fontNormaliseCoordsFromBounds(Font font, vec2f64 coords, range2i shapeBox, f64 pixelSize);
vec2i fontGetGlyphShapeBitmapDimensions(Font font, FontGlyphShape shape, f64 pixelSize);

// you need bitmap bitmapWidth as a param
// as you may pass bitmap "slices" into a bigger bitmap, but the stride for going to the next row should be based on the 
// the stride of the whole bigger bitmap
void fontRasteriseGlyphShapeToBitmap(Font font, FontGlyphShape shape, Bitmap *bitmap, u64 bitmapWidth, f64 pixelSize);
void fontRasteriseCodepointToBitmap(Font font, u32 codepoint, Bitmap *bitmap, u64 bitmapWidth, f64 pixelSize, bool allowNullGlyph);
void fontRasteriseEdgesToBitmap(Font font, FontGlyphShapeEdgeArray edges, range2i shapeBounds, Bitmap *bitmap, u64 bitmapWidth, f64 pixelSize);

void fontRasteriseNaiveAntiAliasedGlyphShapeToBitmap(Font font, FontGlyphShape shape, Bitmap *bitmap, u64 bitmapWidth, f64 pixelSize);
u64 fontGetGlyphIndexFromCodepoint(Font font, u32 codepoint);
FontGlyph fontGetGlyphFromCodepoint(Font font, u32 codepoint);

#endif