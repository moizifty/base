#ifndef BASE_C_LEXER_H
#define BASE_C_LEXER_H

#include "base\baseCore.h"
#include "base\baseMemory.h"
#include "base\baseThreads.h"
#include "base\baseStrings.h"

typedef enum CTokKind
{
    CTOK_KIND_START = 257,
    CTOK_INT_LIT = CTOK_KIND_START,
    CTOK_BOOL_LIT,
    CTOK_STR_LIT,
    CTOK_CHAR_LIT,
    
    CTOK_IDEN,

    CTOK_END_INPUT,
}CTokKind;

typedef struct CTokPos
{
    struct CLexerState *ownerLexer;
    u64 line;
    u64 col;

    U8Array tokRange;
}CTokPos;

typedef struct CTok
{
    CTokKind kind;
    CTokPos pos;

    str8 lexeme;
}CTok;

BASE_CREATE_ARRAY_VIEW_DECLS_DEFS(CTokArray, CTok);

typedef struct CTokChunkListNode
{
    struct CTokChunkListNode *next;
    struct CTokChunkListNode *prev;

    CTokArray chunk;
    u64 cap;
}CTokChunkListNode;

typedef struct CTokChunkList
{
    struct CTokChunkListNode *first;
    struct CTokChunkListNode *last;

    u64 len;
    u64 totalLen;
}CTokChunkList;

typedef struct CLexerState
{
    struct CLexerState *next;
    struct CLexerState *prev;

    str8 filePath;
    U8Array buffer;
    CTokArray lexedToks;

    CTok tok;
    u64 nextTokIndex;

    u8 ch;
    u64 line;
    u64 col;
    u8 *currLocInBuffer;
}CLexerState;

i64 baseCLexerGetEscapeCharValue(str8 escapeCharString);
str8 baseCLexerGetStr8RepFromTokLexeme(BaseArena *arena, CTok tok);

void CTokChunkListPushLast(BaseArena *arena, CTokChunkList *l, CTok tok);
CTokArray CTokChunkListFlattenToArray(BaseArena *arena, CTokChunkList *l);

CLexerState baseCLexerInitFromFile(BaseArena *arena, str8 filePath);
CLexerState baseCLexerInitFromBuffer(U8Array buffer);

CTokArray baseCLexerLexWholeBuffer(BaseArena *arena, CLexerState *lexerState);

bool baseCLexerAdvanceChar(CLexerState *lexerState);
u8 baseCLexerPeekChar(CLexerState *lexerState);
u8 baseCLexerPeekCharEx(CLexerState *lexerState, u64 amount);

CTok baseCLexerNextFromBuffer(CLexerState *lexerState);
CTok baseCLexerNext(CLexerState *lexerState);
CTok baseCLexerPeekEx(CLexerState *lexerState, u64 amount);
CTok baseCLexerPeek(CLexerState *lexerState);

void baseCLexerPrint(CLexerState *lexerState, char *fmt, ...);
void baseCLexerError(CLexerState *lexerState, char *fmt, ...);

void baseCLexerPrintTokenRange(CTok start, CTok end);
#endif