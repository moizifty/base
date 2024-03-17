#include <stdio.h>

#include "..\base\base.h"
#include "..\os\os.h"

#include "..\base\base.c"
#include "..\os\core\win32\osCoreWin32.c"
#include "..\os\core\osEntryPoint.c"

BASE_CREATE_LL_DECLS_DEFS(IntList, int);

void ProgramMain(void)
{
	BaseArena *generalArena = baseArenaAlloc(BASE_GIGABYTES(2));

	IntList list = {0};

	for(int i = 0; i < 100; i++)
	{
		IntListPushFirst(generalArena, &list, i);
	}

	Str8List strs = {0};
	for(int i = 0; i < 100; i++)
	{
		Str8ListPushLastFmt(generalArena, &strs, "Pushing string '%d'\n", i);
	}

	printf("%s\n", Str8ListJoin(generalArena, &strs, null).data);
	
	str8 output = {0};
	str8 errput = {0};
	OSRunProcessEx(generalArena, baseStr8(null, 0), STR8_LIT("cmd.exe /c \"echo hi\""), null, &output, &errput);

	printf("%s\n%s\n", output.data, errput.data);

	baseArenaFree(generalArena);
}