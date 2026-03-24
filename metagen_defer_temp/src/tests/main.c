#define BASE_USE_EXCEPTION_HANDLER
#include "..\base\base.h"
#include "..\os\os.h"
#include "..\renderer\renderer.h"
#include "..\bitmap\bitmap.h"

#include "..\base\base.c"
#include "..\os\os.c"
#include "..\renderer\renderer.c"
#include "..\compression\compression.c"
#include "..\bitmap\bitmap.c"
#include "..\datastructures\bitstream.c"

#include "..\os\core\osEntryPoint.c"
#include "..\bss\bss.c"

BASE_CREATE_LL_DECLS_DEFS(IntList, int);

void printFiles(Arena *arena, str8 path)
{
	// OSFileInfo fileInfo = {0};
	// OSFileFindIter *iter = OSFindFileBegin(arena, path, &(OSFileFindOptionalParams){.type = OS_FILEFIND_TYPE_TOP_LEVEL_DIR});
	// for(;OSFindFileNext(arena, iter, &fileInfo);)
	// {
	// 	if (fileInfo.attrs & OS_FILEATTR_DIR)
	// 	{
	// 		printFiles(arena, fileInfo.path);
	// 	}
	// 	else
	// 	{
	// 		baseColPrintf("%S\n", fileInfo.path);
	// 	}
	// }
}

void ProgramMain(CmdLineHashMap *cmdline)
{
	Arena *generalArena = arenaAlloc(BASE_GIGABYTES(2));
	CLexerState clex = baseCLexerInitFromFile(generalArena, STR8_LIT("C:\\Users\\moizi\\OneDrive\\Documents\\Programming\\C\\base\\src\\tests\\main.c"));
	clex.allowWhitespace = true;
	baseCLexerLexWholeBuffer(generalArena, &clex);

	for(u64 i = 0; i < clex.lexedToks.len; i++)
	{
		basePrintf("%S", clex.lexedToks.data[i].lexeme);
	}

	basePrintf("Hiiipo\n");

	return;
	OSHandle proc = OSProcessOpen(generalArena, STR8_EMPTY, STR8_LIT("cmd /k \"echo moiz\""), null);

	if (OSIsHandleValid(proc))
	{
		str8 out, err;
		while(OSProcessReadStdoutStderr(generalArena, proc, &out, &err))
		{
		}

	}
	else
	{
		basePrintf("not valid\n");
	}
	return;
	OSNetAddrList localIps = OSNetGetLocalIpAddress(generalArena, OS_NET_ADDR_EITHER);

	Path p = pathFromStr8(generalArena, STR8_LIT("examples/moi/i/test.txt"));
	Path d = pathFromStr8(generalArena, STR8_LIT("c:/examples/moi/i/test.txt"));

	BASE_LIST_FOREACH(OSNetAddr, node, localIps)
	{
		str8 s = OSNetAddrToStr8(generalArena, *node);

		baseColPrintf("%S\n", s);
	}

	OSHandle socket = OSNetSocketCreate(OS_NET_ADDR_IPV4, OS_NET_SOCK_DATAGRAM, OS_NET_PROTOCOL_UDP);
	OSNetAddrInfoList addrs = OSNetGetAddrInfo(generalArena, STR8_LIT("0.0.0.0"), STR8_LIT("6970"), &(OSNetAddrInfo){.protoKind = OS_NET_PROTOCOL_UDP, .sockKind = OS_NET_SOCK_DATAGRAM});

	bool result = OSNetSocketBind(socket, addrs.first->addr);

	OSNetSocketClose(socket);
	
	OSGfxState *state = OSGfxInit(generalArena);
	OSHandle window = OSGfxWindowOpen(STR8_LIT("Test"), Vec2i(-1, -1), Vec2i(-1, -1));
	
	RendererState *rend = rendererInit(generalArena, state);
	RendererWindowState *wndState = rendererAttachToWindow(rend, generalArena, window);

	U8Array compressed = OSFileReadAll(generalArena, STR8_LIT("C:\\Users\\Moizi\\OneDrive\\Documents\\Programming\\C\\base\\builds\\test.txt.lz4"));

	// str8 str = STR8("255 255 255 150 150 150 255 255 255 255 255 150");
	// U8Array out = compressionLZ4MCompress(generalArena, (U8Array){.data = str.data, .len = str.len}, null);

	// U8Array uc = {.data = arenaPush(generalArena, str.len), .len = str.len};
	// compressionLZ4MUncompress(out, uc);
	// Bitmap bm =  bitmapFromPath(generalArena,STR8_LIT("C:\\Users\\Moizi\\OneDrive\\Documents\\Programming\\C\\base\\builds\\edgecase.qoi"));
	{
		U8Array f = OSFileReadAll(generalArena, STR8_LIT("C:\\Users\\Moizi\\OneDrive\\Documents\\Programming\\C\\base\\builds\\edgecase.qoi"));
		U8Array compressedBitmap = compressionLZ4MCompress(generalArena, f, null);
		
		U8Array uncompressed = {.data = arenaPush(generalArena, f.len), .len = f.len};
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

	arenaFree(generalArena);
}
