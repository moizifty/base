#include "fontCore.h"
#include "os/core/osCore.h"

f32 F32FromFontF2Dot14(FontF2Dot14 val)
{
    return val / 16384.0f;
}

bool fontTTFParseCmapSubtableFormat4(Arena *arena, FontTTFParsedFont *parsedFont, U8Array subtable)
{
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

    parsedFont->parsedCMAP.format4.segCount = segCount;
    parsedFont->parsedCMAP.format4.startCodes = arenaPushArray(arena, u16, segCount);
    parsedFont->parsedCMAP.format4.endCodes = arenaPushArray(arena, u16, segCount);
    parsedFont->parsedCMAP.format4.idDeltas = arenaPushArray(arena, i16, segCount);
    parsedFont->parsedCMAP.format4.idRangeOffsets = arenaPushArray(arena, u16, segCount);
    for (u16 i = 0; i < segCount; i++)
    {
        parsedFont->parsedCMAP.format4.endCodes[i] = U8ArrayReadBigEndianU16(subtable);
        subtable = U8ArraySkip(subtable, sizeof(u16));
    }
    // skip padding
    subtable = U8ArraySkip(subtable, sizeof(u16));

    for (u16 i = 0; i < segCount; i++)
    {
        parsedFont->parsedCMAP.format4.startCodes[i] = U8ArrayReadBigEndianU16(subtable);
        subtable = U8ArraySkip(subtable, sizeof(u16));
    }
    for (u16 i = 0; i < segCount; i++)
    {
        parsedFont->parsedCMAP.format4.idDeltas[i] = U8ArrayReadBigEndianI16(subtable);
        subtable = U8ArraySkip(subtable, sizeof(i16));
    }

    parsedFont->iroOffset = parsedFont->cmapSubtableOffset + (originalSubtable.len - subtable.len);
    for (u16 i = 0; i < segCount; i++)
    {
        parsedFont->parsedCMAP.format4.idRangeOffsets[i] = U8ArrayReadBigEndianU16(subtable);
        subtable = U8ArraySkip(subtable, sizeof(u16));
    }

    parsedFont->parsedCMAP.format4.numGlyphIds = tableLen - (originalSubtable.len - subtable.len);

    parsedFont->parsedCMAP.format4.glphIds = arenaPushArray(arena, u16, parsedFont->parsedCMAP.format4.numGlyphIds);
    for(u64 i = 0; i < parsedFont->parsedCMAP.format4.numGlyphIds; i++)
    {
        parsedFont->parsedCMAP.format4.glphIds[i] = U8ArrayReadBigEndianU16(subtable);
        subtable = U8ArraySkip(subtable, sizeof(u16));
    }

    return true;
}
bool fontTTFParseCmapSubtableFormat12(Arena *arena, FontTTFParsedFont *parsedFont, U8Array subtable)
{
    U8Array originalSubtable = subtable;

    // skip the format, it should always be 12
    subtable = U8ArraySkip(subtable, sizeof(u16));

    subtable = U8ArraySkip(subtable, sizeof(u16));

    subtable = U8ArraySkip(subtable, sizeof(u32));
    subtable = U8ArraySkip(subtable, sizeof(u32));

    u16 numGroups = U8ArrayReadBigEndianU32(subtable);
    subtable = U8ArraySkip(subtable, sizeof(u32));
    parsedFont->parsedCMAP.isFormat12 = true;
    parsedFont->parsedCMAP.format12.numGroups = numGroups;

    parsedFont->parsedCMAP.format12.groups.data = arenaPushArray(arena, FontTTFParsedCmapFormat12Group, numGroups);
    parsedFont->parsedCMAP.format12.groups.len = numGroups;

    for (u16 i = 0; i < numGroups; i++)
    {
        parsedFont->parsedCMAP.format12.groups.data[i].startCharCode = U8ArrayReadBigEndianU32(subtable);
        subtable = U8ArraySkip(subtable, sizeof(u32));

        parsedFont->parsedCMAP.format12.groups.data[i].endCharCode = U8ArrayReadBigEndianU32(subtable);
        subtable = U8ArraySkip(subtable, sizeof(u32));

        parsedFont->parsedCMAP.format12.groups.data[i].startGlyphId = U8ArrayReadBigEndianU32(subtable);
        subtable = U8ArraySkip(subtable, sizeof(u32));
    }

    return true;
}

bool fontTTFParseCmapTable(Arena *arena, FontTTFParsedFont *parsedFont)
{
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
        parsedFont->cmapSubtableOffset = parsedFont->cmapOffset + chosenSubtableOffset;
        U8Array subtable = U8ArraySkip(parsedFont->fontData, parsedFont->cmapSubtableOffset);
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
                if (format == 12)
                {
                    fontTTFParseCmapSubtableFormat12(arena, parsedFont, subtable);
                }
                else
                {
                    parsedFont->error = FONT_ERROR_UNSUPPORTED_CMAP_SUBTABLE_FORMAT;
                }
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
            shape.points.data[iX].isControlPoint = !(flags[iX] & 0x01);

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

FontGlyphShapePointArray fontExpandContourPoints(Arena *arena, FontGlyphShapeContour contour)
{
    FontGlyphShapePointArray ret = {0};
    u64 expandedPointsCount = 0;

    for (u64 p = 0; p < contour.points.len; p++)
    {
        FontGlyphShapePoint cp = contour.points.data[p];
        FontGlyphShapePoint np = contour.points.data[(p + 1) % contour.points.len];

        if (cp.isControlPoint && np.isControlPoint)
        {
            expandedPointsCount++;
        }
        else if (!cp.isControlPoint)
        {
            expandedPointsCount++;
        }
    }

    ret.data = arenaPushArray(arena, FontGlyphShapePoint, expandedPointsCount);

    for (u64 p = 0; p < contour.points.len; p++)
    {
        FontGlyphShapePoint cp = contour.points.data[p];
        FontGlyphShapePoint np = contour.points.data[(p + 1) % contour.points.len];


        if (cp.isControlPoint && np.isControlPoint)
        {
            FontGlyphShapePoint newPoint = {0};

            newPoint.point.x = (i64)(((f64)np.point.x + (f64)cp.point.x) / 2.0);
            newPoint.point.y = (i64)(((f64)np.point.y + (f64)cp.point.y) / 2.0);

            ret.data[ret.len++] = newPoint;
        }
        else if (!cp.isControlPoint)
        {
            ret.data[ret.len++] = cp;
        }
    }

    return ret;
}
f64 fontGetEmToPixelScale(Font font, f64 pixelSize)
{
    return pixelSize / (f64)font.metrics.unitsPerEm;
}

void drawLine(i32 x0, i32 y0, i32 x1, i32 y1)
{
    i32 dy = y1 - y0;
    i32 dx = x1 - x0;

    i32 stepX = (x1 < x0) ? -1 : 1;
    i32 stepY = (y1 < y0) ? -1 : 1;

    if (dx == 0)
    {
        for (i32 y = y0; y != y1; y += stepY)
        {
            basePrintf("\033[%d;%dH#", y + 1, x0 + 1);
        }
    }
    else
    {
        f32 step = max((f32)abs(dx), (f32)abs(dy));

        f32 stepx = (f32)dx / step;
        f32 stepy = (f32)dy / step;

        for (i32 i = 0; i < (i32)(step + 1); i++)
        {
            basePrintf("\033[%d;%dH#", (i32)((f32)y0 + (f32)i * stepy) + 1, (i32)((f32)x0 + (f32)i * stepx) + 1);
        }
    }
}

void fontPrintDrawLine(i32 x0, i32 y0, i32 x1, i32 y1) 
{
    int dx  =  abs(x1 - x0);
    int dy  = -abs(y1 - y0);
    int sx  = x0 < x1 ? 1 : -1;
    int sy  = y0 < y1 ? 1 : -1;
    int err = dx + dy;

    while (true) 
    {
        basePrintf("\033[%d;%dH#", y0+1, x0+1);
        if (x0==x1 && y0==y1) break;
        int e2 = 2 * err;

        if (e2 >= dy) { err += dy; x0 += sx; }  // step x
        if (e2 <= dx) { err += dx; y0 += sy; }  // step y
    }
}
void fontPrintGlyphShape(Font font, FontGlyphShape shape)
{ 
    u32 termWidth = 124;
    u32 termHeight = 40;

    basePrintf("\033[H\033[2J");

    ArenaTemp temp = baseTempBegin(null, 0);

    for (u64 c = 0; c < shape.contours.len; c++)
    {
        FontGlyphShapeContour contour = shape.contours.data[c];

        FontGlyphShapePointArray expandedPoints = fontExpandContourPoints(temp.arena, contour);
        for (u64 p = 0; p < expandedPoints.len; p++)
        {
            FontGlyphShapePoint cp = expandedPoints.data[p];
            if (cp.isControlPoint)
            {
                continue;
            }
            
            u64 ni = (p + 1) % expandedPoints.len;
            while (expandedPoints.data[ni].isControlPoint)
            {
                ni = (ni + 1) % expandedPoints.len;
            }

            FontGlyphShapePoint np = expandedPoints.data[ni];

            i32 normX2 = (i64)(((f32)(np.point.x - shape.min.x) / (f32)(shape.max.x - shape.min.x)) * (f32)(termWidth - 1));
            i32 normY2 = (i64)(((f32)(np.point.y - shape.min.y) / (f32)(shape.max.y - shape.min.y)) * (f32)(termHeight - 1));
            normY2 = (termHeight - 1) - normY2;

            i32 normX = (i64)(((f32)(cp.point.x - shape.min.x) / (f32)(shape.max.x - shape.min.x)) * (f32)(termWidth - 1));
            i32 normY = (i64)(((f32)(cp.point.y - shape.min.y) / (f32)(shape.max.y - shape.min.y)) * (f32)(termHeight - 1));
            normY = (termHeight - 1) - normY;

            fontPrintDrawLine(normX, normY, normX2, normY2);
            // if ((normX2 - normX) == 0)
            // {
            //     if (normY <= normY2)
            //     {
            //         for (i64 y = normY; y < normY2; y++)
            //         {
            //             basePrintf("\033[%d;%dH", y + 1, normX + 1);
            //             basePrintf("#");
            //         }
            //     }
            //     else
            //     {
            //         for (i64 y = normY; y > normY2; y--)
            //         {
            //             basePrintf("\033[%d;%dH", y + 1, normX + 1);
            //             basePrintf("#");
            //         }
            //     }
            // }
            // else
            // {
            //     f64 grad = (f64)((f64)normY2 - normY) / (f64)((f64)normX2 - normX);

            //     if (normX <= normX2)
            //     {
            //         for (i64 x = normX; x < normX2; x++)
            //         {
            //             i64 y = (i64)(normY + grad * (f64)((f64)x - (f64)normX));
            //             basePrintf("\033[%d;%dH", y + 1, x + 1);
            //             basePrintf("#");
            //         }
            //     }
            //     else
            //     {
            //         for (i64 x = normX; x > normX2; x--)
            //         {
            //             i64 y = (i64)(normY + grad * (f64)((f64)x - (f64)normX));
            //             basePrintf("\033[%d;%dH", y + 1, x + 1);
            //             basePrintf("#");
            //         }
            //     }
            // }
        }

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

    ret.parsed = parsedFont;
    ret.metrics.ascent = parsedFont.parsedHhea.ascent;
    ret.metrics.descent = parsedFont.parsedHhea.descent;
    ret.metrics.lineGap = parsedFont.parsedHhea.lineGap;
    ret.metrics.unitsPerEm = parsedFont.parsedHead.unitsPerEm;
    ret.metrics.min = parsedFont.parsedHead.min;
    ret.metrics.max = parsedFont.parsedHead.max;

    return ret;
}

u64 fontGetGlyphIndexFromCodepoint(Font font, u32 codepoint)
{
    if (font.parsed.parsedCMAP.isFormat12)
    {
        FontTTFParsedCmapFormat12 format12 = font.parsed.parsedCMAP.format12;
        for (u64 i = 0; i < format12.numGroups; i++)
        {
            FontTTFParsedCmapFormat12Group group = format12.groups.data[i];
            if (group.endCharCode < codepoint) continue;
            if (group.startCharCode > codepoint) return 0;

            // found group
            return (codepoint - group.startCharCode) + group.startGlyphId;
        }
    }
    else
    {
        FontTTFParsedCmapFormat4 format4 = font.parsed.parsedCMAP.format4;
        for (u64 i = 0; i < format4.segCount; i++)
        {
            if (format4.endCodes[i] < codepoint) continue;
            if (format4.startCodes[i] > codepoint) return 0;

            // found segment

            u64 index = 0;
            if (format4.idRangeOffsets[i] != 0)
            {
                u64 iro = font.parsed.iroOffset + i * 2 + format4.idRangeOffsets[i] + (codepoint - format4.startCodes[i]) * 2;
                index = U8ArrayReadBigEndianU16(U8ArraySkip(font.parsed.fontData, iro));
                index = index % 65536;
            }
            else
            {
                index = (codepoint + format4.idDeltas[i]) % 65536;
            }

            return index;
        }
    }
    
    // no glyph found, return first
    return 0;
}