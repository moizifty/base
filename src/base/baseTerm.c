#include "baseTerm.h"

void termClear(void)
{
    basePrintf("\033[H\033[2J");
}
void termSetChar(vec2i point, i32 ch)
{
    basePrintf("\033[%d;%dH%c", point.y + 1, point.x + 1, ch);
}
void termDrawLine(vec2i start, vec2i end, i32 ch)
{
    i64 dx = end.x - start.x;
    i64 dy = end.y - start.y;

    i64 step = max(llabs(dx), llabs(dy));
    f64 stepX = (f64)dx / (f64)step;
    f64 stepY = (f64)dy / (f64)step;

    f64 x = (f64)start.x;
    f64 y = (f64)start.y;
    for (i64 i = 0; i <= step; i++)
    {
        basePrintf("\033[%d;%dH%c", (i64)round(y + 1), (i64)round(x + 1), ch);

        x += stepX;
        y += stepY;
    }
}