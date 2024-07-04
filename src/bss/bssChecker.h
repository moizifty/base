#ifndef BSS_CHECKER_H
#define BSS_CHECKER_H

#include "base\baseCore.h"
#include "bssLexer.h"
#include "bssAST.h"
#include "bssParser.h"
#include "bssTypes.h"
#include "bssScopes.h"

typedef struct BssCheckerState
{
    ASTProject *proj;
    BssTypeTable typeTable;

    BssType *runOutput;
    BssType *projectType;
}BssCheckerState;

void bssCheckerError(BSSInterpretorState *iState, BssTok tok, char *msg, ...);
void bssCheckerPrint(char *msg, ...);

BssCheckerState *bssCheckerInitFromProject(BSSInterpretorState *iState, ASTProject *proj);

void bssCheckerCheckWholeProject(BSSInterpretorState *iState, BssCheckerState *cState);

void bssCheckerCheckStmts(BSSInterpretorState *iState, BssCheckerState *cState, ASTStmtList stmts, BssScope *parentScope);
void bssCheckerCheckStmtsEx(BSSInterpretorState *iState, BssCheckerState *cState, ASTStmtList stmts, BssScope *parentScope, BssScope *newScope);
void bssCheckerCheckStmt(BSSInterpretorState *iState, BssCheckerState *cState, ASTStmt *stmt, BssScope *scope);
void bssCheckerCheckExpr(BSSInterpretorState *iState, BssCheckerState *cState, ASTExpr *expr, BssScope *scope);

#endif