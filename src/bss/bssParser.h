#ifndef BSS_PARSER_H
#define BSS_PARSER_H

#include "bssCore.h"
#include "bssAst.h"

#define BSS_PARSER_MATCH(KIND)   ((BSS_PARSER_CURR_TOK).kind == (KIND))
#define BSS_PARSER_CURR_TOK (interp->lexer->tokArray.data[interp->lexer->currTokIndex])
#define BSS_PARSER_PEEK_TOK(N) (bssLexerPeekTok(interp, N))
#define BSS_PARSER_NEXT_TOK() (bssLexerGetNextTok(interp))

BssAstExpr *bssParserParseExprFnCall(BssInterp *interp, BssAstExpr *left, BssTok op);
BssAstExpr *bssParserParseExprSubscript(BssInterp *interp, BssAstExpr *left, BssTok op);
BssAstExpr *bssParserParseExprBinary(BssInterp *interp, BssAstExpr *left, BssTok op);
BssAstExpr *bssParserParseExpr(BssInterp *interp, u64 currPrecedence);
BssAstExprList bssParserParseExprList(BssInterp *interp, BssTokKind endKind);

BssAstStmt *bssParserParseStmtIf(BssInterp *interp);
BssAstStmt *bssParserParseStmtWhile(BssInterp *interp);
BssAstStmt *bssParserParseStmtFor(BssInterp *interp);

bool bssParserParseLexed(BssInterp *interp);
bool bssParserParseFile(BssInterp *interp, str8 file);

#endif