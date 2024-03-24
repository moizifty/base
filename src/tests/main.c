#include <stdio.h>

#include "..\base\base.h"
#include "..\os\os.h"

#include "..\base\base.c"
#include "..\os\core\win32\osCoreWin32.c"
#include "..\os\core\osEntryPoint.c"

BASE_CREATE_LL_DECLS_DEFS(IntList, int);

void printFiles(BaseArena *arena, str8 path)
{
	OSFileInfo fileInfo = {0};
	OSFileFindIter *iter = OSFindFileBegin(arena, path, &(OSFileFindOptionalParams){.type = OS_FILEFIND_TYPE_TOP_LEVEL_DIR});
	for(;OSFindFileNext(arena, iter, &fileInfo);)
	{
		if (fileInfo.attrs & OS_FILEATTR_DIR)
		{
			printFiles(arena, fileInfo.path);
		}
		else
		{
			baseColPrintf("%s\n", fileInfo.path.data);
		}
	}
}

void ProgramMain(CmdLineHashMap *cmdline)
{
	BaseArena *generalArena = baseArenaAlloc(BASE_GIGABYTES(2));

	baseColPrintf("Hiiii {b}%d %s\n", 90, "sds");

	printFiles(generalArena, STR8_LIT("..\\builds"));
	
	baseColPrintf("Hiiii {b}%d %s\n", 90, "sds");
	baseColPrintf("%S\n", OSGetProgramPath(generalArena));
	baseColPrintf("%S\n", OSGetProgramDirectoryPath(generalArena));
	baseArenaFree(generalArena);
}