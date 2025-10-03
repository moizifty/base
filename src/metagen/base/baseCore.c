#include <locale.h>
#include <stdio.h>
#include <stdarg.h>

#include "baseCore.h"
#include "baseMemory.h"
#include "baseThreads.h"
#include "baseStrings.h"
#include "baseCmdLine.h"
#include "..\os\core\osCore.h"

#define STB_SPRINTF_IMPLEMENTATION
#include "thirdparty\ts_stb_sprintf.h"

BASE_CREATE_LL_DEFS(U8ArrayList, U8Array);

void U8ChunkListPushLast(Arena *arena, U8ChunkList *l, u8 u)
{
    if(!BASE_ANY_PTR(l) || (l->last->chunk.len >= l->last->cap))
    {
        U8ChunkListNode *n = arenaPushType(arena, U8ChunkListNode);
        n->cap = l->defaultCap == 0 ? BASE_U8CHUNKLIST_DEFAULT_CAP : l->defaultCap;
        n->chunk.data = arenaPushArray(arena, u8, n->cap);
        n->chunk.len = 0;

        BasePtrListNodePushLast(l, n);
    }
    
    l->last->chunk.data[l->last->chunk.len] = u;
    l->last->chunk.len += 1;
    l->totalLen += 1;
}
void U8ChunkListPushStr8Last(Arena *arena, U8ChunkList *l, str8 str)
{
    for(u64 i = 0; i < str.len; i++)
    {
        U8ChunkListPushLast(arena, l, str.data[i]);
    }
}

U8Array U8ChunkListFlattenToArray(Arena *arena, U8ChunkList *l)
{
    U8Array flattened = {0};

    if (!BASE_ANY_PTR(l))
    {
        return flattened;
    }

    flattened.data = arenaPushArray(arena, u8, l->totalLen);
    flattened.len = l->totalLen;

    u64 i = 0;
    BASE_PTR_LIST_FOREACH(U8ChunkListNode, chunk, l)
    {
        for(u64 j = 0; j < chunk->chunk.len; j++)
        {
            flattened.data[i++] = chunk->chunk.data[j];
        }
    }

    return flattened;
}

void BaseMainThreadEntry(ProgramMainFunc programMain, i64 argc, i8 **argv)
{
    BASE_UNUSED_PARAM(argc);
    BASE_UNUSED_PARAM(argv);
    
    setlocale(LC_ALL, ".utf8");
    OSEnableVirtualTerminalSequenceProcessing();

    BaseThreadCtx ctx = baseThreadsCreateCtx();
    baseThreadsSetCtx(&ctx);

    baseThreadsSetName(STR8_LIT("Main Thread"));
    OSSetThreadDebuggerName(OSGetCurrentThread(), STR8_LIT("Main Thread"));

    Str8List argsList = {0};
    CmdLineHashMap cmdLineMap = {0};

    for(int i = 1; i < argc; i++)
    {
        Str8ListPushLastFmt(ctx.scratchArenas[0], &argsList, "%s", argv[i]);
    }

    cmdLineMap = cmdlineParseCmdLineFromStringList(ctx.scratchArenas[0], argsList);

    OSInit(ctx.scratchArenas[0]);

    programMain(&cmdLineMap);

    logThreadOutputToFile();
}

i64 baseColFprintf(FILE *fp, const char *fmt, ...)
{
    va_list list;
    va_start(list, fmt);

    i64 res = 0;
    ArenaTemp temp = baseTempBegin(null, 0);
    {
        Str8List strList = {0};
        char buf[100] = {0};
        i64 bufOccupied = 0;
        bool escape = false;

        for(i64 i = 0; fmt[i]; i++)
        {
            switch(fmt[i])
            {
                case '{':
                {
                    if (!escape)
                    {
                        i8 nextChar = fmt[i + 1];
                        switch(nextChar)
                        {
                            case '{': escape = true; break;
                            default:
                            {
                                if (bufOccupied > 0)
                                {
                                    str8 s = Str8PushCopy(temp.arena, baseStr8((u8*)buf, bufOccupied));
                                    Str8ListPushLast(temp.arena, &strList, s);
                                    bufOccupied = 0;
                                }
                                

                                i64 k = 0;
                                for(k = i + 1; fmt[k] != '}'; k++)
                                {
                                    switch(fmt[k])
                                    {
                                        case 'r':
                                        {
                                            Str8ListPushLastFmt(temp.arena, &strList, BASE_TERMINAL_FG_RED_CODE, -1);
                                        }break;
                                        case 'g':
                                        {
                                            Str8ListPushLastFmt(temp.arena, &strList, BASE_TERMINAL_FG_GREEN_CODE, -1);
                                        }break;
                                        case 'b':
                                        {
                                            Str8ListPushLastFmt(temp.arena, &strList, BASE_TERMINAL_FG_BLUE_CODE, -1);
                                        }break;
                                        case 'o':
                                        {
                                            Str8ListPushLastFmt(temp.arena, &strList, BASE_TERMINAL_FG_ORANGE_1_CODE, -1);
                                        }break;
                                        case 'u':
                                        {
                                            Str8ListPushLastFmt(temp.arena, &strList, BASE_TERMINAL_UNDERLINE_CODE, -1);
                                        }break;
                                        case 'B':
                                        {
                                            Str8ListPushLastFmt(temp.arena, &strList, BASE_TERMINAL_BOLD_CODE, -1);
                                        }break;
                                    }
                                }
                                if (k == i + 1)
                                {
                                    Str8ListPushLastFmt(temp.arena, &strList, BASE_TERMINAL_RESET_CODE, -1);
                                }

                                i = k;
                            }break;
                        }

                        break;
                    }
                    // fallthrough if its escape so it prints it like normal in default case
                    
                }
                default:
                {
                    escape = false;
                    if (bufOccupied + 1 < BASE_ARRAY_SIZE(buf))
                    {
                        buf[bufOccupied++] = fmt[i];
                    }
                    else
                    {
                        buf[bufOccupied] = fmt[i];

                        str8 s = Str8PushCopy(temp.arena, baseStr8((u8*)buf, bufOccupied));
                        Str8ListPushLast(temp.arena, &strList, s);
                        bufOccupied = 0;
                    }
                }break;
            }
        }

        if(bufOccupied > 0)
        {
            Str8ListPushLast(temp.arena, &strList, baseStr8((u8*)buf, bufOccupied));
        }

        Str8ListPushLastFmt(temp.arena, &strList, BASE_TERMINAL_RESET_CODE, -1);

        str8 finalStr = Str8ListJoin(temp.arena, &strList, null);
        i64 needed = stbsp_vsnprintf(null, 0, (i8 *)finalStr.data, list) + 1;

        i8 *data = arenaPush(temp.arena, needed);
        res = stbsp_vsnprintf(data, (int)needed, (i8 *)finalStr.data, list);

        fprintf(fp, data);
    }

    baseTempEnd(temp);

    va_end(list);

    return res;
}

i8 baseCharHexDigitToU8(u8 ch)
{
    if((ch >= '0') && (ch <= '9'))
    {
        return ch - '0';
    }
    else if((ch >= 'a') && (ch <= 'f'))
    {
        return 10 + (ch - 'a');
    }
    else if((ch >= 'A') && (ch <= 'Z'))
    {
        return 10 + (ch - 'A');
    }

    return -1;
}

i8 baseCharBinDigitToU8(u8 ch)
{
    switch(ch)
    {
        case '0': return 0;
        case '1': return 1;
    }

    return -1;
}

i8 baseCharDigitToU8(u8 ch)
{
    return ch - '0';
}

u8* baseMemcpyBigEndian(void *dst, void* src, u64 size)
{
    for(u64 i = 0; i < size; i++)
    {
        ((u8 *)dst)[(size - 1) - i] = ((u8 *)src)[i];
    }

    return dst;
}
i16 baseConvertToBigEndianI16(i16 num)
{
    return (num << 8) | (num >> 8);
}
i16 baseConvertToLittleEndianI16(i16 num)
{
    return (num >> 8) | (num << 8);
}
i32 baseConvertToBigEndianI32(i32 num)
{
    i8 *bytes = (i8*)&num;
    return (bytes[3] << 24) | (bytes[2] << 16) | (bytes[1] << 8) | bytes[0];
}
i32 baseConvertToLittleEndianI32(i32 num)
{
    i8 *bytes = (i8*)&num;
    return (bytes[0] << 24) | (bytes[1] << 16) | (bytes[2] << 8) | bytes[3];
}

u16 baseConvertToBigEndianU16(u16 num)
{
    return (num << 8) | (num >> 8);
}
u16 baseConvertToLittleEndianU16(u16 num)
{
    return (num >> 8) | (num << 8);
}
u32 baseConvertToBigEndianU32(u32 num)
{
    u8 *bytes = (u8*)&num;
    return (bytes[3] << 24) | (bytes[2] << 16) | (bytes[1] << 8) | bytes[0];
}
u32 baseConvertToLittleEndianU32(u32 num)
{
    u8 *bytes = (u8*)&num;
    return (bytes[0] << 24) | (bytes[1] << 16) | (bytes[2] << 8) | bytes[3];
}