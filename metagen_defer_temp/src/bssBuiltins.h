#ifndef BSS_BUILTINS_H
#define BSS_BUILTINS_H

#include "base/baseCore.h"
#include "bssCore.h"
#include "bssParser.h"
#include "bssAst.h"

#define BSS_BUILTIN_FUNCTION_DEF(NAME)  BssValue *bssBuiltin##NAME(BssInterp *interp, Arena *scopeArena, BssAstExpr *expr)

BSS_BUILTIN_FUNCTION_DEF(Print);
BSS_BUILTIN_FUNCTION_DEF(Run);
BSS_BUILTIN_FUNCTION_DEF(Len);
BSS_BUILTIN_FUNCTION_DEF(ToString);
BSS_BUILTIN_FUNCTION_DEF(Qoute);
BSS_BUILTIN_FUNCTION_DEF(Join);
BSS_BUILTIN_FUNCTION_DEF(Getenv);
BSS_BUILTIN_FUNCTION_DEF(Hasflag);
#endif