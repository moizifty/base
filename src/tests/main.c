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
			baseColPrintf("%S\n", fileInfo.path);
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

	mat4f i = MAT4F_IDENTITY;
	i = quatfToMat4f(quatfGiveRotateAxis(Vec3f(1, 0, 0), baseDegToRadF32(90.0f))); //mat4fGiveRotateX(baseDegToRadF32(45));

	quatf v = quatfFromEulerYXZ(Vec3f(1, 5, 2)); //quatfRotateYXZVec3f(Vec3f(0, 1, 0), baseDegToRadF32(45), 0, 0);

	printf("(%f %f %f %f)\n", v.x, v.y, v.z, v.w);
	baseArenaFree(generalArena);
}