#include "os/core/osCore.h"
#include "bssLexer.h"

str8 gBssBssTokLexemeTable[] =
{
    [TOK_IF_KW] = STR8_LIT_COMP_CONST("if"),
    [TOK_ELSE_KW] = STR8_LIT_COMP_CONST("else"),
    [TOK_FOR_KW] = STR8_LIT_COMP_CONST("for"),
    [TOK_WHILE_KW] = STR8_LIT_COMP_CONST("while"),
    [TOK_IN_KW] = STR8_LIT_COMP_CONST("in"),
    [TOK_FN_KW] = STR8_LIT_COMP_CONST("fn"),
    [TOK_RET_KW] = STR8_LIT_COMP_CONST("return"),
    [TOK_BREAK_KW] = STR8_LIT_COMP_CONST("break"),
    [TOK_CONTINUE_KW] = STR8_LIT_COMP_CONST("continue"),
 
    [TOK_LOGICAL_AND_OP] = STR8_LIT_COMP_CONST("&&"),
    [TOK_LOGICAL_OR_OP] = STR8_LIT_COMP_CONST("||"),
    [TOK_EQ_OP] = STR8_LIT_COMP_CONST("=="),
    [TOK_NEQ_OP] = STR8_LIT_COMP_CONST("!="),

    [TOK_END_INPUT] = STR8_LIT_COMP_CONST("END OF INPUT"),
};

void bssLexerError(BssInterp *interp, BssTokPos pos, i8 *fmt, ...)
{
    BASE_UNUSED_PARAM(interp);
    va_list va;
    va_start(va, fmt);
    baseColEPrintf("{Bu}%S (%lld, %lld)", pos.ownerLexer->path, pos.line, pos.col);
    baseColEPrintf("{B}\n        --> Lexer Error: ");
    baseEPrintfV(fmt, va);

    bssPrintSourceRange(pos, pos, 2);

    va_end(va);
}


bool bssLexerAdvanceChar(BssInterp *interp)
{
    if(interp->lexer->currLocInBuffer > (interp->lexer->buffer.data + interp->lexer->buffer.len))
    {
        return false;
    }

    if(interp->lexer->ch == '\n')
    {
       interp->lexer->line++;
       interp->lexer->col = 1;
    }

    if((interp->lexer->ch != '\n') && (interp->lexer->ch != '\f') && (interp->lexer->ch != '\r'))
        interp->lexer->col++;

    interp->lexer->currLocInBuffer++;
    interp->lexer->ch = *interp->lexer->currLocInBuffer;

    return true;
}
u8 bssLexerPeekCharEx(BssInterp *interp, u64 amount)
{
    return *(interp->lexer->currLocInBuffer + amount);
}
u8 bssLexerPeekChar(BssInterp *interp)
{
    return bssLexerPeekCharEx(interp, 1);
}

BssTok bssLexerLexNextTok(BssInterp *interp)
{
LEX_START:
    u8 *bufEnd = interp->lexer->buffer.data + interp->lexer->buffer.len;
    while ((interp->lexer->currLocInBuffer < bufEnd) && 
            (isspace(*interp->lexer->currLocInBuffer) || (*interp->lexer->currLocInBuffer == '\0')))
    {
        bssLexerAdvanceChar(interp);
    }

    BssTokPos pos = {.line = interp->lexer->line, .col = interp->lexer->col, .ownerLexer = interp->lexer, .range.data = interp->lexer->currLocInBuffer};
    BssTok tok = {.pos = pos};

    if(interp->lexer->currLocInBuffer >= bufEnd)
    {
        tok.pos.range.len = 0;
        tok.kind = TOK_END_INPUT;
        tok.lexeme = gBssBssTokLexemeTable[TOK_END_INPUT];
    }
    else if(interp->lexer->ch == '/' && bssLexerPeekChar(interp) == '/')
    {
        while((interp->lexer->currLocInBuffer < bufEnd) && 
              (interp->lexer->ch != '\n' || interp->lexer->ch == '\0' ))
        {
            bssLexerAdvanceChar(interp);
        }

        goto LEX_START;
    }
    else if(interp->lexer->ch == '\"')
    {
        u64 tokLen = 1;
        if(!bssLexerAdvanceChar(interp)) goto LEX_START;

        while(interp->lexer->ch != '\"')
        {
            if (interp->lexer->ch == '~')
            {
                str8 charLitStr = {0};
                charLitStr.data = interp->lexer->currLocInBuffer;
                charLitStr.len = 2;

                i64 escapeVal = bssGetEscapeCharValue(charLitStr);
                if(escapeVal == '\"')
                {
                    tokLen++;
                    bssLexerAdvanceChar(interp);
                }
                else if (escapeVal == -1)
                {
                    bssLexerError(interp, pos, "Expected a valid escape character");
                }
            }

            tokLen++;
            if(!bssLexerAdvanceChar(interp))
            {
                goto LEX_START;
            }
        }

        bssLexerAdvanceChar(interp);
        tokLen += 1;

        str8 lexeme = {0};
        lexeme.data =  interp->lexer->currLocInBuffer - tokLen;
        lexeme.len = tokLen;

        tok.pos.range.len = tokLen;
        tok.kind = TOK_STR_LIT;
        tok.lexeme = lexeme;
    }
    else if(isdigit(interp->lexer->ch))
    {
        u64 tokLen = 0;
        
        while(isdigit(interp->lexer->ch))
        {
            tokLen++;
            bssLexerAdvanceChar(interp);
        }

        str8 lexeme = {0};
        lexeme.data =  interp->lexer->currLocInBuffer - tokLen;
        lexeme.len = tokLen;

        tok.pos.range.len = tokLen;
        tok.kind = TOK_INT_LIT;
        tok.lexeme = lexeme;
    }
    else
    {
        str8 lexeme = STR8_LIT("");
        u64 tokKind = 0;
        for(u64 i = TOK_KIND_START; i < BASE_ARRAY_SIZE(gBssBssTokLexemeTable); i++)
        {
            u64 longestMatch = 0;
            if (gBssBssTokLexemeTable[i].len > 0 && interp->lexer->ch == gBssBssTokLexemeTable[i].data[0])
            {
                u64 charsLeftInBuffer = interp->lexer->currLocInBuffer - interp->lexer->buffer.data + interp->lexer->buffer.len;
                if(charsLeftInBuffer >= gBssBssTokLexemeTable[i].len)
                {
                    str8 l = baseStr8(interp->lexer->currLocInBuffer, gBssBssTokLexemeTable[i].len);
                    if(Str8Equals(l, gBssBssTokLexemeTable[i], 0) && (l.len > longestMatch))
                    {
                        // for it to match with a keyword identifier,
                        // it should be a token, forexample
                        // fornite should not match with the keyword for, but for tnite should.
                        if (isalpha(l.data[0]) && isalnum(bssLexerPeekCharEx(interp, l.len)))
                        {
                            continue;
                        }

                        longestMatch = l.len;
                        lexeme = l;
                        tokKind = i;
                    }
                }
            }
        }

        // couldnt find one
        if (lexeme.len == 0)
        {
            u64 tokLen = 0;
            if(isalpha(interp->lexer->ch))
            {
                while(isalnum(bssLexerPeekCharEx(interp, tokLen)))
                {
                    tokLen++;
                }

                tokKind = TOK_IDEN;
            }
            else
            {
                tokKind = interp->lexer->ch;
                tokLen = 1;
            }

            lexeme.data =  interp->lexer->currLocInBuffer;
            lexeme.len = tokLen;

            if(Str8Equals(STR8_LIT("false"), lexeme, 0) || Str8Equals(STR8_LIT("true"), lexeme, 0))
            {
                tokKind = TOK_BOOL_LIT;
            }
        }

        tok.kind = tokKind;
        tok.lexeme = lexeme;
        tok.pos.range.len = lexeme.len;

        interp->lexer->ch = *(interp->lexer->currLocInBuffer += lexeme.len);
    }

    return tok;
}

void bssLexerLexWhole(BssInterp *interp)
{
    BssTokChunkList chunkList = {0};
    BssTok tok = {0};
    
    ArenaTemp temp = baseTempBegin(&interp->arena, 1);
    {
        for (tok = bssLexerLexNextTok(interp); tok.kind != TOK_END_INPUT; tok = tok = bssLexerLexNextTok(interp))
        {
            BssTokChunkListPushLast(temp.arena, &chunkList, tok);
        }
        
        // add the end input token too, at end
        BssTokChunkListPushLast(temp.arena, &chunkList, tok);

        interp->lexer->tokArray = BssTokChunkListFlattenToArray(interp->arena, &chunkList);
    }

    baseTempEnd(temp);
}
bool bssLexerFromBuffer(BssInterp *interp, U8Array buf)
{
    interp->lexer = arenaPushType(interp->arena, BssLexer);
    interp->lexer->buffer = buf;
    interp->lexer->line = 1;
    interp->lexer->col = 1;
    interp->lexer->currLocInBuffer = buf.data;
    interp->lexer->ch = *interp->lexer->currLocInBuffer;

    return true;
}
bool bssLexerLexFile(BssInterp *interp, str8 file)
{
    U8Array buf = OSFileReadAll(interp->arena, file);

    bool result = false;
    if (BASE_ANY(buf))
    {
        result = bssLexerFromBuffer(interp, buf);
        interp->lexer->path = file;

        bssLexerLexWhole(interp);
    }
    else
    {
        baseEPrintf("Failed to open file '%S'\n", file);
        return false;
    }

    return result;
}
BssTok bssLexerGetNextTok(BssInterp *interp)
{
    return interp->lexer->currTokIndex >= (interp->lexer->tokArray.len - 1) ? 
           interp->lexer->tokArray.data[interp->lexer->tokArray.len - 1] :
           interp->lexer->tokArray.data[interp->lexer->currTokIndex++];
}
BssTok bssLexerPeekTok(BssInterp *interp, u64 amount)
{
    return interp->lexer->currTokIndex + amount >= (interp->lexer->tokArray.len - 1) ? 
           interp->lexer->tokArray.data[interp->lexer->tokArray.len - 1] :
           interp->lexer->tokArray.data[interp->lexer->currTokIndex + amount];
}