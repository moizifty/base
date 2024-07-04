#ifndef BSS_CORE_H
#define BSS_CORE_H

#include "base\baseCore.h"
#include "base\baseMemory.h"

typedef struct BSSInterpretorState
{
    BaseArena *lexerArena;
    BaseArena *parserArena;
    BaseArena *checkerArena;
    
    struct BssLexerState *lState;
    struct BssParserState *pState;
    struct BssCheckerState *cState;
}BSSInterpretorState;


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


i64 bssGetEscapeCharValue(str8 escapeCharString);
str8 bssGetStr8RepFromTokLexeme(BaseArena *arena, BssTok tok);

#endif