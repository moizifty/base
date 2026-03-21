#ifndef BSS_INTERP_H
#define BSS_INTERP_H

#include "base/baseCore.h"

#include "bssCore.h"
#include "bssLexer.h"
#include "bssAst.h"
#include "bssScope.h"

void bssInterpreterError(BssInterp *interp, BssTokPos start, BssTokPos end, i8 *fmt, ...);

BssScope *bssInterpreterCreateFuncScopeAndPushArgs(BssInterp *interp, BssTokList params, BssAstExprList args);
BssValue *bssInterpreterInterpFunc(BssInterp *interp, Arena *scopeArena, BssAstFunc *func, BssAstExpr *callingExpr);
BssValue *bssInterpreterInterpExprBinary(BssInterp *interp, Arena *scopeArena, BssAstExpr *expr);
BssValue *bssInterpreterInterpExpr(BssInterp *interp, Arena *scopeArena, BssAstExpr *expr);
BssValue *bssInterpreterInterpLValueExprAndGetSym(BssInterp *interp, Arena *scopeArena, BssAstExpr *expr, BssSymTableSlotEntry **outSym);

bool bssInterpreterInterpStmt(BssInterp *interp, Arena *scopeArena, BssAstStmt *stmt);
BssValue *bssInterpreterInterpBlock(BssInterp *interp, BssAstBlock *block, bool createBlock);

bool bssInterpreterInterpParsed(BssInterp *interp);
bool bssInterpreterInterpFile(BssInterp *interp, str8 file);

#endif