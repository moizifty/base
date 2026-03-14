#include "bssParser.h"
#include "bssInterp.h"
#include "bssBuiltins.h"

void bssInterpreterError(BssInterp *interp, BssTokPos start, BssTokPos end, i8 *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    baseColEPrintf("{Bu}%S (%lld, %lld)", start.ownerLexer->path, start.line, start.col);
    baseColEPrintf("{B}\n        --> Interpreter Error: ");
    baseEPrintfV(fmt, va);

    bssPrintSourceRange(start, end, 2);

    va_end(va);
}


BssValue *bssInterpreterInterpExprBinary(BssInterp *interp, BssAstExpr *expr)
{
    BssValue *value = BSS_VALUE_ZERO;

    BssValue *lhs = bssInterpreterInterpExpr(interp, expr->bin.left);
    BssValue *rhs = bssInterpreterInterpExpr(interp, expr->bin.right);
    if (lhs != BSS_VALUE_ZERO && rhs != BSS_VALUE_ZERO)
    {
        value = arenaPushType(interp->arena, BssValue);

        if(lhs->kind == BSS_VALUE_INT && rhs->kind == BSS_VALUE_INT)
        {
            value->kind = BSS_VALUE_INT;

            switch (expr->bin.op.kind)
            {
                case '+':
                {
                    value->num = lhs->num + rhs->num;
                    value->str = Str8PushFmt(interp->arena, "%lld", value->num);
                }break;

                case '-':
                {
                    value->num = lhs->num - rhs->num;
                    value->str = Str8PushFmt(interp->arena, "%lld", value->num);
                }break;

                case '*':
                {
                    value->num = lhs->num * rhs->num;
                    value->str = Str8PushFmt(interp->arena, "%lld", value->num);
                }break;

                case '/':
                {
                    // todo check for div by 0
                    value->num = lhs->num / rhs->num;
                    value->str = Str8PushFmt(interp->arena, "%lld", value->num);
                }break;

                case TOK_EQ_OP:
                {
                    // todo check for div by 0
                    value->kind = BSS_VALUE_BOOL;
                    value->num = lhs->num == rhs->num;
                    value->str = Str8PushFmt(interp->arena, "%s", value->num ? "true" : "false");
                }break;

                case TOK_NEQ_OP:
                {
                    // todo check for div by 0
                    value->kind = BSS_VALUE_BOOL;
                    value->num = lhs->num != rhs->num;
                    value->str = Str8PushFmt(interp->arena, "%s", value->num ? "true" : "false");
                }break;

                default:
                {
                    baseEPrintf("Unhandled binary operator '%S'\n", expr->bin.op.lexeme);
                }break;
            }
        }
    }

    return value;
}

BssValue *bssInterpreterInterpFunc(BssInterp *interp, BssAstFunc *func, BssAstExpr *callingExpr)
{
    if (callingExpr->call.args.len != func->params.len)
    {
        bssInterpreterError(interp, 
                            callingExpr->startTok.pos,
                            callingExpr->endTok.pos, 
                            "Calling function '%S' with '%llu' args when '%llu' are expected", 
                            func->iden.lexeme, 
                            callingExpr->call.args.len, func->params.len);

        return BSS_VALUE_ZERO;
    }

    BssValue *value = BSS_VALUE_VOID_VALUE;

    BssSymTableSlotEntry *entry = bssScopeFindEntry(interp->rootScope, func->iden.lexeme);

    BssScope *prevScope = interp->currScope;
    interp->currScope = entry->value->fn.defined.scope;
    
    // todo put in temp arena
    BssValueArray argValues = 
    {
        .data = arenaPushArray(interp->arena, BssValue*, callingExpr->call.args.len),
        .len = callingExpr->call.args.len,
    };

    u64 index = 0;
    BASE_LIST_FOREACH_INDEX(BssAstExpr, arg, callingExpr->call.args, index)
    {
        argValues.data[index] = bssInterpreterInterpExpr(interp, arg);
        if (argValues.data[index] == BSS_VALUE_ZERO)
        {
            return BSS_VALUE_ZERO;
        }
    }

    index = 0;
    BASE_LIST_FOREACH_INDEX(BssTokListNode, param, func->params, index)
    {
        BssSymTableSlotEntry *paramEntry = bssScopeFindEntry(interp->currScope, param->val.lexeme);
        paramEntry->value = argValues.data[index];
    }

    interp->returnSignaled = false;
    BASE_LIST_FOREACH(BssAstStmt, stmt, func->block->stmts)
    {
        if(!bssInterpreterInterpStmt(interp, stmt))
        {
            break;
        }

        if (interp->returnSignaled)
        {
            value = interp->lastRetValue;
            break;
        }
    }

    interp->currScope = prevScope;
    interp->returnSignaled = false;

    return value;
}

BssValue *bssInterpreterInterpExpr(BssInterp *interp, BssAstExpr *expr)
{
    BssValue *value = BSS_VALUE_ZERO;

    switch (expr->kind)
    {
        case BSS_AST_EXPR_LIT:
        {
            switch (expr->lit.kind)
            {
                case TOK_INT_LIT:
                {
                    value = arenaPushType(interp->arena, BssValue);
                    value->kind = BSS_VALUE_INT;
                    value->num = I64FromStr8(expr->lit.lexeme);
                    value->str = Str8PushCopy(interp->arena, expr->lit.lexeme);
                }break;
                case TOK_STR_LIT:
                {
                    value = arenaPushType(interp->arena, BssValue);
                    value->kind = BSS_VALUE_STRING;
                    value->str = bssGetStr8RepFromTokLexeme(interp->arena, expr->lit);
                }break;
                default:
                {
                    baseEPrintf("{r}Unhandled expression literal kind.\n");
                    bssPrintSourceRange(expr->startTok.pos, expr->endTok.pos, 1);
                }break;
            }
        }break;

        case BSS_AST_EXPR_BINARY:
        {
            value = bssInterpreterInterpExprBinary(interp, expr);
        }break;

        case BSS_AST_EXPR_FUNCCALL:
        {
            if (expr->call.lhs->kind == BSS_AST_EXPR_IDEN)
            {
                str8 fnIden = expr->call.lhs->iden.lexeme;
                BssSymTableSlotEntry *entry = bssScopeFindEntry(interp->rootScope, fnIden);
                
                if (entry != BSS_SYMTABLE_SLOT_ENTRY_ZERO &&
                    entry->value->kind == BSS_VALUE_FUNCTION)
                {
                    if (entry->value->fn.isBuiltin)
                    {
                        BssBuiltinFunc *builtin = bssBuiltinFunctionFindEntry(interp, fnIden);

                        value = builtin->func(interp, expr);
                    }
                    else
                    {
                        value = bssInterpreterInterpFunc(interp, entry->value->fn.defined.ast, expr);
                    }
                }
                else
                {
                    bssInterpreterError(interp, expr->startTok.pos, expr->endTok.pos, "Function '%S' is not defined", fnIden);
                }
            }
            else
            {
                bssInterpreterError(interp, expr->startTok.pos, expr->endTok.pos, "Expected an identifier for function call!");
            }
        }break;

        case BSS_AST_EXPR_IDEN:
        {
            BssSymTableSlotEntry *entry = bssScopeFindEntry(interp->currScope, expr->iden.lexeme);
            if (entry != BSS_SYMTABLE_SLOT_ENTRY_ZERO)
            {
                // todo implement value copying
                value = entry->value;
            }
            else
            {
                bssInterpreterError(interp, expr->startTok.pos, expr->endTok.pos, "Identifier '%S' was not defined.", expr->iden.lexeme);
            }
        }break;

        default:
        {
            baseEPrintf("{r}Unhandled expression kind.\n");
            bssPrintSourceRange(expr->startTok.pos, expr->endTok.pos, 1);
        }break;
    }

    return value;
}

bool bssInterpreterInterpStmt(BssInterp *interp, BssAstStmt *stmt)
{
    switch (stmt->kind)
    {
        case BSS_AST_STMT_EXPR:
        {
            BssValue *exprValue = bssInterpreterInterpExpr(interp, stmt->expr);
            return exprValue != BSS_VALUE_ZERO;
        }break;

        case BSS_AST_STMT_ASSIGN:
        {
            if (stmt->assign.lhs->kind == BSS_AST_EXPR_IDEN)
            {
                str8 iden = stmt->assign.lhs->iden.lexeme;
                BssSymTableSlotEntry *entry = BSS_SYMTABLE_SLOT_ENTRY_ZERO;
                bssScopePushEntry(interp, interp->currScope, iden, &entry);

                if (entry != BSS_SYMTABLE_SLOT_ENTRY_ZERO)
                {
                    entry->value = bssInterpreterInterpExpr(interp, stmt->assign.rhs);
                    return entry->value != BSS_VALUE_ZERO;
                }
            }
            else
            {

            }
        }break;

        case BSS_AST_STMT_RET:
        {
            interp->returnSignaled = true;

            if (stmt->retExpr == BSS_AST_EXPR_ZERO)
            {
                interp->lastRetValue = BSS_VALUE_VOID_VALUE;
                return true;
            }
            else
            {
                interp->lastRetValue = bssInterpreterInterpExpr(interp, stmt->retExpr);
                return interp->lastRetValue != BSS_VALUE_ZERO;
            }
        }break;

        default:
        {
            bssInterpreterError(interp, stmt->startTok.pos, stmt->endTok.pos, "Unhandled stmt");
        }break;
    }

    return false;
}

bool bssInterpreterInterpParsed(BssInterp *interp)
{
    interp->currScope = interp->rootScope;

    bssBuiltinFunctionPushEntry(interp, STR8_LIT("print"), 1, bssBuiltinPrint);

    bool result = true;
    BASE_LIST_FOREACH(BssAstTopLevel, toplevel, interp->parser->file->toplevels)
    {
        switch(toplevel->kind)
        {
            case BSS_AST_TOP_LEVEL_STMT:
            {
                if(!bssInterpreterInterpStmt(interp, toplevel->stmt))
                {
                    result = false;
                    goto LOOP_END;
                }

            }break;
            default: continue; // you have to skip functions
        }
    }
LOOP_END:
    return result;
}
bool bssInterpreterInterpFile(BssInterp *interp, str8 file)
{
    if(bssParserParseFile(interp, file))
    {
        return bssInterpreterInterpParsed(interp);
    }

    return false;
}