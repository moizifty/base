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
}BssTokKind;

typedef struct BssLexer
{
    U8Array buffer;
    str8 path;

    u8 ch;
    u64 line;
    u64 col;

    u8 *currLocInBuffer;
}BssLexer;

typedef struct BssInterp
{
    Arena *arena;
    BssLexer *lexer;
}BssInterp;

typedef struct BssTokPos
{
    BssLexer *ownerLexer;
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

i64 bssGetEscapeCharValue(str8 escapeCharString);
#endif