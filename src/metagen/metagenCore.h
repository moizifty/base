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
    METAGEN_CMD_EMBED_FILE,
    METAGEN_CMD_COUNT,
}MetagenCmdKind;

str8 gMetagenCmdKindStr8Table[METAGEN_CMD_COUNT] = 
{
    [METAGEN_CMD_GEN_TABLE] = STR8_LIT_COMP_CONST("metagen_gentable"),
    [METAGEN_CMD_EMBED_FILE] = STR8_LIT_COMP_CONST("metagen_embedfile"),
};

typedef struct MetagenOutput
{
    struct
    {
        Str8List typedefs;
        Str8List tables;
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

bool metagenHandleEmbedFile(BaseArena *arena, MetagenOutput *output, CTokArray nextToks);
Str8List metagenFindFilesToProcess(BaseArena *arena, str8 path);

#endif