#include "bssBuiltins.h"
#include "bssInterp.h"

BSS_BUILTIN_FUNCTION_DEF(Print)
{
    BssValue *value = BSS_VALUE_VOID_VALUE;
    
    if (expr->call.args.len == 1)
    {
        BssAstExpr *arg = expr->call.args.first;
        BssValue *argValue = bssInterpreterInterpExpr(interp, arg);
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
