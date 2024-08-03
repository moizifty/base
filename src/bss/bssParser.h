#ifndef BSS_PARSER_H
#define BSS_PARSER_H

#include "bssCore.h"
#include "bssLexer.h"
#include "bssAST.h"

#define PARSER_CURR_TOK(iCS)    ((iCS)->lState.tok)
#define PARSER_ADV_TOK(iCS)    ((iCS)->lState.tok = bssLexerNext((iCS)))

void bssParserError(struct BSSInterpretorState *iState, BssTok tok, char *msg, ...);

void bssParserProject(struct BSSInterpretorState *iState);

ASTStmt *bssParserStmt(struct BSSInterpretorState *iState);
ASTStmtList bssParserStmtList(struct BSSInterpretorState *iState, BssTokKind tokToEndParse);

ASTBlock *bssParserBlock(struct BSSInterpretorState *iState);

ASTExpr *bssParserExpr(struct BSSInterpretorState *iState);
ASTExpr *bssParserExprEq(struct BSSInterpretorState *iState);
ASTExpr *bssParserExprLogical(struct BSSInterpretorState *iState);
ASTExpr *bssParserExprPost(struct BSSInterpretorState *iState);
ASTExpr *bssParserExprPrimary(struct BSSInterpretorState *iState);

ASTNamedExpr *bssParserNamedExpr(struct BSSInterpretorState *iState);
ASTNamedExprList bssParserNamedExprList(struct BSSInterpretorState *iState, BssTokKind tokToEndParse);

#endif