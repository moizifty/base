#include "base/base.h"
#include "os/core/osCore.h"
#include "bss.h"

#include "base/base.c"
#include "os/core/osCore.c"
#include "bss.c"

#include "os/core/osEntryPoint.c"

void ProgramMain(Str8List args)
{
    Arena *arena = arenaAllocDefault();

    BssInterp interp = {.arena = arena};

    for(Str8ListNode *flag = args.first->next; flag != null; flag = flag->next)
    {
        Str8ListPushLast(arena, &interp.flags, flag->val);
    }

    if(!bssInterpreterInterpFile(&interp, args.first->val))
    {
        baseEPrintf("{r}Failed to interp file.\n");
        return;
    }
    
    return;
}