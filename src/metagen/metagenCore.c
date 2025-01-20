#include "base\baseHash.h"
#include "metagen\metagenCore.h"
#include "os\core\osCore.h"

extern str8 gMetagenCmdKindStr8Table[METAGEN_CMD_COUNT] = 
{
    [METAGEN_CMD_GEN_TABLE] = STR8_LIT_COMP_CONST("metagen_gentable"),
    [METAGEN_CMD_GEN_PRINT_STRUCT_MEMB] = STR8_LIT_COMP_CONST("metagen_genprintstructmemb"),
    [METAGEN_CMD_INTROSPECT] = STR8_LIT_COMP_CONST("metagen_introspect"),
    [METAGEN_CMD_EMBED_FILE] = STR8_LIT_COMP_CONST("metagen_embedfile"),
};

extern MetagenTypeDict gMetagenTypeDict = {0};

BASE_CREATE_EFFICIENT_LL_DEFS(MetagenCStructMembList, MetagenCStructMemb)
BASE_CREATE_EFFICIENT_LL_DEFS(MetagenTypeDictSlot, MetagenTypeDictSlotEntry)

void metagenInit(BaseArena *arena)
{
    gMetagenTypeDict.slots.data = baseArenaPushArray(arena, MetagenTypeDictSlot, 193);
    gMetagenTypeDict.slots.len = 193;
}

bool metagenTypeDictAddType(BaseArena *arena, MetagenTypeDict *dict, str8 type)
{
    bool existed = false;
    u64 hash = hashDJB2(type.data, type.len);

    MetagenTypeDictSlotEntry *found = null;
    MetagenTypeDictSlot slot = dict->slots.data[hash % dict->slots.len];
    BASE_LIST_FOREACH(MetagenTypeDictSlotEntry, node, slot)
    {
        if (baseStringsStrEquals(node->name, type, 0))
        {
            found = node;
            existed = true;
            break;
        }
    }

    if (found == null)
    {
        found = baseArenaPushType(arena, MetagenTypeDictSlotEntry);
        found->name = type;

        BaseListNodePushLastEx(dict->slots.data[hash % dict->slots.len], found, hashPrev, hashNext);
        BaseListNodePushLast(*dict, found);
    }

    return existed;
}

bool metagenHandleEmbedFile(BaseArena *arena, MetagenOutput *output, CTokArray nextToks)
{
    if (nextToks.len >= 7) // (mode, path) = 5 tokens
    {
        if (METAGEN_TOK_MATCH_KIND(nextToks.data[0], '('))
        {
            if (METAGEN_TOK_MATCH_KIND(nextToks.data[1], CTOK_IDEN))
            {
                str8 name = nextToks.data[1].lexeme;

                if (METAGEN_TOK_MATCH_KIND(nextToks.data[2], ','))
                {
                    if (nextToks.data[3].kind == CTOK_STR_LIT)
                    {
                        str8 path = baseCLexerGetStr8RepFromTokLexeme(arena, nextToks.data[3]);
                        U8Array contents = OSFileReadAll(arena,  path);

                        if (METAGEN_TOK_MATCH_KIND(nextToks.data[4], ','))
                        {
                            if (METAGEN_TOK_MATCH_KIND(nextToks.data[5], CTOK_IDEN))
                            {
                                bool bin = METAGEN_TOK_MATCH_LEXEME(nextToks.data[5], STR8_LIT("bin")) ||
                                           METAGEN_TOK_MATCH_LEXEME(nextToks.data[5], STR8_LIT("binary"));

                                Str8List headerEntryList = {0};
                                Str8List implEntryList = {0};

                                Str8ListPushLastFmt(arena, &implEntryList, "// metagen_embedfile(%S, %S), line: %lld\n", name, path, nextToks.data[0].pos.line);
                                
                                Str8ListPushLastFmt(arena, &headerEntryList, "// metagen_embedfile(%S, %S), line: %lld\n", name, path, nextToks.data[0].pos.line);
                                Str8ListPushLastFmt(arena, &headerEntryList, "extern U8Array %S;\n", name);


                                Str8ListPushLastFmt(arena, &implEntryList, "U8Array %S = \n", name);
                                Str8ListPushLastFmt(arena, &implEntryList, "{");

                                if (bin)
                                {
                                    Str8ListPushLastFmt(arena, &implEntryList, "\t.data=(u8[%lld])\n", contents.len);
                                    Str8ListPushLastFmt(arena, &implEntryList, "\t{");

                                    const u64 width = 16; // bytes

                                    for(u64 i = 0; i < contents.len; i++)
                                    {
                                        if (i % width == 0)
                                        {
                                            Str8ListPushLastFmt(arena, &implEntryList, "\n\t\t");
                                        }

                                        Str8ListPushLastFmt(arena, &implEntryList, "%3lld,", (u64)contents.data[i]);
                                    }
                                    Str8ListPushLastFmt(arena, &implEntryList, "\n\t},\n");
                                }
                                else
                                {
                                    Str8ListPushLastFmt(arena, &implEntryList, "\t.data=(u8*)\n", contents.len);
                                    Str8ListPushLastFmt(arena, &implEntryList, "\t\"");
                                    for(u64 i = 0; i < contents.len; i++)
                                    {
                                        if (contents.data[i] == '\n')
                                        {
                                            Str8ListPushLastFmt(arena, &implEntryList, "\\n\"\n\t\"");
                                        }
                                        else if (contents.data[i] == '\r')
                                        {
                                            Str8ListPushLastFmt(arena, &implEntryList, "\\r");
                                        }
                                        else if (contents.data[i] == '\t')
                                        {
                                            Str8ListPushLastFmt(arena, &implEntryList, "\\t");
                                        }
                                        else if (contents.data[i] == '\\')
                                        {
                                            Str8ListPushLastFmt(arena, &implEntryList, "\\\\");
                                        }
                                        else if (contents.data[i] == '\"')
                                        {
                                            Str8ListPushLastFmt(arena, &implEntryList, "\\\"");
                                        }
                                        else
                                        {
                                            Str8ListPushLastFmt(arena, &implEntryList, "%c", contents.data[i]);
                                        }
                                    }
                                    Str8ListPushLastFmt(arena, &implEntryList, "\",\n");
                                }
                                Str8ListPushLastFmt(arena, &implEntryList, "\t.len=%lld,\n", contents.len);
                                Str8ListPushLastFmt(arena, &implEntryList, "};\n");

                                
                                Str8ListPushLast(arena, &output->header.embeds, Str8ListJoin(arena, &headerEntryList, null));
                                Str8ListPushLast(arena, &output->impl.embeds, Str8ListJoin(arena, &implEntryList, null));
                            }
                        }
                    }
                }
            }
        }
    }

    return true;
}

MetagenCStruct metagenParseCStruct(BaseArena *arena, CTokArray nextToks)
{
    CTokArray temp = nextToks;
    MetagenCStruct parsed = {0};

    while (nextToks.len > 0 && 
          (METAGEN_TOK_MATCH_LEXEME(*nextToks.data, STR8_LIT("typedef")) ||
           METAGEN_TOK_MATCH_LEXEME(*nextToks.data, STR8_LIT("struct")) ||
           METAGEN_TOK_MATCH_LEXEME(*nextToks.data, STR8_LIT("union"))))
    {
        nextToks = CTokArraySkip(nextToks, 1);
    }

    if (METAGEN_TOK_MATCH_KIND(*nextToks.data, CTOK_IDEN))
    {
        parsed.name = nextToks.data->lexeme;

        nextToks = CTokArraySkip(nextToks, 1);

    }

    if (METAGEN_TOK_MATCH_KIND(*nextToks.data, '{'))
    {
        // skip '{'
        nextToks = CTokArraySkip(nextToks, 1);


        u64 bracketCount = 1;

        MetagenCStructMembList membs = {0};

        while (nextToks.len > 0 && (bracketCount != 0))
        {
            if (METAGEN_TOK_MATCH_KIND(*nextToks.data, '{'))
            {
                nextToks = CTokArraySkip(nextToks, 1);

                bracketCount++;
            }
            else if (METAGEN_TOK_MATCH_KIND(*nextToks.data, '}'))
            {
                nextToks = CTokArraySkip(nextToks, 1);

                bracketCount--;
            }
            else
            {
                if (METAGEN_TOK_MATCH_KIND(*nextToks.data, CTOK_IDEN))
                {
                    if (METAGEN_TOK_MATCH_LEXEME(*nextToks.data, STR8_LIT("struct")) ||
                        METAGEN_TOK_MATCH_LEXEME(*nextToks.data, STR8_LIT("union")))
                    {
                        if (METAGEN_TOK_MATCH_KIND(nextToks.data[2], '*')) // handle member: struct somestruct *somemember;
                        {
                            nextToks = CTokArraySkip(nextToks, 1);
                            goto PARSE_NORMAL_MEMBER;
                        }
                        else
                        {
                            // todo optimise, pass tmeo arena
                            MetagenCStruct innerParsed = metagenParseCStruct(arena, nextToks);
                            nextToks = CTokArraySkip(nextToks, innerParsed.tokensAdvanced);

                            if (!BASE_NULL_OR_EMPTY(innerParsed.name))
                            {
                                BASE_LIST_FOREACH(MetagenCStructMemb, membNode, innerParsed.membs)
                                {
                                    MetagenCStructMemb *memb = baseArenaPushType(arena, MetagenCStructMemb);
                                    *memb = *membNode;
                                    memb->name = baseStringsPushStr8Fmt(arena, "%S.%S", innerParsed.name, membNode->name);

                                    memb->next = memb->prev = null;

                                    MetagenCStructMembListPushNodeLast(&membs, memb);
                                }
                            }
                            else
                            {
                                BASE_LIST_FOREACH(MetagenCStructMemb, membNode, innerParsed.membs)
                                {
                                    MetagenCStructMemb *memb = baseArenaPushType(arena, MetagenCStructMemb);
                                    *memb = *membNode;
                                    memb->name = baseStringsPushStr8Fmt(arena, "%S", membNode->name);

                                    memb->next = memb->prev = null;

                                    MetagenCStructMembListPushNodeLast(&membs, memb);
                                }
                            }
                        }
                    }
                    else
                    {
                    PARSE_NORMAL_MEMBER:
                        MetagenCStructMemb *memb = baseArenaPushType(arena, MetagenCStructMemb);
                        memb->type = nextToks.data->lexeme;

                        nextToks = CTokArraySkip(nextToks, 1);

                        // i only support one pointer level :( - right now
                        if (METAGEN_TOK_MATCH_KIND(*nextToks.data, '*'))
                        {
                            memb->isPointer = true;

                            nextToks = CTokArraySkip(nextToks, 1);
                        }
                        
                        if (METAGEN_TOK_MATCH_KIND(*nextToks.data, CTOK_IDEN))
                        {
                            memb->name = nextToks.data->lexeme;
                            nextToks = CTokArraySkip(nextToks, 1);

                            if (METAGEN_TOK_MATCH_KIND(*nextToks.data, '['))
                            {
                                memb->isArray = true;
                                nextToks = CTokArraySkip(nextToks, 1);

                                if (METAGEN_TOK_MATCH_KIND(*nextToks.data, CTOK_INT_LIT))
                                {
                                    memb->arrayLength = baseU64FromStr8(nextToks.data->lexeme);
                                    nextToks = CTokArraySkip(nextToks, 1);
                                }
                                else
                                {
                                    baseEPrintf("Metagen doesnt support arrays with lengths defined via constants.\n");
                                }

                                // for ']'
                                nextToks = CTokArraySkip(nextToks, 1);
                            }
                        }

                        MetagenCStructMembListPushNodeLast(&membs, memb);

                        // for ';'
                        nextToks = CTokArraySkip(nextToks, 1);
                    }
                }
            }
        }

        if (METAGEN_TOK_MATCH_KIND(*nextToks.data, CTOK_IDEN))
        {
            parsed.name = nextToks.data->lexeme;
            nextToks = CTokArraySkip(nextToks, 1);
        }

        // for ';'
        nextToks = CTokArraySkip(nextToks, 1);
        
        parsed.membs = membs;
    }

    parsed.tokensAdvanced = temp.len - nextToks.len; 
    return parsed;
}

bool metagenHandleIntrospect(BaseArena *arena, MetagenOutput *output, CTokArray nextToks)
{
    if (nextToks.len >= 2) // ()
    {
        // no params, you want to skip ()
        nextToks = CTokArraySkip(nextToks, 2);

        if (BASE_ANY(nextToks))
        {
            MetagenCStruct parsedStruct = metagenParseCStruct(arena, nextToks);

            metagenTypeDictAddType(arena, &gMetagenTypeDict, parsedStruct.name);
            Str8ListPushLastFmt(arena, &output->header.tables, "extern MetagenStructMembArray g%SMembDefsTable;\n", parsedStruct.name);
            Str8ListPushLastFmt(arena, &output->impl.tables, "extern MetagenStructMembArray g%SMembDefsTable=\n", parsedStruct.name);
            Str8ListPushLastFmt(arena, &output->impl.tables, "{\n");
            Str8ListPushLastFmt(arena, &output->impl.tables, "\t.data=(MetagenStructMemb[%lld])\n", parsedStruct.membs.len);
            Str8ListPushLastFmt(arena, &output->impl.tables, "\t\t{\n");

            BASE_LIST_FOREACH(MetagenCStructMemb, membNode, parsedStruct.membs)
            {
                Str8ListPushLastFmt(arena, &output->impl.tables, "\t\t\t{.name = STR8_LIT_COMP_CONST(\"%S\"), .type = METAGEN_TYPE_%S, .size = sizeof(((%S*)(0))->%S), .offset = BASE_OFFSETOF(%S, %S),", membNode->name, membNode->type, parsedStruct.name, membNode->name, parsedStruct.name, membNode->name);
                if (membNode->isArray)
                {
                    Str8ListPushLastFmt(arena, &output->impl.tables, ".isArray = true, .arrayLen = %lld,", membNode->arrayLength);
                }

                if (membNode->isPointer)
                {
                    Str8ListPushLastFmt(arena, &output->impl.tables, ".isPointer = true,");
                }

                Str8ListPushLastFmt(arena, &output->impl.tables, "},\n");
            }
            
            Str8ListPushLastFmt(arena, &output->impl.tables, "\t\t},\n");
            Str8ListPushLastFmt(arena, &output->impl.tables, "\t.len=%lld\n", parsedStruct.membs.len);
            Str8ListPushLastFmt(arena, &output->impl.tables, "};\n");

            
            basePrintf("struct %S\n", parsedStruct.name);
            BASE_LIST_FOREACH(MetagenCStructMemb, membNode, parsedStruct.membs)
            {
                basePrintf("\t%S %S\n", membNode->type, membNode->name);
            }
        }
    }

    return true;
}

Str8List metagenFindFilesToProcess(BaseArena *arena, str8 path)
{
    Str8List ret = {0};

    if (baseStringsStrEndsWith(path, STR8_LIT(".c"), STR_MATCHFLAGS_CASE_INSENSITIVE) ||
        baseStringsStrEndsWith(path, STR8_LIT(".h"), STR_MATCHFLAGS_CASE_INSENSITIVE))
    {
        Str8ListPushLast(arena, &ret, baseStringsPushStr8Copy(arena, path));
        return ret;
    }

    typedef struct FindTask
    {
        str8 path;

        struct FindTask *next;
        struct FindTask *prev;
    }FindTask;

    FindTask initialTask = {.path = baseStringsPushStr8Fmt(arena, "%S", path)};

    FindTask *firstTask = &initialTask;
    FindTask *lastTask = &initialTask;

    for(FindTask *task = firstTask; task != null; task = task->next)
    {
        OSFileFindIter *iter = OSFindFileBegin(arena, baseStringsPushStr8Fmt(arena, "%S\\*", task->path), null);
        if (iter != null)
        {
            for(OSFileInfo fileInfo = {0}; OSFindFileNext(arena, iter, &fileInfo); )
            {
                if (fileInfo.attrs & OS_FILEATTR_DIR)
                {
                    FindTask *t = baseArenaPushType(arena, FindTask);
                    t->path = baseStringsPushStr8Fmt(arena, "%S\\%S", task->path, fileInfo.name);

                    BaseDllNodePushLast(firstTask, lastTask, t);
                }
                else if (baseStringsStrEndsWith(fileInfo.name, STR8_LIT(".c"), STR_MATCHFLAGS_CASE_INSENSITIVE) ||
                         baseStringsStrEndsWith(fileInfo.name, STR8_LIT(".h"), STR_MATCHFLAGS_CASE_INSENSITIVE))
                {
                    Str8ListPushLast(arena, &ret, baseStringsPushStr8Fmt(arena, "%S\\%S", task->path, fileInfo.name));
                }
            }

            OSFindFileEnd(iter);
        }
    }

    return ret;
}