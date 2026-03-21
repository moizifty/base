#ifndef BSS_CORE_H
#define BSS_CORE_H

#include "base/baseCore.h"
#include "base/baseMemory.h"
#include "base/baseStrings.h"

typedef enum BssTokKind
{
    TOK_KIND_START = 257,
    TOK_INT_LIT = TOK_KIND_START,
    TOK_BOOL_LIT,
    TOK_STR_LIT,
    TOK_CHAR_LIT,
    
    TOK_IDEN,
    TOK_IF,
    TOK_ELSE,
    TOK_PROJECT,
    TOK_BUILD,
    TOK_RUN,
    TOK_FOR,
    TOK_WHILE,
    TOK_IN,

    TOK_LOGICAL_OR_OP,
    TOK_LOGICAL_AND_OP,
    TOK_EQ_OP,
    TOK_NEQ_OP,

    TOK_END_INPUT,
}BssTokKind;

typedef struct BssTokPos
{
    struct BssLexerState *ownerLexer;
    u64 line;
    u64 col;

    U8Array tokRange;
}BssTokPos;

typedef struct BssTok
{
    BssTokKind kind;
    BssTokPos pos;

    str8 lexeme;
    bool isFmtStr;
}BssTok;

BASE_CREATE_ARRAY_VIEW_DECLS_DEFS(BssTokArray, BssTok);

typedef struct BssTokChunkListNode
{
    struct BssTokChunkListNode *next;
    struct BssTokChunkListNode *prev;

    BssTokArray chunk;
    u64 cap;
}BssTokChunkListNode;

typedef struct BssTokChunkList
{
    struct BssTokChunkListNode *first;
    struct BssTokChunkListNode *last;

    u64 len;
    u64 totalLen;
}BssTokChunkList;

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

typedef struct BssParserState
{
    struct ASTProject *proj;
}BssParserState;

typedef struct BssCheckerState
{
    struct BssScope *rootScope;
    
    struct BssType *runOutput;
    struct BssType *projectType;
}BssCheckerState;

typedef struct BSSInterpretorState
{
    Arena *lexerArena;
    Arena *parserArena;
    Arena *checkerArena;
    
    BssLexerState lState;
    BssParserState pState;
    BssCheckerState cState;

    Str8List buildFlags;
}BSSInterpretorState;

void bssInterpFile(BSSInterpretorState *iState, str8 path);
void bssInterpBuffer(BSSInterpretorState *iState, U8Array buffer);

i64 bssGetEscapeCharValue(str8 escapeCharString);
str8 bssGetStr8RepFromTokLexeme(Arena *arena, BssTok tok);

bool bssHasFlag(BSSInterpretorState *iState, str8 flag);
void bssAddFlag(BSSInterpretorState *iState, str8 flag);
#endif