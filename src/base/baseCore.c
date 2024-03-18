#include <locale.h>
#include <stdio.h>
#include <stdarg.h>

#include "baseCore.h"
#include "baseMemory.h"
#include "baseThreads.h"
#include "baseStrings.h"

void BaseMainThreadEntry(ProgramMainFunc programMain, i64 argc, i8 **argv)
{
    BASE_UNUSED_PARAM(argc);
    BASE_UNUSED_PARAM(argv);

    setlocale(LC_ALL, ".utf8");

    BaseThreadCtx ctx = baseThreadsCreateCtx();
    baseThreadsSetCtx(&ctx);

    programMain();
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

                                        case 'b':
                                        {
                                            Str8ListPushLastFmt(temp.arena, &strList, BASE_TERMINAL_FG_BLUE_CODE, -1);
                                        }break;

                                        case 'g':
                                        {
                                            Str8ListPushLastFmt(temp.arena, &strList, BASE_TERMINAL_FG_GREEN_CODE, -1);
                                        }break;
                                    }
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
        res = vfprintf(fp, (char*)finalStr.data, list);

    }

    baseTempEnd(temp);

    va_end(list);

    return res;
}
