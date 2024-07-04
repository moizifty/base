#include "base\baseMemory.h"
#include "base\baseThreads.h"
#include "os\core\osCore.h"
#include "bssInterp.h"

void bssInterpError(BSSInterpretorState *iState, BssTok start, BssTok end, char *msg, ...)
{
    va_list args;
    va_start(args, msg);

    baseColEPrintf("{Bu}%S (%lld: %lld)",start.pos.ownerLexer->filePath, start.pos.line, start.pos.col);
    baseColEPrintf("{B}\n        --> Runtime Error:\033[0m ");

    BaseArenaTemp temp = baseTempBegin(&iState->checkerArena, 1);
    {
        i64 needed = stbsp_vsnprintf(null, 0, msg, args) + 1;
        i8 *buf = baseArenaPush(temp.arena, needed);

        stbsp_vsnprintf(buf, (int)needed, msg, args);

        baseColEPrintf("%s", buf);
    }
    baseTempEnd(temp);

    va_end(args);

    bssLexerPrintTokenRange(start, end);
}

void bssInterpWholeProject(BSSInterpretorState *iState, BssCheckerState *cState)
{
    bssInterpStmts(iState, cState, cState->proj->stmts);
}

void bssInterpStmts(BSSInterpretorState *iState, BssCheckerState *cState, ASTStmtList stmts)
{
    BASE_LIST_FOREACH(ASTStmt, stmt, stmts)
    {
        bssInterpStmt(iState, cState, stmt);
    }
}
void bssInterpStmt(BSSInterpretorState *iState, BssCheckerState *cState, ASTStmt *stmt)
{
    switch(stmt->kind)
    {
        case AST_STMT_EXPR:
        {
            bssInterpExpr(iState, cState, stmt->expr);
        }break;
        case AST_STMT_BLOCK:
        {
            bssInterpStmts(iState, cState, stmt->block->stmts);
        }break;
        case AST_STMT_IF:
        {
            bssInterpExpr(iState, cState, stmt->ifstmt.cond);

            bool cond = false;
            if(BSS_VALUE_TRYBOOL(stmt->ifstmt.cond->value, cond))
            {
                if(cond)
                {
                    bssInterpStmts(iState, cState, stmt->ifstmt.then->stmts);
                }
                else
                {
                    bssInterpStmts(iState, cState, stmt->ifstmt.elseblock->stmts);
                }
            }
        }break;
        case AST_STMT_FOR_LOOP:
        {
            bssInterpExpr(iState, cState, stmt->forStmt.container);

            BssValue *cont = stmt->forStmt.container->value;

            BssValueList arr = {0};
            if(BSS_VALUE_TRYARRAY(cont, arr))
            {
                for(u64 i = 0; i < arr.len; i++)
                {
                    bssInterpStmts(iState, cState, stmt->forStmt.block->stmts);
                }
            }
            
        }break;
        case AST_STMT_ASSIGN:
        {
            bssInterpExpr(iState, cState, stmt->assign.lhs);
            bssInterpExpr(iState, cState, stmt->assign.rhs);

            if(stmt->assign.lhs->value != null && stmt->assign.rhs->value != null)
            {
                *stmt->assign.lhs->value = *stmt->assign.rhs->value;
            }

        }break;
        case AST_STMT_BUILD:
        case AST_STMT_PROJ_DECL:
        default:
        {
            bssInterpError(iState, stmt->startTok, stmt->endTok, "Unrecognised statement.\n");
        }break;
    }
}

Str8List bssInterpProcessFormatString(BSSInterpretorState *iState, BssCheckerState *cState, ASTExpr *stringLit, str8 in)
{
    Str8List out = {0};
    
    U8ChunkList charList = {0};

    BaseArenaTemp temp = baseTempBegin(&iState->checkerArena, 1);

    {
        u64 i;
        for(i = 0; i < in.len - 1; i++)
        {
            if (in.data[i] == '{' && in.data[i + 1] != '{')
            {
                U8Array fmtBuffer = {.data = in.data + i + 1};
                u64 numBracks = 1;
                while(in.data[i] != '}' || (numBracks != 0))
                {
                    i++;
                    
                    if (in.data[i] == '{' && in.data[i + 1] != '{') numBracks++;
                    else if(in.data[i] == '}' && in.data[i + 1] != '}') numBracks--;

                }

                fmtBuffer.len =  (in.data + i) - (fmtBuffer.data);

                BSSInterpretorState fmtIState = 
                {
                    .lexerArena = iState->lexerArena, 
                    .parserArena = iState->parserArena, 
                    .checkerArena = iState->checkerArena,
                };

                BssLexerState *fmtLexer = bssLexerInitFromBuffer(&fmtIState, fmtBuffer);
                bssLexerLexWholeBuffer(&fmtIState, fmtLexer);

                BssParserState fmtParser = {.lexer = fmtLexer};
                ASTExpr *expr = bssParserExpr(&fmtIState, &fmtParser);

                BssCheckerState fmtChecker = {0};
                bssCheckerCheckExpr(&fmtIState, &fmtChecker, expr, expr->scope);

                bssInterpExpr(iState, &fmtChecker, expr);

                BssValue *val = expr->value;

                str8 valStr = Str8ListJoin(temp.arena, &val->str.val, null);

                for(u64 i = 0; i < valStr.len; i++)
                {
                    U8ChunkListPushLast(temp.arena, &charList, valStr.data[i]);
                }
            }
            else if (in.data[i] == '{') // {{
            {
                U8ChunkListPushLast(temp.arena, &charList, '{');
                i++; // skip the {{
            }
            else
            {
                U8ChunkListPushLast(temp.arena, &charList, in.data[i]);
            }
        }

        if (i < in.len)
        {
            U8ChunkListPushLast(temp.arena, &charList, in.data[i]);
        }

        U8Array flattend = U8ChunkListFlattenToArray(iState->checkerArena, &charList);
        str8 final = {0};
        final.data = flattend.data;
        final.len = flattend.len;
        
        Str8ListPushLast(iState->checkerArena, &out, final);

        str8 joined = Str8ListJoin(iState->checkerArena, &out, null);

        int a = 90;
    }

    baseTempEnd(temp);

    return out;    
}

void bssInterpExpr(BSSInterpretorState *iState, BssCheckerState *cState, ASTExpr *expr)
{
    switch(expr->kind)
    {
        case AST_EXPR_LIT:
        {
            switch(expr->checkType->kind)
            {
                case BSS_TYPE_INT:
                {
                    i64 i = atoll(expr->lit.lexeme.data);

                    expr->value = baseArenaPushType(iState->checkerArena, BssValue);
                    expr->value->type = expr->checkType;
                    expr->value->hasBssValue = true;
                    expr->value->integer.val = i;
                }break;

                case BSS_TYPE_BOOL:
                {
                    bool value = true;
                    if (baseStringsStrEquals(expr->lit.lexeme, STR8_LIT("false"), 0))
                    {
                        value = false;
                    }

                    expr->value = baseArenaPushType(iState->checkerArena, BssValue);
                    expr->value->type = expr->checkType;
                    expr->value->hasBssValue = true;
                    expr->value->boolean.val = value;
                }break;

                case BSS_TYPE_STRING:
                {
                    Str8List list = {0};
                    str8 str = bssGetStr8RepFromTokLexeme(iState->checkerArena, expr->lit);
                    Str8ListPushLast(iState->checkerArena, &list, str);

                    if(expr->lit.isFmtStr)
                    {
                        list = bssInterpProcessFormatString(iState, cState, expr, str);
                    }
                    
                    expr->value = baseArenaPushType(iState->checkerArena, BssValue);
                    expr->value->type = expr->checkType;
                    expr->value->hasBssValue = true;
                    expr->value->str.val = list;
                }break;
            }break;
        }break;
        case AST_EXPR_IDEN:
        {
            if (expr->idenEntry->value == null)
            {
                expr->idenEntry->value = baseArenaPushType(iState->checkerArena, BssValue);
                expr->idenEntry->value->type = expr->checkType;
                expr->idenEntry->value->hasBssValue = false;
            }

            expr->value = expr->idenEntry->value;
        }break;
        case AST_EXPR_MEMBER_ACCESS:
        {
            bssInterpExpr(iState, cState, expr->membAccess.lhs);

            BssValueObjMemb *memb = null;
            BASE_LIST_FOREACH(BssValueObjMemb, m, expr->membAccess.lhs->value->obj.val)
            {
                if(baseStringsStrEquals(m->name, expr->membAccess.memb.lexeme, 0))
                {
                    memb = m;
                    break;
                }
            }

            if(memb != null)
            {
                if(memb->val == null)
                {
                    memb->val = baseArenaPushType(iState->checkerArena, BssValue);
                    memb->val->type = expr->checkType;
                    memb->val->hasBssValue = false;
                }

                expr->value = memb->val;
            }
        }break;
        case AST_EXPR_INDEX_ACCESS:
        {
            bssInterpExpr(iState, cState, expr->index.lhs);
            bssInterpExpr(iState, cState, expr->index.indexExpr);


            u64 index = 0;
            if(BSS_VALUE_TRYINT(expr->index.indexExpr->value, index))
            {
                if(index < expr->index.lhs->value->arr.val.len)
                {
                    u64 i = 0;
                    BASE_LIST_FOREACH_INDEX(BssValue, v, expr->index.lhs->value->arr.val, i)
                    {
                        if(i == index)
                        {
                            expr->value = v;
                            break;
                        }
                    }
                }
                else
                {
                    bssInterpError(iState, expr->index.indexExpr->startTok, expr->index.indexExpr->endTok, "Index out of range.\n");
                }
            }
        }break;
        case AST_EXPR_COMPOUND_LIT:
        {
            BssValue *value = null;
            value = baseArenaPushType(iState->checkerArena, BssValue);
            value->type = expr->checkType;
            value->hasBssValue = true;

            if(bssIsTypeArray(expr->checkType))
            {
                BASE_LIST_FOREACH(ASTNamedExpr, ne, expr->compoundLit.membs)
                {
                    bssInterpExpr(iState, cState, ne->exprLhs);

                    // todo: fix eerror, since ne->exprLhs->value might be a value from identifier
                    // that means that the value in the array will be the same pointer as the value for the identifier,
                    // not good.
                    // you need deep copy functions.
                    BssValueListPushNodeLast(&value->arr.val, ne->exprLhs->value);
                }
            }
            else
            {
                BASE_LIST_FOREACH(ASTNamedExpr, ne, expr->compoundLit.membs)
                {
                    bssInterpExpr(iState, cState, ne->exprRhs);

                    BssValueObjMemb *memb = baseArenaPushType(iState->checkerArena, BssValueObjMemb);
                    memb->name = ne->exprLhs->iden.lexeme;

                    // todo: fix eerror, since ne->exprLhs->value might be a value from identifier
                    // that means that the value in the array will be the same pointer as the value for the identifier,
                    // not good.
                    // you need deep copy functions.
                    memb->val = ne->exprRhs->value;


                    BssValueObjMembListPushNodeLast(&value->obj.val, memb);
                }
            }

            expr->value = value;
        }break;
        case AST_EXPR_FUNC_CALL:
        {
            bssInterpExpr(iState, cState, expr->funcCall.func);

            BASE_LIST_FOREACH(ASTNamedExpr, ne, expr->funcCall.args)
            {
                bssInterpExpr(iState, cState, ne->exprLhs);
            }

            str8 funcName = expr->funcCall.func->iden.lexeme;
            if (baseStringsStrEquals(funcName, STR8_LIT("print"), 0))
            {
                BssValue *p1 = expr->funcCall.args.first->exprLhs->value;
                
                str8 joined = Str8ListJoin(iState->checkerArena, &p1->str.val, null);

                baseColPrintf("%S", joined);
            }
            else
            {
                bssInterpError(iState, expr->startTok, expr->endTok, "Unimplemented function.\n");
            }
        }break;
        case AST_EXPR_BINARY:
        case AST_EXPR_RUN:
        default:
        {
            bssInterpError(iState, expr->startTok, expr->endTok, "Unrecognised expr.\n");
        }break;
    }
}