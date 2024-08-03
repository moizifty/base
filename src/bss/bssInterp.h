#ifndef BSS_INTERP_H
#define BSS_INTERP_H

#include "bssCore.h"
#include "bssLexer.h"
#include "bssChecker.h"
#include "bssTypes.h"
#include "bssScopes.h"

void bssInterpError(struct BSSInterpretorState *iState, BssTok start, BssTok end, char *msg, ...);
void bssInterpPrint(char *msg, ...);

void bssInterpWholeProject(struct BSSInterpretorState *iState);

void bssInterpStmts(struct BSSInterpretorState *iState, ASTStmtList stmts);
void bssInterpStmt(struct BSSInterpretorState *iState, ASTStmt *stmt);
Str8List bssInterpProcessFormatString(struct BSSInterpretorState *iState, ASTExpr *stringLit, str8 in);
void bssInterpExpr(struct BSSInterpretorState *iState, ASTExpr *expr);

#endif