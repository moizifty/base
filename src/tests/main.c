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
#include "..\compression\compression.c"
#include "..\bitmap\bitmap.c"

#include "..\os\core\osEntryPoint.c"
#include "..\bss\bss.c"

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
	
	BSSInterpretorState bs = {.lexerArena = generalArena, .parserArena = generalArena, .checkerArena = generalArena};
	BssLexerState *ls = bssLexerInitFromFile(&bs, STR8_LIT("C:\\Users\\moizi\\OneDrive\\Documents\\Programming\\C\\base\\src\\bss\\tests\\test.bss"));
	BssTokArray toks = bssLexerLexWholeBuffer(&bs, ls);

	BssParserState s = {.lexer = ls};
    ASTProject *proj = bssParserProject(&bs, &s);

	for(u64 i = 0; i < toks.len; i++)
	{
		baseColPrintf("%d: %S\n", toks.data[i].kind, toks.data[i].lexeme);
	}

	BssCheckerState *cs = bssCheckerInitFromProject(&bs, proj);
	bssCheckerCheckWholeProject(&bs, cs);

	OSGfxState *state = OSGfxInit(generalArena);
	OSHandle window = OSGfxWindowOpen(STR8_LIT("Test"), Vec2i(-1, -1), Vec2i(-1, -1));
	
	RendererState *rend = rendererInit(generalArena, state);
	RendererWindowState *wndState = rendererAttachToWindow(rend, generalArena, window);

	U8Array compressed = OSFileReadAll(generalArena, STR8_LIT("C:\\Users\\Moizi\\OneDrive\\Documents\\Programming\\C\\base\\builds\\test.txt.lz4"));

	// str8 str = STR8("255 255 255 150 150 150 255 255 255 255 255 150");
	// U8Array out = compressionLZ4MCompress(generalArena, (U8Array){.data = str.data, .len = str.len}, null);

	// U8Array uc = {.data = baseArenaPush(generalArena, str.len), .len = str.len};
	// compressionLZ4MUncompress(out, uc);
	// Bitmap bm =  bitmapFromPath(generalArena,STR8_LIT("C:\\Users\\Moizi\\OneDrive\\Documents\\Programming\\C\\base\\builds\\edgecase.qoi"));
	{
		U8Array f = OSFileReadAll(generalArena, STR8_LIT("C:\\Users\\Moizi\\OneDrive\\Documents\\Programming\\C\\base\\builds\\edgecase.qoi"));
		U8Array compressedBitmap = compressionLZ4MCompress(generalArena, f, null);
		
		U8Array uncompressed = {.data = baseArenaPush(generalArena, f.len), .len = f.len};
		compressionLZ4MUncompress(compressedBitmap, uncompressed);
	}
	

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
