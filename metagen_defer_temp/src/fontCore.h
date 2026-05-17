#ifndef FONT_CORE_H
#define FONT_CORE_H

#include "base/baseCore.h"
#include "base/baseStrings.h"
#include "base/baseMemory.h"
#include "base/baseThreads.h"
#include "base/baseMath.h"

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
    vec2i point;
    bool isControlPoint;
}FontGlyphShapePoint;

BASE_CREATE_ARRAY_VIEW_DECLS_DEFS(FontGlyphShapePointArray, FontGlyphShapePoint)

typedef struct FontGlyphShapeContour
{
    FontGlyphShapePointArray points;
}FontGlyphShapeContour;

BASE_CREATE_ARRAY_VIEW_DECLS_DEFS(FontGlyphShapeContourArray, FontGlyphShapeContour)

typedef struct FontGlyphShape
{
    FontGlyphShapePointArray points;
    FontGlyphShapeContourArray contours;

    vec2i min;
    vec2i max;
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

typedef struct FontTTFParsedCmap
{
    FontTTFParsedCmapFormat4 format4;
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
    u64 glyfOffset;
    u64 headOffset;
    u64 hheaOffset;
    u64 hmtxOffset;
    u64 locaOffset;
    u64 maxpOffset;
    FontParseErrorKind error;
}FontTTFParsedFont;

typedef struct Font
{
    FontTTFParsedFont parsed;
    FontParseErrorKind error;
}Font;

f32 F32FromFontF2Dot14(FontF2Dot14 val);

bool fontTTFParseCmapSubtableFormat4(Arena *arena, FontTTFParsedFont *parsedFont, U8Array subtable);
bool fontTTFParseCmapTable(Arena *arena, FontTTFParsedFont *parsedFont);

void fontTTFParseCompositeGlyphNumContoursAndPoints(FontTTFParsedFont *parsedFont, U8Array compositeData, u64 *numContours, u64 *numPoints);
void fontTTFParseGlyphShape(Arena *arena, FontTTFParsedFont *parsedFont, u32 glyphIndexToParse);
bool fontTTFParseGlyfTable(Arena *arena, FontTTFParsedFont *parsedFont);
bool fontTTFParseHeadTable(FontTTFParsedFont *parsedFont);
bool fontTTFParseHheaTable(FontTTFParsedFont *parsedFont);
bool fontTTFParseHmtxTable(FontTTFParsedFont *parsedFont);
bool fontTTFParseLocaTable(Arena *arena, FontTTFParsedFont *parsedFont);
bool fontTTFParseMaxpTable(FontTTFParsedFont *parsedFont);

FontParseErrorKind fontTTFValidateRequiredTablesExist(FontTTFParsedFont parsedFont);

Font fontTTFParseFromU8Array(Arena *arena, U8Array fontData);
Font fontTTFParseFromFile(Arena *arena, str8 file);

void fontPrintGlyphShape(FontGlyphShape shape);
u64 fontGetGlyphIndexFromCodepoint(Font font, u32 codepoint);

#endif