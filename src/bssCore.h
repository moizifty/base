#ifndef BSS_CORE_H
#define BSS_CORE_H

#include "base/baseCore.h"
#include "base/baseStrings.h"
#include "base/baseMemory.h"
#include "base/baseThreads.h"

typedef enum BssTokKind
{
    TOK_KIND_START = 257,
    TOK_INT_LIT = TOK_KIND_START,
    TOK_BOOL_LIT,
    TOK_STR_LIT,
    TOK_CHAR_LIT,
    
    TOK_IDEN,
    TOK_IF_KW,
    TOK_ELSE_KW,
    TOK_FOR_KW,
    TOK_BREAK_KW,
    TOK_CONTINUE_KW,
    TOK_WHILE_KW,
    TOK_IN_KW,
    TOK_FN_KW,
    TOK_RET_KW,

    TOK_LOGICAL_OR_OP,
    TOK_LOGICAL_AND_OP,
    TOK_EQ_OP,
    TOK_NEQ_OP,

    TOK_END_INPUT,
    TOK_KIND_END = TOK_END_INPUT
}BssTokKind;


typedef struct BssTokPos
{
    struct BssLexer *ownerLexer;
    u64 line;
    u64 col;

    U8Array range;
}BssTokPos;

typedef struct BssTok
{
    BssTokKind kind;
    BssTokPos pos;
    bool isFmtStr;

    str8 lexeme;
}BssTok;

BASE_CREATE_LL_DECLS(BssTokList, BssTok)
BASE_CREATE_ARRAY_VIEW_DECLS_DEFS(BssTokArray, BssTok)

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

typedef struct BssLexer
{
    U8Array buffer;
    str8 path;

    u8 ch;
    u64 line;
    u64 col;
    u8 *currLocInBuffer;

    BssTokArray tokArray;
    u64 currTokIndex;
}BssLexer;

typedef struct BssParser
{
    struct BssAstFile *file;
}BssParser;

typedef struct BssInterp
{
    Arena *arena;
    BssLexer *lexer;
    BssParser *parser;
}BssInterp;

void BssTokChunkListPushLast(Arena *arena, BssTokChunkList *l, BssTok tok);
BssTokArray BssTokChunkListFlattenToArray(Arena *arena, BssTokChunkList *l);

i64 bssGetEscapeCharValue(str8 escapeCharString);
void bssPrintSourceRange(BssTokPos start, BssTokPos end, u64 contextLines);

#endif