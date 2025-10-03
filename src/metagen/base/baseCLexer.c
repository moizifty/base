#include "base\baseCLexer.h"

#include "os\core\osCore.h"

str8 gBaseCTokLexemeTable[] =
{
    [CTOK_END_INPUT] = STR8_LIT_COMP_CONST("END OF INPUT"),
};

CTokArray CTokArraySkip(CTokArray arr, u64 amount)
{
    return (arr.len > 0) ? (CTokArray){.data = arr.data + amount, .len = arr.len - amount} : arr;
}

i64 baseCLexerGetEscapeCharValue(str8 escapeCharString)
{
     if(escapeCharString.data[0] != '\\')
    {
        return -1;
    }

    if(escapeCharString.len == 2)
    {
        i8 ch = escapeCharString.data[1];

        switch(ch)
        {
            case '\'': return ch;
            case '\"': return ch;
            case '\\': return ch;
            case '0': return '\0';
            case 'r': return '\r';
            case 'n': return '\n';
            case 'a': return '\a';
            case 'b': return '\b';
            case 'f': return '\f';
            case 't': return '\t';
            case 'v': return '\v';
            default: return -1;
        }
    }
    else if(escapeCharString.len == 6) 
    {
        // length six for unicode chars when tht is done eg \uxxxx where xxxx is a hex number
    }

    return -1;
}
str8 baseCLexerGetStr8RepFromTokLexeme(Arena *arena, CTok tok)
{
    str8 ret = {0};
    ArenaTemp temp = baseTempBegin(&arena, 1);
    {
        // '""
        tok.lexeme.data++;

        // the speech marks
        tok.lexeme.len -= 2;

        str8 t = {0}; 
        t.data = arenaPushArray(temp.arena, u8, tok.lexeme.len);
        t.len = 0;

        for(u64 i = 0; i < tok.lexeme.len; i++)
        {
            u8 ch = tok.lexeme.data[i];
            u8 toWrite = ch;

            if (ch == '\\')
            {
                str8 charLitStr = {0};
                charLitStr.data = tok.lexeme.data + i;
                charLitStr.len = 2;

                i64 ev = baseCLexerGetEscapeCharValue(charLitStr);
                toWrite = (u8) ev;

                i++;
            }

            t.data[t.len++] = toWrite;
        }

        ret = Str8PushCopy(arena, t);

    }
    arenaTempEnd(temp);

    return ret;
}

void CTokChunkListPushLast(Arena *arena, CTokChunkList *l, CTok tok)
{
    if(!BASE_ANY_PTR(l) || (l->last->chunk.len >= l->last->cap))
    {
        CTokChunkListNode *n = arenaPushType(arena, CTokChunkListNode);
        n->cap = 50;
        n->chunk.data = arenaPushArray(arena, CTok, n->cap);
        n->chunk.len = 0;

        BasePtrListNodePushLast(l, n);
    }
    
    l->last->chunk.data[l->last->chunk.len] = tok;
    l->last->chunk.len += 1;
    l->totalLen += 1;
}
CTokArray CTokChunkListFlattenToArray(Arena *arena, CTokChunkList *l)
{
    CTokArray flattened = {0};

    if (!BASE_ANY_PTR(l))
    {
        return flattened;
    }

    flattened.data = arenaPushArray(arena, CTok, l->totalLen);
    flattened.len = l->totalLen;

    u64 i = 0;
    BASE_PTR_LIST_FOREACH(CTokChunkListNode, chunk, l)
    {
        for(u64 j = 0; j < chunk->chunk.len; j++)
        {
            flattened.data[i++] = chunk->chunk.data[j];
        }
    }

    return flattened;
}

CLexerState baseCLexerInitFromFile(Arena *arena, str8 filePath)
{
    CLexerState ret = {0};
    U8Array buffer = OSFileReadAll(arena, filePath);

    ret = baseCLexerInitFromBuffer(buffer);
    ret.filePath = filePath;

    return ret;
}
CLexerState baseCLexerInitFromBuffer(U8Array buffer)
{
    CLexerState ret = {0};

    ret.buffer = buffer;
    ret.ch = ' ';
    ret.currLocInBuffer = buffer.data;
    ret.line = 1;
    ret.col = 1;
    ret.nextTokIndex = 0;

    return ret;
}

CTokArray baseCLexerLexWholeBuffer(Arena *arena, CLexerState *lexerState)
{
    CTokChunkList tokChunks = {0};

    ArenaTemp temp = baseTempBegin(&arena, 1);
    {
        while((lexerState->tok = baseCLexerNextFromBuffer(lexerState)).kind != CTOK_END_INPUT)
        {
            CTokChunkListPushLast(arena, &tokChunks, lexerState->tok);
        }

        CTokChunkListPushLast(arena, &tokChunks, lexerState->tok);
    }
    baseTempEnd(temp);

    lexerState->lexedToks = CTokChunkListFlattenToArray(arena, &tokChunks);
    lexerState->tok = baseCLexerNext(lexerState);

    return lexerState->lexedToks;
}

bool baseCLexerAdvanceChar(CLexerState *lexerState)
{
    if(lexerState->currLocInBuffer > (lexerState->buffer.data + lexerState->buffer.len))
    {
        return false;
    }

    if(lexerState->ch == '\n')
    {
       lexerState->line++;
       lexerState->col = 0;
    }

    if((lexerState->ch != '\n') && (lexerState->ch != '\f') && (lexerState->ch != '\r'))
        lexerState->col++;

    lexerState->ch = *lexerState->currLocInBuffer;
    lexerState->currLocInBuffer++;

    return true;
}
u8 baseCLexerPeekChar(CLexerState *lexerState)
{
    return baseCLexerPeekCharEx(lexerState, 1);
}
u8 baseCLexerPeekCharEx(CLexerState *lexerState, u64 amount)
{
    return *((lexerState->currLocInBuffer - 1) + amount);
}

CTok baseCLexerNextFromBuffer(CLexerState *lexerState)
{
LEX_START:
    while(lexerState->currLocInBuffer <= (lexerState->buffer.data + lexerState->buffer.len) 
          && (isspace(lexerState->ch) || lexerState->ch == '\0'))
    {
        baseCLexerAdvanceChar(lexerState);
    }

    CTokPos pos = {.line = lexerState->line, .col = lexerState->col, .ownerLexer = lexerState, .tokRange.data = lexerState->currLocInBuffer - 1};
    CTok tok = {.pos = pos};

    if(lexerState->currLocInBuffer > (lexerState->buffer.data + lexerState->buffer.len))
    {
        tok.pos.tokRange.len = 0;
        tok.kind = CTOK_END_INPUT;
        tok.lexeme = gBaseCTokLexemeTable[CTOK_END_INPUT];
    }
    else if(lexerState->ch == '/' && baseCLexerPeekChar(lexerState) == '/')
    {
        while(lexerState->currLocInBuffer <= (lexerState->buffer.data + lexerState->buffer.len) 
              && (lexerState->ch != '\n' || lexerState->ch == '\0' ))
        {
            baseCLexerAdvanceChar(lexerState);
        }

        goto LEX_START;
    }
    else if(lexerState->ch == '\"')
    {
        u64 tokLen = 1;
        if(!baseCLexerAdvanceChar(lexerState)) goto LEX_START;

        while(lexerState->ch != '\"')
        {
            if (lexerState->ch == '\\')
            {
                str8 charLitStr = {0};
                charLitStr.data = lexerState->currLocInBuffer - 1;
                charLitStr.len = 2;

                if(baseCLexerGetEscapeCharValue(charLitStr) == '\"')
                {
                    tokLen++;
                    baseCLexerAdvanceChar(lexerState);
                }
            }

            tokLen++;
            if(!baseCLexerAdvanceChar(lexerState))
            {
                goto LEX_START;
            }
        }

        baseCLexerAdvanceChar(lexerState);
        tokLen += 1;

        str8 lexeme = {0};
        lexeme.data =  lexerState->currLocInBuffer - 1 - tokLen;
        lexeme.len = tokLen;

        tok.pos.tokRange.len = tokLen;
        tok.kind = CTOK_STR_LIT;
        tok.lexeme = lexeme;
    }
    else if(BASE_ISDIGIT(lexerState->ch))
    {
        u64 tokLen = 0;
        
        while(BASE_ISDIGIT(lexerState->ch))
        {
            tokLen++;
            baseCLexerAdvanceChar(lexerState);
        }

        str8 lexeme = {0};
        lexeme.data =  lexerState->currLocInBuffer - 1 - tokLen;
        lexeme.len = tokLen;

        tok.pos.tokRange.len = tokLen;
        tok.kind = CTOK_INT_LIT;
        tok.lexeme = lexeme;
    }
    else
    {
        str8 lexeme = STR8_LIT("");
        u64 tokKind = 0;
        for(u64 i = CTOK_KIND_START; i < BASE_ARRAY_SIZE(gBaseCTokLexemeTable); i++)
        {
            u64 longestMatch = 0;
            if (gBaseCTokLexemeTable[i].len > 0 && lexerState->ch == gBaseCTokLexemeTable[i].data[0])
            {
                u64 charsLeftInBuffer = (lexerState->currLocInBuffer - 1) - lexerState->buffer.data + lexerState->buffer.len;
                if(charsLeftInBuffer >= gBaseCTokLexemeTable[i].len)
                {
                    str8 l = baseStr8(lexerState->currLocInBuffer - 1, gBaseCTokLexemeTable[i].len);
                    if(Str8Equals(l, gBaseCTokLexemeTable[i], 0) && (l.len > longestMatch))
                    {
                        // for it to match with a keyword identifier,
                        // it should be a token, forexample
                        // fornite should not match with the keyword for, but for tnite should.
                        u8 nxtchar = baseCLexerPeekCharEx(lexerState, l.len);
                        if ((isalnum(l.data[0]) || (l.data[0] == '_')) && (isalnum(nxtchar) || (nxtchar == '_')))
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
            if(isalnum(lexerState->ch) || lexerState->ch == '_')
            {
                u8 nxtchar = baseCLexerPeekCharEx(lexerState, tokLen);
                while(isalnum(nxtchar) || nxtchar == '_')
                {
                    tokLen++;
                    nxtchar = baseCLexerPeekCharEx(lexerState, tokLen);
                }

                tokKind = CTOK_IDEN;
            }
            else
            {
                tokKind = lexerState->ch;
                tokLen = 1;
            }

            lexeme.data =  lexerState->currLocInBuffer - 1;
            lexeme.len = tokLen;

            if(Str8Equals(STR8_LIT("false"), lexeme, 0) || Str8Equals(STR8_LIT("true"), lexeme, 0))
            {
                tokKind = CTOK_BOOL_LIT;
            }
        }

        tok.kind = tokKind;
        tok.lexeme = lexeme;
        tok.pos.tokRange.len = lexeme.len;

        lexerState->ch = *((lexerState->currLocInBuffer += lexeme.len) - 1);
    }

    return tok;
}
CTok baseCLexerNext(CLexerState *lexerState)
{
    return lexerState->tok = lexerState->lexedToks.data[lexerState->nextTokIndex++];
}
CTok baseCLexerPeekEx(CLexerState *lexerState, u64 amount)
{
    if ((lexerState->nextTokIndex + (amount - 1)) >= lexerState->lexedToks.len)
    {
        return lexerState->lexedToks.data[lexerState->lexedToks.len - 1];
    }

    return lexerState->lexedToks.data[lexerState->nextTokIndex + (amount - 1)];
}
CTok baseCLexerPeek(CLexerState *lexerState)
{
    return baseCLexerPeekEx(lexerState, 1);
}

void baseCLexerPrint(CLexerState *lexerState, char *fmt, ...);
void baseCLexerError(CLexerState *lexerState, char *fmt, ...);

void baseCLexerPrintTokenRange(CTok start, CTok end);