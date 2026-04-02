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
        ArenaTemp temp = baseTempBegin(&scopeArena, 1);
        {
            BssAstExpr *arg = expr->call.args.first;
            BssValue *argValue = bssInterpreterInterpExpr(interp, temp.arena, arg);
            if (argValue != BSS_VALUE_ZERO)
            {
                basePrintf("%S", Str8FromBssValue(temp.arena, argValue));
            }
        }
        baseTempEnd(temp);
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
        BssValue *argValue = bssInterpreterInterpExpr(interp, scopeArena, arg);
        if (argValue != BSS_VALUE_ZERO &&
            argValue->kind == BSS_VALUE_STRING)
        {
            ArenaTemp temp = baseTempBegin(&interp->arena, 1);
            {
                str8 command = Str8PushFmt(temp.arena, "cmd.exe /q /k \"@echo off && %S\"", argValue->str);

                OSHandle procHandle = OSProcessOpen(temp.arena, STR8_LIT(""), command, null);

                str8 stdoutStr = STR8_EMPTY;
                str8 stderrStr = STR8_EMPTY;
                Str8List outStrList = {0};
                Str8List errStrList = {0};
                while(OSProcessReadStdoutStderr(temp.arena, procHandle, &stdoutStr, &stderrStr))
                {
                    if (!BASE_NULL_OR_EMPTY(stdoutStr))
                    {
                        Str8ListPushLast(temp.arena, &outStrList, stdoutStr);
                    }

                    if (!BASE_NULL_OR_EMPTY(stderrStr))
                    {
                        Str8ListPushLast(temp.arena, &errStrList, stderrStr);
                    }
                }

                OSProcessWait(procHandle);
                OSProcessClose(procHandle);

                value = bssAllocValueObj(scopeArena, bssAllocScope(scopeArena, null, false));

                BssSymTableSlotEntry *outEntry = null;
                bssScopePushEntry(value->obj, STR8_LIT("out"), &outEntry);
                outEntry->value = bssAllocValueStr8(scopeArena, Str8ListJoin(scopeArena, &outStrList, null));

                BssSymTableSlotEntry *errEntry = null;
                bssScopePushEntry(value->obj, STR8_LIT("err"), &errEntry);
                errEntry->value = bssAllocValueStr8(scopeArena, Str8ListJoin(scopeArena, &errStrList, null));
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

BSS_BUILTIN_FUNCTION_DEF(ToString)
{
    BssValue *value = BSS_VALUE_VOID_VALUE;
    
    if (expr->call.args.len == 1)
    {
        BssAstExpr *arg = expr->call.args.first;
        BssValue *argValue = bssInterpreterInterpExpr(interp, scopeArena, arg);
        if (argValue != BSS_VALUE_ZERO)
        {
            value = bssAllocValueStr8(scopeArena, Str8FromBssValue(scopeArena, argValue));
        }
    }
    else
    {
        bssInterpreterError(interp, expr->startTok.pos, expr->endTok.pos, "Builtin function 'tostring' requires one arguement");
    }

    return value;
}

BSS_BUILTIN_FUNCTION_DEF(Len)
{
    BssValue *value = BSS_VALUE_VOID_VALUE;
    
    if (expr->call.args.len == 1)
    {
        BssAstExpr *arg = expr->call.args.first;
        BssValue *argValue = bssInterpreterInterpExpr(interp, scopeArena, arg);
        if (argValue != BSS_VALUE_ZERO &&
            (argValue->kind == BSS_VALUE_STRING || argValue->kind == BSS_VALUE_ARRAY))
        {
            value = bssAllocValueInt(scopeArena, (argValue->kind == BSS_VALUE_ARRAY) ? argValue->array.len : argValue->str.len);
        }
        else
        {
            bssInterpreterError(interp, expr->startTok.pos, expr->endTok.pos, "Builtin function 'len' requires arguement of type string or array");
        }
    }
    else
    {
        bssInterpreterError(interp, expr->startTok.pos, expr->endTok.pos, "Builtin function 'len' requires one arguement");
    }

    return value;
}

BSS_BUILTIN_FUNCTION_DEF(Join)
{
    BssValue *value = BSS_VALUE_VOID_VALUE;
    
    if (expr->call.args.len == 2)
    {
        BssAstExpr *arrArg = expr->call.args.first;
        BssAstExpr *sepArg = expr->call.args.first->next;
        BssValue *arrVal = bssInterpreterInterpExpr(interp, scopeArena, arrArg);
        BssValue *sepVal = bssInterpreterInterpExpr(interp, scopeArena, sepArg);
        if (arrVal != BSS_VALUE_ZERO &&
            sepVal != BSS_VALUE_ZERO && 
            arrVal->kind == BSS_VALUE_ARRAY &&
            sepVal->kind == BSS_VALUE_STRING)
        {
            ArenaTemp temp = baseTempBegin(null, 0);

            Str8List values = {0};
            BASE_LIST_FOREACH(BssValue, val, arrVal->array)
            {
                Str8ListPushLast(temp.arena, &values, Str8FromBssValue(temp.arena, val));
            }
            
            value = bssAllocValueStr8(scopeArena, Str8ListJoin(scopeArena, &values, &(Str8ListJoinParams){.sep=sepVal->str}));

            baseTempEnd(temp);

        }
        else
        {
            bssInterpreterError(interp, expr->startTok.pos, expr->endTok.pos, "Builtin function 'join' requires arguement of type array followed by a argument string");
        }
    }
    else
    {
        bssInterpreterError(interp, expr->startTok.pos, expr->endTok.pos, "Builtin function 'join' requires 2 arguements");
    }

    return value;
}

BSS_BUILTIN_FUNCTION_DEF(Qoute)
{
    BssValue *value = BSS_VALUE_VOID_VALUE;
    
    if (expr->call.args.len == 1)
    {
        BssAstExpr *arrArg = expr->call.args.first;
        BssValue *arrVal = bssInterpreterInterpExpr(interp, scopeArena, arrArg);
        if (arrVal != BSS_VALUE_ZERO)
        {
            ArenaTemp temp = baseTempBegin(&scopeArena, 1);

            if (arrVal->kind == BSS_VALUE_ARRAY)
            {
                BssValueList values = {0};

                BASE_LIST_FOREACH(BssValue, val, arrVal->array)
                {
                    str8 str = Str8PushFmt(scopeArena, "\"%S\"", Str8FromBssValue(temp.arena, val));
                    BssValue *v = bssAllocValueStr8(scopeArena, str);
                    BssValueListPushNodeLast(&values, v);
                }
                
                value = bssAllocValueArray(scopeArena, values);
            }
            else
            {
                value = bssAllocValueStr8(scopeArena, Str8PushFmt(scopeArena, "\"%S\"", Str8FromBssValue(temp.arena, arrVal)));
            }


            baseTempEnd(temp);

        }
        else
        {
            bssInterpreterError(interp, expr->startTok.pos, expr->endTok.pos, "Builtin function 'qoute' requires arguement of type array");
        }
    }
    else
    {
        bssInterpreterError(interp, expr->startTok.pos, expr->endTok.pos, "Builtin function 'qoute' requires 1 arguement");
    }

    return value;
}

BSS_BUILTIN_FUNCTION_DEF(Getenv)
{
    BssValue *value = BSS_VALUE_VOID_VALUE;
    
    if (expr->call.args.len == 1)
    {
        BssAstExpr *arg = expr->call.args.first;
        BssValue *argValue = bssInterpreterInterpExpr(interp, scopeArena, arg);
        if (argValue != BSS_VALUE_ZERO && argValue->kind == BSS_VALUE_STRING)
        {
            value = bssAllocValueStr8(scopeArena, OSGetEnvironmentVar(scopeArena, argValue->str));
        }
        else
        {
            bssInterpreterError(interp, expr->startTok.pos, expr->endTok.pos, "Builtin function 'getenv' requires arguement of type string");
        }
    }
    else
    {
        bssInterpreterError(interp, expr->startTok.pos, expr->endTok.pos, "Builtin function 'getenv' requires one arguement");
    }

    return value;
}

BSS_BUILTIN_FUNCTION_DEF(Hasflag)
{
    BssValue *value = BSS_VALUE_VOID_VALUE;
    
    if (expr->call.args.len == 1)
    {
        BssAstExpr *arg = expr->call.args.first;
        BssValue *argValue = bssInterpreterInterpExpr(interp, scopeArena, arg);
        if (argValue != BSS_VALUE_ZERO && 
            argValue->kind == BSS_VALUE_STRING)
        {
            bool found = false;
            BASE_LIST_FOREACH(Str8ListNode, node, interp->flags)
            {
                if (Str8Equals(node->val, argValue->str, 0))
                {
                    found = true;
                    break;
                }
            }
            value = bssAllocValueBool(scopeArena, found);
        }
        else
        {
            bssInterpreterError(interp, expr->startTok.pos, expr->endTok.pos, "Builtin function 'hasflag' requires arguement of type string");
        }
    }
    else
    {
        bssInterpreterError(interp, expr->startTok.pos, expr->endTok.pos, "Builtin function 'hasflag' requires one arguement");
    }

    return value;
}

BSS_BUILTIN_FUNCTION_DEF(Pathexists)
{
    BssValue *value = BSS_VALUE_VOID_VALUE;
    
    if (expr->call.args.len == 1)
    {
        BssAstExpr *arg = expr->call.args.first;
        BssValue *argValue = bssInterpreterInterpExpr(interp, scopeArena, arg);
        if (argValue != BSS_VALUE_ZERO && 
            argValue->kind == BSS_VALUE_STRING)
        {
            value = bssAllocValueBool(scopeArena, OSPathExists(argValue->str));
        }
        else
        {
            bssInterpreterError(interp, expr->startTok.pos, expr->endTok.pos, "Builtin function 'hasflag' requires arguement of type string");
        }
    }
    else
    {
        bssInterpreterError(interp, expr->startTok.pos, expr->endTok.pos, "Builtin function 'hasflag' requires one arguement");
    }

    return value;
}