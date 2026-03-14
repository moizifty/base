#include "bssParser.h"
#include "bssLexer.h"
#include "bssScope.h"

BssPrecedenceTableEntry gBssPrecedenceTable[TOK_KIND_END] =
{
    [TOK_EQ_OP] = {.precedence = 1, .prefix = null, .infix = bssParserParseExprBinary},
    [TOK_NEQ_OP] = {.precedence = 1, .prefix = null, .infix = bssParserParseExprBinary},
    [TOK_LOGICAL_AND_OP] = {.precedence = 11, .prefix = null, .infix = bssParserParseExprBinary},
    [TOK_LOGICAL_OR_OP] = {.precedence = 10, .prefix = null, .infix = bssParserParseExprBinary},

    ['+'] = {.precedence = 20, .prefix = null, .infix = bssParserParseExprBinary},
    ['-'] = {.precedence = 20, .prefix = null, .infix = bssParserParseExprBinary},
    ['/'] = {.precedence = 30, .prefix = null, .infix = bssParserParseExprBinary},
    ['*'] = {.precedence = 30, .prefix = null, .infix = bssParserParseExprBinary},

    ['('] = {.precedence = 100, .prefix = null, .infix = bssParserParseExprFnCall},
};


void bssParserError(BssInterp *interp, i8 *fmt, ...)
{
    BssTokPos pos = BSS_PARSER_CURR_TOK.pos;
    va_list va;
    va_start(va, fmt);
    baseColEPrintf("{Bu}%S (%lld, %lld)", pos.ownerLexer->path, pos.line, pos.col);
    baseColEPrintf("{B}\n        --> Parser Error: ");
    baseEPrintfV(fmt, va);

    bssPrintSourceRange(BSS_PARSER_CURR_TOK.pos, BSS_PARSER_CURR_TOK.pos, 2);

    va_end(va);
}

BssAstStmt *bssParserParseStmt(BssInterp *interp)
{
    BssAstStmt *ast = BSS_AST_STMT_ZERO;

    switch (BSS_PARSER_CURR_TOK.kind)
    {
        case TOK_RET_KW:
        {
            BssTok start = BSS_PARSER_CURR_TOK;
            BSS_PARSER_NEXT_TOK();

            BssTok end = {0};
            BssAstExpr *expr = BSS_AST_EXPR_ZERO;

            if (!BSS_PARSER_MATCH(';'))
            {
                expr = bssParserParseExpr(interp, 0);
                if (expr == BSS_AST_EXPR_ZERO)
                {
                    return BSS_AST_STMT_ZERO;
                }
            }

            if (BSS_PARSER_MATCH(';'))
            {
                BSS_PARSER_NEXT_TOK();

                ast = arenaPushType(interp->arena, BssAstStmt);
                ast->kind = BSS_AST_STMT_RET;
                ast->startTok = start;
                ast->endTok = (expr == BSS_AST_EXPR_ZERO) ? start : expr->endTok;
                ast->retExpr = expr;
            }
            else
            {
                bssParserError(interp, "Expected ';' end of statement instead got '%S'\n", BSS_PARSER_CURR_TOK.lexeme);
                return BSS_AST_STMT_ZERO;
            }
        }break;

        default:
        {
            BssAstExpr *lhs = bssParserParseExpr(interp, 0);

            if (lhs != BSS_AST_EXPR_ZERO)
            {
                if (BSS_PARSER_MATCH('='))
                {
                    BSS_PARSER_NEXT_TOK();

                    BssAstExpr *rhs = bssParserParseExpr(interp, 0);

                    ast = arenaPushType(interp->arena, BssAstStmt);
                    ast->startTok = lhs->startTok;
                    ast->endTok = rhs->endTok;
                    ast->kind = BSS_AST_STMT_ASSIGN;
                    ast->assign.lhs = lhs;
                    ast->assign.rhs = rhs;

                    if (lhs->kind = BSS_AST_EXPR_IDEN)
                    {
                        bssScopePushEntry(interp, interp->currScope, lhs->iden.lexeme, null);
                    }
                }
                else
                {
                    ast = arenaPushType(interp->arena, BssAstStmt);
                    ast->startTok = lhs->startTok;
                    ast->endTok = lhs->endTok;
                    ast->kind = BSS_AST_STMT_EXPR;
                    ast->expr = lhs;
                }

                if (BSS_PARSER_MATCH(';'))
                {
                    BSS_PARSER_NEXT_TOK();
                }
                else
                {
                    bssParserError(interp, "Expected ';' end of statement instead got '%S'\n", BSS_PARSER_CURR_TOK.lexeme);
                    return BSS_AST_STMT_ZERO;
                }
            }
        }break;
    }

    return ast;
}
BssAstBlock *bssParserParseBlock(BssInterp *interp, bool createScope)
{
    BssAstBlock *ast = BSS_AST_BLOCK_ZERO;

    BssTok start = {0};
    BssAstStmtList list = {0};

    BssScope *prev = interp->currScope;
    if (createScope)
    {
        interp->currScope = arenaPushType(interp->arena, BssScope);
        interp->currScope->parent = prev;
    }

    if (BSS_PARSER_MATCH('{'))
    {
        start = BSS_PARSER_CURR_TOK;
        BSS_PARSER_NEXT_TOK();

        while(!BSS_PARSER_MATCH('}') && !BSS_PARSER_MATCH(TOK_END_INPUT))
        {
            BssAstStmt *stmt = bssParserParseStmt(interp);
            if (stmt != BSS_AST_STMT_ZERO)
            {
                BssAstStmtListPushNodeLast(&list, stmt);
            }
            else
            {
                break;
            }
        }

        if(BSS_PARSER_MATCH('}'))
        {
            ast = arenaPushType(interp->arena, BssAstBlock);
            ast->startTok = start;
            ast->endTok = BSS_PARSER_CURR_TOK;
            ast->stmts = list;

            BSS_PARSER_NEXT_TOK();
        }
        else
        {
            bssParserError(interp, "Expected '}' but instead got '%S'", BSS_PARSER_CURR_TOK.lexeme);
        }
    }

    interp->currScope = prev;

    return ast;
}

BssTokList bssParserParseTokList(BssInterp *interp)
{
    BssTokList list = {0};

    while (BSS_PARSER_MATCH(TOK_IDEN))
    {
        BssTokListPushLast(interp->arena, &list, BSS_PARSER_CURR_TOK);
        BSS_PARSER_NEXT_TOK();

        if (BSS_PARSER_MATCH(','))
        {
            BSS_PARSER_NEXT_TOK();
        }
        else
        {
            break;
        }
    }

    return list;
}
BssAstFunc *bssParserParseFunc(BssInterp *interp)
{
    BssAstFunc *ast = BSS_AST_FUNC_ZERO;

    BssTok start = {0};
    BssTok iden = {0};
    if (BSS_PARSER_MATCH(TOK_FN_KW))
    {
        start = BSS_PARSER_CURR_TOK;
        BSS_PARSER_NEXT_TOK();

        if (BSS_PARSER_MATCH(TOK_IDEN))
        {
            iden = BSS_PARSER_CURR_TOK;

            if (bssScopeFindEntry(interp->currScope, iden.lexeme) != BSS_SYMTABLE_SLOT_ENTRY_ZERO)
            {
                bssParserError(interp, "Function '%S' already defined", iden.lexeme);
            }
            else
            {
                BSS_PARSER_NEXT_TOK();

                if (BSS_PARSER_MATCH('('))
                {
                    BSS_PARSER_NEXT_TOK();

                    BssTokList params = bssParserParseTokList(interp);
                    if (BSS_PARSER_MATCH(')'))
                    {
                        BSS_PARSER_NEXT_TOK();

                        BssScope *prevScope = interp->currScope;
                        interp->currScope = arenaPushType(interp->arena, BssScope);
                        interp->currScope->parent = prevScope;

                        BASE_LIST_FOREACH(BssTokListNode, node, params)
                        {
                            bssScopePushEntry(interp, interp->currScope, node->val.lexeme, null);
                        }

                        BssAstBlock *block = bssParserParseBlock(interp, false);

                        if (block != BSS_AST_BLOCK_ZERO)
                        {
                            ast = arenaPushType(interp->arena, BssAstFunc);
                            ast->startTok = start;
                            ast->endTok = block->endTok;
                            ast->iden = iden;
                            ast->block = block;
                            ast->params = params;

                            BssSymTableSlotEntry *entry = BSS_SYMTABLE_SLOT_ENTRY_ZERO;
                            bssScopePushEntry(interp, interp->rootScope, iden.lexeme, &entry);

                            entry->value = arenaPushType(interp->arena, BssValue);
                            entry->value->kind = BSS_VALUE_FUNCTION;
                            entry->value->fn.defined.params = params;
                            entry->value->fn.defined.ast = ast;
                            entry->value->fn.defined.scope = interp->currScope;
                        }

                        interp->currScope = prevScope;
                    }
                    else
                    {
                        bssParserError(interp, "Expected closing ')' instead got '%S'", BSS_PARSER_CURR_TOK.lexeme);
                    }
                }
                else
                {
                    bssParserError(interp, "Expected '(' instead got '%S'", BSS_PARSER_CURR_TOK.lexeme);
                }
            }
        }
        else
        {
            bssParserError(interp, "Expected 'identifier' instead got '%S'", BSS_PARSER_CURR_TOK.lexeme);
        }
    }
    else
    {
        bssParserError(interp, "Expected 'fn' instead got '%S'", BSS_PARSER_CURR_TOK.lexeme);
    }
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
            BssAstFunc *func = bssParserParseFunc(interp);
            if (func != BSS_AST_FUNC_ZERO)
            {
                ast = arenaPushType(interp->arena, BssAstTopLevel);
                ast->kind = BSS_AST_TOP_LEVEL_FUNC;
                ast->func = func;
                ast->startTok = ast->func->startTok;
                ast->endTok = ast->func->startTok;
            }
        }break;

        default:
        {
            BssAstStmt *stmt = bssParserParseStmt(interp);
            if (stmt != BSS_AST_STMT_ZERO)
            {
                ast = arenaPushType(interp->arena, BssAstTopLevel);
                ast->kind = BSS_AST_TOP_LEVEL_STMT;
                ast->stmt = stmt;
                ast->startTok = ast->stmt->startTok;
                ast->endTok = ast->stmt->startTok;
            }
        }break;
    }

    return ast;
}

BssAstTopLevelList bssParserParseTopLevels(BssInterp *interp)
{
    BssAstTopLevelList list = {0};

    while (!BSS_PARSER_MATCH(TOK_END_INPUT))
    {
        BssAstTopLevel *topLevel = bssParserParseTopLevel(interp);
        if (topLevel != BSS_AST_TOP_LEVEL_ZERO)
        {
            BssAstTopLevelListPushNodeLast(&list, topLevel);
        }
        else
        {
            break;
        }
    }

    return list;
}

bool bssParserParseLexed(BssInterp *interp)
{
    interp->parser = arenaPushType(interp->arena, BssParser);
    interp->parser->file = arenaPushType(interp->arena, BssAstFile);

    BssAstTopLevelList toplevels = bssParserParseTopLevels(interp);
    interp->parser->file->toplevels = toplevels;
    interp->parser->file->startTok = toplevels.first ? toplevels.first->startTok : (BssTok){0};
    interp->parser->file->endTok = toplevels.last ? toplevels.first->endTok : (BssTok){0};

    return true;
}

BssAstExpr *bssParserParsePrefix(BssInterp *interp)
{
    BssAstExpr *expr = BSS_AST_EXPR_ZERO;
    switch (BSS_PARSER_CURR_TOK.kind)
    {
        case TOK_INT_LIT:
        case TOK_BOOL_LIT:
        case TOK_CHAR_LIT:
        case TOK_STR_LIT:
        {
            expr = bssAllocExprLit(interp, BSS_PARSER_CURR_TOK, BSS_PARSER_CURR_TOK, BSS_PARSER_CURR_TOK);

            BSS_PARSER_NEXT_TOK();
        }break;

        case TOK_IDEN:
        {
            expr = bssAllocExprIden(interp, BSS_PARSER_CURR_TOK, BSS_PARSER_CURR_TOK, BSS_PARSER_CURR_TOK);

            BSS_PARSER_NEXT_TOK();
        }break;

        case '+':
        case '-':
        {
            BssTok op = BSS_PARSER_CURR_TOK;
            BSS_PARSER_NEXT_TOK();
            expr = bssParserParsePrefix(interp);
            if (expr != BSS_AST_EXPR_ZERO)
            {
                expr = bssAllocExprUnary(interp, op, expr->endTok, expr, op);
            }
        }break;

        case '(':
        {
            BSS_PARSER_NEXT_TOK();
            expr = bssParserParseExpr(interp, 0);

            if (BSS_PARSER_MATCH(')'))
            {
                BSS_PARSER_NEXT_TOK();
            }
            else
            {
                bssParserError(interp, "Expected closing ')', instead got '%S'", BSS_PARSER_CURR_TOK.lexeme);
            }
        }break;

        default:
        {
            bssParserError(interp, "Not a valid expression");
        }break;
    }

    return expr;
}

BssAstExpr *bssParserParseExprFnCall(BssInterp *interp, BssAstExpr *left, BssTok op)
{
    BssAstExpr *expr = BSS_AST_EXPR_ZERO;

    BssAstExprList list = bssParserParseExprList(interp, ')');
    if (BSS_PARSER_MATCH(')'))
    {
        expr = bssAllocExprFnCall(interp, left->startTok, BSS_PARSER_CURR_TOK, left, list);

        BSS_PARSER_NEXT_TOK();
    }
    else
    {
        bssParserError(interp, "Expected ')' instead got '%S'", BSS_PARSER_CURR_TOK.lexeme);
    }
    return expr;
}

BssAstExpr *bssParserParseExprBinary(BssInterp *interp, BssAstExpr *left, BssTok op)
{
    BssAstExpr *expr = BSS_AST_EXPR_ZERO;

    switch (op.kind)
    {
        case '+':
        case '-':
        case '/':
        case '*':
        case TOK_LOGICAL_OR_OP:
        case TOK_LOGICAL_AND_OP:
        case TOK_EQ_OP:
        case TOK_NEQ_OP:
        {
            BssAstExpr *rhs = bssParserParseExpr(interp, gBssPrecedenceTable[op.kind].precedence);
            if (rhs != BSS_AST_EXPR_ZERO)
            {
                expr = bssAllocExprBinary(interp, left->startTok, rhs->endTok, left, rhs, op);
            }
        }break;

        default:
        {
            bssParserError(interp, "Unregognised binary operator '%S'", op.lexeme);
        }break;
    }

    return expr;
}

BssAstExpr *bssParserParseExpr(BssInterp *interp, u64 currPrecedence)
{
    BssAstExpr *lhs = bssParserParsePrefix(interp);
    if (lhs != BSS_AST_EXPR_ZERO)
    {
        while(currPrecedence < gBssPrecedenceTable[BSS_PARSER_CURR_TOK.kind].precedence)
        {
            BssTok op = BSS_PARSER_CURR_TOK;
            BSS_PARSER_NEXT_TOK();

            lhs = gBssPrecedenceTable[op.kind].infix(interp, lhs, op);
        }
    }

    return lhs;
}

BssAstExprList bssParserParseExprList(BssInterp *interp, BssTokKind endKind)
{
    BssAstExprList list = {0};
    while (!BSS_PARSER_MATCH(endKind) &&
           !BSS_PARSER_MATCH(TOK_END_INPUT))
    {
        BssAstExpr *expr = bssParserParseExpr(interp, 0);
        if (expr != BSS_AST_EXPR_ZERO)
        {
            if (BSS_PARSER_MATCH(',') || 
                BSS_PARSER_MATCH(endKind))
            {
                if (BSS_PARSER_MATCH(',')) BSS_PARSER_NEXT_TOK();
                BssAstExprListPushNodeLast(&list, expr);
                continue;
            }
            else
            {
                bssParserError(interp, "Expected ',' after arg, instead got '%S'", BSS_PARSER_CURR_TOK.lexeme);
            }
        }
        
        break;
    }

    return list;
}

bool bssParserParseFile(BssInterp *interp, str8 file)
{
    interp->rootScope = arenaPushType(interp->arena, BssScope);
    interp->currScope = interp->rootScope;

    if (bssLexerLexFile(interp, file))
    {
        return bssParserParseLexed(interp);
    }

    return false;
}