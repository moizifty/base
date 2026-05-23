#include "base/base.h"
#include "os/core/osCore.h"
#include "font.h"

#include "base/base.c"
#include "os/core/osCore.c"
#include "font.c"

#include "os/core/osEntryPoint.c"

void ProgramMain(Str8List *args)
{
    Str8List *trailing = cmdlineTrailing(STR8("fontfile"));

    if (!cmdlineParse(*args) || !BASE_ANY_PTR(trailing))
    {
        cmdlineUsage();
        return;
    }

    Arena *arena = arenaAllocDefault();
    Font font = fontTTFParseFromFile(arena, trailing->first->val);

    if (font.error != FONT_ERROR_NONE)
    {
        baseEPrintf("{r}Failed to load font\n");
        return;
    }

    //for (u64 i = 893; i < font.parsed.parsedGlyf.len; i++)
    {
        fontPrintGlyphShapeFilled(font, font.parsed.parsedGlyf.data[897]);
        Sleep(250);
    }

    getchar();
}