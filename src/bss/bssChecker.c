#include "base/baseThreads.h"

#include "bssAST.h"
#include "bssChecker.h"
#include "bssTypes.h"

BssTypeKind *gBssCheckerAllowedTypesBinaryOps[] = 
{
    ['+'] = (BssTypeKind[]){BSS_TYPE_INT, BSS_TYPE_ARRAY, BSS_TYPE_STRING, BSS_TYPE_INVALID},
    [TOK_LOGICAL_OR_OP] = (BssTypeKind[]){BSS_TYPE_BOOL, BSS_TYPE_INVALID},
    [TOK_LOGICAL_AND_OP] = (BssTypeKind[]){BSS_TYPE_BOOL, BSS_TYPE_INVALID},
};

void bssCheckerError(BSSInterpretorState *iState, BssTok tok, char *msg, ...)
{
    va_list args;
    va_start(args, msg);

    baseColEPrintf("{Bu}%S (%lld: %lld)",tok.pos.ownerLexer->filePath, tok.pos.line, tok.pos.col);
    baseColEPrintf("{B}\n        --> Checker Error:\033[0m ");

    ArenaTemp temp = baseTempBegin(&iState->checkerArena, 1);
    {
        i64 needed = stbsp_vsnprintf(null, 0, msg, args) + 1;
        i8 *buf = arenaPush(temp.arena, needed);

        stbsp_vsnprintf(buf, (int)needed, msg, args);

        baseColEPrintf("%s", buf);
    }
    baseTempEnd(temp);

    va_end(args);
}
void bssCheckerPrint(char *msg, ...);

void bssCheckerInit(struct BSSInterpretorState *iState)
{
    iState->cState.rootScope = bssNewScope(iState->checkerArena, null);
    
    {
        str8 cwdPath = Str8ChopPastLastSlash(iState->lState.filePath);
        if (Str8Equals(cwdPath, iState->lState.filePath, 0))
        {
            cwdPath = STR8_LIT(".\\");
        }

        BssSymTableSlotEntry *cwdVar = arenaPushType(iState->checkerArena, BssSymTableSlotEntry);
        cwdVar->name = STR8_LIT("cwd");
        cwdVar->type = bssAllocTypeString(iState->checkerArena);

        cwdVar->value = arenaPushType(iState->checkerArena, BssValue);
        cwdVar->value->type = cwdVar->type;
        cwdVar->value->hasBssValue = true;
        cwdVar->value->strRep = cwdPath;
        
        Str8ListPushLast(iState->checkerArena, &cwdVar->value->str.val, cwdPath);

        bssScopeAddEntry(iState->cState.rootScope, cwdVar);
    }

     //types
    {
        BssScope *s = bssNewScope(iState->checkerArena, null);
        BssType *runOutput = bssAllocTypeObj(iState->checkerArena, s);

        BssSymTableSlotEntry *stdoutMemb = arenaPushType(iState->checkerArena, BssSymTableSlotEntry);
        stdoutMemb->name = STR8_LIT("stdout");
        stdoutMemb->type = bssAllocTypeString(iState->checkerArena);

        BssSymTableSlotEntry *stderrMemb = arenaPushType(iState->checkerArena, BssSymTableSlotEntry);
        stderrMemb->name = STR8_LIT("stderr");
        stderrMemb->type = bssAllocTypeString(iState->checkerArena);

        bssScopeAddEntry(s, stdoutMemb);
        bssScopeAddEntry(s, stderrMemb);

        iState->cState.runOutput = runOutput;
    }

    {
        BssScope *s = bssNewScope(iState->checkerArena, null);
        BssType *projectDeclType = bssAllocTypeObj(iState->checkerArena, s);

        BssSymTableSlotEntry *srcMemb = arenaPushType(iState->checkerArena, BssSymTableSlotEntry);
        srcMemb->name = STR8_LIT("src");
        srcMemb->type = bssAllocTypeArray(iState->checkerArena, bssAllocTypeString(iState->checkerArena));

        BssSymTableSlotEntry *objMemb = arenaPushType(iState->checkerArena, BssSymTableSlotEntry);
        objMemb->name = STR8_LIT("objs");
        objMemb->type = bssAllocTypeArray(iState->checkerArena, bssAllocTypeString(iState->checkerArena));

        BssSymTableSlotEntry *libsMemb = arenaPushType(iState->checkerArena, BssSymTableSlotEntry);
        libsMemb->name = STR8_LIT("libs");
        libsMemb->type = bssAllocTypeArray(iState->checkerArena, bssAllocTypeString(iState->checkerArena));

        bssScopeAddEntry(s, srcMemb);
        bssScopeAddEntry(s, objMemb);
        bssScopeAddEntry(s, libsMemb);

        iState->cState.projectType = projectDeclType;
    }

    //functions
    {
        BssScope *fs = bssNewScope(iState->checkerArena, null);
        BssSymTableSlotEntry *p1 = arenaPushType(iState->checkerArena, BssSymTableSlotEntry);
        p1->name = STR8_LIT("path");
        p1->type = bssAllocTypeString(iState->checkerArena);

        BssSymTableSlotEntry *p2 = arenaPushType(iState->checkerArena, BssSymTableSlotEntry);
        p2->name = STR8_LIT("pattern");
        p2->type = bssAllocTypeString(iState->checkerArena);

        BssSymTableSlotEntry *p3 = arenaPushType(iState->checkerArena, BssSymTableSlotEntry);
        p3->name = STR8_LIT("recursive");
        p3->type = bssAllocTypeBool(iState->checkerArena);

        bssScopeAddEntry(fs, p1);
        bssScopeAddEntry(fs, p2);
        bssScopeAddEntry(fs, p3);

        BssType *r = bssAllocTypeArray(iState->checkerArena, bssAllocTypeString(iState->checkerArena));

        BssSymTableSlotEntry *f = arenaPushType(iState->checkerArena, BssSymTableSlotEntry);
        f->name = STR8_LIT("getfiles");
        f->type = bssAllocTypeFunc(iState->checkerArena, r, fs);

        
        bssScopeAddEntry(iState->cState.rootScope, f);
    }

    {
        BssScope *fs = bssNewScope(iState->checkerArena, null);
        BssSymTableSlotEntry *p1 = arenaPushType(iState->checkerArena, BssSymTableSlotEntry);
        p1->name = STR8_LIT("str");
        p1->type = bssAllocTypeString(iState->checkerArena);

        bssScopeAddEntry(fs, p1);

        BssType *r = bssAllocTypeString(iState->checkerArena);

        BssSymTableSlotEntry *f = arenaPushType(iState->checkerArena, BssSymTableSlotEntry);
        f->name = STR8_LIT("print");
        f->type = bssAllocTypeFunc(iState->checkerArena, r, fs);

        
        bssScopeAddEntry(iState->cState.rootScope, f);
    }

    {
        BssScope *fs = bssNewScope(iState->checkerArena, null);
        BssSymTableSlotEntry *p1 = arenaPushType(iState->checkerArena, BssSymTableSlotEntry);
        p1->name = STR8_LIT("str");
        p1->type = bssAllocTypeString(iState->checkerArena);

        bssScopeAddEntry(fs, p1);

        BssType *r = bssAllocTypeBool(iState->checkerArena);

        BssSymTableSlotEntry *f = arenaPushType(iState->checkerArena, BssSymTableSlotEntry);
        f->name = STR8_LIT("pathexists");
        f->type = bssAllocTypeFunc(iState->checkerArena, r, fs);

        
        bssScopeAddEntry(iState->cState.rootScope, f);
    }

    {
        BssScope *fs = bssNewScope(iState->checkerArena, null);
        BssSymTableSlotEntry *p1 = arenaPushType(iState->checkerArena, BssSymTableSlotEntry);
        p1->name = STR8_LIT("arr");
        p1->type = bssAllocTypeArray(iState->checkerArena, bssAllocTypeString(iState->checkerArena));

        BssSymTableSlotEntry *p2 = arenaPushType(iState->checkerArena, BssSymTableSlotEntry);
        p2->name = STR8_LIT("sep");
        p2->type = bssAllocTypeString(iState->checkerArena);

        bssScopeAddEntry(fs, p1);
        bssScopeAddEntry(fs, p2);

        BssType *r = bssAllocTypeString(iState->checkerArena);

        BssSymTableSlotEntry *f = arenaPushType(iState->checkerArena, BssSymTableSlotEntry);
        f->name = STR8_LIT("join");
        f->type = bssAllocTypeFunc(iState->checkerArena, r, fs);

        
        bssScopeAddEntry(iState->cState.rootScope, f);
    }

    {
        BssScope *fs = bssNewScope(iState->checkerArena, null);
        BssSymTableSlotEntry *p1 = arenaPushType(iState->checkerArena, BssSymTableSlotEntry);
        p1->name = STR8_LIT("arr");
        p1->type = bssAllocTypeArray(iState->checkerArena, bssAllocTypeString(iState->checkerArena));

        bssScopeAddEntry(fs, p1);

        BssType *r = bssAllocTypeArray(iState->checkerArena, bssAllocTypeString(iState->checkerArena));

        BssSymTableSlotEntry *f = arenaPushType(iState->checkerArena, BssSymTableSlotEntry);
        f->name = STR8_LIT("quoteit");
        f->type = bssAllocTypeFunc(iState->checkerArena, r, fs);

        
        bssScopeAddEntry(iState->cState.rootScope, f);
    }

    {
        BssScope *fs = bssNewScope(iState->checkerArena, null);
        BssSymTableSlotEntry *p1 = arenaPushType(iState->checkerArena, BssSymTableSlotEntry);
        p1->name = STR8_LIT("flag");
        p1->type = bssAllocTypeString(iState->checkerArena);

        bssScopeAddEntry(fs, p1);

        BssType *r = bssAllocTypeBool(iState->checkerArena);

        BssSymTableSlotEntry *f = arenaPushType(iState->checkerArena, BssSymTableSlotEntry);
        f->name = STR8_LIT("hasflag");
        f->type = bssAllocTypeFunc(iState->checkerArena, r, fs);

        
        bssScopeAddEntry(iState->cState.rootScope, f);
    }

    {
        BssScope *fs = bssNewScope(iState->checkerArena, null);
        BssSymTableSlotEntry *p1 = arenaPushType(iState->checkerArena, BssSymTableSlotEntry);
        p1->name = STR8_LIT("flag");
        p1->type = bssAllocTypeString(iState->checkerArena);

        bssScopeAddEntry(fs, p1);

        BssType *r = bssAllocTypeBool(iState->checkerArena);

        BssSymTableSlotEntry *f = arenaPushType(iState->checkerArena, BssSymTableSlotEntry);
        f->name = STR8_LIT("addflag");
        f->type = bssAllocTypeFunc(iState->checkerArena, r, fs);

        
        bssScopeAddEntry(iState->cState.rootScope, f);
    }

    {
        BssScope *fs = bssNewScope(iState->checkerArena, null);
        BssSymTableSlotEntry *p1 = arenaPushType(iState->checkerArena, BssSymTableSlotEntry);
        p1->name = STR8_LIT("env");
        p1->type = bssAllocTypeString(iState->checkerArena);

        bssScopeAddEntry(fs, p1);

        BssType *r = bssAllocTypeString(iState->checkerArena);

        BssSymTableSlotEntry *f = arenaPushType(iState->checkerArena, BssSymTableSlotEntry);
        f->name = STR8_LIT("getenv");
        f->type = bssAllocTypeFunc(iState->checkerArena, r, fs);

        
        bssScopeAddEntry(iState->cState.rootScope, f);
    }
}

void bssCheckerCheckWholeProject(struct BSSInterpretorState *iState)
{
    bssCheckerCheckStmtsEx(iState, iState->pState.proj->stmts, iState->cState.rootScope);
}

void bssCheckerCheckStmts(struct BSSInterpretorState *iState, ASTStmtList stmts, BssScope *parentScope)
{
    BssScope *scope = bssNewScope(iState->checkerArena, parentScope);

    bssCheckerCheckStmtsEx(iState, stmts, scope);
}
void bssCheckerCheckStmtsEx(struct BSSInterpretorState *iState, ASTStmtList stmts, BssScope *newScope)
{
    BASE_LIST_FOREACH(ASTStmt, stmt, stmts)
    {
        bssCheckerCheckStmt(iState, stmt, newScope);
    }
}

void bssCheckerCheckStmt(struct BSSInterpretorState *iState, ASTStmt *stmt, BssScope *scope)
{
    switch(stmt->kind)
    {
        case AST_STMT_PROJ_DECL:
        {
            str8 name = stmt->proj.iden.lexeme;
            BssSymTableSlotEntry *entry = bssScopeFindEntryFromName(scope, true, name);
            if(entry == null)
            {
                entry = arenaPushType(iState->checkerArena, BssSymTableSlotEntry);
                entry->name = name;
                entry->type = iState->cState.projectType;

                bssCheckerCheckExpr(iState, stmt->proj.assign, scope);

                if(!bssIsTypeString(stmt->proj.assign->checkType))
                {
                    bssCheckerError(iState, stmt->proj.assign->startTok, "Expected string expression, which is path to a project.\n");
                    bssLexerPrintTokenRange(stmt->proj.assign->startTok, stmt->proj.assign->endTok);
                }

                bssScopeAddEntry(scope, entry);
            }
            else
            {
                bssCheckerError(iState, stmt->startTok, "identifier already exists.\n");
                bssLexerPrintTokenRange(stmt->startTok, stmt->endTok);
            }
        }break;

        case AST_STMT_BUILD:
        {
            bssCheckerCheckExpr(iState, stmt->build.expr, scope);
            if(bssAreBssTypesEqual(iState->cState.projectType, stmt->build.expr->checkType))
            {
                bssCheckerCheckExpr(iState, stmt->build.buildArgs, scope);
            }
            else
            {
                bssCheckerError(iState, stmt->build.expr->startTok, "Expected identifier to have the same type as a project decl.\n");
                bssLexerPrintTokenRange(stmt->build.expr->startTok, stmt->build.expr->endTok);
            }
        }break;

        case AST_STMT_EXPR:
        {
            bssCheckerCheckExpr(iState, stmt->expr, scope);
        }break;

        case AST_STMT_ASSIGN:
        {
            ASTExpr *lhs = stmt->assign.lhs;
            ASTExpr *rhs = stmt->assign.rhs;

            bool addedNewIden = false;
            if(lhs->kind == AST_EXPR_IDEN)
            {
                str8 name = lhs->iden.lexeme;
                BssSymTableSlotEntry *entry = bssScopeFindEntryFromName(scope, true, name);
                if(entry == null)
                {
                    entry = arenaPushType(iState->checkerArena, BssSymTableSlotEntry);
                    entry->name = name;

                    bssCheckerCheckExpr(iState, rhs, scope);
                    entry->type = rhs->checkType;

                    bssScopeAddEntry(scope, entry);

                    lhs->idenEntry = entry;
                    addedNewIden = true;
                }
            }

            if(!addedNewIden)
            {
                bssCheckerCheckExpr(iState, lhs, scope);
                bssCheckerCheckExpr(iState, rhs, scope);

                // todo check if equal types
                if(!bssAreBssTypesEqual(lhs->checkType, rhs->checkType))
                {
                    bssCheckerError(iState, stmt->startTok, "LHS and RHS of assign statement have different types.\n");
                    bssLexerPrintTokenRange(stmt->startTok, stmt->endTok);
                }
            }

        }break;

        case AST_STMT_BLOCK:
        {
            bssCheckerCheckStmts(iState, stmt->block->stmts, scope);
        }break;

        case AST_STMT_FOR_LOOP:
        {
            ASTExpr *container = stmt->forStmt.container;
            bssCheckerCheckExpr(iState, container, scope);

            if(bssIsTypeArray(container->checkType))
            {
                BssScope *s = bssNewScope(iState->checkerArena, scope);


                BssSymTableSlotEntry *entry = bssScopeFindEntryFromName(s, true, stmt->forStmt.item.lexeme);
                if(entry == null)
                {
                    entry = arenaPushType(iState->checkerArena, BssSymTableSlotEntry);
                    entry->name = stmt->forStmt.item.lexeme;
                    entry->type = container->checkType->array.base;

                    bssScopeAddEntry(s, entry);

                    stmt->forStmt.itEntry = entry;
                }
                else
                {
                    bssCheckerError(iState, stmt->startTok, "For loop identifier already declared.\n");
                    bssLexerPrintTokenRange(stmt->forStmt.item, stmt->forStmt.item);
                }

                bssCheckerCheckStmtsEx(iState, stmt->forStmt.block->stmts, s);
            }
            else
            {
                bssCheckerError(iState, stmt->startTok, "For loop expression is not a valid container.\n");
                bssLexerPrintTokenRange(container->startTok, container->endTok);
            }
        }break;

        case AST_STMT_IF:
        {
            ASTExpr *cond = stmt->ifstmt.cond;
            ASTBlock *then = stmt->ifstmt.then;
            ASTBlock *els = stmt->ifstmt.elseblock;

            bssCheckerCheckExpr(iState, cond, scope);
            
            if(bssIsTypeBool(cond->checkType))
            {
                bssCheckerCheckStmts(iState, then->stmts, scope);

                if(els != null)
                {
                    bssCheckerCheckStmts(iState, els->stmts, scope);
                }
            }
            else
            {
                bssCheckerError(iState, stmt->startTok, "If statement condition should be a boolean.\n");
                bssLexerPrintTokenRange(cond->startTok, cond->endTok);
            }
        }break;

        default:
        {
            bssCheckerError(iState, stmt->startTok, "Unregognised Statement\n");
            bssLexerPrintTokenRange(stmt->startTok, stmt->endTok);
        }break;
    }
}

bool bssCheckerCheckIfTypeAllowedForBinaryOp(BssType *a, BssType *b, BssTokKind op)
{
    BssTypeKind *typesAllowed = gBssCheckerAllowedTypesBinaryOps[op];
    if(typesAllowed != null)
    {
        for(u64 i = 0; typesAllowed[i] != BSS_TYPE_INVALID; i++)
        {
            if (typesAllowed[i] == a->kind || typesAllowed[i] == b->kind)
            {
                return true;
            }
        }
    }

    return false;
}

void bssCheckerCheckExpr(struct BSSInterpretorState *iState, ASTExpr *expr, BssScope *scope)
{
    expr->scope = scope;
    switch(expr->kind)
    {
        case AST_EXPR_LIT:
        {
            switch(expr->lit.kind)
            {
                case TOK_INT_LIT:
                {
                    expr->checkType = bssAllocTypeInt(iState->checkerArena);
                }break;

                case TOK_BOOL_LIT:
                {
                    expr->checkType = bssAllocTypeBool(iState->checkerArena);
                }break;

                case TOK_STR_LIT:
                {
                    expr->checkType = bssAllocTypeString(iState->checkerArena);
                }break;
            }
        }break;
        case AST_EXPR_IDEN:
        {
            BssSymTableSlotEntry *entry = bssScopeFindEntryFromName(scope, true, expr->iden.lexeme);
            if (entry != null)
            {
                expr->idenEntry = entry;
                expr->checkType = entry->type;
            }
            else
            {
                bssCheckerError(iState, expr->startTok, "Identifier '%S' is not declared anywhere.\n", expr->iden.lexeme);
                bssLexerPrintTokenRange(expr->startTok, expr->endTok);
            }
        }break;
        case AST_EXPR_BINARY:
        {
            ASTExpr *l = expr->binaryOp.lhs;
            ASTExpr *r = expr->binaryOp.rhs;

            bssCheckerCheckExpr(iState, l, scope);
            bssCheckerCheckExpr(iState, r, scope);

            if (l->checkType && r->checkType)
            {
                if(bssCheckerCheckIfTypeAllowedForBinaryOp(l->checkType, r->checkType, expr->binaryOp.op.kind))
                {
                    bool oneIsArray = bssIsTypeArray(l->checkType) || bssIsTypeArray(r->checkType);
                    bool oneIsString = bssIsTypeString(l->checkType) || bssIsTypeString(r->checkType);

                    if(oneIsArray)
                    {
                        expr->checkType = (bssIsTypeArray(l->checkType)) ? l->checkType : r->checkType;
                    }
                    else if (oneIsString)
                    {
                        expr->checkType = (bssIsTypeString(l->checkType)) ? l->checkType : r->checkType;
                    }
                    else
                    {
                        expr->checkType = l->checkType;
                    }
                }
                else
                {
                    bssCheckerError(iState, expr->binaryOp.op, "Binary op '%S' cannot be used with this type.\n", expr->binaryOp.op.lexeme);
                    bssLexerPrintTokenRange(expr->binaryOp.op, expr->binaryOp.op);
                }
            }
        }break;
        case AST_EXPR_INDEX_ACCESS:
        {
            ASTExpr *arr = expr->index.lhs;
            ASTExpr *index = expr->index.indexExpr;

            bssCheckerCheckExpr(iState, arr, scope);
            bssCheckerCheckExpr(iState, index, scope);

            if(bssIsTypeArray(arr->checkType))
            {
                if(bssIsTypeInt(index->checkType))
                {
                    expr->checkType = arr->checkType->array.base;
                }
                else
                {
                    bssCheckerError(iState, expr->startTok, "Array index expression is not an integer.");
                    bssLexerPrintTokenRange(index->startTok, index->endTok);
                }
            }
            else
            {
                bssCheckerError(iState, expr->startTok, "Indexing a non array type.");
                bssLexerPrintTokenRange(arr->startTok, arr->endTok);
            }
        }break;
        case AST_EXPR_MEMBER_ACCESS:
        {
            ASTExpr *o = expr->membAccess.lhs;
            BssTok iden = expr->membAccess.memb;

            bssCheckerCheckExpr(iState, o, scope);

            if(bssIsTypeObj(o->checkType))
            {
                str8 name = iden.lexeme;
                BssSymTableSlotEntry *entry = bssScopeFindEntryFromName(o->checkType->obj.membScope, false, name);
                if (entry != null)
                {
                    expr->checkType = entry->type;
                }
                else
                {
                    bssCheckerError(iState, expr->startTok, "Obj type on lhs has no member '%S'.\n", name);
                    bssLexerPrintTokenRange(iden, iden);
                }
            }
            else
            {
                bssCheckerError(iState, expr->startTok, "Using '.' operator with a non obj type.\n");
                bssLexerPrintTokenRange(o->startTok, o->endTok);
            }
        }break;
        case AST_EXPR_RUN:
        {
            bssCheckerCheckExpr(iState, expr->run.expr, scope);

            if(!bssIsTypeString(expr->run.expr->checkType))
            {
                bssCheckerError(iState, expr->startTok, "Argument to run command is not of string type.\n");
                bssLexerPrintTokenRange(expr->startTok, expr->endTok);
            }

            expr->checkType = iState->cState.runOutput;
        }break;
        case AST_EXPR_FUNC_CALL:
        {
            ASTExpr *iden = expr->funcCall.func;
            if (iden->kind == AST_EXPR_IDEN)
            {
                bssCheckerCheckExpr(iState, iden, scope);
                if(bssIsTypeFunc(iden->checkType))
                {
                    BssType *funcType = iden->checkType;
                    if(funcType->func.scope->table.len != expr->funcCall.args.len)
                    {
                        bssCheckerError(iState, iden->startTok, "Expected '%llu' args instead got '%llu'\n", funcType->func.scope->table.len, expr->funcCall.args.len);
                        bssLexerPrintTokenRange(iden->startTok, iden->endTok);
                    }
                    else
                    {
                        BssSymTableSlotEntry *currParam = funcType->func.scope->table.first;
                        BASE_LIST_FOREACH(ASTNamedExpr, ne, expr->funcCall.args)
                        {
                            bssCheckerCheckExpr(iState, ne->exprLhs, scope);

                            if(!bssAreBssTypesEqual(currParam->type, ne->exprLhs->checkType))
                            {
                                bssCheckerError(iState, ne->startTok, "Parameter is wrong type for function call.\n");
                                bssLexerPrintTokenRange(ne->startTok, ne->endTok);
                            }

                            currParam = currParam->next;
                        }

                        expr->checkType = funcType->func.ret;
                    }
                }
                else
                {
                    bssCheckerError(iState, iden->startTok, "Identifier is not a valid function.\n");
                    bssLexerPrintTokenRange(iden->startTok, iden->endTok);
                }
            }
            else
            {
                bssCheckerError(iState, iden->startTok, "Can only use function call on identifiers.\n");
                bssLexerPrintTokenRange(iden->startTok, iden->endTok);
            }
        }break;
        case AST_EXPR_COMPOUND_LIT:
        {
            bool allNamed = true;
            bool allUnnamed = true;
            BASE_LIST_FOREACH(ASTNamedExpr, ne, expr->compoundLit.membs)
            {
                allNamed = allNamed && ne->hasName;
                allUnnamed = allUnnamed && !ne->hasName;
            }

            if(!allNamed && !allUnnamed)
            {
                bssCheckerError(iState, expr->startTok, "Compound literals must either have ALL expressions be named or unnamed, not both.\n");
                bssLexerPrintTokenRange(expr->startTok, expr->endTok);
            }
            else if(allNamed)
            {
                BssScope *s = bssNewScope(iState->checkerArena, null);
                BASE_LIST_FOREACH(ASTNamedExpr, ne, expr->compoundLit.membs)
                {
                    BssSymTableSlotEntry *e = arenaPushType(iState->checkerArena, BssSymTableSlotEntry);
                    bssCheckerCheckExpr(iState, ne->exprRhs, scope);

                    e->name = ne->exprLhs->iden.lexeme;
                    e->type = ne->exprRhs->checkType;

                    bssScopeAddEntry(s, e);
                }

                expr->checkType = bssAllocTypeObj(iState->checkerArena, s);
            }
            else
            {
                BssType *base = null;
                BASE_LIST_FOREACH(ASTNamedExpr, ne, expr->compoundLit.membs)
                {
                    bssCheckerCheckExpr(iState, ne->exprLhs, scope);
                    if(base == null)
                    {
                        base = ne->exprLhs->checkType;
                    }

                    if(!bssAreBssTypesEqual(ne->exprLhs->checkType, base))
                    {
                        bssCheckerError(iState, ne->startTok, "Expected all array compound elements to be of the same type.\n");
                        bssLexerPrintTokenRange(ne->startTok, ne->endTok);
                        break;
                    }
                }

                expr->checkType = bssAllocTypeArray(iState->checkerArena, base);
            }

        }break;
        default:
        {
            bssCheckerError(iState, expr->startTok, "Unregognised expression.\n");
            bssLexerPrintTokenRange(expr->startTok, expr->endTok);
        }break;
    }
}