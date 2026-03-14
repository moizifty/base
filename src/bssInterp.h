#ifndef BSS_INTERP_H
#define BSS_INTERP_H

#include "base/baseCore.h"

#include "bssCore.h"
#include "bssLexer.h"
#include "bssAst.h"
#include "bssScope.h"

void bssInterpreterError(BssInterp *interp, BssTokPos start, BssTokPos end, i8 *fmt, ...);

BssValue *bssInterpreterInterpExprBinary(BssInterp *interp, BssAstExpr *expr);
BssValue *bssInterpreterInterpExpr(BssInterp *interp, BssAstExpr *expr);

bool bssInterpreterInterpStmt(BssInterp *interp, BssAstStmt *stmt);

#endif