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
	BaseArena *builderArena = baseArenaAlloc(BASE_GIGABYTES(2));
	BaseStringBuilder builder = baseStringsCreateSB(builderArena, 2);

	str8 s = baseStringsPushStr8Fmt(generalArena, "Hi i am %d years old\n", 9001);
	baseStringsSBAppendCStr(&builder, "Hii ", -1);
	baseStringsSBAppendFmt(&builder, "everyone i am %d years old\n", 90);
	baseStringsSBAppendStr8(&builder, s);

	printf("%s\n", builder.data);

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

	baseArenaFree(generalArena);
	baseArenaFree(builderArena);
}