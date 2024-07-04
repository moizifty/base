#include "base\baseThreads.h"
#include "os\core\osCore.h"

#include "bss\bssLexer.h"

BASE_CREATE_EFFICIENT_LL_DEFS(BssLexerStateList, BssLexerState);

str8 gBssBssTokLexemeTable[] =
{
    [TOK_IF] = STR8_LIT_COMP_CONST("if"),
    [TOK_ELSE] = STR8_LIT_COMP_CONST("else"),
    [TOK_BUILD] = STR8_LIT_COMP_CONST("build"),
    [TOK_PROJECT] = STR8_LIT_COMP_CONST("project"),
    [TOK_RUN] = STR8_LIT_COMP_CONST("run"),
    [TOK_FOR] = STR8_LIT_COMP_CONST("for"),
    [TOK_WHILE] = STR8_LIT_COMP_CONST("while"),
    [TOK_IN] = STR8_LIT_COMP_CONST("in"),
 
    [TOK_LOGICAL_AND_OP] = STR8_LIT_COMP_CONST("&&"),
    [TOK_LOGICAL_OR_OP] = STR8_LIT_COMP_CONST("||"),
    [TOK_EQ_OP] = STR8_LIT_COMP_CONST("=="),
    [TOK_NEQ_OP] = STR8_LIT_COMP_CONST("!="),

    [TOK_END_INPUT] = STR8_LIT_COMP_CONST("END OF INPUT"),
};

void BssTokChunkListPushLast(BaseArena *arena, BssTokChunkList *l, BssTok tok)
{
    if(!BASE_ANY_PTR(l) || (l->last->chunk.len >= l->last->cap))
    {
        BssTokChunkListNode *n = baseArenaPushType(arena, BssTokChunkListNode);
        n->cap = 50;
        n->chunk.data = baseArenaPushArray(arena, BssTok, n->cap);
        n->chunk.len = 0;

        BasePtrListNodePushLast(l, n);
    }
    
    l->last->chunk.data[l->last->chunk.len] = tok;
    l->last->chunk.len += 1;
    l->totalLen += 1;
}
BssTokArray BssTokChunkListFlattenToArray(BaseArena *arena, BssTokChunkList *l)
{
    BssTokArray flattened = {0};

    if (!BASE_ANY_PTR(l))
    {
        return flattened;
    }

    flattened.data = baseArenaPushArray(arena, BssTok, l->totalLen);
    flattened.len = l->totalLen;

    u64 i = 0;
    BASE_PTR_LIST_FOREACH(BssTokChunkListNode, chunk, l)
    {
        for(u64 j = 0; j < chunk->chunk.len; j++)
        {
            flattened.data[i++] = chunk->chunk.data[j];
        }
    }

    return flattened;
}

BssLexerState *bssLexerInitFromFile(BSSInterpretorState *iState, str8 filePath)
{
    U8Array buffer = OSFileReadAll(iState->lexerArena, filePath);
    if(buffer.data == null)
    {
        return null;
    }

    BssLexerState *lState = bssLexerInitFromBuffer(iState, buffer);
    lState->filePath = filePath;

    return lState;
}
BssLexerState *bssLexerInitFromBuffer(BSSInterpretorState *iState, U8Array buffer)
{
    BssLexerState *lState = baseArenaPushType(iState->lexerArena, BssLexerState);
    lState->buffer = buffer;
    lState->ch = ' ';
    lState->currLocInBuffer = buffer.data;
    lState->line = 1;
    lState->col = 1;
    lState->nextBssTokIndex = 0;

    return lState;
}
BssTokArray bssLexerLexWholeBuffer(BSSInterpretorState *iState, BssLexerState *lState)
{
    BssTokChunkList tokChunks = {0};

    BaseArenaTemp temp = baseTempBegin(&iState->lexerArena, 1);
    {
        while((lState->tok = bssLexerNextFromBuffer(iState, lState)).kind != TOK_END_INPUT)
        {
            BssTokChunkListPushLast(iState->lexerArena, &tokChunks, lState->tok);
        }

        BssTokChunkListPushLast(iState->lexerArena, &tokChunks, lState->tok);
    }
    baseTempEnd(temp);

    lState->lexedBssToks = BssTokChunkListFlattenToArray(iState->lexerArena, &tokChunks);
    lState->tok = bssLexerNext(lState);

    return lState->lexedBssToks;
}

bool bssLexerAdvanceChar(BssLexerState *lState)
{
    if(lState->currLocInBuffer > (lState->buffer.data + lState->buffer.len))
    {
        return false;
    }

    if(lState->ch == '\n')
    {
       lState->line++;
       lState->col = 0;
    }

    if((lState->ch != '\n') && (lState->ch != '\f') && (lState->ch != '\r'))
        lState->col++;

    lState->ch = *lState->currLocInBuffer;
    lState->currLocInBuffer++;

    return true;
}
u8 bssLexerPeekChar(BssLexerState *lState)
{
    return bssLexerPeekCharEx(lState, 1);
}
u8 bssLexerPeekCharEx(BssLexerState *lState, u64 amount)
{
    return *((lState->currLocInBuffer - 1) + amount);
}

BssTok bssLexerNextFromBuffer(BSSInterpretorState *iState, BssLexerState *lState)
{
LEX_START:
    while(lState->currLocInBuffer <= (lState->buffer.data + lState->buffer.len) 
          && (isspace(lState->ch) || lState->ch == '\0'))
    {
        bssLexerAdvanceChar(lState);
    }

    BssTokPos pos = {.line = lState->line, .col = lState->col, .ownerLexer = lState, .tokRange.data = lState->currLocInBuffer - 1};
    BssTok tok = {.pos = pos};

    if(lState->currLocInBuffer > (lState->buffer.data + lState->buffer.len))
    {
        tok.pos.tokRange.len = 0;
        tok.kind = TOK_END_INPUT;
        tok.lexeme = gBssBssTokLexemeTable[TOK_END_INPUT];
    }
    else if(lState->ch == '/' && bssLexerPeekChar(lState) == '/')
    {
        while(lState->currLocInBuffer <= (lState->buffer.data + lState->buffer.len) 
              && (lState->ch != '\n' || lState->ch == '\0' ))
        {
            bssLexerAdvanceChar(lState);
        }

        goto LEX_START;
    }
    else if(lState->ch == '\"')
    {
        u64 tokLen = 1;
        if(!bssLexerAdvanceChar(lState)) goto LEX_START;

        while(lState->ch != '\"')
        {
            if (lState->ch == '~')
            {
                str8 charLitStr = {0};
                charLitStr.data = lState->currLocInBuffer - 1;
                charLitStr.len = 2;

                if(bssGetEscapeCharValue(charLitStr) == '\"')
                {
                    tokLen++;
                    bssLexerAdvanceChar(lState);
                }
            }
            else if (lState->ch == '{')
            {
                tok.isFmtStr = true;
                bool prevOpenBrack = true;
                u64 bracketCount = 1;
                while(lState->ch != '}' || (bracketCount != 0))
                {
                    tokLen++;
                    bssLexerAdvanceChar(lState);

                    if(lState->ch == '}')
                    {
                        bracketCount -= 1;
                        prevOpenBrack = false;
                    }
                    else if(lState->ch == '{')
                    {
                        if(prevOpenBrack)
                        {
                            tok.isFmtStr = false;
                        }

                        bracketCount += 1;
                        prevOpenBrack = true;
                    }
                    else
                    {
                        prevOpenBrack = false;
                    }
                }
            }

            tokLen++;
            if(!bssLexerAdvanceChar(lState))
            {
                goto LEX_START;
            }
        }

        bssLexerAdvanceChar(lState);
        tokLen += 1;

        str8 lexeme = {0};
        lexeme.data =  lState->currLocInBuffer - 1 - tokLen;
        lexeme.len = tokLen;

        tok.pos.tokRange.len = tokLen;
        tok.kind = TOK_STR_LIT;
        tok.lexeme = lexeme;
    }
    else if(isdigit(lState->ch))
    {
        u64 tokLen = 0;
        
        while(isdigit(lState->ch))
        {
            tokLen++;
            bssLexerAdvanceChar(lState);
        }

        str8 lexeme = {0};
        lexeme.data =  lState->currLocInBuffer - 1 - tokLen;
        lexeme.len = tokLen;

        tok.pos.tokRange.len = tokLen;
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
            if (gBssBssTokLexemeTable[i].len > 0 && lState->ch == gBssBssTokLexemeTable[i].data[0])
            {
                u64 charsLeftInBuffer = (lState->currLocInBuffer - 1) - lState->buffer.data + lState->buffer.len;
                if(charsLeftInBuffer >= gBssBssTokLexemeTable[i].len)
                {
                    str8 l = baseStr8(lState->currLocInBuffer - 1, gBssBssTokLexemeTable[i].len);
                    if(baseStringsStrEquals(l, gBssBssTokLexemeTable[i], 0) && (l.len > longestMatch))
                    {
                        // for it to match with a keyword identifier,
                        // it should be a token, forexample
                        // fornite should not match with the keyword for, but for tnite should.
                        if (isalpha(l.data[0]) && isalnum(bssLexerPeekCharEx(lState, l.len)))
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
            if(isalpha(lState->ch))
            {
                while(isalnum(bssLexerPeekCharEx(lState, tokLen)))
                {
                    tokLen++;
                }

                tokKind = TOK_IDEN;
            }
            else
            {
                tokKind = lState->ch;
                tokLen = 1;
            }

            lexeme.data =  lState->currLocInBuffer - 1;
            lexeme.len = tokLen;

            if(baseStringsStrEquals(STR8_LIT("false"), lexeme, 0) || baseStringsStrEquals(STR8_LIT("true"), lexeme, 0))
            {
                tokKind = TOK_BOOL_LIT;
            }
        }

        tok.kind = tokKind;
        tok.lexeme = lexeme;
        tok.pos.tokRange.len = lexeme.len;

        lState->ch = *((lState->currLocInBuffer += lexeme.len) - 1);
    }

    return tok;
}
BssTok bssLexerNext(BssLexerState *lState)
{
    return lState->tok = lState->lexedBssToks.data[lState->nextBssTokIndex++];
}
BssTok bssLexerPeekEx(BssLexerState *lState, u64 amount)
{
    if ((lState->nextBssTokIndex + (amount - 1)) >= lState->lexedBssToks.len)
    {
        return lState->lexedBssToks.data[lState->lexedBssToks.len - 1];
    }

    return lState->lexedBssToks.data[lState->nextBssTokIndex + (amount - 1)];
}
BssTok bssLexerPeek(BssLexerState *lState)
{
    return bssLexerPeekEx(lState, 1);
}

void bssLexerPrint(BSSInterpretorState *iState, BssLexerState *lState, char *fmt, ...);
void bssLexerError(BSSInterpretorState *iState, BssLexerState *lState, char *fmt, ...);

void bssLexerPrintTokenRange(BssTok start, BssTok end)
{
    const i64 NUM_CONTEXT_LINES = 3;
    
    u64 endTokLen = (start.kind != TOK_END_INPUT)  ? end.lexeme.len : 1;
    i64 currLine = 0;
    i8 *currCh = (i8 *)start.pos.tokRange.data;
    int colnum = 0;

    i8 *bufStart = (i8 *)start.pos.ownerLexer->buffer.data;
    i8 *bufEnd = (i8 *)(bufStart + start.pos.ownerLexer->buffer.len);

    while( (currLine != -NUM_CONTEXT_LINES) && (currCh != bufStart)) 
    {
        while((currCh != bufStart) && (*(--currCh) != '\n'));

        if(*currCh == '\n') 
        {
            currLine--;
            if(currLine != -NUM_CONTEXT_LINES) currCh--;
        }
    }

    i64 numLinesFromStartTokToEndTok = (i64)(end.pos.line - start.pos.line);

    for(; currLine < NUM_CONTEXT_LINES + numLinesFromStartTokToEndTok; currLine++)
    {
        fprintf(stderr, "%7lld | ", start.pos.line + currLine);

        while((*currCh != '\n') && (currCh != bufEnd))
        {
            if(currLine == 0) colnum++;

            if((currCh >= (i8 *)start.pos.tokRange.data) && (currCh < ((i8 *)end.pos.tokRange.data) + endTokLen))
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
        
        if(currCh != bufEnd) currCh++;
        else break;
        fputc('\n', stderr);
    }
}