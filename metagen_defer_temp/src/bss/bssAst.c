#include "bssAst.h"

readonly BssAstExpr gBssAstExprEmpty = {0};
readonly BssAstStmt gBssAstStmtEmpty = {0};
readonly BssAstBlock gBssAstBlockEmpty = {0};
readonly BssAstFunc gBssAstFuncEmpty = {0};
readonly BssAstTopLevel gBssAstTopLevelEmpty = {0};

BASE_CREATE_EFFICIENT_LL_DEFS(BssAstStmtList, BssAstStmt)
BASE_CREATE_EFFICIENT_LL_DEFS(BssAstExprList, BssAstExpr)
BASE_CREATE_EFFICIENT_LL_DEFS(BssAstTopLevelList, BssAstTopLevel)
BASE_CREATE_EFFICIENT_LL_DEFS(BssAstNamedExprList, BssAstNamedExpr)

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
BssAstExpr *bssAllocExprFnCall(BssInterp *interp, BssTok start, BssTok end, BssAstExpr *lhs, BssAstExprList args)
{
    BssAstExpr *ast = bssAllocExpr(interp, start, end, BSS_AST_EXPR_FUNCCALL);
    ast->call.lhs = lhs;
    ast->call.args = args;

    return ast; 
}
BssAstExpr *bssAllocExprCompound(BssInterp *interp, BssTok start, BssTok end, BssAstNamedExprList compound)
{
    BssAstExpr *ast = bssAllocExpr(interp, start, end, BSS_AST_EXPR_COMPOUND);
    ast->compound = compound;

    return ast; 
}
BssAstExpr *bssAllocExprSubscript(BssInterp *interp, BssTok start, BssTok end, BssAstExpr *container, BssAstExpr *index)
{
    BssAstExpr *ast = bssAllocExpr(interp, start, end, BSS_AST_EXPR_SUBSCRIPT);
    ast->subscript.container = container;
    ast->subscript.index = index;

    return ast; 
}
