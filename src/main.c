#include "base/base.h"
#include "os/core/osCore.h"
#include "bssCore.h"
#include "bssLexer.h"

#include "base/base.c"
#include "os/core/osCore.c"
#include "bssCore.c"
#include "bssLexer.c"

#include "os/core/osEntryPoint.c"

void ProgramMain(CmdLineHashMap *line)
{
    Arena *arena = arenaAllocDefault();
    BssInterp interp = {.arena = arena};

    if(!bssLexerLexFile(&interp, STR8_LIT("tests/test.bss")))
    {
        baseEPrintf("{r}Failed to open bss file\n");
    }

    BssTok tok = bssLexerLexNextTok(&interp);
    basePrintf("%S\n", tok.lexeme);

    basePrintf("{g}Done\n");
    return;
}