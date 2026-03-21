#ifndef BSS_CHECKER_H
#define BSS_CHECKER_H

#include "base/baseCore.h"
#include "bssLexer.h"
#include "bssAST.h"
#include "bssParser.h"
#include "bssTypes.h"
#include "bssScopes.h"


void bssCheckerError(struct BSSInterpretorState *iState, BssTok tok, char *msg, ...);
void bssCheckerPrint(char *msg, ...);

void bssCheckerInit(struct BSSInterpretorState *iState);
void bssCheckerCheckWholeProject(struct BSSInterpretorState *iState);

void bssCheckerCheckStmts(struct BSSInterpretorState *iState, ASTStmtList stmts, BssScope *parentScope);
void bssCheckerCheckStmtsEx(struct BSSInterpretorState *iState, ASTStmtList stmts, BssScope *newScope);
void bssCheckerCheckStmt(struct BSSInterpretorState *iState, ASTStmt *stmt, BssScope *scope);
void bssCheckerCheckExpr(struct BSSInterpretorState *iState, ASTExpr *expr, BssScope *scope);

#endif