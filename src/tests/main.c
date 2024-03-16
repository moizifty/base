#include <stdio.h>

#include "..\base\base.h"
#include "..\os\os.h"

#include "..\base\base.c"
#include "..\os\core\win32\osCoreWin32.c"
#include "..\os\core\osEntryPoint.c"

BASE_CREATE_LL_STRUCT(IntList, int);

void ProgramMain(void)
{
	BaseArena *arena = baseArenaAlloc(BASE_GIGABYTES(64));

	BaseStringBuilder builder = baseStringsCreateSB(arena, 2);

	baseStringsSBAppendCStr(&builder, "Hii ", -1);
	baseStringsSBAppendFmt(&builder, "everyone i am %d years old", 90);

	printf("%s\n", builder.data);


	IntListNode *first = NULL;
	IntListNode *last = NULL;

	for(int i = 0; i < 10; i++)
	{
		IntListNode *m = baseArenaPush(arena, sizeof(IntListNode));
		m->val = i;

		BaseDllNodeInsert(first, last, last, m);
	}
	
	
	for(int i = 0; i < 10; i++)
	{
		IntListNode *m = baseArenaPush(arena, sizeof(IntListNode));
		m->val = i + 10;
	}
}