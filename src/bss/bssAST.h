#ifndef BSS_AST_H
#define BSS_AST_H

#include "base\baseCore.h"
#include "base\baseMemory.h"
#include "bssCore.h"
#include "bssTypes.h"

typedef enum ASTStmtKind ASTStmtKind;
typedef enum ASTExprKind ASTExprKind;

typedef struct ASTProject ASTProject;
typedef struct ASTStmt ASTStmt;
typedef struct ASTBlock ASTBlock;
typedef struct ASTExpr ASTExpr;
typedef struct ASTNamedExpr ASTNamedExpr;

typedef struct ASTStmtList ASTStmtList;
typedef struct ASTExprList ASTExprList;
typedef struct ASTNamedExprList ASTNamedExprList;

typedef enum ASTStmtKind
{ 
    AST_STMT_ASSIGN,
    AST_STMT_EXPR,
    
    AST_STMT_PROJ_DECL,
    AST_STMT_BUILD,
    AST_STMT_IF,
    AST_STMT_FOR_LOOP,
    AST_STMT_BLOCK,
}ASTStmtKind;

typedef enum ASTExprKind
{ 
    AST_EXPR_IDEN,
    AST_EXPR_LIT,
    AST_EXPR_MEMBER_ACCESS,
    AST_EXPR_INDEX_ACCESS,
    AST_EXPR_BINARY,
    AST_EXPR_FUNC_CALL,
    AST_EXPR_COMPOUND_LIT,
    AST_EXPR_RUN,
}ASTExprKind;

typedef struct ASTStmt
{ 
    BssTok startTok;
    BssTok endTok;

    struct ASTStmt *next;
    struct ASTStmt *prev;

    ASTStmtKind kind;
    union
    {
        struct
        {
            ASTExpr *lhs;
            ASTExpr *rhs;
        }assign;

        struct
        {
            BssTok iden;
            ASTExpr *assign;
        }proj;

        struct
        {
            ASTExpr *expr;

            ASTExpr *buildArgs;
        }build;

        struct
        {
            ASTExpr *cond;

            ASTBlock *then;
            ASTBlock *elseblock;
        }ifstmt;

        struct
        {
            BssTok item;

            ASTExpr *container;
            ASTBlock *block;
        }forStmt;

        ASTBlock *block;
        ASTExpr *expr;
    };
}ASTStmt;

BASE_CREATE_EFFICIENT_LL_DECLS(ASTStmtList, ASTStmt);

typedef struct ASTBlock
{
    BssTok startTok;
    BssTok endTok;

    ASTStmtList stmts;
}ASTBlock;

typedef struct ASTNamedExpr
{ 
    BssTok startTok;
    BssTok endTok;
    
    struct ASTNamedExpr *next;
    struct ASTNamedExpr *prev;

    bool hasName;

    ASTExpr *exprLhs;
    ASTExpr *exprRhs;
}ASTNamedExpr;

BASE_CREATE_EFFICIENT_LL_DECLS(ASTNamedExprList, ASTNamedExpr);

typedef struct ASTExpr
{ 
    BssTok startTok;
    BssTok endTok;

    ASTExprKind kind;
    union
    {
        BssTok iden;
        BssTok lit;

        struct
        {
            ASTExpr *lhs;
            BssTok memb;
        } membAccess;

        struct
        {
            ASTExpr *lhs;
            ASTExpr *indexExpr;
        } index;

        struct
        {
            ASTExpr *lhs;
            BssTok op;
            ASTExpr *rhs;
        } binaryOp;

        struct
        {
            ASTExpr *func;
            ASTNamedExprList args;
        }funcCall;

        struct
        {
            ASTNamedExprList membs;
        }compoundLit;

        struct
        {
            ASTExpr *expr;
        }run;
    };

    BssType *checkType;
    struct BssScope *scope;
    struct BssSymTableSlotEntry *idenEntry;
    struct BssValue *value;
}ASTExpr;

typedef struct ASTProject
{
    BssTok startTok;
    BssTok endTok;

    ASTStmtList stmts;
}ASTProject;

ASTStmt *bssAllocASTStmt(BaseArena *arena, ASTStmtKind kind, BssTok startTok, BssTok endTok);
ASTStmt *bssAllocASTStmtAssign(BaseArena *arena, ASTExpr *lhs, ASTExpr *rhs);
ASTStmt *bssAllocASTStmtExpr(BaseArena *arena, ASTExpr *expr);
ASTStmt *bssAllocASTStmtProjDecl(BaseArena *arena, BssTok iden, ASTExpr *expr);
ASTStmt *bssAllocASTStmtBuild(BaseArena *arena, ASTExpr *expr, ASTExpr *buildArgs);
ASTStmt *bssAllocASTStmtIf(BaseArena *arena, ASTExpr *cond, ASTBlock *thenBlock, ASTBlock *elseBlock);
ASTStmt *bssAllocASTStmtFor(BaseArena *arena, BssTok iden, ASTExpr *container, ASTBlock *block);
ASTStmt *bssAllocASTStmtBlock(BaseArena *arena, ASTBlock *block);

ASTExpr *bssAllocASTExpr(BaseArena *arena, ASTExprKind kind, BssTok startTok, BssTok endTok);
ASTExpr *bssAllocASTExprIden(BaseArena *arena, BssTok iden);
ASTExpr *bssAllocASTExprLit(BaseArena *arena, BssTok lit);
ASTExpr *bssAllocASTExprMembAccess(BaseArena *arena, BssTok startTok, BssTok endTok, ASTExpr *lhs, BssTok memb);
ASTExpr *bssAllocASTExprIndex(BaseArena *arena, BssTok startTok, BssTok endTok, ASTExpr *lhs, ASTExpr *index);
ASTExpr *bssAllocASTExprBinary(BaseArena *arena, BssTok startTok, BssTok endTok, BssTok op, ASTExpr *lhs, ASTExpr *rhs);
ASTExpr *bssAllocASTExprFunc(BaseArena *arena, BssTok startTok, BssTok endTok, ASTExpr *expr, ASTNamedExprList args);
ASTExpr *bssAllocASTExprCompound(BaseArena *arena, BssTok startTok, BssTok endTok, ASTNamedExprList exprs);
ASTExpr *bssAllocASTExprRun(BaseArena *arena, BssTok startTok, BssTok endTok, ASTExpr *expr);

ASTNamedExpr *bssAllocASTNamedExpr(BaseArena *arena, BssTok startTok, BssTok endTok, bool hasName, ASTExpr *lhs, ASTExpr *rhs);

#endif