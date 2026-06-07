#include "base/baseTerm.h"
#include "os/core/osCore.h"

#include "fontCore.h"

f32 F32FromFontF2Dot14(FontF2Dot14 val)
{
    return val / 16384.0f;
}
FontGlyphShapeEdgeArray fontExpandContourPoints(Arena *arena, FontGlyphShapeContour contour)
{
    FontGlyphShapePointArray expandedMidpoints = {0};

    ArenaTemp temp = baseTempBegin(&arena, 1);
    const i32 bezierPoints = 12;
    u64 expandedMidPointsCount = 0;

    i64 firstOnCurvePointIndex = 0;
    for (u64 p = 0; p < contour.points.len; p++)
    {
        FontGlyphShapePoint cp = contour.points.data[p];
        FontGlyphShapePoint np = contour.points.data[(p + 1) % contour.points.len];
        FontGlyphShapePoint nnp = contour.points.data[(p + 2) % contour.points.len];

        expandedMidPointsCount++;

        if (cp.isControlPoint && np.isControlPoint)
        {
            expandedMidPointsCount++;
        }
    }

    expandedMidpoints.data = arenaPushArray(temp.arena, FontGlyphShapePoint, expandedMidPointsCount);

    for (u64 p = 0; p < contour.points.len; p++)
    {
        FontGlyphShapePoint cp = contour.points.data[p];
        FontGlyphShapePoint np = contour.points.data[(p + 1) % contour.points.len];
        FontGlyphShapePoint nnp = contour.points.data[(p + 2) % contour.points.len];

        expandedMidpoints.data[expandedMidpoints.len++] = cp;

        if (cp.isControlPoint && np.isControlPoint)
        {
            FontGlyphShapePoint newPoint = {0};

            newPoint.point.x = (((f64)np.point.x + (f64)cp.point.x) / 2.0);
            newPoint.point.y = (((f64)np.point.y + (f64)cp.point.y) / 2.0);

            expandedMidpoints.data[expandedMidpoints.len++] = newPoint;
        }
    }

    FontGlyphShapePointArray expandedPoints = {0};

    u64 expandedTotalPoints = 0;

    for (u64 p = 0; p < expandedMidpoints.len; p++)
    {
        FontGlyphShapePoint cp = expandedMidpoints.data[p];
        FontGlyphShapePoint np = expandedMidpoints.data[(p + 1) % expandedMidpoints.len];
        FontGlyphShapePoint nnp = expandedMidpoints.data[(p + 2) % expandedMidpoints.len];

        if (!cp.isControlPoint && np.isControlPoint && !nnp.isControlPoint)
        {
            expandedTotalPoints += (bezierPoints + 1);
            p += 1;
        }
        else if (!cp.isControlPoint)
        {
            expandedTotalPoints++;
        }
    }

    expandedPoints.data = arenaPushArray(temp.arena, FontGlyphShapePoint, expandedTotalPoints);
    for (u64 p = 0; p < expandedMidpoints.len; p++)
    {
        FontGlyphShapePoint cp = expandedMidpoints.data[p];
        FontGlyphShapePoint np = expandedMidpoints.data[(p + 1) % expandedMidpoints.len];
        FontGlyphShapePoint nnp = expandedMidpoints.data[(p + 2) % expandedMidpoints.len];

        if (!cp.isControlPoint && np.isControlPoint && !nnp.isControlPoint)
        {
            f64 step = 1.0 / bezierPoints;
            for (f64 i = 0; i <= 1.0; i += step)
            {
                vec2f64 point = quadraticBezierVec2f64(cp.point, np.point, nnp.point, i);
                expandedPoints.data[expandedPoints.len].isControlPoint = false;
                expandedPoints.data[expandedPoints.len].point = point;

                expandedPoints.len++;
            }
            p += 1;
        }
        else if (!cp.isControlPoint)
        {
            expandedPoints.data[expandedPoints.len++] = cp;
        }
    }

    // edge count is equal to total points, since its a closed shape
    u64 totalEdgeCount = expandedTotalPoints;
    FontGlyphShapeEdgeArray edges = {0};
    edges.data = arenaPushArray(arena, FontGlyphShapeEdge, totalEdgeCount);
    edges.len = totalEdgeCount;

    for (u64 i = 0; i < expandedPoints.len; i++)
    {
        FontGlyphShapePoint cp = expandedPoints.data[i];
        FontGlyphShapePoint np = expandedPoints.data[(i + 1) % expandedPoints.len];

        edges.data[i].start = cp.point;
        edges.data[i].end = np.point;
    }

    baseTempEnd(temp);

    return edges;
}

i32 fontGlyphShapeEdgeCompareIntersectionX(const void *p1, const void *p2)
{
    FontGlyphShapeEdge e1 = *(FontGlyphShapeEdge *)p1;
    FontGlyphShapeEdge e2 = *(FontGlyphShapeEdge *)p2;

    if (e1.intersection.x < e2.intersection.x) return -1;
    if (e1.intersection.x > e2.intersection.x) return 1;

    return 0;
}
f64 fontGetEmToPixelScale(Font font, f64 pixelSize)
{
    return pixelSize / (f64)font.metrics.unitsPerEm;
}

range2i fontEmBoundsToPixelBounds(Font font, range2i emBounds, f64 pixelSize)
{
    f64 scale = fontGetEmToPixelScale(font, pixelSize);

    f64 x0f = (f64)emBounds.min.x * scale;
    f64 x1f = (f64)emBounds.max.x * scale;
    f64 y0f = -(f64)emBounds.max.y * scale;  // top    (negated yMax)
    f64 y1f = -(f64)emBounds.min.y * scale;  // bottom (negated yMin)

    i64 ix0 = (i64)floor(x0f);
    i64 iy0 = (i64)floor(y0f);
    i64 ix1 = (i64)ceil(x1f);
    i64 iy1 = (i64)ceil(y1f);

    return Range2iFromVec2i(Vec2i(ix0, iy0), Vec2i(ix1, iy1));
}
vec2i fontGetGlyphShapeBitmapDimensions(Font font, FontGlyphShape shape, f64 pixelSize)
{
    f64 scale = fontGetEmToPixelScale(font, pixelSize);

    f64 x0f = (f64)shape.bounds.min.x * scale;
    f64 x1f = (f64)shape.bounds.max.x * scale;
    f64 y0f = -(f64)shape.bounds.max.y * scale;  // top    (negated yMax)
    f64 y1f = -(f64)shape.bounds.min.y * scale;  // bottom (negated yMin)

    i64 ix0 = (i64)floor(x0f);
    i64 iy0 = (i64)floor(y0f);
    i64 ix1 = (i64)ceil(x1f);
    i64 iy1 = (i64)ceil(y1f);

    range2i pixelBounds = fontEmBoundsToPixelBounds(font, shape.bounds, pixelSize);

    i64 bitmapW = Range2iDim(pixelBounds).w;
    i64 bitmapH = Range2iDim(pixelBounds).h;

    return Vec2i(bitmapW, bitmapH);
}
vec2f64 fontNormaliseCoordsFromBounds(Font font, vec2f64 coords, range2i shapeBox, f64 pixelSize)
{
    f64 scale = fontGetEmToPixelScale(font, pixelSize);

    range2i pixelBounds = fontEmBoundsToPixelBounds(font, shapeBox, pixelSize);

    f64 px = (f64)coords.x * scale - (f64)pixelBounds.min.x;
    f64 py = -(f64)coords.y * scale - (f64)pixelBounds.min.y;  // negate Y, subtract box top

    return Vec2f64(px, py);
}
vec2i fontFloatingCoordsToPixelPerfect(Font font, vec2f64 coords, range2i shapeBox, f64 pixelSize)
{
    range2i pixelBounds = fontEmBoundsToPixelBounds(font, shapeBox, pixelSize);

    i64 nx = (i64)floor(coords.x);
    i64 ny = (i64)floor(coords.y);

    if (nx >= Range2iDim(pixelBounds).w) nx = Range2iDim(pixelBounds).w - 1;
    if (ny >= Range2iDim(pixelBounds).h) ny = Range2iDim(pixelBounds).h - 1;

    return Vec2i(nx, ny);
}

void fontRasteriseEdgesToBitmap(Font font, FontGlyphShapeEdgeArray edges, range2i shapeBounds, Bitmap *bitmap, u64 bitmapWidth, f64 pixelSize)
{
    ArenaTemp temp = baseTempBegin(null, 0);

    // naive allocation,. active edges will be never greater then total number of edges
    FontGlyphShapeEdgeArray activeEdges = {0};
    activeEdges.data = arenaPushArray(temp.arena, FontGlyphShapeEdge, edges.len);

    for (f64 y = 0; y < (f64)bitmap->size.height; y++)
    {
        activeEdges.len = 0;
        for (u64 e = 0; e < edges.len; e++)
        {
            vec2f64 edgeNormStart = fontNormaliseCoordsFromBounds(font, edges.data[e].start, shapeBounds, pixelSize);
            vec2f64 edgeNormEnd = fontNormaliseCoordsFromBounds(font, edges.data[e].end, shapeBounds, pixelSize);

            f64 dy = edgeNormEnd.y - edgeNormStart.y;

            if (y >= edgeNormStart.y && y < edgeNormEnd.y ||
                y >= edgeNormEnd.y && y < edgeNormStart.y)
            {
                activeEdges.data[activeEdges.len++] = edges.data[e];
            }
            else if (y == edgeNormStart.y && edgeNormStart.y == edgeNormEnd.y)
            {
                u64 tempWidth = bitmap->size.w;
                bitmap->size.w = bitmapWidth;

                vec2i coordsStart = fontFloatingCoordsToPixelPerfect(font, edgeNormStart, shapeBounds, pixelSize);
                vec2i coordsEnd = fontFloatingCoordsToPixelPerfect(font, edgeNormEnd, shapeBounds, pixelSize);
                bitmapDrawLine(bitmap, coordsStart, coordsEnd, Vec4u8(255, 255, 255, 255), BitmapSamplerDefault);
                bitmap->size.w = tempWidth;
            }
            else if (y == (f64)(bitmap->size.height - 1))
            {
                // having an issue, (only applicable for binary fill, change when u do scanline acumalation coverage)
                // but, if the edge would fall on the last row, it needs to be handled,
                // especially for the null glyph, since it starts at ymin and ends at ymax
                if (edgeNormStart.y == (f64)bitmap->size.height ||
                    edgeNormEnd.y == (f64)bitmap->size.height)
                {
                    if (edgeNormStart.y == edgeNormEnd.y)
                    {
                        u64 tempWidth = bitmap->size.w;
                        bitmap->size.w = bitmapWidth;

                        vec2i coordsStart = fontFloatingCoordsToPixelPerfect(font, edgeNormStart, shapeBounds, pixelSize);
                        vec2i coordsEnd = fontFloatingCoordsToPixelPerfect(font, edgeNormEnd, shapeBounds, pixelSize);
                        bitmapDrawLine(bitmap, coordsStart, coordsEnd, Vec4u8(255, 255, 255, 255), BitmapSamplerDefault);
                        bitmap->size.w = tempWidth;
                    }
                    else
                    {
                        activeEdges.data[activeEdges.len++] = edges.data[e];
                    }
                }
            }
        }

        for(u64 e = 0; e < activeEdges.len; e++)
        {
            vec2f64 edgeNormStart = fontNormaliseCoordsFromBounds(font, activeEdges.data[e].start, shapeBounds, pixelSize);
            vec2f64 edgeNormEnd = fontNormaliseCoordsFromBounds(font, activeEdges.data[e].end, shapeBounds, pixelSize);

            f64 s = edgeNormEnd.y - edgeNormStart.y;
            activeEdges.data[e].winding = (s < 0.0) ? -1.0 : ((s > 0.0) ? 1.0 : 0.0);
            activeEdges.data[e].intersection.y = y;

            f64 dx = (edgeNormEnd.x - edgeNormStart.x);
            f64 dy = (edgeNormEnd.y - edgeNormStart.y);

            // make sure the start is always below the end
            // so we can correctly  handle the intersection
            vec2f64 start = edgeNormStart.y > edgeNormEnd.y ? edgeNormEnd : edgeNormStart;
            vec2f64 end = edgeNormStart.y > edgeNormEnd.y ? edgeNormStart : edgeNormEnd;

            if (start.y != end.y)
            {
                f64 m = dx / dy;

                f64 x0 = start.x;
                f64 y0 = start.y;
                f64 y1 = y;

                if (y0 > y1)
                {
                    activeEdges.data[e].intersection.x = x0;
                }
                else
                {
                    activeEdges.data[e].intersection.x = x0 + (y1 - y0) * m;
                }
            }
        }

        qsort(activeEdges.data, activeEdges.len, sizeof(FontGlyphShapeEdge), fontGlyphShapeEdgeCompareIntersectionX);

        i32 winding = 0;
        f64 prevX = 0;
        for (u64 a = 0; a < activeEdges.len; a++)
        {
            if (winding != 0)
            {
                u64 tempWidth = bitmap->size.w;
                bitmap->size.w = bitmapWidth;

                vec2i startCoords = fontFloatingCoordsToPixelPerfect(font, activeEdges.data[a].intersection, shapeBounds, pixelSize);
                vec2i endCoords = fontFloatingCoordsToPixelPerfect(font, Vec2f64(prevX, y), shapeBounds, pixelSize);

                bitmapDrawLine(bitmap, startCoords, endCoords, Vec4u8(255, 255, 255, 255), BitmapSamplerDefault);

                bitmap->size.w = tempWidth;
            }

            prevX = activeEdges.data[a].intersection.x;
            winding += activeEdges.data[a].winding;
        }

    }

    baseTempEnd(temp);
}
void fontRasteriseGlyphShapeToBitmap(Font font, FontGlyphShape shape, Bitmap *bitmap, u64 bitmapWidth, f64 pixelSize)
{
    ArenaTemp temp = baseTempBegin(null, 0);

    u64 totalEdgesRequired = 0;
    FontGlyphShapeEdgeArray allEdges = {0};

    for (u64 c = 0; c < shape.contours.len; c++)
    {
        FontGlyphShapeContour contour = shape.contours.data[c];

        FontGlyphShapeEdgeArray edges = fontExpandContourPoints(temp.arena, contour);
        totalEdgesRequired += edges.len;
    }

    {
        allEdges.data = arenaPushArray(temp.arena, FontGlyphShapeEdge, totalEdgesRequired);

        for (u64 c = 0; c < shape.contours.len; c++)
        {
            FontGlyphShapeContour contour = shape.contours.data[c];

            FontGlyphShapeEdgeArray edges = fontExpandContourPoints(temp.arena, contour);

            BASE_MEMCPY(allEdges.data + allEdges.len, edges.data, edges.len * sizeof(FontGlyphShapeEdge));
            allEdges.len += edges.len;
        }

        fontRasteriseEdgesToBitmap(font, allEdges, shape.bounds, bitmap, bitmapWidth, pixelSize);
    }

    baseTempEnd(temp);
}
void fontRasteriseAntiAliasedGlyphShapeToBitmap(Font font, FontGlyphShape shape, Bitmap *bitmap, u64 bitmapWidth, f64 pixelSize)
{
    ArenaTemp temp = baseTempBegin(null, 0);

    u64 totalEdgesRequired = 0;
    FontGlyphShapeEdgeArray allEdges = {0};

    for (u64 c = 0; c < shape.contours.len; c++)
    {
        FontGlyphShapeContour contour = shape.contours.data[c];

        FontGlyphShapeEdgeArray edges = fontExpandContourPoints(temp.arena, contour);
        totalEdgesRequired += edges.len;
    }

    {
        allEdges.data = arenaPushArray(temp.arena, FontGlyphShapeEdge, totalEdgesRequired);

        for (u64 c = 0; c < shape.contours.len; c++)
        {
            FontGlyphShapeContour contour = shape.contours.data[c];

            FontGlyphShapeEdgeArray edges = fontExpandContourPoints(temp.arena, contour);

            BASE_MEMCPY(allEdges.data + allEdges.len, edges.data, edges.len * sizeof(FontGlyphShapeEdge));
            allEdges.len += edges.len;
        }

        fontRasteriseEdgesToBitmap(font, allEdges, shape.bounds, bitmap, bitmapWidth, pixelSize);
    }

    baseTempEnd(temp);
}

void fontRasteriseCodepointToBitmap(Font font, u32 codepoint, Bitmap *bitmap, u64 bitmapWidth, f64 pixelSize, bool allowInvalidGlyph)
{
    FontGlyph glyph = fontGetGlyphFromCodepoint(font, codepoint);
    if (glyph.glyphIndex == 0 && !allowInvalidGlyph)
    {
        return;
    }

    fontRasteriseAntiAliasedGlyphShapeToBitmap(font, glyph.shape, bitmap, bitmapWidth, pixelSize);
}

FontGlyph fontGetGlyphFromCodepoint(Font font, u32 codepoint)
{
    u64 index = fontGetGlyphIndexFromCodepoint(font, codepoint);
    FontGlyph glyph = {0};
    glyph.shape = font.parsed.parsedGlyf.data[index];
    glyph.codepoint = codepoint;
    glyph.glyphIndex = index;

    return glyph;
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