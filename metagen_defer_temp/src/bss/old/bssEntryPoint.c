#include "base/base.h"

#include "os/os.h"
#include "bss\bss.h"

#include "base/base.c"
#include "os/os.c"
#include "bss\bss.c"

#include "os/core/osEntryPoint.c"

void ProgramMain(CmdLineHashMap *cmdline)
{
    if (!BASE_ANY(cmdline->originalInputs))
    {
        baseColEPrintf("No arguments passed, expected atleast a file.\n");
        return;
    }

    Arena *arena = arenaAlloc(BASE_GIGABYTES(8));

    Str8List buildFlags = {0};
    str8 file = cmdline->originalInputs.first->val;

    for(Str8ListNode *n = cmdline->originalInputs.first->next; n != null; n = n->next)
    {
        Str8ListPushLast(arena, &buildFlags, n->val);
    }
    
    BSSInterpretorState iState = 
    {
        .lexerArena = arena,
        .parserArena = arena,
        .checkerArena = arena,
        .buildFlags = buildFlags
    };
    bssInterpFile(&iState, file);
}