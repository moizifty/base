#ifndef BSS_AST_H
#define BSS_AST_H

#include "bssCore.h"

#define BSS_AST_POS_DEFS                 \
                        BssTok startTok; \
                        BssTok endTok;   \

typedef enum BssAstTopLevelKind
{
    BSS_AST_TOP_LEVEL_NONE, // ZII
    BSS_AST_TOP_LEVEL_STMT,
    BSS_AST_TOP_LEVEL_FUNC,
}BssAstTopLevelKind;

typedef enum BssAstStmtKind
{
    BSS_AST_STMT_EXPR,
    BSS_AST_STMT_ASSIGN,
    BSS_AST_STMT_RET,
    BSS_AST_STMT_IF,
    BSS_AST_STMT_WHILE,
    BSS_AST_STMT_FOR,
    BSS_AST_STMT_CONT,
    BSS_AST_STMT_BREAK,
}BssAstStmtKind;

typedef enum BssAstExprKind
{
    BSS_AST_EXPR_LIT,
    BSS_AST_EXPR_IDEN,
    BSS_AST_EXPR_BINARY,
    BSS_AST_EXPR_UNARY,
    BSS_AST_EXPR_FUNCCALL,
    BSS_AST_EXPR_COMPOUND,
    BSS_AST_EXPR_SUBSCRIPT,
    BSS_AST_EXPR_ACCESS,
}BssAstExprKind;

typedef struct BssAstExpr BssAstExpr;
typedef struct BssAstNamedExpr BssAstNamedExpr;
BASE_CREATE_EFFICIENT_LL_DECLS(BssAstExprList, BssAstExpr)
BASE_CREATE_EFFICIENT_LL_DECLS(BssAstNamedExprList, BssAstNamedExpr)

typedef struct BssAstExpr
{
    BSS_AST_POS_DEFS

    struct BssAstExpr *next;
    struct BssAstExpr *prev;
    
    BssAstExprKind kind;
    union
    {
        BssTok lit, iden;
        BssAstNamedExprList compound;

        struct
        {
            struct BssAstExpr *left;
            struct BssAstExpr *right;
            BssTok op;
        }bin;

        struct
        {
            BssTok op;
            struct BssAstExpr *expr;
        }unary;

        struct 
        {
            BssAstExpr *lhs;
            BssAstExprList args;
        }call;
        
        struct
        {
            BssAstExpr *container;
            BssAstExpr *index;
        }subscript;
    };
}BssAstExpr;

typedef struct BssAstNamedExpr
{
    BSS_AST_POS_DEFS

    struct BssAstNamedExpr *next;
    struct BssAstNamedExpr *prev;

    bool isNamed;

    struct BssAstExpr *lhs;
    struct BssAstExpr *rhs;
}BssAstNamedExpr;

typedef struct BssAstStmt
{
    BSS_AST_POS_DEFS

    struct BssAstStmt *next;
    struct BssAstStmt *prev;
    
    BssAstStmtKind kind;
    union
    {
        BssAstExpr *expr, *retExpr;
        
        struct
        {
            BssAstExpr *lhs;
            BssAstExpr *rhs;
        }assign;

        struct
        {
            BssAstExpr *cond;
            struct BssAstBlock *thenBlock;
            struct BssAstBlock *elseBlock;
        }ifStmt;

        struct
        {
            BssAstExpr *cond;
            struct BssAstBlock *block;
        }whileStmt;

        struct
        {
            BssTok iden;
            BssAstExpr *container;
            struct BssAstBlock *block;
        }forStmt;
    };
}BssAstStmt;

BASE_CREATE_EFFICIENT_LL_DECLS(BssAstStmtList, BssAstStmt)

typedef struct BssAstBlock
{
    BSS_AST_POS_DEFS

    BssAstStmtList stmts;
}BssAstBlock;

typedef struct BssAstFunc
{
    BSS_AST_POS_DEFS

    BssTok iden;
    BssTokList params;

    BssAstBlock *block;
}BssAstFunc;

typedef struct BssAstTopLevel
{
    BSS_AST_POS_DEFS

    struct BssAstTopLevel *next;
    struct BssAstTopLevel *prev;
    BssAstTopLevelKind kind;
    union
    {
        BssAstFunc *func;
        BssAstStmt *stmt;
    };
}BssAstTopLevel;

BASE_CREATE_EFFICIENT_LL_DECLS(BssAstTopLevelList, BssAstTopLevel);

typedef struct BssAstFile
{
    BSS_AST_POS_DEFS

    BssAstTopLevelList toplevels;
}BssAstFile;

typedef BssAstExpr* (*BssPrecedencePrefixFunc) (BssInterp *);
typedef BssAstExpr* (*BssPrecedenceInfixFunc) (BssInterp *, BssAstExpr *left, BssTok op);

typedef struct BssPrecedenceTableEntry
{
    BssPrecedencePrefixFunc prefix;
    BssPrecedenceInfixFunc infix;
    u64 precedence;
}BssPrecedenceTableEntry;

#define BSS_AST_EXPR_ZERO (&gBssAstExprEmpty)
#define BSS_AST_STMT_ZERO (&gBssAstStmtEmpty)
#define BSS_AST_BLOCK_ZERO (&gBssAstBlockEmpty)
#define BSS_AST_FUNC_ZERO (&gBssAstFuncEmpty)
#define BSS_AST_TOP_LEVEL_ZERO (&gBssAstTopLevelEmpty)

global BssAstExpr gBssAstExprEmpty;
global BssAstStmt gBssAstStmtEmpty;
global BssAstBlock gBssAstBlockEmpty;
global BssAstFunc gBssAstFuncEmpty;
global BssAstTopLevel gBssAstTopLevelEmpty;

BssAstExpr *bssAllocExpr(BssInterp *interp, BssTok start, BssTok end, BssAstExprKind kind);
BssAstExpr *bssAllocExprLit(BssInterp *interp, BssTok start, BssTok end, BssTok lit);
BssAstExpr *bssAllocExprIden(BssInterp *interp, BssTok start, BssTok end, BssTok iden);
BssAstExpr *bssAllocExprBinary(BssInterp *interp, BssTok start, BssTok end, BssAstExpr *lhs, BssAstExpr *rhs, BssTok op);
BssAstExpr *bssAllocExprUnary(BssInterp *interp, BssTok start, BssTok end, BssAstExpr *rhs, BssTok op);
BssAstExpr *bssAllocExprFnCall(BssInterp *interp, BssTok start, BssTok end, BssAstExpr *lhs, BssAstExprList args);
BssAstExpr *bssAllocExprCompound(BssInterp *interp, BssTok start, BssTok end, BssAstNamedExprList compound);
BssAstExpr *bssAllocExprSubscript(BssInterp *interp, BssTok start, BssTok end, BssAstExpr *container, BssAstExpr *index);
#endif