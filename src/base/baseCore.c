#include <locale.h>

#include "baseCore.h"
#include "baseThreads.h"

void BaseMainThreadEntry(ProgramMainFunc programMain, i64 argc, i8 **argv)
{
    BASE_UNUSED_PARAM(argc);
    BASE_UNUSED_PARAM(argv);

    setlocale(LC_ALL, ".utf8");

    BaseThreadCtx ctx = baseThreadsCreateCtx();
    baseThreadsSetCtx(&ctx);

    programMain();
}
