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

typedef struct BssAstStmt
{
    BSS_AST_POS_DEFS

    struct BssAstStmt *next;
    struct BssAstStmt *prev;
}BssAstStmt;

BASE_CREATE_EFFICIENT_LL_DECLS(BssAstStmtList, BssAstStmt);

typedef struct BssAstFunc
{
    BSS_AST_POS_DEFS

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


#define BSS_AST_STMT_ZERO (&gBssAstStmtEmpty)
#define BSS_AST_FUNC_ZERO (&gBssAstFuncEmpty)
#define BSS_AST_TOP_LEVEL_ZERO (&gBssAstTopLevelEmpty)

global BssAstStmt gBssAstStmtEmpty;
global BssAstFunc gBssAstFuncEmpty;
global BssAstTopLevel gBssAstTopLevelEmpty;

#endif