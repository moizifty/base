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

void BaseMainThreadEntry(ProgramMainFunc programMain, i64 argc, i8 **argv)
{
    BASE_UNUSED_PARAM(argc);
    BASE_UNUSED_PARAM(argv);
    
    setlocale(LC_ALL, ".utf8");
    OSEnableVirtualTerminalSequenceProcessing();

    BaseThreadCtx ctx = baseThreadsCreateCtx();
    baseThreadsSetCtx(&ctx);

    Str8List argsList = {0};
    CmdLineHashMap cmdLineMap = {0};

    for(int i = 1; i < argc; i++)
    {
        Str8ListPushLastFmt(ctx.scratchArenas[0], &argsList, "%s", argv[i]);
    }

    cmdLineMap = cmdlineParseCmdLineFromStringList(ctx.scratchArenas[0], argsList);

    OSInit(ctx.scratchArenas[0]);

    programMain(&cmdLineMap);

    logClose(OSGetState()->thisProcState.processLog);
}

i64 baseColFprintf(FILE *fp, const char *fmt, ...)
{
    va_list list;
    va_start(list, fmt);

    i64 res = 0;
    BaseArenaTemp temp = baseTempBegin(null, 0);
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
                                    str8 s = baseStringsPushStr8Copy(temp.arena, baseStr8((u8*)buf, bufOccupied));
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

                        str8 s = baseStringsPushStr8Copy(temp.arena, baseStr8((u8*)buf, bufOccupied));
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

        i8 *data = baseArenaPush(temp.arena, needed);
        res = stbsp_vsnprintf(data, (int)needed, (i8 *)finalStr.data, list);

        fprintf(fp, data);
    }

    baseTempEnd(temp);

    va_end(list);

    return res;
}

i64 baseHexDigitToInt(int ch)
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
    else
    {
        baseColEPrintf("{ur} Unexpected Hex digit '%c'", ch);
    }

    return 0;
}

i64 baseBinDigitToInt(int ch)
{
    switch(ch)
    {
        case '0': return 0;
        case '1': return 1;
    }

    return -1;
}

i64 baseCStyleIntLiteralToInt(str8 str)
{
    if((str.data[0] == '0') && (str.data[1] == 'x'))
    {
        //hex number
        int64_t num = 0;
        i8 *numStr = (i8 *) str.data + 2;

        while(*numStr != '\0')
        {
            num = (num * 16) + baseHexDigitToInt(*numStr);

            numStr++;
        }

        return num;
    }
    else if((str.data[0] == '0') && (str.data[1] == 'b'))
    {
        //binary number
        int64_t num = 0;
        i8 *numStr = (i8 *) str.data + 2;

        while(*numStr != '\0')
        {
            num = (num * 2) + baseBinDigitToInt(*numStr);

            numStr++;
        }

        return num;
    }
    else return atoll((i8 *)str.data);
}