#include "base\baseThreads.h"
#include "bssParser.h"

void bssParserError(BSSInterpretorState *iState, BssTok tok, char *msg, ...)
{
    va_list args;
    va_start(args, msg);

    baseColEPrintf("{Bu}%S (%lld: %lld)",tok.pos.ownerLexer->filePath, tok.pos.line, tok.pos.col);
    baseColEPrintf("{B}\n        --> Parser Error:\033[0m ");

    BaseArenaTemp temp = baseTempBegin(&iState->parserArena, 1);
    {
        i64 needed = stbsp_vsnprintf(null, 0, msg, args) + 1;
        i8 *buf = baseArenaPush(temp.arena, needed);

        stbsp_vsnprintf(buf, (int)needed, msg, args);

        baseColEPrintf("%s", buf);
    }
    baseTempEnd(temp);

    baseColEPrintf("\n        |\n");
    
    const int NUM_CONTEXT_LINES = 3;
    
    size_t tokLen = (tok.kind != TOK_END_INPUT)  ? tok.lexeme.len : 1;
    int currLine = 0;
    i8 *currCh = (i8 *)tok.pos.tokRange.data;
    int colnum = 0;

    i8 *bufStart = (i8 *)tok.pos.ownerLexer->buffer.data;
    i8 *bufEnd = (i8 *)(bufStart + tok.pos.ownerLexer->buffer.len);

    while( (currLine != -NUM_CONTEXT_LINES) && (currCh != bufStart)) 
    {
        while((currCh != bufStart) && (*(--currCh) != '\n'));

        if(*currCh == '\n') 
        {
            currLine--;
            if(currLine != -NUM_CONTEXT_LINES) currCh--;
        }
    }

    // if(*currCh == '\n') 
    // {
    //     currLine++;
    //     currCh++;
    // }

    for(; currLine < NUM_CONTEXT_LINES; currLine++)
    {
        fprintf(stderr, "%7lld | ", tok.pos.line + currLine);

        while((*currCh != '\n') && (currCh != bufEnd))
        {
            if(currLine == 0) colnum++;

            if((currCh >= (i8 *)tok.pos.tokRange.data) && (currCh < ((i8 *)tok.pos.tokRange.data) + tokLen))
            {
                if(isspace(*currCh))
                {
                    baseColEPrintf("{ur}%c", (*currCh));
                }
                else
                {
                    baseColEPrintf("{r}%c", (*currCh));
                }
            }
            else
            {
                baseColEPrintf("%c", (*currCh));
            }

            currCh++;
        }

        if(currLine == 0)
        {
            fputc('\n', stderr);
            fprintf(stderr, "        | ");
            for(i8 *c = currCh - colnum; c < (i8 *)tok.pos.tokRange.data; c++)
            {
                if(isspace(*c))
                    fputc((char)*c, stderr);
                else fputc(' ', stderr);

            }

            for(u64 i = 0; i < tokLen; i++) 
            {
                baseColEPrintf("{r}^");
            }
        }
        
        if(currCh != bufEnd) currCh++;
        else break;
        fputc('\n', stderr);
    }
    
    fputc('\n', stderr);
    va_end(args);
}

void bssParserProject(BSSInterpretorState *iState)
{
    ASTStmtList stmts = bssParserStmtList(iState, TOK_END_INPUT);

    BssTok startTok, endTok;
    if(BASE_ANY(stmts))
    {
        startTok = stmts.first->startTok;
        endTok = stmts.last->endTok;
    }
    else
    {
        startTok = iState->lState.tok;
        endTok = startTok;
    }

    ASTProject *project = baseArenaPushType(iState->parserArena, ASTProject);
    project->startTok = startTok;
    project->endTok = endTok;
    project->stmts = stmts;

    iState->pState.proj = project;
}

bool bssParserDoesStmtNeedSemiColon(ASTStmt *stmt)
{
    switch(stmt->kind)
    {
        case AST_STMT_BLOCK:
        case AST_STMT_IF:
        case AST_STMT_FOR_LOOP:
        {
            return false;
        }

        default: return true;
    }
}

ASTStmt *bssParserStmt(BSSInterpretorState *iState)
{
    ASTStmt *stmt = null;

    switch(PARSER_CURR_TOK(iState).kind)
    {
        case TOK_PROJECT:
        {
            PARSER_ADV_TOK(iState);

            BssTok iden = {0};
            if(PARSER_CURR_TOK(iState).kind != TOK_IDEN)
            {
                bssParserError(iState, PARSER_CURR_TOK(iState), "Expected identifier instead got '%S'", PARSER_CURR_TOK(iState).lexeme);
            }

            iden = PARSER_CURR_TOK(iState);
            PARSER_ADV_TOK(iState);

            if(PARSER_CURR_TOK(iState).kind != '=')
            {
                bssParserError(iState, PARSER_CURR_TOK(iState), "Expected '=' instead got '%S'", PARSER_CURR_TOK(iState).lexeme);
            }

            PARSER_ADV_TOK(iState);

            ASTExpr *expr = bssParserExpr(iState);

            stmt = bssAllocASTStmtProjDecl(iState->parserArena, iden, expr);
        }break;

        case TOK_BUILD:
        {
            PARSER_ADV_TOK(iState);

            ASTExpr *expr = bssParserExpr(iState);

            ASTExpr *args = bssParserExpr(iState);

            stmt = bssAllocASTStmtBuild(iState->parserArena, expr, args);
        }break;
        
        case TOK_IF:
        {
            PARSER_ADV_TOK(iState);

            ASTExpr *expr = bssParserExpr(iState);
            ASTBlock *block = bssParserBlock(iState);
            ASTBlock *elseblock = null;

            if (PARSER_CURR_TOK(iState).kind == TOK_ELSE)
            {
                PARSER_ADV_TOK(iState);
                elseblock = bssParserBlock(iState);
            }

            stmt = bssAllocASTStmtIf(iState->parserArena, expr, block, elseblock);
        }break;

        case TOK_FOR:
        {
            PARSER_ADV_TOK(iState);

            BssTok iden = {0};
            if(PARSER_CURR_TOK(iState).kind != TOK_IDEN)
            {
                bssParserError(iState, PARSER_CURR_TOK(iState), "Expected identifier instead got '%S'", PARSER_CURR_TOK(iState).lexeme);
            }

            iden = PARSER_CURR_TOK(iState);
            PARSER_ADV_TOK(iState);

            if(PARSER_CURR_TOK(iState).kind != TOK_IN)
            {
                bssParserError(iState, PARSER_CURR_TOK(iState), "Expected identifier instead got '%S'", PARSER_CURR_TOK(iState).lexeme);
            }

            PARSER_ADV_TOK(iState);

            ASTExpr *expr = bssParserExpr(iState);
            ASTBlock *block = bssParserBlock(iState);

            stmt = bssAllocASTStmtFor(iState->parserArena, iden, expr, block);
        }break;

        case (BssTokKind)'{':
        {
            ASTBlock *block = bssParserBlock(iState);
            stmt = bssAllocASTStmtBlock(iState->parserArena, block);
        }break;

        default:
        {
            ASTExpr *lhs = bssParserExpr(iState);
            if(PARSER_CURR_TOK(iState).kind == '=')
            {
                PARSER_ADV_TOK(iState);

                ASTExpr *rhs = bssParserExpr(iState);
                stmt = bssAllocASTStmtAssign(iState->parserArena, lhs, rhs);
            }
            else
            {
                stmt = bssAllocASTStmtExpr(iState->parserArena, lhs);
            }
        }break;
    }

    return stmt;
}
ASTStmtList bssParserStmtList(BSSInterpretorState *iState, BssTokKind tokToEndParse)
{
    ASTStmtList list = {0};
    
    while ((PARSER_CURR_TOK(iState).kind != tokToEndParse) &&
           (PARSER_CURR_TOK(iState).kind != TOK_END_INPUT))
    {
        ASTStmt *stmt = bssParserStmt(iState);

        if(bssParserDoesStmtNeedSemiColon(stmt))
        {
            if (PARSER_CURR_TOK(iState).kind == ';')
            {
                PARSER_ADV_TOK(iState);
            }
            else
            {
                bssParserError(iState, PARSER_CURR_TOK(iState), "Expected a semi colon after statement, instead got '%S'", PARSER_CURR_TOK(iState).lexeme);
            }
        }

        ASTStmtListPushNodeLast(&list, stmt);
    }

    return list;
}

ASTBlock *bssParserBlock(BSSInterpretorState *iState)
{
    ASTBlock *block = null;
    
    BssTok start = {0}, end = {0};
    if(PARSER_CURR_TOK(iState).kind == '{')
    {
        start = PARSER_CURR_TOK(iState);

        PARSER_ADV_TOK(iState);

        ASTStmtList stmts = bssParserStmtList(iState, '}');
        
        if(PARSER_CURR_TOK(iState).kind == '}')
        {
            end = PARSER_CURR_TOK(iState);
            PARSER_ADV_TOK(iState);
        }
        else
        {
            bssParserError(iState, PARSER_CURR_TOK(iState), "Expected '}' to close block instead got '%S'", PARSER_CURR_TOK(iState).lexeme);
        }

        block = baseArenaPushType(iState->parserArena, ASTBlock);
        block->stmts = stmts;
        block->endTok = end;
        block->startTok = start;
    }

    return block;
}

ASTExpr *bssParserExpr(BSSInterpretorState *iState)
{
    ASTExpr *expr = null;

    expr = bssParserExprEq(iState);
    while((PARSER_CURR_TOK(iState).kind == '+'))
    {
        BssTok op = PARSER_CURR_TOK(iState);
        PARSER_ADV_TOK(iState);

        ASTExpr *rhs = bssParserExprEq(iState);
        
        expr = bssAllocASTExprBinary(iState->parserArena, expr->startTok, rhs->endTok, op, expr, rhs);
    }

    return expr;
}
ASTExpr *bssParserExprEq(BSSInterpretorState *iState)
{
    ASTExpr *expr = null;

    expr = bssParserExprLogical(iState);
    while((PARSER_CURR_TOK(iState).kind == TOK_EQ_OP) || 
          (PARSER_CURR_TOK(iState).kind == TOK_NEQ_OP))
    {
        BssTok op = PARSER_CURR_TOK(iState);
        PARSER_ADV_TOK(iState);

        ASTExpr *rhs = bssParserExprLogical(iState);
        
        expr = bssAllocASTExprBinary(iState->parserArena, expr->startTok, rhs->endTok, op, expr, rhs);
    }

    return expr;
}
ASTExpr *bssParserExprLogical(BSSInterpretorState *iState)
{
    ASTExpr *expr = null;

    expr = bssParserExprPost(iState);
    while((PARSER_CURR_TOK(iState).kind == TOK_LOGICAL_OR_OP))
    {
        BssTok op = PARSER_CURR_TOK(iState);
        PARSER_ADV_TOK(iState);

        ASTExpr *rhs = bssParserExprPost(iState);
        
        expr = bssAllocASTExprBinary(iState->parserArena, expr->startTok, rhs->endTok, op, expr, rhs);
    }

    return expr;
}
ASTExpr *bssParserExprPost(BSSInterpretorState *iState)
{
    ASTExpr *expr = bssParserExprPrimary(iState);

    while (((PARSER_CURR_TOK(iState).kind == '[') || 
            (PARSER_CURR_TOK(iState).kind == '.') || 
            (PARSER_CURR_TOK(iState).kind == '(')))
    {
        if(PARSER_CURR_TOK(iState).kind == '[')
        {
            PARSER_ADV_TOK(iState);

            ASTExpr *index = bssParserExpr(iState);

            if(PARSER_CURR_TOK(iState).kind == ']')
            {
                BssTok endtok = PARSER_CURR_TOK(iState);
                PARSER_ADV_TOK(iState);

                expr = bssAllocASTExprIndex(iState->parserArena, expr->startTok, endtok, expr, index);
            }
            else
            {
                bssParserError(iState, PARSER_CURR_TOK(iState), "Expected closing ']' instead got '%S'", PARSER_CURR_TOK(iState).lexeme);
            }
        }
        else if(PARSER_CURR_TOK(iState).kind == '.')
        {
            PARSER_ADV_TOK(iState);

            if(PARSER_CURR_TOK(iState).kind == TOK_IDEN)
            {
                BssTok memb = PARSER_CURR_TOK(iState);
                expr = bssAllocASTExprMembAccess(iState->parserArena, expr->startTok, memb, expr, memb);

                PARSER_ADV_TOK(iState);
            }
            else
            {
                bssParserError(iState, PARSER_CURR_TOK(iState), "Expected an identifier after '.' but instead got '%S'", PARSER_CURR_TOK(iState).lexeme);
            }
        }
        else if(PARSER_CURR_TOK(iState).kind == '(')
        {
            PARSER_ADV_TOK(iState);

            BssTok endToken = {0};

            ASTNamedExprList argsList = {0};

            if(PARSER_CURR_TOK(iState).kind == ')')
            {
                endToken = PARSER_CURR_TOK(iState);
                PARSER_ADV_TOK(iState);
            }
            else
            {
                argsList = bssParserNamedExprList(iState, ')');

                if(PARSER_CURR_TOK(iState).kind == ')')
                {
                    endToken = PARSER_CURR_TOK(iState);
                    PARSER_ADV_TOK(iState);
                }
                else
                {
                    bssParserError(iState, PARSER_CURR_TOK(iState), "Expected ')' to end argument lists for function call, instead got '%S'", PARSER_CURR_TOK(iState).lexeme);
                }
            }

            expr = bssAllocASTExprFunc(iState->parserArena, expr->startTok, endToken, expr, argsList);
        }
    }

    return expr;
}
ASTExpr *bssParserExprPrimary(BSSInterpretorState *iState)
{
    ASTExpr *expr = null;
    switch(PARSER_CURR_TOK(iState).kind)
    {
        case TOK_INT_LIT:
        case TOK_BOOL_LIT:
        case TOK_STR_LIT:
        {
            expr = bssAllocASTExprLit(iState->parserArena, PARSER_CURR_TOK(iState));

            PARSER_ADV_TOK(iState);
        }break;

        case '(':
        {
            PARSER_ADV_TOK(iState);

            expr = bssParserExpr(iState);

            if(PARSER_CURR_TOK(iState).kind == ')')
            {
                PARSER_ADV_TOK(iState);
            }
            else
            {
                bssParserError(iState, PARSER_CURR_TOK(iState), "Expected a ')' after expression, instead got' '%S'", PARSER_CURR_TOK(iState).lexeme);
            }

        }break;

        case TOK_IDEN:
        {
            expr = bssAllocASTExprIden(iState->parserArena, PARSER_CURR_TOK(iState));

            PARSER_ADV_TOK(iState);
        }break;

        case '{':
        {
            ASTNamedExprList membs = {0};
            BssTok startTok = PARSER_CURR_TOK(iState);
            BssTok endTok = {0};

            PARSER_ADV_TOK(iState);

            if(PARSER_CURR_TOK(iState).kind == '}')
            {
                endTok = PARSER_CURR_TOK(iState);
                PARSER_ADV_TOK(iState);
            }
            else 
            {
                membs = bssParserNamedExprList(iState, '}');

                if(PARSER_CURR_TOK(iState).kind == '}')
                {
                    endTok = PARSER_CURR_TOK(iState);

                    PARSER_ADV_TOK(iState);
                }
                else
                {
                    bssParserError(iState, PARSER_CURR_TOK(iState), "Expected '}' to end compound literal but instead got '%S'", PARSER_CURR_TOK(iState).lexeme);
                }
            }

            expr = bssAllocASTExprCompound(iState->parserArena, startTok, endTok, membs);
        }break;

        case TOK_RUN:
        {
            BssTok startTok = PARSER_CURR_TOK(iState);

            PARSER_ADV_TOK(iState);
            ASTExpr *e = bssParserExpr(iState);

            expr = bssAllocASTExprRun(iState->parserArena, startTok, e->endTok, e);

        }break;

        default:
        {
            bssParserError(iState, PARSER_CURR_TOK(iState), "Unexpected tok to begin expression, got '%S'", PARSER_CURR_TOK(iState).lexeme);
        }break;
    }

    return expr;
}

ASTNamedExpr *bssParserNamedExpr(BSSInterpretorState *iState)
{
    ASTExpr *exprLhs = bssParserExpr(iState);
    ASTExpr *exprRhs = null;

    if(PARSER_CURR_TOK(iState).kind == '=')
    {
        if(exprLhs->kind == AST_EXPR_IDEN)
        {
            PARSER_ADV_TOK(iState);
            exprRhs = bssParserExpr(iState); 
        }
        else
        {
            bssParserError(iState, PARSER_CURR_TOK(iState), "Expected a identifier tok for beginning named expression");
        }
    }

    bool hasName = (exprRhs != null);
    ASTNamedExpr *ret = bssAllocASTNamedExpr(iState->parserArena, exprLhs->startTok, (hasName) ? exprRhs->endTok : exprLhs->endTok, hasName, exprLhs, exprRhs);
    
    return ret;
}
ASTNamedExprList bssParserNamedExprList(BSSInterpretorState *iState,  BssTokKind tokToEndParse)
{
    ASTNamedExprList list = {0};
    
    while ((PARSER_CURR_TOK(iState).kind != tokToEndParse) &&
           (PARSER_CURR_TOK(iState).kind != TOK_END_INPUT))
    {
        ASTNamedExpr *namedExpr = bssParserNamedExpr(iState);
        ASTNamedExprListPushNodeLast(&list, namedExpr);

        if(PARSER_CURR_TOK(iState).kind == ',')
        {
            PARSER_ADV_TOK(iState);
        }
        else if((PARSER_CURR_TOK(iState).kind != tokToEndParse) &&
                (PARSER_CURR_TOK(iState).kind != TOK_END_INPUT))
        {
            bssParserError(iState, PARSER_CURR_TOK(iState), "Expected a ',' after expression, instead got '%S'", PARSER_CURR_TOK(iState).lexeme);
        }
    }
    

    return list;
}
