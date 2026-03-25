#ifndef METAGEN_CORE_H
#define METAGEN_CORE_H

#include "metagen\base\baseCore.h"
#include "metagen\base\baseStrings.h"
#include "metagen\base\baseMemory.h"
#include "metagen\base\baseThreads.h"
#include "metagen\base\baseCLexer.h"

#define METAGEN_TOK_MATCH_KIND(T, K) ((T).kind == (K))
#define METAGEN_TOK_MATCH_LEXEME(T, L) (Str8Equals((T).lexeme, (L), 0))
#define METAGEN_TOK_MATCH_KIND_LEXEME(T, K, L) (METAGEN_TOK_MATCH_KIND(T, K) && METAGEN_TOK_MATCH_LEXEME(T, L))
#define METAGEN_DEFER_TEMP_FOLDER_NAME (STR8_LIT("metagen_defer_temp"))
typedef enum MetagenCmdKind
{
    METAGEN_CMD_GEN_TABLE,
    METAGEN_CMD_GEN_PRINT_STRUCT_MEMB,
    METAGEN_CMD_INTROSPECT,
    METAGEN_CMD_INTROSPECT_EXCLUDE,
    METAGEN_CMD_EMBED_FILE,
    METAGEN_CMD_DEFER,
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
        Str8List raw;
        str8 path;
    }impl;

    str8 inputPath;

    struct MetagenOutput *next;
    struct MetagenOutput *prev;
}MetagenOutput;

BASE_CREATE_EFFICIENT_LL_DECLS(MetagenOutputList, MetagenOutput);

typedef struct MetagenCStructMemb MetagenCStructMemb;
BASE_CREATE_EFFICIENT_LL_DECLS(MetagenCStructMembList, MetagenCStructMemb);
BASE_CREATE_ARRAY_VIEW_DECLS_DEFS(MetagenCStructMembArray, MetagenCStructMemb);

typedef struct MetagenCTypeInfo
{
    u64 size;
    u64 alignment;
}MetagenCTypeInfo;

struct MetagenCStructMemb
{
    str8 name;
    str8 type;

    u8 isPointer : 1;
    u8 isArray : 1; 
    u8 isUnion : 1;
    u8 isStruct : 1;

    u64 arrayLength;
    
    MetagenCTypeInfo typeInfo;
    u64 offset;

    MetagenCStructMembList aggrMembs;     
    struct MetagenCStructMemb* next;
    struct MetagenCStructMemb* prev;
};

typedef enum MetagenTypeCheckStatus
{
    METAGEN_TYPECHECK_STATUS_NONE,
    METAGEN_TYPECHECK_STATUS_CHECKING,
    METAGEN_TYPECHECK_STATUS_DONE,
}MetagenTypeCheckStatus;

typedef struct MetagenCStruct
{
    str8 name;
    MetagenCStructMembList membs;
    MetagenCStructMembArray flattenedMembs;

    MetagenTypeCheckStatus checkStatus;

    MetagenOutput *ownerOutput;

    MetagenCTypeInfo typeInfo;
    u64 tokensAdvanced;
    struct MetagenCStruct *next;
    struct MetagenCStruct *prev;
}MetagenCStruct;

BASE_CREATE_EFFICIENT_LL_DECLS(MetagenCStructList, MetagenCStruct);

typedef struct MetagenTypeDictSlotEntry
{
    MetagenCStruct *type;

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

typedef enum MetagenScopeOwnerKind
{
    METAGEN_SCOPE_OWNER_OTHER,
    METAGEN_SCOPE_OWNER_LOOP,
    METAGEN_SCOPE_OWNER_SWITCH,
}MetagenScopeOwnerKind;

typedef struct MetagenLabel
{
    str8 name;
    CTokPos pos;

    struct MetagenLabel *next;
    struct MetagenLabel *prev;
}MetagenLabel;

BASE_CREATE_EFFICIENT_LL_DECLS(MetagenLabelList, MetagenLabel)

typedef struct MetagenDefer
{
    str8 content;
    CTokPos start;

    struct MetagenDefer *next;
    struct MetagenDefer *prev;
}MetagenDefer;

BASE_CREATE_EFFICIENT_LL_DECLS(MetagenDeferList, MetagenDefer)

typedef struct MetagenScope
{
    u64 nestLevel;
    MetagenLabelList labels;
    MetagenDeferList defers;

    MetagenScopeOwnerKind owner;
    struct MetagenScope *parent;
}MetagenScope;

extern str8 gMetagenCmdKindStr8Table[METAGEN_CMD_COUNT];
extern MetagenTypeDict gMetagenTypeDict;

bool metagenTypeDictAddType(Arena *arena, MetagenTypeDict *dict, MetagenCStruct *type);
MetagenCStruct *metagenTypeDictFindTypeByName(MetagenTypeDict *dict, str8 name);
bool metagenHandleEmbedFile(Arena *arena, MetagenOutput *output, CTokArray nextToks);
void metagenHandleIntrospect(Arena *arena, MetagenOutput *output, CTokArray nextToks, MetagenCStructList *out);

bool metagenTryGetAggregateTypeInfoForMemb(Arena *arena, MetagenCStructMemb memb, MetagenTypeDict *dict, MetagenCTypeInfo *info);
bool metagenTryGetNonAggregateTypeInfoForMemb(MetagenCStructMemb memb, MetagenCTypeInfo *info);
bool metagenTryGetTypeInfoForMemb(Arena *arena, MetagenCStructMemb memb, MetagenTypeDict *dict, MetagenCTypeInfo *info);

void metagenFillFlattenedMemb(Arena *arena, MetagenCStruct *type, MetagenCStructMembList membs, bool first);
bool metagenCheckType(Arena *arena, MetagenCStruct *type, MetagenTypeDict *dict);
u64 metagenGetTotalMembersIncludingAnonStructAndUnion(MetagenCStructMembList membs);
Str8List metagenFindFilesToProcess(Arena *arena, str8 path);

// pass on introspect, embed tags
CTok metagenGetNextNonWhitespaceTok(Arena *arena, Str8List *output, CLexerState *lex, bool outputWhitespace);
MetagenDefer metagenDefersParseDefer(Arena *arena, Str8List *output, CLexerState *clex, MetagenScope *parent);
bool metagenDefersProcessScope(Arena *arena, Str8List *output, CLexerState *clex, MetagenScope *parent, MetagenScopeOwnerKind ownerKind);
void metagenMetadataPass(Arena *arena, str8 baseFolder, Str8List *inputPaths);
void metagenDefersPass(Arena *arena, Str8List inputPaths);
#endif