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

ASTProject *bssParserProject(BSSInterpretorState *iState, BssParserState *pState)
{
    ASTStmtList stmts = bssParserStmtList(iState, pState, TOK_END_INPUT);

    BssTok startTok, endTok;
    if(BASE_ANY(stmts))
    {
        startTok = stmts.first->startTok;
        endTok = stmts.last->endTok;
    }
    else
    {
        startTok = pState->lexer->tok;
        endTok = startTok;
    }

    ASTProject *project = baseArenaPushType(iState->parserArena, ASTProject);
    project->startTok = startTok;
    project->endTok = endTok;
    project->stmts = stmts;

    return project;
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

ASTStmt *bssParserStmt(BSSInterpretorState *iState, BssParserState *pState)
{
    ASTStmt *stmt = null;

    switch(PARSER_CURR_TOK(pState).kind)
    {
        case TOK_PROJECT:
        {
            PARSER_ADV_TOK(pState);

            BssTok iden = {0};
            if(PARSER_CURR_TOK(pState).kind != TOK_IDEN)
            {
                bssParserError(iState, PARSER_CURR_TOK(pState), "Expected identifier instead got '%S'", pState->lexer->tok);
            }

            iden = PARSER_CURR_TOK(pState);
            PARSER_ADV_TOK(pState);

            if(PARSER_CURR_TOK(pState).kind != '=')
            {
                bssParserError(iState, PARSER_CURR_TOK(pState), "Expected '=' instead got '%S'", pState->lexer->tok);
            }

            PARSER_ADV_TOK(pState);

            ASTExpr *expr = bssParserExpr(iState, pState);

            stmt = bssAllocASTStmtProjDecl(iState->parserArena, iden, expr);
        }break;

        case TOK_BUILD:
        {
            PARSER_ADV_TOK(pState);

            ASTExpr *expr = bssParserExpr(iState, pState);

            ASTExpr *args = bssParserExpr(iState, pState);

            stmt = bssAllocASTStmtBuild(iState->parserArena, expr, args);
        }break;
        
        case TOK_IF:
        {
            PARSER_ADV_TOK(pState);

            ASTExpr *expr = bssParserExpr(iState, pState);
            ASTBlock *block = bssParserBlock(iState, pState);
            ASTBlock *elseblock = null;

            if (PARSER_CURR_TOK(pState).kind == TOK_ELSE)
            {
                PARSER_ADV_TOK(pState);
                elseblock = bssParserBlock(iState, pState);
            }

            stmt = bssAllocASTStmtIf(iState->parserArena, expr, block, elseblock);
        }break;

        case TOK_FOR:
        {
            PARSER_ADV_TOK(pState);

            BssTok iden = {0};
            if(PARSER_CURR_TOK(pState).kind != TOK_IDEN)
            {
                bssParserError(iState, PARSER_CURR_TOK(pState), "Expected identifier instead got '%S'", pState->lexer->tok);
            }

            iden = PARSER_CURR_TOK(pState);
            PARSER_ADV_TOK(pState);

            if(PARSER_CURR_TOK(pState).kind != TOK_IN)
            {
                bssParserError(iState, PARSER_CURR_TOK(pState), "Expected identifier instead got '%S'", pState->lexer->tok);
            }

            PARSER_ADV_TOK(pState);

            ASTExpr *expr = bssParserExpr(iState, pState);
            ASTBlock *block = bssParserBlock(iState, pState);

            stmt = bssAllocASTStmtFor(iState->parserArena, iden, expr, block);
        }break;

        case (BssTokKind)'{':
        {
            ASTBlock *block = bssParserBlock(iState, pState);
            stmt = bssAllocASTStmtBlock(iState->parserArena, block);
        }break;

        default:
        {
            ASTExpr *lhs = bssParserExpr(iState, pState);
            if(PARSER_CURR_TOK(pState).kind == '=')
            {
                PARSER_ADV_TOK(pState);

                ASTExpr *rhs = bssParserExpr(iState, pState);
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
ASTStmtList bssParserStmtList(BSSInterpretorState *iState, BssParserState *pState, BssTokKind tokToEndParse)
{
    ASTStmtList list = {0};
    
    while ((PARSER_CURR_TOK(pState).kind != tokToEndParse) &&
           (PARSER_CURR_TOK(pState).kind != TOK_END_INPUT))
    {
        ASTStmt *stmt = bssParserStmt(iState, pState);

        if(bssParserDoesStmtNeedSemiColon(stmt))
        {
            if (PARSER_CURR_TOK(pState).kind == ';')
            {
                PARSER_ADV_TOK(pState);
            }
            else
            {
                bssParserError(iState, PARSER_CURR_TOK(pState), "Expected a semi colon after statement, instead got '%S'", PARSER_CURR_TOK(pState).lexeme);
            }
        }

        ASTStmtListPushNodeLast(&list, stmt);
    }

    return list;
}

ASTBlock *bssParserBlock(BSSInterpretorState *iState, BssParserState *pState)
{
    ASTBlock *block = null;
    
    BssTok start = {0}, end = {0};
    if(PARSER_CURR_TOK(pState).kind == '{')
    {
        start = PARSER_CURR_TOK(pState);

        PARSER_ADV_TOK(pState);

        ASTStmtList stmts = bssParserStmtList(iState, pState, '}');
        
        if(PARSER_CURR_TOK(pState).kind == '}')
        {
            end = PARSER_CURR_TOK(pState);
            PARSER_ADV_TOK(pState);
        }
        else
        {
            bssParserError(iState, pState->lexer->tok, "Expected '}' to close block instead got '%S'", pState->lexer->tok.lexeme);
        }

        block = baseArenaPushType(iState->parserArena, ASTBlock);
        block->stmts = stmts;
        block->endTok = end;
        block->startTok = start;
    }

    return block;
}

ASTExpr *bssParserExpr(BSSInterpretorState *iState, BssParserState *pState)
{
    ASTExpr *expr = null;

    expr = bssParserExprLogical(iState, pState);
    while((PARSER_CURR_TOK(pState).kind == '+'))
    {
        BssTok op = PARSER_CURR_TOK(pState);
        PARSER_ADV_TOK(pState);

        ASTExpr *rhs = bssParserExprLogical(iState, pState);
        
        expr = bssAllocASTExprBinary(iState->parserArena, expr->startTok, rhs->endTok, op, expr, rhs);
    }

    return expr;
}
ASTExpr *bssParserExprLogical(BSSInterpretorState *iState, BssParserState *pState)
{
    ASTExpr *expr = null;

    expr = bssParserExprPost(iState, pState);
    while((PARSER_CURR_TOK(pState).kind == TOK_LOGICAL_OR_OP))
    {
        BssTok op = PARSER_CURR_TOK(pState);
        PARSER_ADV_TOK(pState);

        ASTExpr *rhs = bssParserExprPost(iState, pState);
        
        expr = bssAllocASTExprBinary(iState->parserArena, expr->startTok, rhs->endTok, op, expr, rhs);
    }

    return expr;
}
ASTExpr *bssParserExprPost(BSSInterpretorState *iState, BssParserState *pState)
{
    ASTExpr *expr = bssParserExprPrimary(iState, pState);

    while (((PARSER_CURR_TOK(pState).kind == '[') || 
            (PARSER_CURR_TOK(pState).kind == '.') || 
            (PARSER_CURR_TOK(pState).kind == '(')))
    {
        if(PARSER_CURR_TOK(pState).kind == '[')
        {
            PARSER_ADV_TOK(pState);

            ASTExpr *index = bssParserExpr(iState, pState);

            if(PARSER_CURR_TOK(pState).kind == ']')
            {
                BssTok endtok = PARSER_CURR_TOK(pState);
                PARSER_ADV_TOK(pState);

                expr = bssAllocASTExprIndex(iState->parserArena, expr->startTok, endtok, expr, index);
            }
            else
            {
                bssParserError(iState, PARSER_CURR_TOK(pState), "Expected closing ']' instead got '%S'", PARSER_CURR_TOK(pState).lexeme);
            }
        }
        else if(PARSER_CURR_TOK(pState).kind == '.')
        {
            PARSER_ADV_TOK(pState);

            if(PARSER_CURR_TOK(pState).kind == TOK_IDEN)
            {
                BssTok memb = PARSER_CURR_TOK(pState);
                expr = bssAllocASTExprMembAccess(iState->parserArena, expr->startTok, memb, expr, memb);

                PARSER_ADV_TOK(pState);
            }
            else
            {
                bssParserError(iState, PARSER_CURR_TOK(pState), "Expected an identifier after '.' but instead got '%S'", PARSER_CURR_TOK(pState).lexeme);
            }
        }
        else if(PARSER_CURR_TOK(pState).kind == '(')
        {
            PARSER_ADV_TOK(pState);

            BssTok endToken = {0};

            ASTNamedExprList argsList = {0};

            if(PARSER_CURR_TOK(pState).kind == ')')
            {
                endToken = PARSER_CURR_TOK(pState);
                PARSER_ADV_TOK(pState);
            }
            else
            {
                argsList = bssParserNamedExprList(iState, pState, ')');

                if(PARSER_CURR_TOK(pState).kind == ')')
                {
                    endToken = PARSER_CURR_TOK(pState);
                    PARSER_ADV_TOK(pState);
                }
                else
                {
                    bssParserError(iState, PARSER_CURR_TOK(pState), "Expected ')' to end argument lists for function call, instead got '%S'", PARSER_CURR_TOK(pState).lexeme);
                }
            }

            expr = bssAllocASTExprFunc(iState->parserArena, expr->startTok, endToken, expr, argsList);
        }
    }

    return expr;
}
ASTExpr *bssParserExprPrimary(BSSInterpretorState *iState, BssParserState *pState)
{
    ASTExpr *expr = null;
    switch(PARSER_CURR_TOK(pState).kind)
    {
        case TOK_INT_LIT:
        case TOK_BOOL_LIT:
        case TOK_STR_LIT:
        {
            expr = bssAllocASTExprLit(iState->parserArena, PARSER_CURR_TOK(pState));

            PARSER_ADV_TOK(pState);
        }break;

        case '(':
        {
            PARSER_ADV_TOK(pState);

            expr = bssParserExpr(iState, pState);

            if(PARSER_CURR_TOK(pState).kind == ')')
            {
                PARSER_ADV_TOK(pState);
            }
            else
            {
                bssParserError(iState, PARSER_CURR_TOK(pState), "Expected a ')' after expression, instead got' '%S'", PARSER_CURR_TOK(pState).lexeme);
            }

        }break;

        case TOK_IDEN:
        {
            expr = bssAllocASTExprIden(iState->parserArena, PARSER_CURR_TOK(pState));

            PARSER_ADV_TOK(pState);
        }break;

        case '{':
        {
            ASTNamedExprList membs = {0};
            BssTok startTok = PARSER_CURR_TOK(pState);
            BssTok endTok = {0};

            PARSER_ADV_TOK(pState);

            if(PARSER_CURR_TOK(pState).kind == '}')
            {
                endTok = PARSER_CURR_TOK(pState);
                PARSER_ADV_TOK(pState);
            }
            else 
            {
                membs = bssParserNamedExprList(iState, pState, '}');

                if(PARSER_CURR_TOK(pState).kind == '}')
                {
                    endTok = PARSER_CURR_TOK(pState);

                    PARSER_ADV_TOK(pState);
                }
                else
                {
                    bssParserError(iState, PARSER_CURR_TOK(pState), "Expected '}' to end compound literal but instead got '%S'", PARSER_CURR_TOK(pState).lexeme);
                }
            }

            expr = bssAllocASTExprCompound(iState->parserArena, startTok, endTok, membs);
        }break;

        case TOK_RUN:
        {
            BssTok startTok = PARSER_CURR_TOK(pState);

            PARSER_ADV_TOK(pState);
            ASTExpr *e = bssParserExpr(iState, pState);

            expr = bssAllocASTExprRun(iState->parserArena, startTok, e->endTok, e);

        }break;

        default:
        {
            bssParserError(iState, PARSER_CURR_TOK(pState), "Unexpected tok to begin expression, got '%S'", PARSER_CURR_TOK(pState).lexeme);
        }break;
    }

    return expr;
}

ASTNamedExpr *bssParserNamedExpr(BSSInterpretorState *iState, BssParserState *pState)
{
    ASTExpr *exprLhs = bssParserExpr(iState, pState);
    ASTExpr *exprRhs = null;

    if(PARSER_CURR_TOK(pState).kind == '=')
    {
        if(exprLhs->kind == AST_EXPR_IDEN)
        {
            PARSER_ADV_TOK(pState);
            exprRhs = bssParserExpr(iState, pState); 
        }
        else
        {
            bssParserError(iState, PARSER_CURR_TOK(pState), "Expected a identifier tok for beginning named expression");
        }
    }

    bool hasName = (exprRhs != null);
    ASTNamedExpr *ret = bssAllocASTNamedExpr(iState->parserArena, exprLhs->startTok, (hasName) ? exprRhs->endTok : exprLhs->endTok, hasName, exprLhs, exprRhs);
    
    return ret;
}
ASTNamedExprList bssParserNamedExprList(BSSInterpretorState *iState, BssParserState *pState, BssTokKind tokToEndParse)
{
    ASTNamedExprList list = {0};
    
    while ((PARSER_CURR_TOK(pState).kind != tokToEndParse) &&
           (PARSER_CURR_TOK(pState).kind != TOK_END_INPUT))
    {
        ASTNamedExpr *namedExpr = bssParserNamedExpr(iState, pState);
        ASTNamedExprListPushNodeLast(&list, namedExpr);

        if(PARSER_CURR_TOK(pState).kind == ',')
        {
            PARSER_ADV_TOK(pState);
        }
        else if((PARSER_CURR_TOK(pState).kind != tokToEndParse) &&
                (PARSER_CURR_TOK(pState).kind != TOK_END_INPUT))
        {
            bssParserError(iState, PARSER_CURR_TOK(pState), "Expected a ',' after expression, instead got '%S'", PARSER_CURR_TOK(pState).lexeme);
        }
    }
    

    return list;
}
