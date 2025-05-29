#ifndef METAGEN_CORE_H
#define METAGEN_CORE_H

#include "base\baseCore.h"
#include "base\baseStrings.h"
#include "base\baseMemory.h"
#include "base\baseThreads.h"
#include "base\baseCLexer.h"

#define METAGEN_TOK_MATCH_KIND(T, K) ((T).kind == (K))
#define METAGEN_TOK_MATCH_LEXEME(T, L) (baseStringsStrEquals((T).lexeme, (L), 0))
#define METAGEN_TOK_MATCH_KIND_LEXEME(T, K, L) (METAGEN_TOK_MATCH_KIND(T, K) && METAGEN_TOK_MATCH_LEXEME(T, L))

typedef enum MetagenCmdKind
{
    METAGEN_CMD_GEN_TABLE,
    METAGEN_CMD_GEN_PRINT_STRUCT_MEMB,
    METAGEN_CMD_INTROSPECT,
    METAGEN_CMD_INTROSPECT_EXCLUDE,
    METAGEN_CMD_EMBED_FILE,
    METAGEN_CMD_COUNT,
}MetagenCmdKind;

typedef struct MetagenOutput
{
    struct
    {
        Str8List typedefs;
        Str8List tables;
        Str8List defines;
        Str8List embeds;
        str8 path;
    }header;

    struct
    {
        Str8List tables;
        Str8List embeds;
        str8 path;
    }impl;
}MetagenOutput;

typedef struct MetagenCStructMemb
{
    str8 name;
    str8 type;

    u8 isPointer : 1;
    u8 isArray : 1; 

    u64 arrayLength;
    
    struct MetagenCStructMemb* next;
    struct MetagenCStructMemb* prev;
}MetagenCStructMemb;

BASE_CREATE_EFFICIENT_LL_DECLS(MetagenCStructMembList, MetagenCStructMemb);

typedef struct MetagenCStruct
{
    str8 name;
    MetagenCStructMembList membs;

    u64 tokensAdvanced;
}MetagenCStruct;

typedef struct MetagenTypeDictSlotEntry
{
    str8 name;

    struct MetagenTypeDictSlotEntry *hashNext;
    struct MetagenTypeDictSlotEntry *hashPrev;

    struct MetagenTypeDictSlotEntry *next;
    struct MetagenTypeDictSlotEntry *prev;
}MetagenTypeDictSlotEntry;

BASE_CREATE_EFFICIENT_LL_DECLS(MetagenTypeDictSlot, MetagenTypeDictSlotEntry);
BASE_CREATE_ARRAY_VIEW_DECLS_DEFS(MetagenTypeDictSlotArray, MetagenTypeDictSlot);

typedef struct MetagenTypeDict
{
    MetagenTypeDictSlotArray slots;

    struct MetagenTypeDictSlotEntry *first;
    struct MetagenTypeDictSlotEntry *last;

    u64 len;
}MetagenTypeDict;

extern str8 gMetagenCmdKindStr8Table[METAGEN_CMD_COUNT];
extern MetagenTypeDict gMetagenTypeDict;

void metagenInit(BaseArena *arena);
bool metagenTypeDictAddType(BaseArena *arena, MetagenTypeDict *dict, str8 type);
bool metagenHandleEmbedFile(BaseArena *arena, MetagenOutput *output, CTokArray nextToks);
bool metagenHandleIntrospect(BaseArena *arena, MetagenOutput *output, CTokArray nextToks);

Str8List metagenFindFilesToProcess(BaseArena *arena, str8 path);

#endif