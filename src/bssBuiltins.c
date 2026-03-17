#include "base/baseMemory.h"
#include "base/baseThreads.h"
#include "os/core/osCore.h"

#include "bssBuiltins.h"
#include "bssInterp.h"


BSS_BUILTIN_FUNCTION_DEF(Print)
{
    BssValue *value = BSS_VALUE_VOID_VALUE;
    
    if (expr->call.args.len == 1)
    {
        BssAstExpr *arg = expr->call.args.first;
        BssValue *argValue = bssInterpreterInterpExpr(interp, interp->currScope->scopeArena, arg);
        if (argValue != BSS_VALUE_ZERO)
        {
            basePrintf("%S", argValue->str);
        }
    }
    else
    {
        bssInterpreterError(interp, expr->startTok.pos, expr->endTok.pos, "Builtin function 'print' requires one arguement");
    }

    return value;
}

BSS_BUILTIN_FUNCTION_DEF(Run)
{
    BssValue *value = BSS_VALUE_VOID_VALUE;
    
    if (expr->call.args.len == 1)
    {
        BssAstExpr *arg = expr->call.args.first;
        BssValue *argValue = bssInterpreterInterpExpr(interp, interp->currScope->scopeArena, arg);
        if (argValue != BSS_VALUE_ZERO &&
            argValue->kind == BSS_VALUE_STRING)
        {
            ArenaTemp temp = baseTempBegin(&interp->arena, 1);
            {
                str8 command = Str8PushFmt(temp.arena, "cmd.exe /q /k \"@echo off && %S\"", argValue->str);

                OSHandle procHandle = OSProcessOpen(temp.arena, STR8_LIT(""), command, null);
                OSProcessWait(procHandle);
                OSProcessClose(procHandle);
            }
            baseTempEnd(temp);
        }
        else
        {
            bssInterpreterError(interp, expr->startTok.pos, expr->endTok.pos, "Builtin function 'run' requires arguement of type string");
        }
    }
    else
    {
        bssInterpreterError(interp, expr->startTok.pos, expr->endTok.pos, "Builtin function 'run' requires one arguement");
    }

    return value;
}
