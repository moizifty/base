#ifndef BSS_LEXER_H
#define BSS_LEXER_H

#include "base\baseCore.h"
#include "base\baseStrings.h"
#include "base\baseMemory.h"

#include "bss\bssCore.h"

typedef struct BssLexerState BssLexerState;

typedef struct BssLexerState
{
    struct BssLexerState *next;
    struct BssLexerState *prev;

    str8 filePath;
    U8Array buffer;
    BssTokArray lexedBssToks;

    BssTok tok;
    u64 nextBssTokIndex;

    u8 ch;
    u64 line;
    u64 col;
    u8 *currLocInBuffer;
}BssLexerState;

BASE_CREATE_EFFICIENT_LL_DECLS(BssLexerStateList, BssLexerState);

void BssTokChunkListPushLast(BaseArena *arena, BssTokChunkList *l, BssTok tok);
BssTokArray BssTokChunkListFlattenToArray(BaseArena *arena, BssTokChunkList *l);

BssLexerState *bssLexerInitFromFile(BSSInterpretorState *iState, str8 filePath);
BssLexerState *bssLexerInitFromBuffer(BSSInterpretorState *iState, U8Array buffer);

BssTokArray bssLexerLexWholeBuffer(BSSInterpretorState *iState, BssLexerState *lState);

bool bssLexerAdvanceChar(BssLexerState *lState);
u8 bssLexerPeekChar(BssLexerState *lState);
u8 bssLexerPeekCharEx(BssLexerState *lState, u64 amount);

BssTok bssLexerNextFromBuffer(BSSInterpretorState *iState, BssLexerState *lState);
BssTok bssLexerNext(BssLexerState *lState);
BssTok bssLexerPeekEx(BssLexerState *lState, u64 amount);
BssTok bssLexerPeek(BssLexerState *lState);

void bssLexerPrint(BSSInterpretorState *iState, BssLexerState *lState, char *fmt, ...);
void bssLexerError(BSSInterpretorState *iState, BssLexerState *lState, char *fmt, ...);

void bssLexerPrintTokenRange(BssTok start, BssTok end);
#endif