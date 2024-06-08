#include "base\baseThreads.h"
#include "bssChecker.h"

void bssCheckerError(BSSInterpretorState *iState, BssTok tok, char *msg, ...)
{
    va_list args;
    va_start(args, msg);

    baseColEPrintf("{Bu}%S (%lld: %lld)",tok.pos.ownerLexer->filePath, tok.pos.line, tok.pos.col);
    baseColEPrintf("{B}\n        --> Checker Error:\033[0m ");

    BaseArenaTemp temp = baseTempBegin(&iState->checkerArena, 1);
    {
        i64 needed = stbsp_vsnprintf(null, 0, msg, args) + 1;
        i8 *buf = baseArenaPush(temp.arena, needed);

        stbsp_vsnprintf(buf, (int)needed, msg, args);

        baseColEPrintf("%s", buf);
    }
    baseTempEnd(temp);

    va_end(args);
}
void bssCheckerPrint(char *msg, ...);

BssCheckerState *bssCheckerInitFromProject(BSSInterpretorState *iState, ASTProject *proj)
{
    BssCheckerState *cState = baseArenaPushType(iState->checkerArena, BssCheckerState);
    
    cState->proj = proj;

    return cState;
}

void bssCheckerCheckWholeProject(BSSInterpretorState *iState, BssCheckerState *cState)
{
    bssCheckerCheckStmts(iState, cState, cState->proj->stmts, null);
}

void bssCheckerCheckStmts(BSSInterpretorState *iState, BssCheckerState *cState, ASTStmtList stmts, BssScope *parentScope)
{
    BssScope *scope = bssNewScope(iState->checkerArena, parentScope);

    BASE_LIST_FOREACH(ASTStmt, stmt, stmts)
    {
        bssCheckerCheckStmt(iState, cState, stmt, scope);
    }
}

void bssCheckerCheckStmt(BSSInterpretorState *iState, BssCheckerState *cState, ASTStmt *stmt, BssScope *scope)
{
    switch(stmt->kind)
    {
        case AST_STMT_PROJ_DECL:
        {

        }break;

        case AST_STMT_BUILD:
        {

        }break;

        case AST_STMT_EXPR:
        {
            bssCheckerCheckExpr(iState, cState, stmt->expr, scope);
        }break;

        case AST_STMT_ASSIGN:
        {
            
        }break;

        case AST_STMT_BLOCK:
        {
            bssCheckerCheckStmts(iState, cState, stmt->block->stmts, scope);
        }break;

        case AST_STMT_FOR_LOOP:
        {

        }break;

        case AST_STMT_IF:
        {

        }break;

        default:
        {
            bssCheckerError(iState, stmt->startTok, "Unregognised Statement\n");
            bssLexerPrintTokenRange(stmt->startTok, stmt->endTok);
        }break;
    }
}


void bssCheckerCheckExpr(BSSInterpretorState *iState, BssCheckerState *cState, ASTExpr *expr, BssScope *scope)
{
    switch(expr->kind)
    {
        case AST_EXPR_LIT:
        {
            switch(expr->lit.kind)
            {
                case TOK_INT_LIT:
                {
                    expr->checkType = bssAllocTypeInt(iState->checkerArena, &cState->typeTable);
                }break;

                case TOK_BOOL_LIT:
                {
                    expr->checkType = bssAllocTypeBool(iState->checkerArena, &cState->typeTable);
                }break;

                case TOK_STR_LIT:
                {
                    expr->checkType = bssAllocTypeString(iState->checkerArena, &cState->typeTable);
                }break;
            }
        }break;
        case AST_EXPR_IDEN:
        {
            BssSymTableSlotEntry *entry = bssScopeFindEntryFromName(scope, true, expr->iden.lexeme);
            if (entry != null)
            {
                expr->checkType = entry->value->type;
            }
            else
            {
                bssCheckerError(iState, expr->startTok, "Identifier '%S' is not declared anywhere.\n", expr->iden.lexeme);
            }
        }break;
        case AST_EXPR_BINARY:
        case AST_EXPR_FUNC_CALL:
        case AST_EXPR_INDEX_ACCESS:
        case AST_EXPR_COMPOUND_LIT:
        case AST_EXPR_MEMBER_ACCESS:
        case AST_EXPR_RUN:
        default:
        {
            bssCheckerError(iState, expr->startTok, "Unregognised expression.\n");
            bssLexerPrintTokenRange(expr->startTok, expr->endTok);
        }break;
    }
}