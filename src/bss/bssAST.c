#include "bssAST.h"

BASE_CREATE_EFFICIENT_LL_DEFS(ASTStmtList, ASTStmt);
BASE_CREATE_EFFICIENT_LL_DEFS(ASTNamedExprList, ASTNamedExpr);

ASTStmt *bssAllocASTStmt(Arena *arena, ASTStmtKind kind, BssTok startTok, BssTok endTok)
{
    ASTStmt *s = arenaPushType(arena, ASTStmt);
    s->kind = kind;
    s->startTok = startTok;
    s->endTok = endTok;

    return s;
}
ASTStmt *bssAllocASTStmtAssign(Arena *arena, ASTExpr *lhs, ASTExpr *rhs)
{
    ASTStmt *s = bssAllocASTStmt(arena, AST_STMT_ASSIGN, lhs->startTok, rhs->endTok);
    s->assign.lhs = lhs;
    s->assign.rhs = rhs;

    return s;
}
ASTStmt *bssAllocASTStmtExpr(Arena *arena, ASTExpr *expr)
{
    ASTStmt *s = bssAllocASTStmt(arena, AST_STMT_EXPR, expr->startTok, expr->endTok);
    s->expr = expr;

    return s;
}
ASTStmt *bssAllocASTStmtProjDecl(Arena *arena, BssTok iden, ASTExpr *expr)
{
    ASTStmt *s = bssAllocASTStmt(arena, AST_STMT_PROJ_DECL, iden, expr->endTok);
    s->proj.iden = iden;
    s->proj.assign = expr;

    return s;
}
ASTStmt *bssAllocASTStmtBuild(Arena *arena, ASTExpr *expr, ASTExpr *buildArgs)
{
    ASTStmt *s = bssAllocASTStmt(arena, AST_STMT_BUILD, expr->startTok, (buildArgs) ? buildArgs->endTok : expr->endTok);
    s->build.expr = expr;
    s->build.buildArgs = buildArgs;

    return s;
}
ASTStmt *bssAllocASTStmtIf(Arena *arena, ASTExpr *cond, ASTBlock *thenBlock, ASTBlock *elseBlock)
{
    ASTStmt *s = bssAllocASTStmt(arena, AST_STMT_IF, cond->startTok, (elseBlock) ? elseBlock->endTok : thenBlock->endTok);
    s->ifstmt.cond = cond;
    s->ifstmt.then = thenBlock;
    s->ifstmt.elseblock = elseBlock;

    return s;
}
ASTStmt *bssAllocASTStmtFor(Arena *arena, BssTok iden, ASTExpr *container, ASTBlock *block)
{
    ASTStmt *s = bssAllocASTStmt(arena, AST_STMT_FOR_LOOP, iden, block->endTok);
    s->forStmt.item = iden;
    s->forStmt.container = container;
    s->forStmt.block = block;

    return s;
}
ASTStmt *bssAllocASTStmtBlock(Arena *arena, ASTBlock *block)
{
    ASTStmt *s = bssAllocASTStmt(arena, AST_STMT_BLOCK, block->startTok, block->endTok);
    s->block = block;

    return s;
}

ASTExpr *bssAllocASTExpr(Arena *arena, ASTExprKind kind, BssTok startTok, BssTok endTok)
{
    ASTExpr *e = arenaPushType(arena, ASTExpr);
    e->kind = kind;
    e->startTok = startTok;
    e->endTok = endTok;

    return e;
}
ASTExpr *bssAllocASTExprIden(Arena *arena, BssTok iden)
{
    ASTExpr *e = bssAllocASTExpr(arena, AST_EXPR_IDEN, iden, iden);
    e->iden = iden;

    return e;
}
ASTExpr *bssAllocASTExprLit(Arena *arena, BssTok lit)
{
    ASTExpr *e = bssAllocASTExpr(arena, AST_EXPR_LIT, lit, lit);
    e->lit = lit;

    return e;
}
ASTExpr *bssAllocASTExprMembAccess(Arena *arena, BssTok startTok, BssTok endTok, ASTExpr *lhs, BssTok memb)
{
    ASTExpr *e = bssAllocASTExpr(arena, AST_EXPR_MEMBER_ACCESS, startTok, endTok);
    e->membAccess.lhs = lhs;
    e->membAccess.memb = memb;

    return e;
}
ASTExpr *bssAllocASTExprIndex(Arena *arena, BssTok startTok, BssTok endTok, ASTExpr *lhs, ASTExpr *index)
{
    ASTExpr *e = bssAllocASTExpr(arena, AST_EXPR_INDEX_ACCESS, startTok, endTok);
    e->index.lhs = lhs;
    e->index.indexExpr = index;

    return e;
}
ASTExpr *bssAllocASTExprBinary(Arena *arena, BssTok startTok, BssTok endTok, BssTok op, ASTExpr *lhs, ASTExpr *rhs)
{
    ASTExpr *e = bssAllocASTExpr(arena, AST_EXPR_BINARY, startTok, endTok);
    e->binaryOp.lhs = lhs;
    e->binaryOp.rhs = rhs;
    e->binaryOp.op = op;

    return e;
}
ASTExpr *bssAllocASTExprFunc(Arena *arena, BssTok startTok, BssTok endTok, ASTExpr *expr, ASTNamedExprList args)
{
    ASTExpr *e = bssAllocASTExpr(arena, AST_EXPR_FUNC_CALL, startTok, endTok);
    e->funcCall.args = args;
    e->funcCall.func = expr;

    return e;
}
ASTExpr *bssAllocASTExprCompound(Arena *arena, BssTok startTok, BssTok endTok, ASTNamedExprList exprs)
{
    ASTExpr *e = bssAllocASTExpr(arena, AST_EXPR_COMPOUND_LIT, startTok, endTok);
    e->compoundLit.membs = exprs;

    return e;
}
ASTExpr *bssAllocASTExprRun(Arena *arena, BssTok startTok, BssTok endTok, ASTExpr *expr)
{
    ASTExpr *e = bssAllocASTExpr(arena, AST_EXPR_RUN, startTok, endTok);
    e->run.expr = expr;

    return e;
}

ASTNamedExpr *bssAllocASTNamedExpr(Arena *arena, BssTok startTok, BssTok endTok, bool hasName, ASTExpr *lhs, ASTExpr *rhs)
{
    ASTNamedExpr *e = arenaPushType(arena, ASTNamedExpr);
    e->startTok = startTok;
    e->endTok = endTok;
    e->hasName = hasName;
    e->exprLhs = lhs;
    e->exprRhs = rhs;
    
    return e;
}
