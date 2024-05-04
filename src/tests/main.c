#include <stdio.h>

#define BASE_USE_EXCEPTION_HANDLER
#include "..\base\base.h"
#include "..\os\os.h"
#include "..\renderer\renderer.h"
#include "..\bitmap\bitmap.h"

#include "..\base\base.c"
#include "..\os\os.c"
#include "..\renderer\renderer.c"
#include "..\log\log.c"
#include "..\bitmap\bitmap.c"

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
	
	OSGfxState *state = OSGfxInit(generalArena);
	OSHandle window = OSGfxWindowOpen(STR8_LIT("Test"), Vec2i(-1, -1), Vec2i(-1, -1));

	RendererState *rend = rendererInit(generalArena, state);
	RendererWindowState *wndState = rendererAttachToWindow(rend, generalArena, window);

	Bitmap bm =  bitmapFromPath(generalArena,STR8_LIT("C:\\Users\\Moizi\\OneDrive\\Documents\\Programming\\C\\base\\builds\\test3.dds"));
	
	bool quit = false;
	while(!quit)
	{
		OSGfxProcessEvents(generalArena);
		
		range2i r = OSClientRectFromWindow(window);
		vec2i d = Range2iDim(r);
		rendererWindowBegin(rend, wndState, d);
		rendererWindowEnd(rend, wndState);

		BASE_LIST_FOREACH(OSEvent, event, gOSWin32TLEvents)
		{
			if(event->kind == OS_EVENT_WINDOW_CLOSE)
			{
				quit = true;
				break;
			}
		}

		if(!wndState->preformedFirstPaint)
		{
			OSGfxWindowFirstPaint(window);
		}
	}

	baseArenaFree(generalArena);
}
