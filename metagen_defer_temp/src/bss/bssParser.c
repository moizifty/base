#include "bssParser.h"
#include "bssLexer.h"
#include "bssScope.h"

BssPrecedenceTableEntry gBssPrecedenceTable[TOK_KIND_END + 1] =
{
    [TOK_EQ_OP] = {.precedence = 1, .prefix = null, .infix = bssParserParseExprBinary},
    [TOK_NEQ_OP] = {.precedence = 1, .prefix = null, .infix = bssParserParseExprBinary},
    [TOK_LOGICAL_AND_OP] = {.precedence = 11, .prefix = null, .infix = bssParserParseExprBinary},
    [TOK_LOGICAL_OR_OP] = {.precedence = 10, .prefix = null, .infix = bssParserParseExprBinary},

    ['<'] = {.precedence = 11, .prefix = null, .infix = bssParserParseExprBinary},
    ['>'] = {.precedence = 11, .prefix = null, .infix = bssParserParseExprBinary},

    ['+'] = {.precedence = 20, .prefix = null, .infix = bssParserParseExprBinary},
    ['-'] = {.precedence = 20, .prefix = null, .infix = bssParserParseExprBinary},
    ['/'] = {.precedence = 30, .prefix = null, .infix = bssParserParseExprBinary},
    ['*'] = {.precedence = 30, .prefix = null, .infix = bssParserParseExprBinary},

    ['('] = {.precedence = 100, .prefix = null, .infix = bssParserParseExprFnCall},
    ['['] = {.precedence = 100, .prefix = null, .infix = bssParserParseExprSubscript},
    ['.'] = {.precedence = 100, .prefix = null, .infix = bssParserParseExprBinary},
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
        case TOK_IF_KW:
        {
            ast = bssParserParseStmtIf(interp);
        }break;

        case TOK_WHILE_KW:
        {
            ast = bssParserParseStmtWhile(interp);
        }break;

        case TOK_FOR_KW:
        {
            ast = bssParserParseStmtFor(interp);
        }break;

        case TOK_RET_KW:
        {
            BssTok start = BSS_PARSER_CURR_TOK;
            BSS_PARSER_NEXT_TOK();

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
BssAstBlock *bssParserParseBlock(BssInterp *interp)
{
    BssAstBlock *ast = BSS_AST_BLOCK_ZERO;

    BssTok start = {0};
    BssAstStmtList list = {0};

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

            if (bssScopeFindEntry(interp->rootScope, iden.lexeme) != BSS_SYMTABLE_SLOT_ENTRY_ZERO)
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

                        BssAstBlock *block = bssParserParseBlock(interp);

                        if (block != BSS_AST_BLOCK_ZERO)
                        {
                            ast = arenaPushType(interp->arena, BssAstFunc);
                            ast->startTok = start;
                            ast->endTok = block->endTok;
                            ast->iden = iden;
                            ast->block = block;
                            ast->params = params;

                            BssSymTableSlotEntry *entry = BSS_SYMTABLE_SLOT_ENTRY_ZERO;
                            bssScopePushEntry(interp->rootScope, iden.lexeme, &entry);

                            entry->value = bssAllocValueFn(interp->rootScope->scopeArena, ast);
                        }
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

BssAstStmt *bssParserParseStmtIf(BssInterp *interp)
{
    BssAstStmt *ast = BSS_AST_STMT_ZERO;
    BssTok start = BSS_PARSER_CURR_TOK;

    if (BSS_PARSER_MATCH(TOK_IF_KW))
    {
        BSS_PARSER_NEXT_TOK();

        BssAstExpr *cond = bssParserParseExpr(interp, 0);
        if (cond != BSS_AST_EXPR_ZERO)
        {
            BssAstBlock *thenBlock = bssParserParseBlock(interp);

            if (thenBlock != BSS_AST_BLOCK_ZERO)
            {
                BssTok end = thenBlock->endTok;
                BssAstBlock *elseBlock = BSS_AST_BLOCK_ZERO;
                if (BSS_PARSER_MATCH(TOK_ELSE_KW))
                {
                    BSS_PARSER_NEXT_TOK();

                    if (BSS_PARSER_MATCH(TOK_IF_KW))
                    {
                        BssAstStmt *elseIf = bssParserParseStmtIf(interp);
                        if (elseIf != BSS_AST_STMT_ZERO)
                        {
                            elseBlock = arenaPushType(interp->arena, BssAstBlock);
                            elseBlock->startTok = elseIf->startTok;
                            elseBlock->endTok = elseIf->endTok;

                            BssAstStmtListPushNodeLast(&elseBlock->stmts, elseIf);
                        }
                        else
                        {
                            return BSS_AST_STMT_ZERO;
                        }
                    }
                    else
                    {
                        elseBlock = bssParserParseBlock(interp);
                        if (elseBlock == BSS_AST_BLOCK_ZERO)
                        {
                            return BSS_AST_STMT_ZERO;
                        }
                    }

                    end = elseBlock->endTok;
                }

                ast = arenaPushType(interp->arena, BssAstStmt);
                ast->startTok = start;
                ast->endTok = end;
                ast->kind = BSS_AST_STMT_IF;
                ast->ifStmt.cond = cond;
                ast->ifStmt.thenBlock = thenBlock;
                ast->ifStmt.elseBlock = elseBlock;
            }
        }
    }
    else
    {
        bssParserError(interp, "Expected 'if' kw, instead got '%S'", BSS_PARSER_CURR_TOK.lexeme);
    }

    return ast;
}
BssAstStmt *bssParserParseStmtWhile(BssInterp *interp)
{
    BssAstStmt *ast = BSS_AST_STMT_ZERO;
    BssTok start = BSS_PARSER_CURR_TOK;

    if (BSS_PARSER_MATCH(TOK_WHILE_KW))
    {
        BSS_PARSER_NEXT_TOK();

        BssAstExpr *cond = bssParserParseExpr(interp, 0);
        if (cond != BSS_AST_EXPR_ZERO)
        {
            BssAstBlock *block = bssParserParseBlock(interp);

            if (block != BSS_AST_BLOCK_ZERO)
            {
                BssTok end = block->endTok;

                ast = arenaPushType(interp->arena, BssAstStmt);
                ast->startTok = start;
                ast->endTok = end;
                ast->kind = BSS_AST_STMT_WHILE;
                ast->whileStmt.cond = cond;
                ast->whileStmt.block = block;
            }
        }
    }
    else
    {
        bssParserError(interp, "Expected 'while' kw, instead got '%S'", BSS_PARSER_CURR_TOK.lexeme);
    }

    return ast;
}
BssAstStmt *bssParserParseStmtFor(BssInterp *interp)
{
    BssAstStmt *ast = BSS_AST_STMT_ZERO;
    BssTok start = BSS_PARSER_CURR_TOK;

    if (BSS_PARSER_MATCH(TOK_FOR_KW))
    {
        BSS_PARSER_NEXT_TOK();

        if (BSS_PARSER_MATCH(TOK_IDEN) && BSS_PARSER_PEEK_TOK(1).kind == TOK_IN_KW)
        {
            BssTok iden = BSS_PARSER_CURR_TOK;
            BSS_PARSER_NEXT_TOK(); //iden
            BSS_PARSER_NEXT_TOK(); //in

            BssAstExpr *container = bssParserParseExpr(interp, 0);
            if (container != BSS_AST_EXPR_ZERO)
            {
                BssAstBlock *block = bssParserParseBlock(interp);
                if (block != BSS_AST_BLOCK_ZERO)
                {
                    ast = arenaPushType(interp->arena, BssAstStmt);
                    ast->startTok = start;
                    ast->endTok = block->endTok;
                    ast->kind = BSS_AST_STMT_FOR;
                    ast->forStmt.iden = iden;
                    ast->forStmt.block = block;
                    ast->forStmt.container = container;
                }
            }
        }
        else
        {
            bssParserError(interp, "Expected identifier followed by 'in kw.");
        }
    }
    else
    {
        bssParserError(interp, "Expected 'for' kw, instead got '%S'", BSS_PARSER_CURR_TOK.lexeme);
    }

    return ast;
}

BssAstTopLevel *bssParserParseTopLevel(BssInterp *interp)
{
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

        case '{':
        {
            BssTok startTok = BSS_PARSER_CURR_TOK;
            BSS_PARSER_NEXT_TOK();

            BssTok endTok = BSS_PARSER_CURR_TOK;

            BssAstNamedExprList list = {0};
            while (!BSS_PARSER_MATCH('}') && !BSS_PARSER_MATCH(TOK_END_INPUT))
            {
                bool isNamed = false;
                BssAstExpr *lhs = bssParserParseExpr(interp, 0);
                BssAstExpr *rhs = BSS_AST_EXPR_ZERO;
                if (lhs != BSS_AST_EXPR_ZERO)
                {
                    if (BSS_PARSER_MATCH('='))
                    {
                        isNamed = true;

                        BSS_PARSER_NEXT_TOK();

                        rhs = bssParserParseExpr(interp, 0);
                        if (rhs == BSS_AST_EXPR_ZERO)
                        {
                            return BSS_AST_EXPR_ZERO;
                        }
                    }

                    BssAstNamedExpr *ne = arenaPushType(interp->arena, BssAstNamedExpr);
                    ne->startTok = lhs->startTok;
                    ne->endTok = (isNamed) ? rhs->endTok : lhs->endTok;
                    ne->isNamed = isNamed;
                    ne->lhs = lhs;
                    ne->rhs = rhs;

                    BssAstNamedExprListPushNodeLast(&list, ne);

                    if (BSS_PARSER_MATCH(','))
                    {
                        BSS_PARSER_NEXT_TOK();
                    }
                    else
                    {
                        break;
                    }
                }
                else
                {
                    return BSS_AST_EXPR_ZERO;
                }
            }

            if (BSS_PARSER_MATCH('}'))
            {
                endTok = BSS_PARSER_CURR_TOK;
                BSS_PARSER_NEXT_TOK();

                expr = bssAllocExprCompound(interp, startTok, endTok, list);
            }
            else
            {
                bssParserError(interp, "Expected '}' to close compound list instead got '%S'", BSS_PARSER_CURR_TOK.lexeme);
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
    BASE_UNUSED_PARAM(op);

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
BssAstExpr *bssParserParseExprSubscript(BssInterp *interp, BssAstExpr *left, BssTok op)
{
    BASE_UNUSED_PARAM(op);

    BssAstExpr *expr = BSS_AST_EXPR_ZERO;

    BssAstExpr *index = bssParserParseExpr(interp, 0);

    if (index != BSS_AST_EXPR_ZERO)
    {
        if (BSS_PARSER_MATCH(']'))
        {
            BssTok end = BSS_PARSER_CURR_TOK;
            BSS_PARSER_NEXT_TOK();

            expr = bssAllocExprSubscript(interp, left->startTok, end, left, index);
        }
        else
        {
            bssParserError(interp, "Expected ']', instead got '%S'", BSS_PARSER_CURR_TOK.lexeme);
        }
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
        case '>':
        case '<':
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

        case '.':
        {
            if (BSS_PARSER_MATCH(TOK_IDEN))
            {
                BssAstExpr *rhs = bssAllocExprIden(interp, BSS_PARSER_CURR_TOK, BSS_PARSER_CURR_TOK, BSS_PARSER_CURR_TOK);
                BSS_PARSER_NEXT_TOK();
                
                expr = bssAllocExprBinary(interp, left->startTok, rhs->endTok, left, rhs, op);
            }
            else
            {
                bssParserError(interp, "Expected identifier after access operator, instead got '%S'", BSS_PARSER_CURR_TOK.lexeme);
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

bool bssParserParseLexed(BssInterp *interp)
{
    interp->parser = arenaPushType(interp->arena, BssParser);
    interp->parser->file = arenaPushType(interp->arena, BssAstFile);
    interp->rootScope = bssAllocScope(arenaAllocDefault(), null, false);

    BssAstTopLevelList toplevels = bssParserParseTopLevels(interp);
    interp->parser->file->toplevels = toplevels;
    interp->parser->file->startTok = toplevels.first ? toplevels.first->startTok : (BssTok){0};
    interp->parser->file->endTok = toplevels.last ? toplevels.first->endTok : (BssTok){0};

    return true;
}
bool bssParserParseFile(BssInterp *interp, str8 file)
{
    if (bssLexerLexFile(interp, file))
    {
        return bssParserParseLexed(interp);
    }

    return false;
}