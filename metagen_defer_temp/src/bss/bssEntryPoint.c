#include "base/base.h"
#include "os/core/osCore.h"
#include "bss.h"

#include "base/base.c"
#include "os/core/osCore.c"
#include "bss.c"

#include "os/core/osEntryPoint.c"

void ProgramMain(CmdLineHashMap *line)
{
    if (line->originalInputs.len < 1)
    {
        baseEPrintf("{r}Expected atleast 1 arg for bss interp");
    }

    Arena *arena = arenaAllocDefault();

    BssInterp interp = {.arena = arena};

    for(Str8ListNode *flag = line->originalInputs.first->next; flag != null; flag = flag->next)
    {
        Str8ListPushLast(arena, &interp.flags, flag->val);
    }

    if(!bssInterpreterInterpFile(&interp, line->originalInputs.first->val))
    {
        baseEPrintf("{r}Failed to interp file.\n");
        return;
    }
    
    return;
}