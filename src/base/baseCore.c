#include <locale.h>

#include "baseCore.h"
#include "baseThreads.h"

void BaseMainThreadEntry(ProgramMainFunc programMain, u64 argc, u8 **argv)
{
    setlocale(LC_ALL, ".utf8");

    BaseThreadCtx ctx = baseThreadsCreateCtx();
    baseThreadsSetCtx(&ctx);

    programMain();
}
