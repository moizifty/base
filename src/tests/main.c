#include <stdio.h>

#include "..\base\base.h"
#include "..\os\os.h"

#include "..\base\base.c"
#include "..\os\core\win32\osCoreWin32.c"

int main(void)
{
	BaseThreadCtx ctx = baseThreadsCreateCtx();
	baseThreadsSetCtx(&ctx);

	BaseArena *arena = baseArenaAlloc(BASE_GIGABYTES(64));

	return 0;
}