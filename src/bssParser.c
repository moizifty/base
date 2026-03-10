#include "bssParser.h"
#include "bssLexer.h"

BssAstStmt *bssParserParseStmt(BssInterp *interp)
{
    BssAstStmt *ast = BSS_AST_STMT_ZERO;

    return ast;
}

BssAstFunc *bssParserParseFunc(BssInterp *interp)
{
    BssAstFunc *ast = BSS_AST_FUNC_ZERO;

    return ast;
}

BssAstTopLevel *bssParserParseTopLevel(BssInterp *interp)
{
    BssTok start = BSS_PARSER_CURR_TOK;

    BssAstTopLevel *ast = BSS_AST_TOP_LEVEL_ZERO; 

    switch (BSS_PARSER_CURR_TOK.kind)
    {
        case TOK_FN_KW:
        {
            ast = arenaPushType(interp->arena, BssAstTopLevel);
            ast->kind = BSS_AST_TOP_LEVEL_FUNC;
            ast->func = bssParserParseFunc(interp);
            ast->startTok = ast->func->startTok;
            ast->endTok = ast->func->startTok;
        }break;

        default:
        {
        }break;
    }

    return ast;
}

BssAstTopLevelList bssParserParseTopLevels(BssInterp *interp)
{

}

void bssParserParseLexed(BssInterp *interp)
{
    
}
void bssParserParseFile(BssInterp *interp, str8 file)
{
    if (bssLexerLexFile(interp, file))
    {
        bssParserParseLexed(interp);
    }
}