#include "base/baseThreads.h"
#include "os/core/osCore.h"

#include "bss\bssLexer.h"

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

void BssTokChunkListPushLast(Arena *arena, BssTokChunkList *l, BssTok tok)
{
    if(!BASE_ANY_PTR(l) || (l->last->chunk.len >= l->last->cap))
    {
        BssTokChunkListNode *n = arenaPushType(arena, BssTokChunkListNode);
        n->cap = 50;
        n->chunk.data = arenaPushArray(arena, BssTok, n->cap);
        n->chunk.len = 0;

        BasePtrListNodePushLast(l, n);
    }
    
    l->last->chunk.data[l->last->chunk.len] = tok;
    l->last->chunk.len += 1;
    l->totalLen += 1;
}
BssTokArray BssTokChunkListFlattenToArray(Arena *arena, BssTokChunkList *l)
{
    BssTokArray flattened = {0};

    if (!BASE_ANY_PTR(l))
    {
        return flattened;
    }

    flattened.data = arenaPushArray(arena, BssTok, l->totalLen);
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

bool bssLexerInitFromFile(BSSInterpretorState *iState, str8 filePath)
{
    U8Array buffer = OSFileReadAll(iState->lexerArena, filePath);
    if(buffer.data == null)
    {
        return false;
    }

    bssLexerInitFromBuffer(iState, buffer);
    iState->lState.filePath = filePath;

    return true;
}
bool bssLexerInitFromBuffer(BSSInterpretorState *iState, U8Array buffer)
{
    iState->lState = (BssLexerState){0};
    iState->lState.buffer = buffer;
    iState->lState.ch = ' ';
    iState->lState.currLocInBuffer = buffer.data;
    iState->lState.line = 1;
    iState->lState.col = 1;
    iState->lState.nextBssTokIndex = 0;

    return true;
}
BssTokArray bssLexerLexWholeBuffer(struct BSSInterpretorState *iState)
{
    BssTokChunkList tokChunks = {0};

    ArenaTemp temp = baseTempBegin(&iState->lexerArena, 1);
    {
        while((iState->lState.tok = bssLexerNextFromBuffer(iState)).kind != TOK_END_INPUT)
        {
            BssTokChunkListPushLast(iState->lexerArena, &tokChunks, iState->lState.tok);
        }

        BssTokChunkListPushLast(iState->lexerArena, &tokChunks, iState->lState.tok);
    }
    baseTempEnd(temp);

    iState->lState.lexedBssToks = BssTokChunkListFlattenToArray(iState->lexerArena, &tokChunks);
    iState->lState.tok = bssLexerNext(iState);

    return iState->lState.lexedBssToks;
}

bool bssLexerAdvanceChar(struct BSSInterpretorState *iState)
{
    if(iState->lState.currLocInBuffer > (iState->lState.buffer.data + iState->lState.buffer.len))
    {
        return false;
    }

    if(iState->lState.ch == '\n')
    {
       iState->lState.line++;
       iState->lState.col = 0;
    }

    if((iState->lState.ch != '\n') && (iState->lState.ch != '\f') && (iState->lState.ch != '\r'))
        iState->lState.col++;

    iState->lState.ch = *iState->lState.currLocInBuffer;
    iState->lState.currLocInBuffer++;

    return true;
}
u8 bssLexerPeekChar(struct BSSInterpretorState *iState)
{
    return bssLexerPeekCharEx(iState, 1);
}
u8 bssLexerPeekCharEx(struct BSSInterpretorState *iState, u64 amount)
{
    return *((iState->lState.currLocInBuffer - 1) + amount);
}

BssTok bssLexerNextFromBuffer(struct BSSInterpretorState *iState)
{
LEX_START:
    while(iState->lState.currLocInBuffer <= (iState->lState.buffer.data + iState->lState.buffer.len) 
          && (isspace(iState->lState.ch) || iState->lState.ch == '\0'))
    {
        bssLexerAdvanceChar(iState);
    }

    BssTokPos pos = {.line = iState->lState.line, .col = iState->lState.col, .ownerLexer = &iState->lState, .tokRange.data = iState->lState.currLocInBuffer - 1};
    BssTok tok = {.pos = pos};

    if(iState->lState.currLocInBuffer > (iState->lState.buffer.data + iState->lState.buffer.len))
    {
        tok.pos.tokRange.len = 0;
        tok.kind = TOK_END_INPUT;
        tok.lexeme = gBssBssTokLexemeTable[TOK_END_INPUT];
    }
    else if(iState->lState.ch == '/' && bssLexerPeekChar(iState) == '/')
    {
        while(iState->lState.currLocInBuffer <= (iState->lState.buffer.data + iState->lState.buffer.len) 
              && (iState->lState.ch != '\n' || iState->lState.ch == '\0' ))
        {
            bssLexerAdvanceChar(iState);
        }

        goto LEX_START;
    }
    else if(iState->lState.ch == '\"')
    {
        u64 tokLen = 1;
        if(!bssLexerAdvanceChar(iState)) goto LEX_START;

        while(iState->lState.ch != '\"')
        {
            if (iState->lState.ch == '~')
            {
                str8 charLitStr = {0};
                charLitStr.data = iState->lState.currLocInBuffer - 1;
                charLitStr.len = 2;

                if(bssGetEscapeCharValue(charLitStr) == '\"')
                {
                    tokLen++;
                    bssLexerAdvanceChar(iState);
                }
            }
            else if (iState->lState.ch == '{')
            {
                tok.isFmtStr = true;
                bool prevOpenBrack = true;
                u64 bracketCount = 1;
                while(iState->lState.ch != '}' || (bracketCount != 0))
                {
                    tokLen++;
                    bssLexerAdvanceChar(iState);

                    if(iState->lState.ch == '}')
                    {
                        bracketCount -= 1;
                        prevOpenBrack = false;
                    }
                    else if(iState->lState.ch == '{')
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
            if(!bssLexerAdvanceChar(iState))
            {
                goto LEX_START;
            }
        }

        bssLexerAdvanceChar(iState);
        tokLen += 1;

        str8 lexeme = {0};
        lexeme.data =  iState->lState.currLocInBuffer - 1 - tokLen;
        lexeme.len = tokLen;

        tok.pos.tokRange.len = tokLen;
        tok.kind = TOK_STR_LIT;
        tok.lexeme = lexeme;
    }
    else if(isdigit(iState->lState.ch))
    {
        u64 tokLen = 0;
        
        while(isdigit(iState->lState.ch))
        {
            tokLen++;
            bssLexerAdvanceChar(iState);
        }

        str8 lexeme = {0};
        lexeme.data =  iState->lState.currLocInBuffer - 1 - tokLen;
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
            if (gBssBssTokLexemeTable[i].len > 0 && iState->lState.ch == gBssBssTokLexemeTable[i].data[0])
            {
                u64 charsLeftInBuffer = (iState->lState.currLocInBuffer - 1) - iState->lState.buffer.data + iState->lState.buffer.len;
                if(charsLeftInBuffer >= gBssBssTokLexemeTable[i].len)
                {
                    str8 l = baseStr8(iState->lState.currLocInBuffer - 1, gBssBssTokLexemeTable[i].len);
                    if(Str8Equals(l, gBssBssTokLexemeTable[i], 0) && (l.len > longestMatch))
                    {
                        // for it to match with a keyword identifier,
                        // it should be a token, forexample
                        // fornite should not match with the keyword for, but for tnite should.
                        if (isalpha(l.data[0]) && isalnum(bssLexerPeekCharEx(iState, l.len)))
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
            if(isalpha(iState->lState.ch))
            {
                while(isalnum(bssLexerPeekCharEx(iState, tokLen)))
                {
                    tokLen++;
                }

                tokKind = TOK_IDEN;
            }
            else
            {
                tokKind = iState->lState.ch;
                tokLen = 1;
            }

            lexeme.data =  iState->lState.currLocInBuffer - 1;
            lexeme.len = tokLen;

            if(Str8Equals(STR8_LIT("false"), lexeme, 0) || Str8Equals(STR8_LIT("true"), lexeme, 0))
            {
                tokKind = TOK_BOOL_LIT;
            }
        }

        tok.kind = tokKind;
        tok.lexeme = lexeme;
        tok.pos.tokRange.len = lexeme.len;

        iState->lState.ch = *((iState->lState.currLocInBuffer += lexeme.len) - 1);
    }

    return tok;
}
BssTok bssLexerNext(struct BSSInterpretorState *iState)
{
    return iState->lState.tok = iState->lState.lexedBssToks.data[iState->lState.nextBssTokIndex++];
}
BssTok bssLexerPeekEx(struct BSSInterpretorState *iState, u64 amount)
{
    if ((iState->lState.nextBssTokIndex + (amount - 1)) >= iState->lState.lexedBssToks.len)
    {
        return iState->lState.lexedBssToks.data[iState->lState.lexedBssToks.len - 1];
    }

    return iState->lState.lexedBssToks.data[iState->lState.nextBssTokIndex + (amount - 1)];
}
BssTok bssLexerPeek(struct BSSInterpretorState *iState)
{
    return bssLexerPeekEx(iState, 1);
}

void bssLexerPrint(struct BSSInterpretorState *iState, char *fmt, ...);
void bssLexerError(struct BSSInterpretorState *iState, char *fmt, ...);

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