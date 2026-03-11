#include "bssAst.h"

readonly BssAstExpr gBssAstExprEmpty = {0};
readonly BssAstStmt gBssAstStmtEmpty = {0};
readonly BssAstBlock gBssAstBlockEmpty = {0};
readonly BssAstFunc gBssAstFuncEmpty = {0};
readonly BssAstTopLevel gBssAstTopLevelEmpty = {0};

BASE_CREATE_EFFICIENT_LL_DEFS(BssAstStmtList, BssAstStmt)
BASE_CREATE_EFFICIENT_LL_DEFS(BssAstTopLevelList, BssAstTopLevel)

BssAstExpr *bssAllocExpr(BssInterp *interp, BssTok start, BssTok end, BssAstExprKind kind)
{
    BssAstExpr *ast = arenaPushType(interp->arena, BssAstExpr);
    ast->startTok = start;
    ast->endTok = end;
    ast->kind = kind;

    return ast;
}
BssAstExpr *bssAllocExprLit(BssInterp *interp, BssTok start, BssTok end, BssTok lit)
{
    BssAstExpr *ast = bssAllocExpr(interp, start, end, BSS_AST_EXPR_LIT);
    ast->lit = lit;

    return ast;
}
BssAstExpr *bssAllocExprIden(BssInterp *interp, BssTok start, BssTok end, BssTok iden)
{
    BssAstExpr *ast = bssAllocExpr(interp, start, end, BSS_AST_EXPR_IDEN);
    ast->iden = iden;

    return ast;
}
BssAstExpr *bssAllocExprBinary(BssInterp *interp, BssTok start, BssTok end, BssAstExpr *lhs, BssAstExpr *rhs, BssTok op)
{
    BssAstExpr *ast = bssAllocExpr(interp, start, end, BSS_AST_EXPR_BINARY);
    ast->bin.left = lhs;
    ast->bin.right = rhs;
    ast->bin.op = op;

    return ast; 
}
BssAstExpr *bssAllocExprUnary(BssInterp *interp, BssTok start, BssTok end, BssAstExpr *rhs, BssTok op)
{
    BssAstExpr *ast = bssAllocExpr(interp, start, end, BSS_AST_EXPR_UNARY);
    ast->unary.expr = rhs;
    ast->unary.op = op;

    return ast; 
}