#ifndef BSS_BUILTINS_H
#define BSS_BUILTINS_H

#include "base/baseCore.h"
#include "bssCore.h"
#include "bssParser.h"
#include "bssAst.h"

#define BSS_BUILTIN_FUNCTION_DEF(NAME)  BssValue *bssBuiltin##NAME(BssInterp *interp, BssAstExpr *expr)

BSS_BUILTIN_FUNCTION_DEF(Print);
#endif