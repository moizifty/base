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

typedef enum BssValueKind
{
    BSS_VALUE_VOID,
    BSS_VALUE_INT,
    BSS_VALUE_BOOL,
    BSS_VALUE_CHAR,
    BSS_VALUE_STRING,
    BSS_VALUE_OBJECT,
    BSS_VALUE_ARRAY,
    BSS_VALUE_FUNCTION,
}BssValueKind;

typedef struct BssTokPos
{
    struct BssLexer *ownerLexer;
    u64 line;
    u64 col;

    U8Array range;
}BssTokPos;

typedef struct BssTokFmtStrPos
{
    U8Array range;

    struct BssTokFmtStrPos *next;
    struct BssTokFmtStrPos *prev;
}BssTokFmtStrPos;

BASE_CREATE_EFFICIENT_LL_DECLS(BssTokFmtStrPosList, BssTokFmtStrPos)

typedef struct BssTok
{
    BssTokKind kind;
    BssTokPos pos;

    str8 lexeme;

    BssTokFmtStrPosList fmts;
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

typedef struct BssValue BssValue;
BASE_CREATE_EFFICIENT_LL_DECLS(BssValueList, BssValue)

typedef struct BssValue
{
    BssValueKind kind;

    struct BssValue *next;
    struct BssValue *prev;

    str8 str; //every value also has a str value so this stored alongside the other kind
    union
    {
        i64 num; //bool, int, char
        struct BssScope *obj;
        BssValueList array;

        struct 
        {
            bool isBuiltin;

            union
            {
                struct
                {
                    struct BssBuiltinFunc *fn;
                    u64 numParams;
                }builtin;
                struct
                {
                    struct BssAstFunc *ast;
                }defined;
            };

        }fn;
    };
}BssValue;

BASE_CREATE_ARRAY_VIEW_DECLS_DEFS(BssValueArray, BssValue*)

typedef struct BssInterp BssInterp;
typedef struct BssAstExpr BssAstExpr;
typedef struct BssValue *(*BssFunc)(BssInterp *interp, Arena *scopeArena, BssAstExpr *expr);
typedef struct BssBuiltinFunc
{
    str8 name;
    BssFunc func;

    struct BssBuiltinFunc *next;
    struct BssBuiltinFunc *prev;
}BssBuiltinFunc;

BASE_CREATE_EFFICIENT_LL_DECLS(BssBuiltinFuncList, BssBuiltinFunc)

typedef struct BssInterp
{
    Arena *arena;
    BssLexer *lexer;
    BssParser *parser;

    BssBuiltinFuncList builtins;

    // previous scope which called a function
    struct BssScope *lastFnCalleeScope;
    BssValue *lastRetValue;

    Str8List flags;
    
    struct BssScope *rootScope;
    struct BssScope *currScope;
}BssInterp;

#define BSS_VALUE_ZERO  (&gBssValueEmpty)
#define BSS_VALUE_VOID_VALUE  (&gBssValueVoid)
#define BSS_BUILTIN_FUNC_ZERO  (&gBssBuiltinFuncEmpty)

global BssValue gBssValueEmpty;
global BssValue gBssValueVoid;
global BssBuiltinFunc gBssBuiltinFuncEmpty;

void BssTokChunkListPushLast(Arena *arena, BssTokChunkList *l, BssTok tok);
BssTokArray BssTokChunkListFlattenToArray(Arena *arena, BssTokChunkList *l);

i64 bssGetEscapeCharValue(str8 escapeCharString);
str8 bssGetStr8RepFromTokLexeme(Arena *arena, BssTok tok);

void bssBuiltinFunctionPushEntry(BssInterp *interp, str8 name, int numParams, BssFunc fn);
BssBuiltinFunc *bssBuiltinFunctionFindEntry(BssInterp *interp, str8 name);

void bssPrintSourceRange(BssTokPos start, BssTokPos end, u64 contextLines);

BssValue *bssAllocValue(Arena *arena, BssValueKind kind);
BssValue *bssAllocValueCopy(Arena *arena, BssValue *other);
BssValue *bssAllocValueInt(Arena *arena, i64 val);
BssValue *bssAllocValueStr8(Arena *arena, str8 val);
BssValue *bssAllocValueBool(Arena *arena, bool val);
BssValue *bssAllocValueFn(Arena *arena, struct BssAstFunc *ast);
BssValue *bssAllocValueArray(Arena *arena, BssValueList values);
BssValue *bssAllocValueObj(Arena *arena, struct BssScope *scope);

str8 Str8FromBssValue(Arena *arena, BssValue *value);
#endif