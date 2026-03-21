#include "bssParser.h"
#include "bssInterp.h"
#include "bssBuiltins.h"

void bssInterpreterError(BssInterp *interp, BssTokPos start, BssTokPos end, i8 *fmt, ...)
{
    BASE_UNUSED_PARAM(interp);

    va_list va;
    va_start(va, fmt);
    baseColEPrintf("{Bu}%S (%lld, %lld)", start.ownerLexer->path, start.line, start.col);
    baseColEPrintf("{B}\n        --> Interpreter Error: ");
    baseEPrintfV(fmt, va);

    bssPrintSourceRange(start, end, 2);

    va_end(va);
}

BssScope *bssInterpreterCreateFuncScopeAndPushArgs(BssInterp *interp, BssTokList params, BssAstExprList args)
{
    BssScope *fnScope = bssAllocScope(arenaAllocDefault(), interp->rootScope, true);
    BssScope *prevScope = interp->currScope;

    // todo put in temp arena
    BssValueArray argValues = 
    {
        .data = arenaPushArray(fnScope->scopeArena, BssValue*, args.len),
        .len = args.len,
    };

    u64 index = 0;
    BASE_LIST_FOREACH_INDEX(BssAstExpr, arg, args, index)
    {
        argValues.data[index] = bssInterpreterInterpExpr(interp, fnScope->scopeArena, arg);
        if (argValues.data[index] == BSS_VALUE_ZERO)
        {
            return BSS_SCOPE_ZERO;
        }
    }

    interp->currScope = fnScope;

    index = 0;
    BASE_LIST_FOREACH_INDEX(BssTokListNode, param, params, index)
    {
        BssSymTableSlotEntry *paramEntry = BSS_SYMTABLE_SLOT_ENTRY_ZERO;
        bssScopePushEntry(interp->currScope, param->val.lexeme, &paramEntry);

        paramEntry->value = argValues.data[index];
    }

    interp->currScope = prevScope;

    return fnScope;
}
BssValue *bssInterpreterInterpFunc(BssInterp *interp, Arena *scopeArena, BssAstFunc *func, BssAstExpr *callingExpr)
{
    BASE_UNUSED_PARAM(scopeArena);

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

    BssScope *fnScope = bssInterpreterCreateFuncScopeAndPushArgs(interp, func->params, callingExpr->call.args);
    BssScope *prevLastScope = interp->lastFnCalleeScope;
    interp->lastFnCalleeScope = interp->currScope;
    
    interp->currScope = fnScope;
    interp->currScope->isReturnedSignaled = false;

    BssValue *value = bssInterpreterInterpBlock(interp, func->block, false);

    // pass the value up the scope, so it doesnt get destroyed
    // we allocate the return value on interp->lastFnCalleeScope, but if this is a function scope
    // it will get free below in arenaFree
    // so copy it
    value = bssAllocValueCopy(interp->lastFnCalleeScope->scopeArena, value);

    arenaFree(fnScope->scopeArena);
    interp->currScope = interp->lastFnCalleeScope;
    interp->lastFnCalleeScope = prevLastScope;

    return value;
}

BssValue *bssInterpreterInterpExprDotAccess(BssInterp *interp, Arena *scopeArena, BssAstExpr *expr, BssSymTableSlotEntry **outSym)
{
    *outSym = BSS_SYMTABLE_SLOT_ENTRY_ZERO;

    BssValue *lhs = bssInterpreterInterpExpr(interp, scopeArena, expr->bin.left);
    if (lhs != BSS_VALUE_ZERO)
    {
        if (lhs->kind == BSS_VALUE_OBJECT)
        {
            str8 iden = expr->bin.right->iden.lexeme;
            *outSym = bssScopeFindEntry(lhs->obj, iden);
            if (*outSym == BSS_SYMTABLE_SLOT_ENTRY_ZERO)
            {
                bssInterpreterError(interp,
                                    expr->bin.right->startTok.pos,
                                    expr->bin.right->endTok.pos,
                                    "'%S' not found in object",
                                    iden);

                return BSS_VALUE_ZERO;
            }

            return (*outSym)->value;
        }
        else
        {
            bssInterpreterError(interp,
                                expr->bin.left->startTok.pos,
                                expr->bin.left->endTok.pos,
                                "Expected lhs of dot operator to be of type obj.");

            return BSS_VALUE_ZERO;
        }
    }

    return BSS_VALUE_ZERO;
}
BssValue *bssInterpreterInterpExprBinary(BssInterp *interp, Arena *scopeArena, BssAstExpr *expr)
{
    BssValue *value = BSS_VALUE_ZERO;
    BssValue *lhs = bssInterpreterInterpExpr(interp, scopeArena, expr->bin.left);

    if (expr->bin.op.kind == '.')
    {
        BssSymTableSlotEntry *outSym = BSS_SYMTABLE_SLOT_ENTRY_ZERO;
        return bssInterpreterInterpExprDotAccess(interp, scopeArena, expr, &outSym);
    }

    BssValue *rhs = bssInterpreterInterpExpr(interp, scopeArena, expr->bin.right);
    if (lhs != BSS_VALUE_ZERO && rhs != BSS_VALUE_ZERO)
    {
        if(lhs->kind == BSS_VALUE_INT && rhs->kind == BSS_VALUE_INT)
        {
            switch (expr->bin.op.kind)
            {
                case '+':
                {
                    value = bssAllocValueInt(scopeArena, lhs->num + rhs->num);
                }break;

                case '-':
                {
                    value = bssAllocValueInt(scopeArena, lhs->num - rhs->num);
                }break;

                case '*':
                {
                    value = bssAllocValueInt(scopeArena, lhs->num * rhs->num);
                }break;

                case '/':
                {
                    if (rhs->num == 0)
                    {
                        bssInterpreterError(interp, expr->bin.right->startTok.pos, expr->bin.right->endTok.pos, "Expression evaluates to 0, cannot divide by 0.");
                        return BSS_VALUE_ZERO;
                    }
                    else
                    {
                        value = bssAllocValueInt(scopeArena, lhs->num / rhs->num);
                    }
                }break;

                case '<':
                {
                    value = bssAllocValueBool(scopeArena, lhs->num < rhs->num);
                }break;

                case '>':
                {
                    value = bssAllocValueBool(scopeArena, lhs->num > rhs->num);
                }break;

                case TOK_EQ_OP:
                {
                    value = bssAllocValueBool(scopeArena, lhs->num == rhs->num);
                }break;

                case TOK_NEQ_OP:
                {
                    value = bssAllocValueBool(scopeArena, lhs->num != rhs->num);
                }break;

                default:
                {
                    bssInterpreterError(interp, expr->startTok.pos, expr->endTok.pos, "Operator '%S' not valid between types int", expr->bin.op.lexeme);
                    return BSS_VALUE_ZERO;
                }break;
            }
        }
        else if (lhs->kind == BSS_VALUE_BOOL && rhs->kind == BSS_VALUE_BOOL)
        {
            switch (expr->bin.op.kind)
            {
                case TOK_EQ_OP:
                {
                    value = bssAllocValueBool(scopeArena, lhs->num == rhs->num);
                }break;
                case TOK_NEQ_OP:
                {
                    value = bssAllocValueBool(scopeArena, lhs->num != rhs->num);
                }break;
                case TOK_LOGICAL_OR_OP:
                {
                    value = bssAllocValueBool(scopeArena, lhs->num || rhs->num);
                }break;
                case TOK_LOGICAL_AND_OP:
                {
                    value = bssAllocValueBool(scopeArena, lhs->num && rhs->num);
                }break;
                default:
                {
                    bssInterpreterError(interp, expr->startTok.pos, expr->endTok.pos, "Operator '%S' not valid between types bool", expr->bin.op.lexeme);
                    return BSS_VALUE_ZERO;
                }break;
            }
        }
        else if (lhs->kind == BSS_VALUE_ARRAY || rhs->kind == BSS_VALUE_ARRAY)
        {
            BssValue *arr = (lhs->kind == BSS_VALUE_ARRAY) ? lhs : rhs;
            BssValue *item = (lhs->kind == BSS_VALUE_ARRAY) ? rhs : lhs;

            switch (expr->bin.op.kind)
            {
                case '+':
                {
                    BssValueListPushNodeLast(&arr->array, item);
                }break;

                default:
                {
                    bssInterpreterError(interp, expr->startTok.pos, expr->endTok.pos, "Operator '%S' not valid with array", expr->bin.op.lexeme);
                    return BSS_VALUE_ZERO;
                }break;
            }

            return arr;
        }
        else if (lhs->kind == BSS_VALUE_STRING || rhs->kind == BSS_VALUE_STRING)
        {
            ArenaTemp temp = baseTempBegin(&scopeArena, 1);
            {
                switch (expr->bin.op.kind)
                {
                    case '+':
                    {
                        value = bssAllocValueStr8(scopeArena, Str8PushFmt(scopeArena, "%S%S", Str8FromBssValue(temp.arena, lhs), Str8FromBssValue(temp.arena, rhs)));
                    }break;

                    case TOK_EQ_OP:
                    {
                        value = bssAllocValueBool(scopeArena, Str8Equals(Str8FromBssValue(temp.arena, lhs), Str8FromBssValue(temp.arena, rhs), 0));
                    }break;

                    case TOK_NEQ_OP:
                    {
                        value = bssAllocValueBool(scopeArena, !Str8Equals(Str8FromBssValue(temp.arena, lhs), Str8FromBssValue(temp.arena, rhs), 0));
                    }break;

                    default:
                    {
                        bssInterpreterError(interp, expr->startTok.pos, expr->endTok.pos, "Operator '%S' not valid between types int", expr->bin.op.lexeme);
                        return BSS_VALUE_ZERO;
                    }break;
                }
            }

            baseTempEnd(temp);
        }
        else
        {
            bssInterpreterError(interp, expr->startTok.pos, expr->endTok.pos, "Binary op is not valid with these types.");
            return BSS_VALUE_ZERO;
        }
    }

    return value;
}
BssValue *bssInterpreterInterpExprUnary(BssInterp *interp, Arena *scopeArena, BssAstExpr *expr)
{
    BssValue *value = BSS_VALUE_ZERO;

    BssValue *term = bssInterpreterInterpExpr(interp, scopeArena, expr->unary.expr);
    if (term != BSS_VALUE_ZERO)
    {
        if(term->kind == BSS_VALUE_INT)
        {
            switch (expr->unary.op.kind)
            {
                case '+':
                {
                    value = bssAllocValueInt(scopeArena, term->num);
                }break;
                case '-':
                {
                    value = bssAllocValueInt(scopeArena, -term->num);
                }break;

                default:
                {
                    bssInterpreterError(interp, expr->startTok.pos, expr->endTok.pos, "Operator '%S' for type int", expr->unary.op.lexeme);
                    return BSS_VALUE_ZERO;
                }break;
            }
        }
        else
        {
            bssInterpreterError(interp, expr->startTok.pos, expr->endTok.pos, "Unary op is not valid for type.");
            return BSS_VALUE_ZERO;
        }
    }

    return value;
}

str8 bssInterpreterInterpStringSubst(BssInterp *interp, Arena *scopeArena, BssTok lit)
{
    U8ChunkList chunkList = {0};

    str8 lexeme = lit.lexeme;

    ArenaTemp temp = baseTempBegin(&scopeArena, 1);
    {
        lexeme = bssGetStr8RepFromTokLexeme(temp.arena, lit);
        for (u64 i = 0; i < lexeme.len; i++)
        {
            if (lexeme.data[i] == '{' && lexeme.data[i + 1] != '{')
            {
                i++;

                U8Array fmtStrBuffer = {.data = lexeme.data + i};
                u64 bracketCount = 1;
                while (i < lexeme.len && bracketCount != 0)
                {
                    if(lexeme.data[i] == '{') bracketCount++;
                    else if (lexeme.data[i] == '}') bracketCount--;

                    fmtStrBuffer.len++;
                    i++;
                }

                if (bracketCount != 0)
                {
                    bssInterpreterError(interp, lit.pos, lit.pos, "Mismatch of fmt string start brackets and end brackets");
                    baseTempEnd(temp);
                    return bssGetStr8RepFromTokLexeme(scopeArena, lit);
                }

                fmtStrBuffer.len -= 1; // get rid of included }

                BssInterp fmtInterp = {.arena = temp.arena};
                if(bssLexerFromBuffer(&fmtInterp, fmtStrBuffer))
                {
                    fmtInterp.lexer->path = interp->lexer->path;
                    bssLexerLexWhole(&fmtInterp);

                    BssAstExpr *expr = bssParserParseExpr(&fmtInterp, 0);
                    if (expr != BSS_AST_EXPR_ZERO &&
                        fmtInterp.lexer->tokArray.data[fmtInterp.lexer->currTokIndex].kind == TOK_END_INPUT)
                    {
                        expr->startTok.pos = lit.pos;
                        expr->endTok.pos = lit.pos;
                        BssValue *value = bssInterpreterInterpExpr(interp, temp.arena, expr);
                        if (value != BSS_VALUE_ZERO)
                        {
                            U8ChunkListPushStr8Last(temp.arena, &chunkList, Str8FromBssValue(temp.arena, value));
                        }
                    }
                    else
                    {
                        bssInterpreterError(interp, lit.pos, lit.pos, "Failed to parse singular expression, there should only be one expression inside fmt string");
                    }
                }
            }
        
            if (i < lexeme.len) U8ChunkListPushLast(temp.arena, &chunkList, lexeme.data[i]);
        }

        lexeme = Str8FromU8Array(U8ChunkListFlattenToArray(scopeArena, &chunkList));
    }

    baseTempEnd(temp);

    return lexeme;
}
BssValue *bssInterpreterInterpExpr(BssInterp *interp, Arena *scopeArena, BssAstExpr *expr)
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
                    value = bssAllocValueInt(scopeArena, I64FromStr8(expr->lit.lexeme));
                }break;
                case TOK_STR_LIT:
                {
                    value = bssAllocValueStr8(scopeArena, bssInterpreterInterpStringSubst(interp, scopeArena, expr->lit));
                }break;
                case TOK_BOOL_LIT:
                {
                    value = bssAllocValueBool(scopeArena, (expr->lit.lexeme.data[0] == 'f') ? false : true);
                }break;
                default:
                {
                    baseEPrintf("{r}Unhandled expression literal kind.\n");
                    bssPrintSourceRange(expr->startTok.pos, expr->endTok.pos, 1);
                }break;
            }
        }break;

        case BSS_AST_EXPR_UNARY:
        {
            value = bssInterpreterInterpExprUnary(interp, scopeArena, expr);
        }break;

        case BSS_AST_EXPR_BINARY:
        {
            value = bssInterpreterInterpExprBinary(interp, scopeArena, expr);
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

                        value = builtin->func(interp, scopeArena, expr);
                    }
                    else
                    {
                        value = bssInterpreterInterpFunc(interp, scopeArena, entry->value->fn.defined.ast, expr);
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

        case BSS_AST_EXPR_SUBSCRIPT:
        {
            BssValue *container = bssInterpreterInterpExpr(interp, scopeArena, expr->subscript.container);
            if (container != BSS_VALUE_ZERO)
            {
                if (container->kind == BSS_VALUE_ARRAY)
                {
                    BssValue *index = bssInterpreterInterpExpr(interp, scopeArena, expr->subscript.index);
                    if (index != BSS_VALUE_ZERO &&
                        index->kind == BSS_VALUE_INT)
                    {
                        if ((u64)index->num >= container->array.len)
                        {
                            bssInterpreterError(interp, 
                                                expr->subscript.index->startTok.pos, 
                                                expr->subscript.index->endTok.pos, 
                                                "Trying to index container of size '%lld' with index '%lld'",
                                                container->array.len,
                                                index->num);
                        }
                        else
                        {
                            i64 i = 0;
                            BASE_LIST_FOREACH_INDEX(BssValue, v, container->array, i)
                            {
                                if (i == index->num)
                                {
                                    value = v;
                                    break;
                                }
                            }
                        }
                    }
                    else
                    {
                        bssInterpreterError(interp, expr->subscript.index->startTok.pos, expr->subscript.index->endTok.pos, "Index expr needs to be of type int.");
                    }
                }
                else
                {
                    bssInterpreterError(interp, expr->startTok.pos, expr->endTok.pos, "Indexing a non indexable type.");
                }
            }
        }break;

        case BSS_AST_EXPR_COMPOUND:
        {
            // empty compounds are treated like arrays
            bool isArray = true;

            u64 numNamed = 0;
            // validate named expressions are idens, on the lhs
            bool valid = true;
            BASE_LIST_FOREACH(BssAstNamedExpr, ne, expr->compound)
            {
                if (ne->isNamed)
                {
                    isArray = false;

                    if (ne->lhs->kind != BSS_AST_EXPR_IDEN)
                    {
                        bssInterpreterError(interp, ne->lhs->startTok.pos, ne->lhs->endTok.pos, "Expected an identifier in named expression.");
                        valid = false;
                    }

                    numNamed += 1;
                }
            }

            if (!isArray && numNamed != expr->compound.len)
            {
                bssInterpreterError(interp, expr->startTok.pos, expr->endTok.pos, "All expressions need to be either named or unnamed.");
                return BSS_VALUE_ZERO;
            }

            if(valid)
            {
                BssValueList values = {0};
                BssScope *scope = !isArray ? bssAllocScope(scopeArena, null, false) : BSS_SCOPE_ZERO;

                BASE_LIST_FOREACH(BssAstNamedExpr, ne, expr->compound)
                {
                    if (ne->isNamed)
                    {
                        BssSymTableSlotEntry *entry = BSS_SYMTABLE_SLOT_ENTRY_ZERO;
                        bssScopePushEntry(scope, ne->lhs->iden.lexeme, &entry);

                        entry->value = bssInterpreterInterpExpr(interp, scopeArena, ne->rhs);
                        if (entry->value == BSS_VALUE_ZERO)
                        {
                            return BSS_VALUE_ZERO;
                        }
                    }
                    else
                    {
                        BssValue *val = bssInterpreterInterpExpr(interp, scopeArena, ne->lhs);
                        if (val != BSS_VALUE_ZERO)
                        {
                            BssValueListPushNodeLast(&values, val);
                        }
                        else
                        {
                            return BSS_VALUE_ZERO;
                        }
                    }
                }

                if (isArray)
                {
                    value = bssAllocValueArray(scopeArena, values);
                }
                else
                {
                    value = bssAllocValueObj(scopeArena, scope);
                }
            }
        }break;

        case BSS_AST_EXPR_IDEN:
        {
            BssSymTableSlotEntry *entry = bssScopeFindEntry(interp->currScope, expr->iden.lexeme);
            if (entry != BSS_SYMTABLE_SLOT_ENTRY_ZERO)
            {
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

BssValue *bssInterpreterInterpLValueExprAndGetSym(BssInterp *interp, Arena *scopeArena, BssAstExpr *expr, BssSymTableSlotEntry **outSym)
{
    *outSym = BSS_SYMTABLE_SLOT_ENTRY_ZERO;
    if (expr->kind == BSS_AST_EXPR_IDEN)
    {
        str8 iden = expr->iden.lexeme;
        *outSym = bssScopeFindEntry(interp->currScope, iden);

        return (*outSym)->value;
    }
    else if (expr->kind == BSS_AST_EXPR_SUBSCRIPT)
    {
        BssValue *val = bssInterpreterInterpLValueExprAndGetSym(interp, scopeArena, expr->subscript.container, outSym);
        if (val != BSS_VALUE_ZERO)
        {
            if (val->kind == BSS_VALUE_ARRAY)
            {
                BssValue *newValue = BSS_VALUE_ZERO;
                ArenaTemp temp = baseTempBegin(&scopeArena, 1);
                {
                    BssValue *indexValue = bssInterpreterInterpExpr(interp, temp.arena, expr->subscript.index);

                    if (indexValue != BSS_VALUE_ZERO)
                    {
                        if ((u64)indexValue->num >= val->array.len)
                        {
                            bssInterpreterError(interp, 
                                                expr->subscript.index->startTok.pos, 
                                                expr->subscript.index->endTok.pos, 
                                                "Trying to index container of size '%lld' with index '%lld'",
                                                val->array.len,
                                                indexValue->num);
                        }
                        else
                        {
                            i64 i = 0;
                            BASE_LIST_FOREACH_INDEX(BssValue, v, val->array, i)
                            {
                                if (i == indexValue->num)
                                {
                                    newValue = v;
                                    break;
                                }
                            }
                        }
                    }
                }
                
                baseTempEnd(temp);

                return newValue;
            }
            else
            {
                bssInterpreterError(interp, expr->subscript.container->startTok.pos, expr->subscript.container->endTok.pos, "Indexing a non indexable type.");
            }
        }
    }
    else if (expr->kind == BSS_AST_EXPR_BINARY && expr->bin.op.kind == '.')
    {
        BssValue *val = bssInterpreterInterpExprDotAccess(interp, scopeArena, expr, outSym);
        return val;
    }
    else
    {
        bssInterpreterError(interp, expr->startTok.pos, expr->endTok.pos, "Expression does not evaluate to an assignable value.");
    }

    return BSS_VALUE_ZERO;
}

bool bssInterpreterInterpStmt(BssInterp *interp, Arena *scopeArena, BssAstStmt *stmt)
{
    switch (stmt->kind)
    {
        case BSS_AST_STMT_EXPR:
        {
            BssValue *exprValue = bssInterpreterInterpExpr(interp, scopeArena, stmt->expr);
            return exprValue != BSS_VALUE_ZERO;
        }break;

        case BSS_AST_STMT_ASSIGN:
        {
            if (stmt->assign.lhs->kind == BSS_AST_EXPR_IDEN)
            {
                str8 iden = stmt->assign.lhs->iden.lexeme;
                BssSymTableSlotEntry *entry = BSS_SYMTABLE_SLOT_ENTRY_ZERO;
                bssScopePushEntry(interp->currScope, iden, &entry);

                if (entry != BSS_SYMTABLE_SLOT_ENTRY_ZERO)
                {
                    entry->value = bssInterpreterInterpExpr(interp, entry->scopeDefinedIn->scopeArena, stmt->assign.rhs);
                    return entry->value != BSS_VALUE_ZERO;
                }
            }
            else if (stmt->assign.lhs->kind == BSS_AST_EXPR_SUBSCRIPT)
            {
                BssSymTableSlotEntry *entry = BSS_SYMTABLE_SLOT_ENTRY_ZERO;
                BssValue *val = bssInterpreterInterpLValueExprAndGetSym(interp, scopeArena, stmt->assign.lhs, &entry);

                if (entry != BSS_SYMTABLE_SLOT_ENTRY_ZERO && val != BSS_VALUE_ZERO)
                {
                    BssValue *newValue = BSS_VALUE_ZERO;
                    ArenaTemp temp = baseTempBegin(&scopeArena, 1);
                    {
                        newValue = bssInterpreterInterpExpr(interp, temp.arena, stmt->assign.rhs);
                        if (newValue != BSS_VALUE_ZERO)
                        {
                            newValue = bssAllocValueCopy(entry->scopeDefinedIn->scopeArena, newValue);
                            newValue->next = val->next;
                            newValue->prev = val->prev;

                            *val = *newValue;
                        }
                    }

                    baseTempEnd(temp);

                    return newValue != BSS_VALUE_ZERO;
                }
            }
            else if (stmt->assign.lhs->kind == BSS_AST_EXPR_BINARY && stmt->assign.lhs->bin.op.kind == '.')
            {
                BssSymTableSlotEntry *entry = BSS_SYMTABLE_SLOT_ENTRY_ZERO;
                BssValue *val = bssInterpreterInterpLValueExprAndGetSym(interp, scopeArena, stmt->assign.lhs, &entry);

                if (entry != BSS_SYMTABLE_SLOT_ENTRY_ZERO && val != BSS_VALUE_ZERO)
                {
                    entry->value = bssInterpreterInterpExpr(interp, entry->scopeDefinedIn->scopeArena, stmt->assign.rhs);
                    return entry->value != BSS_VALUE_ZERO;
                }
            }
            else
            {
                bssInterpreterError(interp, stmt->startTok.pos, stmt->endTok.pos, "Lhs is not a valid LValue");
            }
        }break;
        
        case BSS_AST_STMT_IF:
        {
            BssValue *condValue = bssInterpreterInterpExpr(interp, scopeArena, stmt->ifStmt.cond);
            if (condValue != BSS_VALUE_ZERO)
            {
                if (condValue->kind == BSS_VALUE_BOOL)
                {
                    if (condValue->num)
                    {
                        if(bssInterpreterInterpBlock(interp, stmt->ifStmt.thenBlock, true) == BSS_VALUE_ZERO)
                        {
                            return false;
                        }
                    }
                    else if (stmt->ifStmt.elseBlock != BSS_AST_BLOCK_ZERO)
                    {
                        if(bssInterpreterInterpBlock(interp, stmt->ifStmt.elseBlock, true) == BSS_VALUE_ZERO)
                        {
                            return false;
                        }
                    }

                    return true;
                }
                else
                {
                    bssInterpreterError(interp,
                                        stmt->ifStmt.cond->startTok.pos,
                                        stmt->ifStmt.cond->endTok.pos,
                                        "Expected expression of boolean type for if condition");
                }
            }
        }break;

        case BSS_AST_STMT_WHILE:
        {
            bool loop = false;

            do
            {
                BssScope *blockScope = bssAllocScope(arenaAllocDefault(), interp->currScope, interp->currScope->isScopeInFunction);
                BssScope *prev = interp->currScope;
                interp->currScope = blockScope;

                BssValue *condValue = bssInterpreterInterpExpr(interp, blockScope->scopeArena, stmt->whileStmt.cond);
                bool valid = condValue != BSS_VALUE_ZERO && 
                                condValue->kind == BSS_VALUE_BOOL;

                if (valid)
                {
                    loop = condValue->num;
                    if (loop)
                    {
                        if(bssInterpreterInterpBlock(interp, stmt->whileStmt.block, false) == BSS_VALUE_ZERO)
                        {
                            arenaFree(blockScope->scopeArena);
                            return false;
                        }

                        interp->currScope = prev;
                    }
                }
                else
                {
                    bssInterpreterError(interp,
                                        stmt->whileStmt.cond->startTok.pos,
                                        stmt->whileStmt.cond->endTok.pos,
                                        "Expected expression of boolean type for while condition");
                    arenaFree(blockScope->scopeArena);
                    return false;
                }

                arenaFree(blockScope->scopeArena);
            }while(loop);

            return true;
        }break;

        case BSS_AST_STMT_FOR:
        {
            ArenaTemp temp = baseTempBegin(&interp->currScope->scopeArena, 1);
            {
                BssValue *containerValue = bssInterpreterInterpExpr(interp, temp.arena, stmt->forStmt.container);

                if (containerValue != BSS_VALUE_ZERO &&
                    containerValue->kind == BSS_VALUE_ARRAY)
                {
                    BASE_LIST_FOREACH(BssValue, v, containerValue->array)
                    {
                        BssScope *blockScope = bssAllocScope(arenaAllocDefault(), interp->currScope, interp->currScope->isScopeInFunction);
                        BssScope *prev = interp->currScope;
                        interp->currScope = blockScope;
                        
                        BssSymTableSlotEntry *idenEntry = BSS_SYMTABLE_SLOT_ENTRY_ZERO;
                        bssScopePushEntry(blockScope, stmt->forStmt.iden.lexeme, &idenEntry);
                        idenEntry->value = v;

                        if(bssInterpreterInterpBlock(interp, stmt->forStmt.block, false) == BSS_VALUE_ZERO)
                        {
                            arenaFree(blockScope->scopeArena);
                            baseTempEnd(temp);
                            return false;
                        }

                        interp->currScope = prev;
                        arenaFree(blockScope->scopeArena);

                    }
                }
                else
                {
                    bssInterpreterError(interp,
                                        stmt->forStmt.container->startTok.pos,
                                        stmt->forStmt.container->endTok.pos,
                                        "Expected container expression of array type");
                    baseTempEnd(temp);
                    
                    return false;
                }
            }

            baseTempEnd(temp);
            return true;
        }break;

        case BSS_AST_STMT_RET:
        {
            if (interp->currScope->isScopeInFunction)
            {
                interp->currScope->isReturnedSignaled = true;

                if (stmt->retExpr == BSS_AST_EXPR_ZERO)
                {
                    interp->lastRetValue = BSS_VALUE_VOID_VALUE;
                    return true;
                }
                else
                {
                    interp->lastRetValue = bssInterpreterInterpExpr(interp, interp->lastFnCalleeScope->scopeArena, stmt->retExpr);
                    return interp->lastRetValue != BSS_VALUE_ZERO;
                }
            }
            else
            {
                bssInterpreterError(interp, stmt->startTok.pos, stmt->endTok.pos, "Return stmt can only be used in function block!");
                return false;
            }
        }break;

        default:
        {
            bssInterpreterError(interp, stmt->startTok.pos, stmt->endTok.pos, "Unhandled stmt");
        }break;
    }

    return false;
}
BssValue *bssInterpreterInterpBlock(BssInterp *interp, BssAstBlock *block, bool createBlock)
{
    BssValue *value = BSS_VALUE_VOID_VALUE;
    BssScope *prevScope = interp->currScope;

    interp->currScope->isReturnedSignaled = false;

    if (createBlock)
    {
        BssScope *scope = bssAllocScope(arenaAllocDefault(), prevScope, prevScope->isScopeInFunction);
        interp->currScope = scope;
    }
    
    BASE_LIST_FOREACH(BssAstStmt, stmt, block->stmts)
    {
        if(!bssInterpreterInterpStmt(interp, interp->currScope->scopeArena, stmt))
        {
            break;
        }

        if (interp->currScope->isReturnedSignaled)
        {
            value = interp->lastRetValue;
            prevScope->isReturnedSignaled = true;
            break;
        }
    }

    if (createBlock)
    {
        arenaFree(interp->currScope->scopeArena);
        interp->currScope = prevScope;
    }

    return value;
}

bool bssInterpreterInterpParsed(BssInterp *interp)
{
    interp->currScope = interp->rootScope;

    bssBuiltinFunctionPushEntry(interp, STR8_LIT("print"), 1, bssBuiltinPrint);
    bssBuiltinFunctionPushEntry(interp, STR8_LIT("run"), 1, bssBuiltinRun);
    bssBuiltinFunctionPushEntry(interp, STR8_LIT("len"), 1, bssBuiltinLen);
    bssBuiltinFunctionPushEntry(interp, STR8_LIT("tostring"), 1, bssBuiltinToString);
    bssBuiltinFunctionPushEntry(interp, STR8_LIT("join"), 1, bssBuiltinJoin);
    bssBuiltinFunctionPushEntry(interp, STR8_LIT("qoute"), 1, bssBuiltinQoute);
    bssBuiltinFunctionPushEntry(interp, STR8_LIT("getenv"), 1, bssBuiltinGetenv);
    bssBuiltinFunctionPushEntry(interp, STR8_LIT("hasflag"), 1, bssBuiltinHasflag);

    bool result = true;
    BASE_LIST_FOREACH(BssAstTopLevel, toplevel, interp->parser->file->toplevels)
    {
        switch(toplevel->kind)
        {
            case BSS_AST_TOP_LEVEL_STMT:
            {
                if(!bssInterpreterInterpStmt(interp, interp->currScope->scopeArena, toplevel->stmt))
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