#include "baseCore.h"
#include "baseThreads.h"

void BaseMainThreadEntry(ProgramMainFunc programMain, u64 argc, u8 **argv)
{
    BaseThreadCtx ctx = baseThreadsCreateCtx();
    baseThreadsSetCtx(&ctx);

    programMain();
}
