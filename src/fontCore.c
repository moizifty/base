#include "fontCore.h"
#include "os/core/osCore.h"

f32 F32FromFontF2Dot14(FontF2Dot14 val)
{
    return val / 16384.0f;
}

bool fontTTFParseCmapSubtableFormat4(Arena *arena, FontTTFParsedFont *parsedFont, U8Array subtable)
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

    return true;
}
bool fontTTFParseCmapTable(Arena *arena, FontTTFParsedFont *parsedFont)
{
    FontTTFParsedCmap parsed = {0};
    U8Array cmapTable = U8ArraySkip(parsedFont->fontData, parsedFont->cmapOffset);

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
        U8Array subtable = U8ArraySkip(U8ArraySkip(parsedFont->fontData, parsedFont->cmapOffset), chosenSubtableOffset);
        u16 format = U8ArrayReadBigEndianU16(subtable);
        // dont skip the 2 format bytes, you just want to peak the format
        
        switch (chosenEncoding)
        {
            case FONT_TTF_UNICODE_ENCODING_BMP:
            {
                if (format == 4)
                {
                    fontTTFParseCmapSubtableFormat4(arena, parsedFont, subtable);
                }
                else
                {
                    parsedFont->error = FONT_ERROR_UNSUPPORTED_CMAP_SUBTABLE_FORMAT;
                }
            }break;
            
            case FONT_TTF_UNICODE_ENCODING_FULL:
            {

            }break;

            default:
            {
                baseEPrintf("{r}Unhandled cmap unicode encoding, when parsing ttf file.\n");
                parsedFont->error = FONT_ERROR_UNSUPPORTED_CMAP;
            }break;
        }
    }
    else
    {
        parsedFont->error = FONT_ERROR_UNSUPPORTED_CMAP;
    }
    
    parsedFont->parsedCMAP = parsed;

    return parsedFont->error == FONT_ERROR_NONE;
}

void fontTTFParseCompositeGlyphNumContoursAndPoints(FontTTFParsedFont *parsedFont, U8Array compositeData, u64 *numContours, u64 *numPoints)
{
    *numContours = 0;
    *numPoints = 0;

    u16 flags = 0;

    ArenaTemp temp = baseTempBegin(null, 0);

    do
    {
        flags = U8ArrayReadBigEndianU16(compositeData);
        compositeData = U8ArraySkip(compositeData, sizeof(u16));

        u16 glyphIndex = U8ArrayReadBigEndianU16(compositeData);
        compositeData = U8ArraySkip(compositeData, sizeof(u16));

        fontTTFParseGlyphShape(temp.arena, parsedFont, glyphIndex);

        FontGlyphShape componentShape = parsedFont->parsedGlyf.data[glyphIndex];

        *numContours += componentShape.contours.len;
        *numPoints += componentShape.points.len;

        if ((flags & ARG_1_AND_2_ARE_WORDS) && (flags & ARGS_ARE_XY_VALUES))
        {
            compositeData = U8ArraySkip(compositeData, sizeof(i16));
            compositeData = U8ArraySkip(compositeData, sizeof(i16));
        }
        else if (!(flags & ARG_1_AND_2_ARE_WORDS) && (flags & ARGS_ARE_XY_VALUES))
        {
            compositeData = U8ArraySkip(compositeData, sizeof(i8));
            compositeData = U8ArraySkip(compositeData, sizeof(i8));
        }
        else if ((flags & ARG_1_AND_2_ARE_WORDS) && !(flags & ARGS_ARE_XY_VALUES))
        {
            compositeData = U8ArraySkip(compositeData, sizeof(u16));
            compositeData = U8ArraySkip(compositeData, sizeof(u16));
        }
        else if (!(flags & ARG_1_AND_2_ARE_WORDS) && !(flags & ARGS_ARE_XY_VALUES))
        {
            compositeData = U8ArraySkip(compositeData, sizeof(u8));
            compositeData = U8ArraySkip(compositeData, sizeof(u8));
        }

        if (flags & WE_HAVE_A_SCALE)
        {
            compositeData = U8ArraySkip(compositeData, sizeof(FontF2Dot14));
        }
        else if (flags & WE_HAVE_AN_X_AND_Y_SCALE)
        {
            compositeData = U8ArraySkip(compositeData, sizeof(FontF2Dot14));
            compositeData = U8ArraySkip(compositeData, sizeof(FontF2Dot14));
        }
        else if (flags & WE_HAVE_A_TWO_BY_TWO)
        {
            compositeData = U8ArraySkip(compositeData, sizeof(FontF2Dot14));
            compositeData = U8ArraySkip(compositeData, sizeof(FontF2Dot14));
            compositeData = U8ArraySkip(compositeData, sizeof(FontF2Dot14));
            compositeData = U8ArraySkip(compositeData, sizeof(FontF2Dot14));
        }
    }while(flags & MORE_COMPONENTS);

    if (flags & WE_HAVE_INSTRUCTIONS)
    {
        u16 instLen = U8ArrayReadBigEndianU16(compositeData);
        compositeData = U8ArraySkip(compositeData, sizeof(u16));
        compositeData = U8ArraySkip(compositeData, sizeof(u8) * instLen);
    }

    baseTempEnd(temp);
}
void fontTTFParseGlyphShape(Arena *arena, FontTTFParsedFont *parsedFont, u32 glyphIndexToParse)
{
    FontGlyphOffset *glyphOffset = parsedFont->parsedLoca.data + glyphIndexToParse;
    if (glyphOffset->parsed)
    {
        return;
    }

    FontGlyphShape shape = {0};
    FontGlyphOffset nextGlyphOffset = parsedFont->parsedLoca.data[glyphIndexToParse + 1];

    u8 *glyphDataStart = U8ArraySkip(U8ArraySkip(parsedFont->fontData, parsedFont->glyfOffset), glyphOffset->offset).data;
    u8 *glyphDataEnd = U8ArraySkip(U8ArraySkip(parsedFont->fontData, parsedFont->glyfOffset), nextGlyphOffset.offset).data;

    if (glyphDataStart == glyphDataEnd) // 0 contours
    {
        glyphOffset->parsed = true;
        parsedFont->parsedGlyf.data[glyphIndexToParse] = shape;
        return;
    }

    ArenaTemp temp = baseTempBegin(&arena, 1);
    U8Array glyphData = U8ArraySkip(U8ArraySkip(parsedFont->fontData, parsedFont->glyfOffset), glyphOffset->offset);

    i16 numContours = U8ArrayReadBigEndianI16(glyphData);
    glyphData = U8ArraySkip(glyphData, sizeof(i16));

    i16 minX = U8ArrayReadBigEndianI16(glyphData);
    glyphData = U8ArraySkip(glyphData, sizeof(i16));

    i16 minY = U8ArrayReadBigEndianI16(glyphData);
    glyphData = U8ArraySkip(glyphData, sizeof(i16));

    i16 maxX = U8ArrayReadBigEndianI16(glyphData);
    glyphData = U8ArraySkip(glyphData, sizeof(i16));

    i16 maxY = U8ArrayReadBigEndianI16(glyphData);
    glyphData = U8ArraySkip(glyphData, sizeof(i16));

    shape.min = Vec2i(minX, minY);
    shape.max = Vec2i(maxX, maxY);

    if (numContours > 0)
    {
        u16 *endPointsOfContour = arenaPushArray(temp.arena, u16, numContours);
        for (u16 c = 0; c < numContours; c++)
        {
            endPointsOfContour[c] = U8ArrayReadBigEndianU16(glyphData);
            glyphData = U8ArraySkip(glyphData, sizeof(u16));
        }

        u16 instLength = U8ArrayReadBigEndianU16(glyphData);
        glyphData = U8ArraySkip(glyphData, sizeof(u16));
        glyphData = U8ArraySkip(glyphData, sizeof(u8) * instLength);
        
        u32 numPoints = endPointsOfContour[numContours - 1] + 1;
        
        u8 *flags = arenaPushArray(temp.arena, u8, numPoints);
        i64 *xCoords = arenaPushArray(temp.arena, i64, numPoints);
        i64 *yCoords = arenaPushArray(temp.arena, i64, numPoints);

        shape.points.data = arenaPushArray(arena, FontGlyphShapePoint, numPoints);
        shape.points.len = numPoints;

        u32 f = 0;
        while (f < numPoints)
        {
            flags[f] = glyphData.data[0];
            glyphData = U8ArraySkip(glyphData, sizeof (u8));

            if (flags[f] & 0x08)
            {
                u8 flagToRepeat = flags[f];
                u8 repeatCount = glyphData.data[0];
                glyphData = U8ArraySkip(glyphData, sizeof (u8));

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
                shape.points.data[iX].point.x = (i64)glyphData.data[0];
                glyphData = U8ArraySkip(glyphData, sizeof(u8));
                
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
                shape.points.data[iX].point.x = U8ArrayReadBigEndianI16(glyphData);
                glyphData = U8ArraySkip(glyphData, sizeof(i16));

                accumPoint.x += shape.points.data[iX].point.x;
                shape.points.data[iX].point.x = accumPoint.x;
            }
        }

        accumPoint = Vec2i(0, 0);
        for (u32 iY = 0; iY < numPoints; iY++)
        {
            if (flags[iY] & 0x04)
            {
                shape.points.data[iY].point.y = (i64)glyphData.data[0];
                glyphData = U8ArraySkip(glyphData, sizeof(u8));
                
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
                shape.points.data[iY].point.y = U8ArrayReadBigEndianI16(glyphData);
                glyphData = U8ArraySkip(glyphData, sizeof(i16));

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

        fontTTFParseCompositeGlyphNumContoursAndPoints(parsedFont, glyphData, &numTotalContours, &numTotalPoints);

        shape.points.data = arenaPushArray(arena, FontGlyphShapePoint, numTotalPoints);
        shape.points.len = 0;

        shape.contours.data = arenaPushArray(arena, FontGlyphShapeContour, numTotalContours);
        shape.contours.len = 0;

        u16 flags = 0;

        u64 currContourTotalPointsCount = 0;
        i32 prevEndIndex = -1;
        do
        {
            flags = U8ArrayReadBigEndianU16(glyphData);
            glyphData = U8ArraySkip(glyphData, sizeof(u16));

            u16 glyphIndex = U8ArrayReadBigEndianU16(glyphData);
            glyphData = U8ArraySkip(glyphData, sizeof(u16));

            fontTTFParseGlyphShape(arena, parsedFont, glyphIndex);

            FontGlyphShape componentShape = parsedFont->parsedGlyf.data[glyphIndex];

            i32 arg1 = 0;
            i32 arg2 = 0;

            f32 scaleX = 1;
            f32 scaleY = 1;

            f32 sheerX = 0;
            f32 sheerY = 0;
            if ((flags & ARG_1_AND_2_ARE_WORDS) && (flags & ARGS_ARE_XY_VALUES))
            {
                arg1 = (i32)U8ArrayReadBigEndianI16(glyphData);
                glyphData = U8ArraySkip(glyphData, sizeof(i16));

                arg2 = (i32)U8ArrayReadBigEndianI16(glyphData);
                glyphData = U8ArraySkip(glyphData, sizeof(i16));
            }
            else if (!(flags & ARG_1_AND_2_ARE_WORDS) && (flags & ARGS_ARE_XY_VALUES))
            {
                arg1 = (i32)glyphData.data[0];
                glyphData = U8ArraySkip(glyphData, sizeof(i8));

                arg2 = (i32)glyphData.data[0];
                glyphData = U8ArraySkip(glyphData, sizeof(i8));
            }
            else if ((flags & ARG_1_AND_2_ARE_WORDS) && !(flags & ARGS_ARE_XY_VALUES))
            {
                arg1 = (i32)U8ArrayReadBigEndianU16(glyphData);
                glyphData = U8ArraySkip(glyphData, sizeof(u16));

                arg2 = (i32)U8ArrayReadBigEndianU16(glyphData);
                glyphData = U8ArraySkip(glyphData, sizeof(u16));

                basePrintf("{r}Dont handle yet\n");
            }
            else if (!(flags & ARG_1_AND_2_ARE_WORDS) && !(flags & ARGS_ARE_XY_VALUES))
            {
                arg1 = (i32)(u8)glyphData.data[0];
                glyphData = U8ArraySkip(glyphData, sizeof(u8));

                arg2 = (i32)(u8)glyphData.data[0];
                glyphData = U8ArraySkip(glyphData, sizeof(u8));

                basePrintf("{r}Dont handle yet\n");
            }

            if (flags & WE_HAVE_A_SCALE)
            {
                FontF2Dot14 scaleRaw = U8ArrayReadBigEndianI16(glyphData);
                glyphData = U8ArraySkip(glyphData, sizeof(FontF2Dot14));

                scaleX = scaleY = F32FromFontF2Dot14(scaleRaw);
            }
            else if (flags & WE_HAVE_AN_X_AND_Y_SCALE)
            {
                FontF2Dot14 xScaleRaw = U8ArrayReadBigEndianI16(glyphData);
                glyphData = U8ArraySkip(glyphData, sizeof(FontF2Dot14));

                FontF2Dot14 yScaleRaw = U8ArrayReadBigEndianI16(glyphData);
                glyphData = U8ArraySkip(glyphData, sizeof(FontF2Dot14));

                scaleX = F32FromFontF2Dot14(xScaleRaw);
                scaleY = F32FromFontF2Dot14(yScaleRaw);
            }
            else if (flags & WE_HAVE_A_TWO_BY_TWO)
            {
                FontF2Dot14 xScaleRaw = U8ArrayReadBigEndianI16(glyphData);
                glyphData = U8ArraySkip(glyphData, sizeof(FontF2Dot14));

                FontF2Dot14 scale01Raw = U8ArrayReadBigEndianI16(glyphData);
                glyphData = U8ArraySkip(glyphData, sizeof(FontF2Dot14));

                FontF2Dot14 scale10Raw = U8ArrayReadBigEndianI16(glyphData);
                glyphData = U8ArraySkip(glyphData, sizeof(FontF2Dot14));

                FontF2Dot14 yScaleRaw = U8ArrayReadBigEndianI16(glyphData);
                glyphData = U8ArraySkip(glyphData, sizeof(FontF2Dot14));

                scaleX = F32FromFontF2Dot14(xScaleRaw);
                scaleY = F32FromFontF2Dot14(yScaleRaw);
                
                sheerX = F32FromFontF2Dot14(scale01Raw);
                sheerY = F32FromFontF2Dot14(scale10Raw);
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
            u16 instLen = U8ArrayReadBigEndianU16(glyphData);
            glyphData = U8ArraySkip(glyphData, sizeof(u16));
            glyphData = U8ArraySkip(glyphData, sizeof(u8) * instLen);
        }
    }

    baseTempEnd(temp);

    glyphOffset->parsed = true;
    parsedFont->parsedGlyf.data[glyphIndexToParse] = shape;
    return;
}
bool fontTTFParseGlyfTable(Arena *arena, FontTTFParsedFont *parsedFont)
{
    parsedFont->parsedGlyf.data = arenaPushArray(arena, FontGlyphShape, parsedFont->maxGlyphs);
    parsedFont->parsedGlyf.len = parsedFont->maxGlyphs;

    for (u32 i = 0; i < parsedFont->maxGlyphs; i++)
    {
        fontTTFParseGlyphShape(arena, parsedFont, i);
    }

    return true;
}
bool fontTTFParseHeadTable(FontTTFParsedFont *parsedFont)
{
    U8Array headTable = U8ArraySkip(parsedFont->fontData, parsedFont->headOffset);
    FontTTFParsedHead parsed = {0};

    headTable = U8ArraySkip(headTable, sizeof(u16));
    headTable = U8ArraySkip(headTable, sizeof(u16));
    headTable = U8ArraySkip(headTable, sizeof(u32));
    headTable = U8ArraySkip(headTable, sizeof(u32));

    u32 magicNumber = U8ArrayReadBigEndianU32(headTable);
    headTable = U8ArraySkip(headTable, sizeof(u32));

    if (magicNumber != 0x5F0F3CF5)
    {
        parsedFont->error = FONT_ERROR_UNSUPPORTED_HEAD;
        return false;
    }

    headTable = U8ArraySkip(headTable, sizeof(u16));

    parsed.unitsPerEm = U8ArrayReadBigEndianU16(headTable);
    headTable = U8ArraySkip(headTable, sizeof(u16));
    
    headTable = U8ArraySkip(headTable, sizeof(i64));
    headTable = U8ArraySkip(headTable, sizeof(i64));

    parsed.min.x = U8ArrayReadBigEndianI16(headTable);
    headTable = U8ArraySkip(headTable, sizeof(i16));
    parsed.min.y = U8ArrayReadBigEndianI16(headTable);
    headTable = U8ArraySkip(headTable, sizeof(i16));
    parsed.max.x = U8ArrayReadBigEndianI16(headTable);
    headTable = U8ArraySkip(headTable, sizeof(i16));
    parsed.max.y = U8ArrayReadBigEndianI16(headTable);
    headTable = U8ArraySkip(headTable, sizeof(i16));

    headTable = U8ArraySkip(headTable, sizeof(u16));
    headTable = U8ArraySkip(headTable, sizeof(u16));
    headTable = U8ArraySkip(headTable, sizeof(i16));

    parsed.indexToLocFormat = U8ArrayReadBigEndianI16(headTable);
    headTable = U8ArraySkip(headTable, sizeof(i16));
    headTable = U8ArraySkip(headTable, sizeof(i16));

    parsedFont->parsedHead = parsed;

    return parsedFont->error == FONT_ERROR_NONE;
}
bool fontTTFParseHheaTable(FontTTFParsedFont *parsedFont)
{
    U8Array hheaTable = U8ArraySkip(parsedFont->fontData, parsedFont->hheaOffset);
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

    parsedFont->parsedHhea = parsed;

    return parsedFont->error == FONT_ERROR_NONE;
}

bool fontTTFParseHmtxTable(FontTTFParsedFont *parsedFont)
{
    u64 numLeftSideBearingEntries = parsedFont->maxGlyphs - parsedFont->parsedHhea.numOfLongHorMetrics;
    U8Array hmtxTable = U8ArraySkip(parsedFont->fontData, parsedFont->hmtxOffset);

    for (u64 i = 0; i < parsedFont->parsedHhea.numOfLongHorMetrics; i++)
    {
        parsedFont->parsedGlyf.data[i].advanceWidth = U8ArrayReadBigEndianU16(hmtxTable);
        hmtxTable = U8ArraySkip(hmtxTable, sizeof(u16));

        parsedFont->parsedGlyf.data[i].leftSideBearing = U8ArrayReadBigEndianI16(hmtxTable);
        hmtxTable = U8ArraySkip(hmtxTable, sizeof(i16));
    }

    for (u64 i = 0; i < numLeftSideBearingEntries; i++)
    {
        parsedFont->parsedGlyf.data[i + parsedFont->parsedHhea.numOfLongHorMetrics].advanceWidth = parsedFont->parsedGlyf.data[parsedFont->parsedHhea.numOfLongHorMetrics - 1].advanceWidth;
        parsedFont->parsedGlyf.data[i + parsedFont->parsedHhea.numOfLongHorMetrics].leftSideBearing = U8ArrayReadBigEndianI16(hmtxTable);
        hmtxTable = U8ArraySkip(hmtxTable, sizeof(i16));
    }

    return true;
}
bool fontTTFParseLocaTable(Arena *arena, FontTTFParsedFont *parsedFont)
{
    U8Array locaTable = U8ArraySkip(parsedFont->fontData, parsedFont->locaOffset);

    parsedFont->parsedLoca.data = arenaPushArray(arena, FontGlyphOffset, parsedFont->maxGlyphs + 1);
    parsedFont->parsedLoca.len = parsedFont->maxGlyphs + 1;

    u32 offsetStride = parsedFont->parsedHead.indexToLocFormat ? sizeof(u32) : sizeof(u16);

    for (u16 i = 0; i < parsedFont->maxGlyphs + 1; i++)
    {
        parsedFont->parsedLoca.data[i].offset = parsedFont->parsedHead.indexToLocFormat ? U8ArrayReadBigEndianU32(locaTable) : (U8ArrayReadBigEndianU16(locaTable) * 2);
        parsedFont->parsedLoca.data[i].parsed = false;

        locaTable = U8ArraySkip(locaTable, offsetStride);
    }

    return true;
}
bool fontTTFParseMaxpTable(FontTTFParsedFont *parsedFont)
{
    U8Array maxpTable = U8ArraySkip(parsedFont->fontData, parsedFont->maxpOffset);

    maxpTable = U8ArraySkip(maxpTable, sizeof(u32));

    parsedFont->maxGlyphs = (u32)U8ArrayReadBigEndianU16(maxpTable);
    return true;
}

FontParseErrorKind fontTTFValidateRequiredTablesExist(FontTTFParsedFont parsedFont)
{
    if (parsedFont.cmapOffset == 0)
    {
        return FONT_ERROR_MISSING_CMAP;
    }

    if (parsedFont.glyfOffset == 0)
    {
        return FONT_ERROR_MISSING_GLYF;
    }

    if (parsedFont.headOffset == 0)
    {
        return FONT_ERROR_MISSING_HEAD;
    }

    if (parsedFont.hheaOffset == 0)
    {
        return FONT_ERROR_MISSING_HHEA;
    }
    
    if (parsedFont.hmtxOffset == 0)
    {
        return FONT_ERROR_MISSING_HMTX;
    }

    if (parsedFont.locaOffset == 0)
    {
        return FONT_ERROR_MISSING_LOCA;
    }

    if (parsedFont.maxpOffset == 0)
    {
        return FONT_ERROR_MISSING_MAXP;
    }

    return FONT_ERROR_NONE;
}

void fontPrintGlyphShape(FontGlyphShape shape)
{
    u32 termWidth = 80;
    u32 termHeight = 36;

    basePrintf("\033[H\033[2J");

    for (u64 i = 0; i < shape.points.len; i++)
    {
        vec2i point = shape.points.data[i].point;
        u32 normX = (u32)(((f32)(point.x - shape.min.x) / (f32)(shape.max.x - shape.min.x)) * (f32)(termWidth - 1));
        u32 normY = (u32)(((f32)(point.y - shape.min.y) / (f32)(shape.max.y - shape.min.y)) * (f32)(termHeight - 1));
        normY = (termHeight - 1) - normY;

        basePrintf("\033[%d;%dH", normY + 1, normX + 1);
        basePrintf("#");
    }
}

Font fontTTFParseFromFile(Arena *arena, str8 file)
{
    ArenaTemp temp = baseTempBegin(&arena, 1);

    Font ret = {0};
    FontTTFParsedFont parsed = {0};

    U8Array fileContents = OSFileReadAll(temp.arena, file);
    if (BASE_ANY(fileContents))
    {
        ret = fontTTFParseFromU8Array(arena, fileContents);
    }
    else
    {
        ret.error = FONT_ERROR_EMPTY_OR_FILE_DOESNT_EXIST;
    }

    baseTempEnd(temp);

    return ret;
}
Font fontTTFParseFromU8Array(Arena *arena, U8Array fontData)
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

    Font ret = {0};
    FontTTFParsedFont parsedFont = {0};
    parsedFont.fontData = originalFontData;

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
                if (!fontTTFParseCmapTable(arena, &parsedFont))
                {
                    ret.error = parsedFont.error;
                    return ret;
                }
            }break;
            
            case (u32)'glyf':
            {
                parsedFont.glyfOffset = offset;
            }break;

            case (u32)'head':
            {
                parsedFont.headOffset = offset;
                if (!fontTTFParseHeadTable(&parsedFont))
                {
                    ret.error = parsedFont.error;
                    return ret;
                }
            }break;

            case (u32)'hhea':
            {
                parsedFont.hheaOffset = offset;
                if (!fontTTFParseHheaTable(&parsedFont))
                {
                    ret.error = parsedFont.error;
                    return ret;
                }
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
                if (!fontTTFParseMaxpTable(&parsedFont))
                {
                    ret.error = parsedFont.error;
                    return ret;
                }
            }break;
        }
    }

    if ((parsedFont.error =  fontTTFValidateRequiredTablesExist(parsedFont)) != FONT_ERROR_NONE)
    {
        ret.error = parsedFont.error;
        return ret;
    }    

    if (!fontTTFParseLocaTable(arena, &parsedFont))
    {
        ret.error = parsedFont.error;
        return ret;
    }

    if (!fontTTFParseGlyfTable(arena, &parsedFont))
    {
        ret.error = parsedFont.error;
        return ret;
    }

    if (!fontTTFParseHmtxTable(&parsedFont))
    {
        ret.error = parsedFont.error;
        return ret;
    }

    fontPrintGlyphShape(parsedFont.parsedGlyf.data[98]);

    return ret;
}