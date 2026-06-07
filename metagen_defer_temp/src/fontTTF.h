#ifndef FONT_TTF_H
#define FONT_TTF_H

#include "base/baseCore.h"
#include "base/baseStrings.h"
#include "base/baseMemory.h"
#include "base/baseThreads.h"
#include "base/baseMath.h"

#include "bitmap/bitmapCore.h"

#include "fontCore.h"

bool fontTTFParseCmapSubtableFormat4(Arena *arena, FontTTFParsedFont *parsedFont, U8Array subtable);
bool fontTTFParseCmapSubtableFormat12(Arena *arena, FontTTFParsedFont *parsedFont, U8Array subtable);
bool fontTTFParseCmapTable(Arena *arena, FontTTFParsedFont *parsedFont);

void fontTTFParseCompositeGlyphNumContoursAndPoints(Arena *arena, FontTTFParsedFont *parsedFont, U8Array compositeData, u64 *numContours, u64 *numPoints);
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

#endif