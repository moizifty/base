#include "base/base.h"
#include "os/core/osCore.h"
#include "bss.h"

#include "base/base.c"
#include "os/core/osCore.c"
#include "bss.c"

#include "os/core/osEntryPoint.c"

void ProgramMain(CmdLineHashMap *line)
{
    Arena *arena = arenaAllocDefault();
    BssInterp interp = {.arena = arena};

    if(!bssParserParseFile(&interp, STR8_LIT("tests/test.bss")))
    {
        baseEPrintf("{r}Failed to open bss file\n");
    }

    basePrintf("{g}Done!");
    return;
}