#ifndef BSS_LEXER_H
#define BSS_LEXER_H

#include "base/baseCore.h"
#include "base/baseStrings.h"
#include "base/baseMemory.h"

#include "bss\bssCore.h"

void BssTokChunkListPushLast(Arena *arena, BssTokChunkList *l, BssTok tok);
BssTokArray BssTokChunkListFlattenToArray(Arena *arena, BssTokChunkList *l);

bool bssLexerInitFromFile(struct BSSInterpretorState *iState, str8 filePath);
bool bssLexerInitFromBuffer(struct BSSInterpretorState *iState, U8Array buffer);

BssTokArray bssLexerLexWholeBuffer(struct BSSInterpretorState *iState);

bool bssLexerAdvanceChar(struct BSSInterpretorState *iState);
u8 bssLexerPeekChar(struct BSSInterpretorState *iState);
u8 bssLexerPeekCharEx(struct BSSInterpretorState *iState, u64 amount);

BssTok bssLexerNextFromBuffer(struct BSSInterpretorState *iState);
BssTok bssLexerNext(struct BSSInterpretorState *iState);
BssTok bssLexerPeekEx(struct BSSInterpretorState *iState, u64 amount);
BssTok bssLexerPeek(struct BSSInterpretorState *iState);

void bssLexerPrint(struct BSSInterpretorState *iState, char *fmt, ...);
void bssLexerError(struct BSSInterpretorState *iState, char *fmt, ...);

void bssLexerPrintTokenRange(BssTok start, BssTok end);
#endif