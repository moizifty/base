#include "base/base.h"
#include "os/core/osCore.h"

#include "base/base.c"
#include "os/core/osCore.c"

#include "os/core/osEntryPoint.c"

const u32 ARG_1_AND_2_ARE_WORDS = 0x0001;
const u32 ARGS_ARE_XY_VALUES = 0x0002;
const u32 WE_HAVE_A_SCALE = 0x0008;
const u32 MORE_COMPONENTS = 0x0020;
const u32 WE_HAVE_AN_X_AND_Y_SCALE = 0x0040;
const u32 WE_HAVE_A_TWO_BY_TWO = 0x0080;
const u32 WE_HAVE_INSTRUCTIONS = 0x0100;

typedef i16 F2Dot14;

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
    FONT_TTF_ERROR_NONE = 0,
    FONT_TTF_ERROR_UNSUPPORTED_CMAP,
    FONT_TTF_ERROR_UNSUPPORTED_HEAD,
    FONT_TTF_ERROR_UNSUPPORTED_CMAP_SUBTABLE_FORMAT,
    
    FONT_TTF_ERROR_MISSING_CMAP,
    FONT_TTF_ERROR_MISSING_GLYF,
    FONT_TTF_ERROR_MISSING_HEAD,
    FONT_TTF_ERROR_MISSING_HHEA,
    FONT_TTF_ERROR_MISSING_HMTX,
    FONT_TTF_ERROR_MISSING_LOCA,
    FONT_TTF_ERROR_MISSING_MAXP,
}FontParseErrorKind;

// references
// https://learn.microsoft.com/en-us/typography/opentype/spec/
// https://developer.apple.com/fonts/TrueType-Reference-Manual/RM06/Chap6.html

typedef enum FontTTFPlatformID
{
    FONT_TTF_PLATFORMID_UNICODE = 0,
}FontTTFPlatformID;

// https://learn.microsoft.com/en-us/typography/opentype/spec/cmap#unicode-platform-platform-id--0
typedef enum FontTTFUnicodeEncodingKind
{
    FONT_TTF_UNICODE_ENCODING_INVALID = -1,
    FONT_TTF_UNICODE_ENCODING_BMP = 3,
    FONT_TTF_UNICODE_ENCODING_FULL = 4,
}FontTTFUnicodeEncodingKind;

typedef struct FontTTFParsedCMAP
{
    FontParseErrorKind error;
}FontTTFParsedCMAP;

typedef struct FontTTFParsedHead
{
    u16 unitsPerEm;
    i16 xMin;
    i16 yMin;
    i16 xMax;
    i16 yMax;
    i16 indexToLocFormat;
    FontParseErrorKind error;
}FontTTFParsedHead;

typedef struct FontTTFParsedHhea
{
    i16 ascent;
    i16 descent;
    i16 lineGap;
    u16 numOfLongHorMetrics;
    FontParseErrorKind error;
}FontTTFParsedHhea;

typedef struct FontTTFParsedFont
{
    FontTTFParsedCMAP parsedCMAP;
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


FontGlyphShape ParseGlyphShape(Arena *arena, U8Array glyfTable, FontGlyphShapeArray glyphShapes, FontGlyphOffsetArray glyphOffsets, u32 glyphIndexToParse);

f32 F32FromF2Dot14(F2Dot14 val)
{
    return val / 16384.0f;
}

void ParseCMAPSubtableFormat4(Arena *arena, U8Array subtable)
{
    ArenaTemp temp = baseTempBegin(&arena, 1);

    U8Array originalSubtable = subtable;

    // skip the format, it should always be 4
    subtable = U8ArraySkip(subtable, sizeof(u16));

    u16 tableLen = U8ArrayReadBigEndianU16(subtable);
    subtable = U8ArraySkip(subtable, sizeof(u16));
    // skip lang field
    subtable = U8ArraySkip(subtable, sizeof(u16));

    // segCount x 2, god knows why its stored double
    u16 segCountx2 = U8ArrayReadBigEndianU16(subtable);
    u16 segCount = segCountx2 / 2;
    subtable = U8ArraySkip(subtable, sizeof(u16));

    // skip the search helpers
    subtable = U8ArraySkip(subtable, sizeof(u16));
    subtable = U8ArraySkip(subtable, sizeof(u16));
    subtable = U8ArraySkip(subtable, sizeof(u16));

    u16 *startCodes = arenaPushArray(temp.arena, u16, segCount);
    u16 *endCodes = arenaPushArray(temp.arena, u16, segCount);
    i16 *idDeltas = arenaPushArray(temp.arena, i16, segCount);
    u16 *idRangeOffsets = arenaPushArray(temp.arena, u16, segCount);
    for (u16 i = 0; i < segCount; i++)
    {
        endCodes[i] = U8ArrayReadBigEndianU16(subtable);
        subtable = U8ArraySkip(subtable, sizeof(u16));
    }
    // skip padding
    subtable = U8ArraySkip(subtable, sizeof(u16));

    for (u16 i = 0; i < segCount; i++)
    {
        startCodes[i] = U8ArrayReadBigEndianU16(subtable);
        subtable = U8ArraySkip(subtable, sizeof(u16));
    }
    for (u16 i = 0; i < segCount; i++)
    {
        idDeltas[i] = U8ArrayReadBigEndianI16(subtable);
        subtable = U8ArraySkip(subtable, sizeof(i16));
    }
    for (u16 i = 0; i < segCount; i++)
    {
        idRangeOffsets[i] = U8ArrayReadBigEndianU16(subtable);
        subtable = U8ArraySkip(subtable, sizeof(u16));
    }

    u64 glphIdArrayLen = tableLen - (originalSubtable.len - subtable.len);

    u16 *glphIds = arenaPushArray(temp.arena, u16, glphIdArrayLen);
    for(u64 i = 0; i < glphIdArrayLen; i++)
    {
        glphIds[i] = U8ArrayReadBigEndianU16(subtable);
        subtable = U8ArraySkip(subtable, sizeof(u16));
    }

    baseTempEnd(temp);
}
FontTTFParsedCMAP ParseCMAP(Arena *arena, U8Array cmapTable)
{
    FontTTFParsedCMAP parsed = {0};
    U8Array originalCmapTable = cmapTable;

    // skip version
    cmapTable = U8ArraySkip(cmapTable, sizeof(u16));

    u16 numberSubTables = U8ArrayReadBigEndianU16(cmapTable);
    cmapTable = U8ArraySkip(cmapTable, sizeof(u16));

    FontTTFUnicodeEncodingKind chosenEncoding = FONT_TTF_UNICODE_ENCODING_INVALID;
    u32 chosenSubtableOffset = 0;

    for (u16 i = 0; i < numberSubTables; i++)
    {
        u16 platformId = U8ArrayReadBigEndianU16(cmapTable);
        cmapTable = U8ArraySkip(cmapTable, sizeof(u16));

        u16 encodingId = U8ArrayReadBigEndianU16(cmapTable);
        cmapTable = U8ArraySkip(cmapTable, sizeof(u16));

        u32 offset = U8ArrayReadBigEndianU32(cmapTable);
        cmapTable = U8ArraySkip(cmapTable, sizeof(u32));

        if (platformId == FONT_TTF_PLATFORMID_UNICODE)
        {
            if (encodingId == FONT_TTF_UNICODE_ENCODING_BMP ||
                encodingId == FONT_TTF_UNICODE_ENCODING_FULL)
            {
                // prefer FULL encoding over BMP
                if (chosenEncoding == FONT_TTF_UNICODE_ENCODING_INVALID || 
                    chosenEncoding == FONT_TTF_UNICODE_ENCODING_BMP)
                {
                    chosenEncoding = encodingId;
                    chosenSubtableOffset = offset;
                }
            }
        }
    }

    if (chosenEncoding != FONT_TTF_UNICODE_ENCODING_INVALID)
    {
        U8Array subtable = U8ArraySkip(originalCmapTable, chosenSubtableOffset);
        u16 format = U8ArrayReadBigEndianU16(subtable);
        // dont skip the 2 format bytes, you just want to peak the format
        
        switch (chosenEncoding)
        {
            case FONT_TTF_UNICODE_ENCODING_BMP:
            {
                if (format == 4)
                {
                    ParseCMAPSubtableFormat4(arena, subtable);
                }
                else
                {
                    parsed.error = FONT_TTF_ERROR_UNSUPPORTED_CMAP_SUBTABLE_FORMAT;
                }
            }break;
            
            case FONT_TTF_UNICODE_ENCODING_FULL:
            {

            }break;

            default:
            {
                baseEPrintf("{r}Unhandled cmap unicode encoding, when parsing ttf file.\n");
                parsed.error = FONT_TTF_ERROR_UNSUPPORTED_CMAP;
            }break;
        }
    }
    else
    {
        parsed.error = FONT_TTF_ERROR_UNSUPPORTED_CMAP;
    }
    
    return parsed;
}

void ParseCompositeGlyphNumContoursAndPoints(U8Array originalGlyfTable, U8Array glyfTable, FontGlyphShapeArray glyphShapes, FontGlyphOffsetArray glyphOffsets, u64 *numContours, u64 *numPoints)
{
    *numContours = 0;
    *numPoints = 0;

    u16 flags = 0;

    ArenaTemp temp = baseTempBegin(null, 0);

    do
    {
        flags = U8ArrayReadBigEndianU16(glyfTable);
        glyfTable = U8ArraySkip(glyfTable, sizeof(u16));

        u16 glyphIndex = U8ArrayReadBigEndianU16(glyfTable);
        glyfTable = U8ArraySkip(glyfTable, sizeof(u16));

        FontGlyphShape componentShape = ParseGlyphShape(temp.arena, originalGlyfTable, glyphShapes, glyphOffsets, glyphIndex);

        *numContours += componentShape.contours.len;
        *numPoints += componentShape.points.len;

        if ((flags & ARG_1_AND_2_ARE_WORDS) && (flags & ARGS_ARE_XY_VALUES))
        {
            glyfTable = U8ArraySkip(glyfTable, sizeof(i16));
            glyfTable = U8ArraySkip(glyfTable, sizeof(i16));
        }
        else if (!(flags & ARG_1_AND_2_ARE_WORDS) && (flags & ARGS_ARE_XY_VALUES))
        {
            glyfTable = U8ArraySkip(glyfTable, sizeof(i8));
            glyfTable = U8ArraySkip(glyfTable, sizeof(i8));
        }
        else if ((flags & ARG_1_AND_2_ARE_WORDS) && !(flags & ARGS_ARE_XY_VALUES))
        {
            glyfTable = U8ArraySkip(glyfTable, sizeof(u16));
            glyfTable = U8ArraySkip(glyfTable, sizeof(u16));
        }
        else if (!(flags & ARG_1_AND_2_ARE_WORDS) && !(flags & ARGS_ARE_XY_VALUES))
        {
            glyfTable = U8ArraySkip(glyfTable, sizeof(u8));
            glyfTable = U8ArraySkip(glyfTable, sizeof(u8));
        }

        if (flags & WE_HAVE_A_SCALE)
        {
            glyfTable = U8ArraySkip(glyfTable, sizeof(F2Dot14));
        }
        else if (flags & WE_HAVE_AN_X_AND_Y_SCALE)
        {
            glyfTable = U8ArraySkip(glyfTable, sizeof(F2Dot14));
            glyfTable = U8ArraySkip(glyfTable, sizeof(F2Dot14));
        }
        else if (flags & WE_HAVE_A_TWO_BY_TWO)
        {
            glyfTable = U8ArraySkip(glyfTable, sizeof(F2Dot14));
            glyfTable = U8ArraySkip(glyfTable, sizeof(F2Dot14));
            glyfTable = U8ArraySkip(glyfTable, sizeof(F2Dot14));
            glyfTable = U8ArraySkip(glyfTable, sizeof(F2Dot14));
        }
    }while(flags & MORE_COMPONENTS);

    if (flags & WE_HAVE_INSTRUCTIONS)
    {
        u16 instLen = U8ArrayReadBigEndianU16(glyfTable);
        glyfTable = U8ArraySkip(glyfTable, sizeof(u16));
        glyfTable = U8ArraySkip(glyfTable, sizeof(u8) * instLen);
    }

    baseTempEnd(temp);
}
FontGlyphShape ParseGlyphShape(Arena *arena, U8Array glyfTable, FontGlyphShapeArray glyphShapes, FontGlyphOffsetArray glyphOffsets, u32 glyphIndexToParse)
{

    U8Array originalGlyphTable = glyfTable;

    FontGlyphOffset *glyphOffset = glyphOffsets.data + glyphIndexToParse;
    if (glyphOffset->parsed)
    {
        return glyphShapes.data[glyphIndexToParse];
    }

    FontGlyphShape shape = {0};
    FontGlyphOffset nextGlyphOffset = glyphOffsets.data[glyphIndexToParse + 1];

    u8 *glyphDataStart = U8ArraySkip(glyfTable, glyphOffset->offset).data;
    u8 *glyphDataEnd = U8ArraySkip(glyfTable, nextGlyphOffset.offset).data;

    if (glyphDataStart == glyphDataEnd) // 0 contours
    {
        glyphOffset->parsed = true;
        return shape;
    }

    ArenaTemp temp = baseTempBegin(&arena, 1);

    glyfTable = U8ArraySkip(glyfTable, glyphOffset->offset);

    i16 numContours = U8ArrayReadBigEndianI16(glyfTable);
    glyfTable = U8ArraySkip(glyfTable, sizeof(i16));

    i16 minX = U8ArrayReadBigEndianI16(glyfTable);
    glyfTable = U8ArraySkip(glyfTable, sizeof(i16));

    i16 minY = U8ArrayReadBigEndianI16(glyfTable);
    glyfTable = U8ArraySkip(glyfTable, sizeof(i16));

    i16 maxX = U8ArrayReadBigEndianI16(glyfTable);
    glyfTable = U8ArraySkip(glyfTable, sizeof(i16));

    i16 maxY = U8ArrayReadBigEndianI16(glyfTable);
    glyfTable = U8ArraySkip(glyfTable, sizeof(i16));

    shape.min = Vec2i(minX, minY);
    shape.max = Vec2i(maxX, maxY);

    if (numContours > 0)
    {
        u16 *endPointsOfContour = arenaPushArray(temp.arena, u16, numContours);
        for (u16 c = 0; c < numContours; c++)
        {
            endPointsOfContour[c] = U8ArrayReadBigEndianU16(glyfTable);
            glyfTable = U8ArraySkip(glyfTable, sizeof(u16));
        }

        u16 instLength = U8ArrayReadBigEndianU16(glyfTable);
        glyfTable = U8ArraySkip(glyfTable, sizeof(u16));
        glyfTable = U8ArraySkip(glyfTable, sizeof(u8) * instLength);
        
        u32 numPoints = endPointsOfContour[numContours - 1] + 1;
        
        u8 *flags = arenaPushArray(temp.arena, u8, numPoints);
        i64 *xCoords = arenaPushArray(temp.arena, i64, numPoints);
        i64 *yCoords = arenaPushArray(temp.arena, i64, numPoints);

        shape.points.data = arenaPushArray(arena, FontGlyphShapePoint, numPoints);
        shape.points.len = numPoints;

        u32 f = 0;
        while (f < numPoints)
        {
            flags[f] = glyfTable.data[0];
            glyfTable = U8ArraySkip(glyfTable, sizeof (u8));

            if (flags[f] & 0x08)
            {
                u8 flagToRepeat = flags[f];
                u8 repeatCount = glyfTable.data[0];
                glyfTable = U8ArraySkip(glyfTable, sizeof (u8));

                f += 1;
                while(repeatCount--)
                {
                    flags[f++] = flagToRepeat;
                }
            }
            else
            {
                f++;
            }
        }

        vec2i accumPoint = Vec2i(0, 0);
        u32 accumX = 0;
        for (u32 iX = 0; iX < numPoints; iX++)
        {
            shape.points.data[iX].isControlPoint = flags[iX] & 0x01;

            if (flags[iX] & 0x02)
            {
                shape.points.data[iX].point.x = (i64)glyfTable.data[0];
                glyfTable = U8ArraySkip(glyfTable, sizeof(u8));
                
                if (!(flags[iX] & 0x10))
                {
                    shape.points.data[iX].point.x *= -1;
                }

                accumPoint.x += shape.points.data[iX].point.x;
                shape.points.data[iX].point.x = accumPoint.x;
            }
            else if (flags[iX] & 0x10)
            {
                shape.points.data[iX].point.x = accumPoint.x;
            }
            else if (!(flags[iX] & 0x02) && !(flags[iX] & 0x10))
            {
                shape.points.data[iX].point.x = U8ArrayReadBigEndianI16(glyfTable);
                glyfTable = U8ArraySkip(glyfTable, sizeof(i16));

                accumPoint.x += shape.points.data[iX].point.x;
                shape.points.data[iX].point.x = accumPoint.x;
            }
        }

        accumPoint = Vec2i(0, 0);
        for (u32 iY = 0; iY < numPoints; iY++)
        {
            if (flags[iY] & 0x04)
            {
                shape.points.data[iY].point.y = (i64)glyfTable.data[0];
                glyfTable = U8ArraySkip(glyfTable, sizeof(u8));
                
                if (!(flags[iY] & 0x20))
                {
                    shape.points.data[iY].point.y *= -1;
                }

                accumPoint.y += shape.points.data[iY].point.y;
                shape.points.data[iY].point.y = accumPoint.y;
            }
            else if (flags[iY] & 0x20)
            {
                shape.points.data[iY].point.y = accumPoint.y;
            }
            else if (!(flags[iY] & 0x04) && !(flags[iY] & 0x20))
            {
                shape.points.data[iY].point.y = U8ArrayReadBigEndianI16(glyfTable);
                glyfTable = U8ArraySkip(glyfTable, sizeof(i16));

                accumPoint.y += shape.points.data[iY].point.y;
                shape.points.data[iY].point.y = accumPoint.y;
            }
        }

        shape.contours.data = arenaPushArray(arena, FontGlyphShapeContour, numContours);
        shape.contours.len = numContours;

        i32 prevEndIndex = -1;
        for (u64 c = 0; c < numContours; c++)
        {
            u16 endIndex = endPointsOfContour[c];

            FontGlyphShapePointArray pointsForContour = 
            {
                .data = shape.points.data + (prevEndIndex + 1),
                .len = endIndex - prevEndIndex,
            };

            shape.contours.data[c].points = pointsForContour;

            prevEndIndex = endIndex;
        }
    }
    else if (numContours < 0)
    {
        u64 numTotalContours = 0;
        u64 numTotalPoints = 0;

        ParseCompositeGlyphNumContoursAndPoints(originalGlyphTable, glyfTable, glyphShapes, glyphOffsets, &numTotalContours, &numTotalPoints);

        shape.points.data = arenaPushArray(arena, FontGlyphShapePoint, numTotalPoints);
        shape.points.len = 0;

        shape.contours.data = arenaPushArray(arena, FontGlyphShapeContour, numTotalContours);
        shape.contours.len = 0;

        u16 flags = 0;

        u64 currContourTotalPointsCount = 0;
        i32 prevEndIndex = -1;
        do
        {
            flags = U8ArrayReadBigEndianU16(glyfTable);
            glyfTable = U8ArraySkip(glyfTable, sizeof(u16));

            u16 glyphIndex = U8ArrayReadBigEndianU16(glyfTable);
            glyfTable = U8ArraySkip(glyfTable, sizeof(u16));

            // TODO THIS WILL DOUBLE ALLOCATE THE COTNOURS AND POINTS 
            // so skip parsing simple shapes if its already been parsed
            FontGlyphShape componentShape = ParseGlyphShape(arena, originalGlyphTable, glyphShapes, glyphOffsets, glyphIndex);

            i32 arg1 = 0;
            i32 arg2 = 0;

            f32 scaleX = 1;
            f32 scaleY = 1;

            f32 sheerX = 0;
            f32 sheerY = 0;
            if ((flags & ARG_1_AND_2_ARE_WORDS) && (flags & ARGS_ARE_XY_VALUES))
            {
                arg1 = (i32)U8ArrayReadBigEndianI16(glyfTable);
                glyfTable = U8ArraySkip(glyfTable, sizeof(i16));

                arg2 = (i32)U8ArrayReadBigEndianI16(glyfTable);
                glyfTable = U8ArraySkip(glyfTable, sizeof(i16));
            }
            else if (!(flags & ARG_1_AND_2_ARE_WORDS) && (flags & ARGS_ARE_XY_VALUES))
            {
                arg1 = (i32)glyfTable.data[0];
                glyfTable = U8ArraySkip(glyfTable, sizeof(i8));

                arg2 = (i32)glyfTable.data[0];
                glyfTable = U8ArraySkip(glyfTable, sizeof(i8));
            }
            else if ((flags & ARG_1_AND_2_ARE_WORDS) && !(flags & ARGS_ARE_XY_VALUES))
            {
                arg1 = (i32)U8ArrayReadBigEndianU16(glyfTable);
                glyfTable = U8ArraySkip(glyfTable, sizeof(u16));

                arg2 = (i32)U8ArrayReadBigEndianU16(glyfTable);
                glyfTable = U8ArraySkip(glyfTable, sizeof(u16));

                basePrintf("{r}Dont handle yet\n");
            }
            else if (!(flags & ARG_1_AND_2_ARE_WORDS) && !(flags & ARGS_ARE_XY_VALUES))
            {
                arg1 = (i32)(u8)glyfTable.data[0];
                glyfTable = U8ArraySkip(glyfTable, sizeof(u8));

                arg2 = (i32)(u8)glyfTable.data[0];
                glyfTable = U8ArraySkip(glyfTable, sizeof(u8));

                basePrintf("{r}Dont handle yet\n");
            }

            if (flags & WE_HAVE_A_SCALE)
            {
                F2Dot14 scaleRaw = U8ArrayReadBigEndianI16(glyfTable);
                glyfTable = U8ArraySkip(glyfTable, sizeof(F2Dot14));

                scaleX = scaleY = F32FromF2Dot14(scaleRaw);
            }
            else if (flags & WE_HAVE_AN_X_AND_Y_SCALE)
            {
                F2Dot14 xScaleRaw = U8ArrayReadBigEndianI16(glyfTable);
                glyfTable = U8ArraySkip(glyfTable, sizeof(F2Dot14));

                F2Dot14 yScaleRaw = U8ArrayReadBigEndianI16(glyfTable);
                glyfTable = U8ArraySkip(glyfTable, sizeof(F2Dot14));

                scaleX = F32FromF2Dot14(xScaleRaw);
                scaleY = F32FromF2Dot14(yScaleRaw);
            }
            else if (flags & WE_HAVE_A_TWO_BY_TWO)
            {
                F2Dot14 xScaleRaw = U8ArrayReadBigEndianI16(glyfTable);
                glyfTable = U8ArraySkip(glyfTable, sizeof(F2Dot14));

                F2Dot14 scale01Raw = U8ArrayReadBigEndianI16(glyfTable);
                glyfTable = U8ArraySkip(glyfTable, sizeof(F2Dot14));

                F2Dot14 scale10Raw = U8ArrayReadBigEndianI16(glyfTable);
                glyfTable = U8ArraySkip(glyfTable, sizeof(F2Dot14));

                F2Dot14 yScaleRaw = U8ArrayReadBigEndianI16(glyfTable);
                glyfTable = U8ArraySkip(glyfTable, sizeof(F2Dot14));

                scaleX = F32FromF2Dot14(xScaleRaw);
                scaleY = F32FromF2Dot14(yScaleRaw);
                
                sheerX = F32FromF2Dot14(scale01Raw);
                sheerY = F32FromF2Dot14(scale10Raw);
            }

            for (u64 i = 0; i < componentShape.points.len; i++)
            {
                i64 newX = 0;
                i64 newY = 0;

                newX = componentShape.points.data[i].point.x * (i64)scaleX + (i64)sheerY * componentShape.points.data[i].point.y + arg1;
                newY = componentShape.points.data[i].point.x * (i64)sheerX + (i64)scaleY * componentShape.points.data[i].point.y + arg2;
                shape.points.data[shape.points.len].point.x = newX;
                shape.points.data[shape.points.len].point.y = newY;

                shape.points.len++;
            }

            for (u64 i = 0; i < componentShape.contours.len; i++)
            {
                u16 endIndex = (componentShape.contours.data[i].points.len - 1) + currContourTotalPointsCount;

                FontGlyphShapePointArray pointsForContour = 
                {
                    .data = shape.points.data + (prevEndIndex + 1),
                    .len = endIndex - prevEndIndex,
                };

                shape.contours.data[shape.contours.len].points = pointsForContour;
                
                shape.contours.len++;
                currContourTotalPointsCount += componentShape.contours.data[i].points.len;
                prevEndIndex = endIndex;
            }

        }while(flags & MORE_COMPONENTS);

        if (flags & WE_HAVE_INSTRUCTIONS)
        {
            u16 instLen = U8ArrayReadBigEndianU16(glyfTable);
            glyfTable = U8ArraySkip(glyfTable, sizeof(u16));
            glyfTable = U8ArraySkip(glyfTable, sizeof(u8) * instLen);
        }
    }

    baseTempEnd(temp);

    glyphOffset->parsed = true;
    return shape;
}
FontGlyphShapeArray ParseGLYF(Arena *arena, U8Array glyfTable, u32 maxGlyphs, FontGlyphOffsetArray glyphOffsets)
{
    U8Array originalGlyphTable = glyfTable;
    FontGlyphShapeArray shapes = {.data = arenaPushArray(arena, FontGlyphShape, maxGlyphs), .len = maxGlyphs};

    for (u32 i = 0; i < maxGlyphs; i++)
    {
        FontGlyphShape *shape = shapes.data + i;

        *shape = ParseGlyphShape(arena, originalGlyphTable, shapes, glyphOffsets, i);
    }

    return shapes;
}
FontTTFParsedHead ParseHEAD(U8Array headTable)
{
    FontTTFParsedHead parsed = {0};

    headTable = U8ArraySkip(headTable, sizeof(u16));
    headTable = U8ArraySkip(headTable, sizeof(u16));
    headTable = U8ArraySkip(headTable, sizeof(u32));
    headTable = U8ArraySkip(headTable, sizeof(u32));

    u32 magicNumber = U8ArrayReadBigEndianU32(headTable);
    headTable = U8ArraySkip(headTable, sizeof(u32));

    if (magicNumber != 0x5F0F3CF5)
    {
        parsed.error = FONT_TTF_ERROR_UNSUPPORTED_HEAD;
        return parsed;
    }

    headTable = U8ArraySkip(headTable, sizeof(u16));

    parsed.unitsPerEm = U8ArrayReadBigEndianU16(headTable);
    headTable = U8ArraySkip(headTable, sizeof(u16));
    
    headTable = U8ArraySkip(headTable, sizeof(i64));
    headTable = U8ArraySkip(headTable, sizeof(i64));

    parsed.xMin = U8ArrayReadBigEndianI16(headTable);
    headTable = U8ArraySkip(headTable, sizeof(i16));
    parsed.yMin = U8ArrayReadBigEndianI16(headTable);
    headTable = U8ArraySkip(headTable, sizeof(i16));
    parsed.xMax = U8ArrayReadBigEndianI16(headTable);
    headTable = U8ArraySkip(headTable, sizeof(i16));
    parsed.yMax = U8ArrayReadBigEndianI16(headTable);
    headTable = U8ArraySkip(headTable, sizeof(i16));

    headTable = U8ArraySkip(headTable, sizeof(u16));
    headTable = U8ArraySkip(headTable, sizeof(u16));
    headTable = U8ArraySkip(headTable, sizeof(i16));

    parsed.indexToLocFormat = U8ArrayReadBigEndianI16(headTable);
    headTable = U8ArraySkip(headTable, sizeof(i16));
    headTable = U8ArraySkip(headTable, sizeof(i16));

    return parsed;
}
FontTTFParsedHhea ParseHHEA(U8Array hheaTable)
{
    FontTTFParsedHhea parsed = {0};

    hheaTable = U8ArraySkip(hheaTable, sizeof(u32));

    parsed.ascent = U8ArrayReadBigEndianI16(hheaTable);
    hheaTable = U8ArraySkip(hheaTable, sizeof(i16));

    parsed.descent = U8ArrayReadBigEndianI16(hheaTable);
    hheaTable = U8ArraySkip(hheaTable, sizeof(i16));

    parsed.lineGap = U8ArrayReadBigEndianI16(hheaTable);
    hheaTable = U8ArraySkip(hheaTable, sizeof(i16));

    hheaTable = U8ArraySkip(hheaTable, sizeof(u16));

    hheaTable = U8ArraySkip(hheaTable, sizeof(i16));
    hheaTable = U8ArraySkip(hheaTable, sizeof(i16));
    hheaTable = U8ArraySkip(hheaTable, sizeof(i16));

    hheaTable = U8ArraySkip(hheaTable, sizeof(i16));
    hheaTable = U8ArraySkip(hheaTable, sizeof(i16));
    hheaTable = U8ArraySkip(hheaTable, sizeof(i16));
    hheaTable = U8ArraySkip(hheaTable, sizeof(i16));
    hheaTable = U8ArraySkip(hheaTable, sizeof(i16));
    hheaTable = U8ArraySkip(hheaTable, sizeof(i16));
    hheaTable = U8ArraySkip(hheaTable, sizeof(i16));
    hheaTable = U8ArraySkip(hheaTable, sizeof(i16));

    parsed.numOfLongHorMetrics = U8ArrayReadBigEndianU16(hheaTable);
    hheaTable = U8ArraySkip(hheaTable, sizeof(u16));

    return parsed;
}

void ParseHMTX(U8Array hmtxTable, FontGlyphShapeArray glyphs, u16 numEntries)
{
    u64 numLeftSideBearingEntries = glyphs.len - numEntries;

    for (u64 i = 0; i < numEntries; i++)
    {
        glyphs.data[i].advanceWidth = U8ArrayReadBigEndianU16(hmtxTable);
        hmtxTable = U8ArraySkip(hmtxTable, sizeof(u16));

        glyphs.data[i].leftSideBearing = U8ArrayReadBigEndianI16(hmtxTable);
        hmtxTable = U8ArraySkip(hmtxTable, sizeof(i16));
    }

    for (u64 i = 0; i < numLeftSideBearingEntries; i++)
    {
        glyphs.data[i + numEntries].advanceWidth = glyphs.data[numEntries - 1].advanceWidth;
        glyphs.data[i + numEntries].leftSideBearing = U8ArrayReadBigEndianI16(hmtxTable);
        hmtxTable = U8ArraySkip(hmtxTable, sizeof(i16));
    }
}
FontGlyphOffsetArray ParseLOCA(Arena *arena, U8Array locaTable, u16 numGlyphs, bool offset32)
{
    FontGlyphOffsetArray ret = {0};
    ret.data = arenaPushArray(arena, FontGlyphOffset, numGlyphs + 1);
    ret.len = numGlyphs + 1;

    u32 offsetStride = offset32 ? sizeof(u32) : sizeof(u16);

    for (u16 i = 0; i < numGlyphs + 1; i++)
    {
        ret.data[i].offset = offset32 ? U8ArrayReadBigEndianU32(locaTable) : U8ArrayReadBigEndianU16(locaTable) * 2;
        ret.data[i].parsed = false;

        locaTable = U8ArraySkip(locaTable, offsetStride);
    }

    return ret;
}
u16 ParseMAXP(U8Array maxpTable)
{
    maxpTable = U8ArraySkip(maxpTable, sizeof(u32));

    return U8ArrayReadBigEndianU16(maxpTable);
}

FontParseErrorKind ParseFontValidateRequiredTablesExist(FontTTFParsedFont parsedFont)
{
    if (parsedFont.cmapOffset == 0)
    {
        return FONT_TTF_ERROR_MISSING_CMAP;
    }

    if (parsedFont.glyfOffset == 0)
    {
        return FONT_TTF_ERROR_MISSING_GLYF;
    }

    if (parsedFont.headOffset == 0)
    {
        return FONT_TTF_ERROR_MISSING_HEAD;
    }

    if (parsedFont.hheaOffset == 0)
    {
        return FONT_TTF_ERROR_MISSING_HHEA;
    }
    
    if (parsedFont.hmtxOffset == 0)
    {
        return FONT_TTF_ERROR_MISSING_HMTX;
    }

    if (parsedFont.locaOffset == 0)
    {
        return FONT_TTF_ERROR_MISSING_LOCA;
    }

    if (parsedFont.maxpOffset == 0)
    {
        return FONT_TTF_ERROR_MISSING_MAXP;
    }

    return FONT_TTF_ERROR_NONE;
}

void printGlyphShape(FontGlyphShape shape)
{
    u32 termWidth = 80;
    u32 termHeight = 36;

    basePrintf("\033[H\033[2J");
    basePrintf("\033[%d;%dH", 5, 5);

    for (u64 i = 0; i < shape.points.len; i++)
    {
        vec2i point = shape.points.data[i].point;
        u32 normX = (u32)(((point.x - shape.min.x) / (f32)(shape.max.x - shape.min.x)) * (f32)(termWidth - 1));
        u32 normY = (u32)(((point.y - shape.min.y) / (f32)(shape.max.y - shape.min.y)) * (f32)(termHeight - 1));
        normY = (termHeight - 1) - normY;

        basePrintf("\033[%d;%dH", normY + 1, normX + 1);
        basePrintf("#");
    }
}
FontTTFParsedFont ParseFont(Arena *arena, U8Array fontData)
{
    U8Array originalFontData = fontData;

    // skip scalar type
    fontData = U8ArraySkip(fontData, sizeof(u32));

    u16 numTables = U8ArrayReadBigEndianU16(fontData);
    fontData = U8ArraySkip(fontData, sizeof(u16));

    // skip rest of table
    fontData = U8ArraySkip(fontData, sizeof(u16)); 
    fontData = U8ArraySkip(fontData, sizeof(u16)); 
    fontData = U8ArraySkip(fontData, sizeof(u16)); 

    FontTTFParsedFont parsedFont = {0};
    for(u16 i = 0; i < numTables; i++)
    {
        u32 tag = U8ArrayReadBigEndianU32(fontData);
        basePrintf("%c%c%c%c\n", fontData.data[0], fontData.data[1], fontData.data[2], fontData.data[3]);

        fontData = U8ArraySkip(fontData, sizeof(u32));
        fontData = U8ArraySkip(fontData, sizeof(u32));

        u32 offset = U8ArrayReadBigEndianU32(fontData);
        fontData = U8ArraySkip(fontData, sizeof(u32));
        fontData = U8ArraySkip(fontData, sizeof(u32));

        // some tables requre other tables to be parsed first, hence why u can parse some here in place
        // and others after this loop
        switch (tag)
        {
            case (u32)'cmap':
            {
                parsedFont.cmapOffset = offset;
                parsedFont.parsedCMAP = ParseCMAP(arena, U8ArraySkip(originalFontData, offset));
            }break;
            
            case (u32)'glyf':
            {
                parsedFont.glyfOffset = offset;
            }break;

            case (u32)'head':
            {
                parsedFont.headOffset = offset;
                parsedFont.parsedHead = ParseHEAD(U8ArraySkip(originalFontData, offset));
            }break;

            case (u32)'hhea':
            {
                parsedFont.hheaOffset = offset;
                parsedFont.parsedHhea = ParseHHEA(U8ArraySkip(originalFontData, offset));
            }break;

            case (u32)'hmtx':
            {
                parsedFont.hmtxOffset = offset;
            }break;

            case (u32)'loca':
            {
                parsedFont.locaOffset = offset;
            }

            case (u32)'maxp':
            {
                parsedFont.maxpOffset = offset;
                parsedFont.maxGlyphs = ParseMAXP(U8ArraySkip(originalFontData, offset));
            }break;
        }
    }

    if ((parsedFont.error =  ParseFontValidateRequiredTablesExist(parsedFont)) != FONT_TTF_ERROR_NONE)
    {
        return parsedFont;
    }    

    parsedFont.parsedLoca = ParseLOCA(arena, U8ArraySkip(originalFontData, parsedFont.locaOffset), parsedFont.maxGlyphs, parsedFont.parsedHead.indexToLocFormat);
    parsedFont.parsedGlyf = ParseGLYF(arena, U8ArraySkip(originalFontData, parsedFont.glyfOffset), parsedFont.maxGlyphs, parsedFont.parsedLoca);
    ParseHMTX(U8ArraySkip(originalFontData, parsedFont.hmtxOffset), parsedFont.parsedGlyf, parsedFont.parsedHhea.numOfLongHorMetrics);

    printGlyphShape(parsedFont.parsedGlyf.data[26]);

    return parsedFont;
}
void ProgramMain(Str8List *args)
{
    Str8List *trailing = cmdlineTrailing(STR8("fontfile"));

    if (!cmdlineParse(*args) || !BASE_ANY_PTR(trailing))
    {
        cmdlineUsage();
        return;
    }

    Arena *arena = arenaAllocDefault();
    U8Array fontData = OSFileReadAll(arena, trailing->first->val);

    if (!BASE_ANY(fontData))
    {
        baseEPrintf("{r}Failed to find font\n");
        return;
    }

    ParseFont(arena, fontData);
}