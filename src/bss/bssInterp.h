#ifndef BSS_INTERP_H
#define BSS_INTERP_H

#include "bssCore.h"
#include "bssLexer.h"
#include "bssChecker.h"
#include "bssTypes.h"
#include "bssScopes.h"

void bssInterpError(BSSInterpretorState *iState, BssTok start, BssTok end, char *msg, ...);
void bssInterpPrint(char *msg, ...);

void bssInterpWholeProject(BSSInterpretorState *iState, BssCheckerState *cState);

void bssInterpStmts(BSSInterpretorState *iState, BssCheckerState *cState, ASTStmtList stmts);
void bssInterpStmt(BSSInterpretorState *iState, BssCheckerState *cState, ASTStmt *stmt);
Str8List bssInterpProcessFormatString(BSSInterpretorState *iState, BssCheckerState *cState, ASTExpr *stringLit, str8 in);
void bssInterpExpr(BSSInterpretorState *iState, BssCheckerState *cState, ASTExpr *expr);

#endif