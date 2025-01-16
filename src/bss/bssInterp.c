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

void bssInterpWholeProject(struct BSSInterpretorState *iState)
{
    bssInterpStmts(iState, iState->pState.proj->stmts);
}

void bssInterpStmts(struct BSSInterpretorState *iState, ASTStmtList stmts)
{
    BASE_LIST_FOREACH(ASTStmt, stmt, stmts)
    {
        bssInterpStmt(iState, stmt);
    }
}
void bssInterpStmt(struct BSSInterpretorState *iState, ASTStmt *stmt)
{
    switch(stmt->kind)
    {
        case AST_STMT_EXPR:
        {
            bssInterpExpr(iState, stmt->expr);
        }break;
        case AST_STMT_BLOCK:
        {
            bssInterpStmts(iState, stmt->block->stmts);
        }break;
        case AST_STMT_IF:
        {
            bssInterpExpr(iState, stmt->ifstmt.cond);

            bool cond = false;
            if(BSS_VALUE_TRYBOOL(stmt->ifstmt.cond->value, cond))
            {
                if(cond)
                {
                    bssInterpStmts(iState, stmt->ifstmt.then->stmts);
                }
                else if(stmt->ifstmt.elseblock != null)
                {
                    bssInterpStmts(iState, stmt->ifstmt.elseblock->stmts);
                }
            }
        }break;
        case AST_STMT_FOR_LOOP:
        {
            bssInterpExpr(iState, stmt->forStmt.container);

            BssValue *cont = stmt->forStmt.container->value;

            BssValueList arr = {0};
            if(BSS_VALUE_TRYARRAY(cont, arr))
            {
                BASE_LIST_FOREACH(BssValue, v, cont->arr.val)
                {
                    stmt->forStmt.itEntry->value = v;
                    bssInterpStmts(iState, stmt->forStmt.block->stmts);
                }
            }
            
        }break;
        case AST_STMT_ASSIGN:
        {
            bssInterpExpr(iState, stmt->assign.lhs);
            bssInterpExpr(iState, stmt->assign.rhs);

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

Str8List bssInterpProcessFormatString(struct BSSInterpretorState *iState, ASTExpr *stringLit, str8 in)
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

                bssLexerInitFromBuffer(&fmtIState, fmtBuffer);
                fmtIState.lState.filePath = iState->lState.filePath;

                bssLexerLexWholeBuffer(&fmtIState);

                ASTExpr *expr = bssParserExpr(&fmtIState);

                bssCheckerInit(&fmtIState);
                bssCheckerCheckExpr(&fmtIState, expr, stringLit->scope);

                bssInterpExpr(&fmtIState, expr);

                BssValue *val = expr->value;

                for(u64 c = 0; c < val->strRep.len; c++)
                {
                    U8ChunkListPushLast(temp.arena, &charList, val->strRep.data[c]);
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
    }

    baseTempEnd(temp);

    return out;    
}

BssValueList BssValueListFromStr8List(BaseArena *arena, Str8List list)
{
    BssValueList ret = {0};

    BASE_LIST_FOREACH(Str8ListNode, sN, list)
    {
        BssValue *value = baseArenaPushType(arena, BssValue);
        value->hasBssValue = true;
        value->type = bssAllocTypeString(arena);
        Str8ListPushLast(arena, &value->str.val, sN->val);
        value->strRep = sN->val;

        BssValueListPushNodeLast(&ret, value);
    }

    return ret;
}
void bssInterpExpr(struct BSSInterpretorState *iState, ASTExpr *expr)
{
    switch(expr->kind)
    {
        case AST_EXPR_LIT:
        {
            switch(expr->checkType->kind)
            {
                case BSS_TYPE_INT:
                {
                    i64 i = atoll((i8*)expr->lit.lexeme.data);

                    expr->value = baseArenaPushType(iState->checkerArena, BssValue);
                    expr->value->type = expr->checkType;
                    expr->value->hasBssValue = true;
                    expr->value->integer.val = i;

                    expr->value->strRep = expr->lit.lexeme;
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

                    expr->value->strRep = expr->lit.lexeme;
                }break;

                case BSS_TYPE_STRING:
                {
                    Str8List list = {0};
                    str8 str = bssGetStr8RepFromTokLexeme(iState->checkerArena, expr->lit);
                    Str8ListPushLast(iState->checkerArena, &list, str);

                    if(expr->lit.isFmtStr)
                    {
                        list = bssInterpProcessFormatString(iState, expr, str);
                    }
                    
                    expr->value = baseArenaPushType(iState->checkerArena, BssValue);
                    expr->value->type = expr->checkType;
                    expr->value->hasBssValue = true;
                    expr->value->str.val = list;
                    expr->value->strRep = Str8ListJoin(iState->checkerArena, &list, null);

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
            bssInterpExpr(iState, expr->membAccess.lhs);

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
            bssInterpExpr(iState, expr->index.lhs);
            bssInterpExpr(iState, expr->index.indexExpr);


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
                    bssInterpExpr(iState, ne->exprLhs);

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
                    bssInterpExpr(iState, ne->exprRhs);

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
            bssInterpExpr(iState, expr->funcCall.func);

            BASE_LIST_FOREACH(ASTNamedExpr, ne, expr->funcCall.args)
            {
                bssInterpExpr(iState, ne->exprLhs);
            }

            str8 funcName = expr->funcCall.func->iden.lexeme;
            if (baseStringsStrEquals(funcName, STR8_LIT("print"), 0))
            {
                BssValue *p1 = expr->funcCall.args.first->exprLhs->value;
                
                str8 joined = Str8ListJoin(iState->checkerArena, &p1->str.val, null);

                baseColPrintf("%S", joined);
            }
            else if (baseStringsStrEquals(funcName, STR8_LIT("getfiles"), 0))
            {
                str8 path = expr->funcCall.args.first->exprLhs->value->strRep;
                str8 pattern = expr->funcCall.args.first->next->exprLhs->value->strRep;
                bool recursive = expr->funcCall.args.first->next->next->exprLhs->value->boolean.val;

                Str8List paths = OSGetFilePaths(iState->checkerArena, path, pattern, recursive);

                if(!BASE_ANY(paths))
                {
                    bssInterpError(iState, 
                                    expr->startTok,
                                    expr->endTok,
                                    "No files found in directory '%S', with pattern '%S'.\n", path, pattern);
                }

                BssValue *val = baseArenaPushType(iState->checkerArena, BssValue);
                val->hasBssValue = true;
                val->type = expr->checkType;
                val->arr.val = BssValueListFromStr8List(iState->checkerArena, paths);
                val->strRep = Str8ListJoin(iState->checkerArena, &paths, &(Str8ListJoinParams){.pre = STR8_LIT("["), .sep = STR8_LIT(","), .post = STR8_LIT("]")});

                expr->value = val;
            }
            else if (baseStringsStrEquals(funcName, STR8_LIT("quoteit"), 0))
            {
                BssValue *arrVal = expr->funcCall.args.first->exprLhs->value;

                Str8List list = {0};

                BASE_LIST_FOREACH(BssValue, v, arrVal->arr.val)
                {
                    Str8ListPushLastFmt(iState->checkerArena, &list, "\"%S\"", v->strRep);
                }

                BssValue *val = baseArenaPushType(iState->checkerArena, BssValue);
                val->hasBssValue = true;
                val->type = expr->checkType;
                val->arr.val = BssValueListFromStr8List(iState->checkerArena, list);
                val->strRep = Str8ListJoin(iState->checkerArena, &list, &(Str8ListJoinParams){.pre = STR8_LIT("["), .sep = STR8_LIT(","), .post = STR8_LIT("]")});

                expr->value = val;
            }
            else if (baseStringsStrEquals(funcName, STR8_LIT("join"), 0))
            {
                BssValue *arrVal = expr->funcCall.args.first->exprLhs->value;
                str8 sep = expr->funcCall.args.first->next->exprLhs->value->strRep;

                Str8List list = {0};

                BASE_LIST_FOREACH(BssValue, v, arrVal->arr.val)
                {
                    Str8ListPushLast(iState->checkerArena, &list, v->strRep);
                }

                BssValue *val = baseArenaPushType(iState->checkerArena, BssValue);
                val->hasBssValue = true;
                val->type = expr->checkType;

                val->strRep = Str8ListJoin(iState->checkerArena, &list, &(Str8ListJoinParams){.sep = sep});
                Str8ListPushLast(iState->checkerArena, &val->str.val, val->strRep);

                expr->value = val;
            }
            else if (baseStringsStrEquals(funcName, STR8_LIT("hasflag"), 0))
            {
                str8 flag = expr->funcCall.args.first->exprLhs->value->strRep;

                BssValue *val = baseArenaPushType(iState->checkerArena, BssValue);
                val->hasBssValue = true;
                val->type = expr->checkType;
                val->boolean.val = bssHasFlag(iState, flag);
                val->strRep = (val->boolean.val) ? STR8_LIT("true") : STR8_LIT("false");

                expr->value = val;
            }
            else if (baseStringsStrEquals(funcName, STR8_LIT("addflag"), 0))
            {
                str8 flag = expr->funcCall.args.first->exprLhs->value->strRep;
                bssAddFlag(iState, flag);

                BssValue *val = baseArenaPushType(iState->checkerArena, BssValue);
                val->hasBssValue = true;
                val->type = expr->checkType;
                val->boolean.val = false;
                val->strRep = STR8_LIT("false");

                expr->value = val;
            }
            else if (baseStringsStrEquals(funcName, STR8_LIT("getenv"), 0))
            {
                str8 env = expr->funcCall.args.first->exprLhs->value->strRep;
                str8 envVal = OSGetEnvironmentVar(iState->checkerArena, env);

                BssValue *val = baseArenaPushType(iState->checkerArena, BssValue);
                val->hasBssValue = true;
                val->type = expr->checkType;
                Str8ListPushLast(iState->checkerArena, &val->str.val, envVal);
                val->strRep = envVal;

                expr->value = val;
            }
            
            else
            {
                bssInterpError(iState, expr->startTok, expr->endTok, "Unimplemented function.\n");
            }
        }break;
        case AST_EXPR_RUN:
        {
            bssInterpExpr(iState, expr->run.expr);

            str8 runString = expr->run.expr->value->strRep;
            str8 command = baseStringsPushStr8Fmt(iState->checkerArena, "cmd.exe /q /k \"@echo off && %S\"", runString);

            str8 out = {0}, err = {0};
            OSRunProcessEx(iState->checkerArena, STR8_LIT(""), command, null, &out, &err);

            BssValue *val = baseArenaPushType(iState->checkerArena, BssValue);
            val->hasBssValue = true;
            val->type = expr->checkType;
            
            {
                BssValueObjMemb *stdoutMemb = baseArenaPushType(iState->checkerArena, BssValueObjMemb);
                stdoutMemb->name = STR8_LIT("stdout");
                stdoutMemb->val = baseArenaPushType(iState->checkerArena, BssValue);
                stdoutMemb->val->type = val->type->obj.membScope->table.first->type;
                stdoutMemb->val->hasBssValue = true;
                Str8ListPushLast(iState->checkerArena, &stdoutMemb->val->str.val, out);

                stdoutMemb->val->strRep = out;

                BssValueObjMembListPushNodeLast(&val->obj.val, stdoutMemb);
            }

            {
                BssValueObjMemb *stderrMemb = baseArenaPushType(iState->checkerArena, BssValueObjMemb);
                stderrMemb->name = STR8_LIT("stderr");
                stderrMemb->val = baseArenaPushType(iState->checkerArena, BssValue);
                stderrMemb->val->type = val->type->obj.membScope->table.first->next->type;
                stderrMemb->val->hasBssValue = true;
                Str8ListPushLast(iState->checkerArena, &stderrMemb->val->str.val, err);

                stderrMemb->val->strRep = err;

                BssValueObjMembListPushNodeLast(&val->obj.val, stderrMemb);
            }

            expr->value = val;
        }break;
        case AST_EXPR_BINARY:
        {
            bssInterpExpr(iState, expr->binaryOp.lhs);
            bssInterpExpr(iState, expr->binaryOp.rhs);

            if(bssIsTypeArray(expr->binaryOp.lhs->checkType) && bssIsTypeArray(expr->binaryOp.rhs->checkType))
            {
                BssValue *val = baseArenaPushType(iState->checkerArena, BssValue);
                val->hasBssValue = true;
                val->type = expr->checkType;

                BssValueList vals = {0};
                BASE_LIST_FOREACH(BssValue, v, expr->binaryOp.lhs->value->arr.val)
                {
                    BssValue *vcopy = baseArenaPushType(iState->checkerArena, BssValue);
                    *vcopy= *v;

                    BssValueListPushNodeLast(&vals, vcopy);
                }
                BASE_LIST_FOREACH(BssValue, v, expr->binaryOp.rhs->value->arr.val)
                {
                    BssValue *vcopy = baseArenaPushType(iState->checkerArena, BssValue);
                    *vcopy= *v;

                    BssValueListPushNodeLast(&vals, vcopy);
                }

                baseColPrintf("{r}%lld\n", vals.len);
                val->arr.val = vals;

                expr->value = val;
            }
            else if (bssIsTypeArray(expr->binaryOp.lhs->checkType) || bssIsTypeArray(expr->binaryOp.rhs->checkType))
            {
                bssInterpError(iState, expr->startTok, expr->endTok, "Unrecognised expr.\n");
            }
            else
            {
                if (bssIsTypeString(expr->binaryOp.lhs->checkType) && bssIsTypeString(expr->binaryOp.rhs->checkType))
                {
                    BssValue *val = baseArenaPushType(iState->checkerArena, BssValue);
                    val->hasBssValue = true;
                    val->type = expr->checkType;
                    Str8ListPushLast(iState->checkerArena, &val->str.val, expr->binaryOp.lhs->value->strRep);
                    Str8ListPushLast(iState->checkerArena, &val->str.val, expr->binaryOp.rhs->value->strRep);

                    val->strRep = Str8ListJoin(iState->checkerArena, &val->str.val, null);
                    expr->value = val;
                }
                else
                {
                    bssInterpError(iState, expr->startTok, expr->endTok, "Unrecognised expr.\n");
                }
            }
        }break;
        default:
        {
            bssInterpError(iState, expr->startTok, expr->endTok, "Unrecognised expr.\n");
        }break;
    }
}