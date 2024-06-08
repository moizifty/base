#ifndef BSS_PARSER_H
#define BSS_PARSER_H

#include "bssCore.h"
#include "bssLexer.h"
#include "bssAST.h"

#define PARSER_CURR_TOK(pCS)    ((pCS)->lexer->tok)
#define PARSER_ADV_TOK(pCS)    ((pCS)->lexer->tok = bssLexerNext((pCS)->lexer))

typedef struct BssParserState
{
    BssLexerState *lexer;
}BssParserState;

void bssParserError(BSSInterpretorState *iState, BssTok tok, char *msg, ...);

ASTProject *bssParserProject(BSSInterpretorState *iState, BssParserState *pState);

ASTStmt *bssParserStmt(BSSInterpretorState *iState, BssParserState *pState);
ASTStmtList bssParserStmtList(BSSInterpretorState *iState, BssParserState *pState, BssTokKind tokToEndParse);

ASTBlock *bssParserBlock(BSSInterpretorState *iState, BssParserState *pState);

ASTExpr *bssParserExpr(BSSInterpretorState *iState, BssParserState *pState);
ASTExpr *bssParserExprLogical(BSSInterpretorState *iState, BssParserState *pState);
ASTExpr *bssParserExprPost(BSSInterpretorState *iState, BssParserState *pState);
ASTExpr *bssParserExprPrimary(BSSInterpretorState *iState, BssParserState *pState);

ASTNamedExpr *bssParserNamedExpr(BSSInterpretorState *iState, BssParserState *pState);
ASTNamedExprList bssParserNamedExprList(BSSInterpretorState *iState, BssParserState *pState, BssTokKind tokToEndParse);

#endif