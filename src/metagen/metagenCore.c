#include "metagen\base\baseHash.h"
#include "metagen\metagenCore.h"
#include "metagen\os\core\osCore.h"

MetagenTypeDict gMetagenTypeDict = {0};

str8 gMetagenCmdKindStr8Table[METAGEN_CMD_COUNT] = 
{
    [METAGEN_CMD_GEN_TABLE] = STR8_LIT_COMP_CONST("metagen_gentable"),
    [METAGEN_CMD_GEN_PRINT_STRUCT_MEMB] = STR8_LIT_COMP_CONST("metagen_genprintstructmemb"),
    [METAGEN_CMD_INTROSPECT] = STR8_LIT_COMP_CONST("metagen_introspect"),
    [METAGEN_CMD_INTROSPECT_EXCLUDE] = STR8_LIT_COMP_CONST("metagen_introspectexclude"),
    [METAGEN_CMD_EMBED_FILE] = STR8_LIT_COMP_CONST("metagen_embedfile"),
    [METAGEN_CMD_DEFER] = STR8_LIT_COMP_CONST("metagen_defer"),
};

typedef struct MetagenHasBlock
{
    str8 name;
    CTokKind nextExpected;
    CTokKind skipUntil;
    MetagenScopeOwnerKind kind;
}MetagenHasBlock;

BASE_CREATE_ARRAY_VIEW_DECLS_DEFS(MetagenHasBlockArray, MetagenHasBlock);
MetagenHasBlock gMetagenHasBlockTableData[] = 
{
    {.name = STR8_LIT_COMP_CONST("for"), .nextExpected = '(', .skipUntil = ')', .kind = METAGEN_SCOPE_OWNER_LOOP},
    {.name = STR8_LIT_COMP_CONST("while"), .nextExpected = '(', .skipUntil = ')', .kind = METAGEN_SCOPE_OWNER_LOOP},
    {.name = STR8_LIT_COMP_CONST("if"), .nextExpected = '(', .skipUntil = ')',  .kind = METAGEN_SCOPE_OWNER_OTHER},
    {.name = STR8_LIT_COMP_CONST("else"), .nextExpected = 0, .skipUntil = 0,  .kind = METAGEN_SCOPE_OWNER_OTHER},
    {.name = STR8_LIT_COMP_CONST("switch"), .nextExpected = '(', .skipUntil = ')',  .kind = METAGEN_SCOPE_OWNER_SWITCH},
    {.name = STR8_LIT_COMP_CONST("BASE_LIST_FOREACH"), .nextExpected = '(', .skipUntil = ')', .kind = METAGEN_SCOPE_OWNER_LOOP},
    {.name = STR8_LIT_COMP_CONST("BASE_LIST_FOREACH_EX"), .nextExpected = '(', .skipUntil = ')', .kind = METAGEN_SCOPE_OWNER_LOOP},
    {.name = STR8_LIT_COMP_CONST("BASE_LIST_FOREACH_INDEX"), .nextExpected = '(', .skipUntil = ')', .kind = METAGEN_SCOPE_OWNER_LOOP},
    {.name = STR8_LIT_COMP_CONST("BASE_LIST_FOREACH_INDEX_EX"), .nextExpected = '(', .skipUntil = ')', .kind = METAGEN_SCOPE_OWNER_LOOP},
    {.name = STR8_LIT_COMP_CONST("BASE_LIST_REVFOREACH"), .nextExpected = '(', .skipUntil = ')', .kind = METAGEN_SCOPE_OWNER_LOOP},
    {.name = STR8_LIT_COMP_CONST("BASE_LIST_REVFOREACH_EX"), .nextExpected = '(', .skipUntil = ')', .kind = METAGEN_SCOPE_OWNER_LOOP},
    {.name = STR8_LIT_COMP_CONST("BASE_PTR_LIST_FOREACH"), .nextExpected = '(', .skipUntil = ')', .kind = METAGEN_SCOPE_OWNER_LOOP},
    {.name = STR8_LIT_COMP_CONST("BASE_PTR_LIST_FOREACH"), .nextExpected = '(', .skipUntil = ')', .kind = METAGEN_SCOPE_OWNER_LOOP},
    {.name = STR8_LIT_COMP_CONST("BASE_PTR_LIST_FOREACH_EX"), .nextExpected = '(', .skipUntil = ')', .kind = METAGEN_SCOPE_OWNER_LOOP},
};

MetagenHasBlockArray gMetagenHasBlockTable =
{
    .data = gMetagenHasBlockTableData,
    .len = BASE_ARRAY_SIZE(gMetagenHasBlockTableData),
};

BASE_CREATE_EFFICIENT_LL_DEFS(MetagenCStructMembList, MetagenCStructMemb)
BASE_CREATE_EFFICIENT_LL_DEFS(MetagenTypeDictSlot, MetagenTypeDictSlotEntry)
BASE_CREATE_EFFICIENT_LL_DEFS(MetagenCStructList, MetagenCStruct)
BASE_CREATE_EFFICIENT_LL_DEFS(MetagenOutputList, MetagenOutput)

void metagenInit(Arena *arena)
{
    gMetagenTypeDict.slots.data = arenaPushArray(arena, MetagenTypeDictSlot, 193);
    gMetagenTypeDict.slots.len = 193;
}

bool metagenTypeDictAddType(Arena *arena, MetagenTypeDict *dict, MetagenCStruct *type)
{
    bool existed = false;
    u64 hash = baseHashDJB2(type->name.data, type->name.len);

    MetagenTypeDictSlotEntry *found = null;
    MetagenTypeDictSlot slot = dict->slots.data[hash % dict->slots.len];
    BASE_LIST_FOREACH(MetagenTypeDictSlotEntry, node, slot)
    {
        if (Str8Equals(node->type->name, type->name, 0))
        {
            found = node;
            existed = true;
            break;
        }
    }

    if (found == null)
    {
        found = arenaPushType(arena, MetagenTypeDictSlotEntry);
        found->type = type;

        BaseListNodePushLastEx(dict->slots.data[hash % dict->slots.len], found, hashPrev, hashNext);
        BaseListNodePushLast(*dict, found);
    }

    return existed;
}

MetagenCStruct *metagenTypeDictFindTypeByName(MetagenTypeDict *dict, str8 name)
{
    u64 hash = baseHashDJB2(name.data, name.len);

    MetagenTypeDictSlot slot = dict->slots.data[hash % dict->slots.len];
    BASE_LIST_FOREACH(MetagenTypeDictSlotEntry, node, slot)
    {
        if (Str8Equals(node->type->name, name, 0))
        {
            return node->type;
        }
    }

    return null;
}

bool metagenHandleEmbedFile(Arena *arena, MetagenOutput *output, CTokArray nextToks)
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

MetagenCStruct metagenParseCStruct(Arena *arena, Str8List onlyList, CTokArray nextToks)
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
            bool skipMember = false;

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
                    if (METAGEN_TOK_MATCH_LEXEME(*nextToks.data, gMetagenCmdKindStr8Table[METAGEN_CMD_INTROSPECT_EXCLUDE]))
                    {
                        skipMember = true;

                        nextToks = CTokArraySkip(nextToks, 3);
                    }

                    if (METAGEN_TOK_MATCH_LEXEME(*nextToks.data, STR8_LIT("struct")) ||
                        METAGEN_TOK_MATCH_LEXEME(*nextToks.data, STR8_LIT("union")))
                    {
                        bool isUnion = METAGEN_TOK_MATCH_LEXEME(*nextToks.data, STR8_LIT("union"));
                        bool isStruct = !isUnion;

                        if (METAGEN_TOK_MATCH_KIND(nextToks.data[2], '*')) // handle member: struct somestruct *somemember;
                        {
                            nextToks = CTokArraySkip(nextToks, 1);
                            goto PARSE_NORMAL_MEMBER;
                        }
                        else
                        {
                            // todo optimise, pass tmp arena
                            MetagenCStruct innerParsed = metagenParseCStruct(arena, onlyList, nextToks);
                            nextToks = CTokArraySkip(nextToks, innerParsed.tokensAdvanced);

                            if (!skipMember)
                            {
                                if (!BASE_NULL_OR_EMPTY(innerParsed.name))
                                {
                                    MetagenCStructMemb *aggrMemb = arenaPushType(arena, MetagenCStructMemb);
                                    aggrMemb->isUnion = isUnion;
                                    aggrMemb->isStruct = isStruct;

                                    BASE_LIST_FOREACH(MetagenCStructMemb, membNode, innerParsed.membs)
                                    {
                                        str8 name = Str8PushFmt(arena, "%S.%S", innerParsed.name, membNode->name);
                                        if (BASE_ANY(onlyList))
                                        {
                                            skipMember = Str8ListFindFirst(&onlyList, name, 0) == onlyList.len;
                                        }

                                        if (!skipMember)
                                        {
                                            MetagenCStructMemb *memb = arenaPushType(arena, MetagenCStructMemb);
                                            *memb = *membNode;
                                            memb->name = name;

                                            memb->next = memb->prev = null;

                                            MetagenCStructMembListPushNodeLast(&aggrMemb->aggrMembs, memb);
                                        }
                                    }

                                    if (BASE_ANY(aggrMemb->aggrMembs))
                                    {
                                        MetagenCStructMembListPushNodeLast(&membs, aggrMemb);
                                    }
                                }
                                else
                                {
                                    MetagenCStructMemb *aggrMemb = arenaPushType(arena, MetagenCStructMemb);
                                    aggrMemb->isUnion = isUnion;
                                    aggrMemb->isStruct = isStruct;

                                    BASE_LIST_FOREACH(MetagenCStructMemb, membNode, innerParsed.membs)
                                    {
                                        str8 name = Str8PushFmt(arena, "%S", membNode->name);

                                        MetagenCStructMemb *memb = arenaPushType(arena, MetagenCStructMemb);
                                        *memb = *membNode;
                                        memb->name = name;

                                        memb->next = memb->prev = null;

                                        MetagenCStructMembListPushNodeLast(&aggrMemb->aggrMembs, memb);
                                    }

                                    if (BASE_ANY(aggrMemb->aggrMembs))
                                    {
                                        MetagenCStructMembListPushNodeLast(&membs, aggrMemb);
                                    }
                                }
                            }
                        }
                    }
                    else
                    {
                    PARSE_NORMAL_MEMBER:

                        MetagenCStructMemb memb = {0};
                        memb.type = nextToks.data->lexeme;
                        nextToks = CTokArraySkip(nextToks, 1);

                        // i only support one pointer level :( - right now
                        if (METAGEN_TOK_MATCH_KIND(*nextToks.data, '*'))
                        {
                            memb.isPointer = true;

                            nextToks = CTokArraySkip(nextToks, 1);
                        }
                        
                        if (METAGEN_TOK_MATCH_KIND(*nextToks.data, CTOK_IDEN))
                        {
                            memb.name = nextToks.data->lexeme;
                            nextToks = CTokArraySkip(nextToks, 1);

                            if (METAGEN_TOK_MATCH_KIND(*nextToks.data, '['))
                            {
                                memb.isArray = true;
                                nextToks = CTokArraySkip(nextToks, 1);

                                if (METAGEN_TOK_MATCH_KIND(*nextToks.data, CTOK_INT_LIT))
                                {
                                    memb.arrayLength = U64FromStr8(nextToks.data->lexeme);
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

                        if(!skipMember && BASE_ANY(onlyList))
                        {
                            skipMember = Str8ListFindFirst(&onlyList, memb.name, 0) == onlyList.len;
                        }

                        if (!skipMember)
                        {
                            MetagenCStructMemb *membNode = arenaPushType(arena, MetagenCStructMemb);
                            *membNode = memb;

                            MetagenCStructMembListPushNodeLast(&membs, membNode);
                        }

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

void metagenHandleIntrospect(Arena *arena, MetagenOutput *output, CTokArray nextToks, MetagenCStructList *out)
{
    if (nextToks.len >= 2) // ()
    {
        // skip (
        nextToks = CTokArraySkip(nextToks, 1);
        
        Str8List onlyList = {0};

        if (!METAGEN_TOK_MATCH_KIND(*nextToks.data, ')'))
        {
            if (METAGEN_TOK_MATCH_LEXEME(*nextToks.data, STR8_LIT("only")))
            {
                // skip 'only'':'
                nextToks = CTokArraySkip(nextToks, 2);
                
                while(METAGEN_TOK_MATCH_KIND(*nextToks.data, CTOK_STR_LIT))
                {
                    Str8ListPushLast(arena, &onlyList, baseCLexerGetStr8RepFromTokLexeme(arena, *nextToks.data));
                    nextToks = CTokArraySkip(nextToks, 1);

                    if(METAGEN_TOK_MATCH_KIND(*nextToks.data, ','))
                    {
                        nextToks = CTokArraySkip(nextToks, 1);
                    }
                }
            }
        }

        if (METAGEN_TOK_MATCH_KIND(*nextToks.data, ')'))
        {
            nextToks = CTokArraySkip(nextToks, 1);
        }

        if (BASE_ANY(nextToks))
        {
            MetagenCStruct *parsedStruct = arenaPushType(arena, MetagenCStruct);
            *parsedStruct = metagenParseCStruct(arena, onlyList, nextToks);
            parsedStruct->ownerOutput = output;

            MetagenCStructListPushNodeLast(out, parsedStruct);

            metagenTypeDictAddType(arena, &gMetagenTypeDict, parsedStruct);
            // Str8ListPushLastFmt(arena, &output->header.tables, "extern MetagenStructMembArray g%SMembDefsTable;\n", parsedStruct.name);
            // Str8ListPushLastFmt(arena, &output->impl.tables, "extern MetagenStructMembArray g%SMembDefsTable=\n", parsedStruct.name);
            // Str8ListPushLastFmt(arena, &output->impl.tables, "{\n");
            // Str8ListPushLastFmt(arena, &output->impl.tables, "\t.data=(MetagenStructMemb[%lld])\n", parsedStruct.membs.len);
            // Str8ListPushLastFmt(arena, &output->impl.tables, "\t\t{\n");

            // BASE_LIST_FOREACH(MetagenCStructMemb, membNode, parsedStruct.membs)
            // {
            //     Str8ListPushLastFmt(arena, &output->impl.tables, "\t\t\t{.name = STR8_LIT_COMP_CONST(\"%S\"), .type = METAGEN_TYPE_%S, .size = sizeof(((%S*)(0))->%S), .offset = BASE_OFFSETOF(%S, %S),", membNode->name, membNode->type, parsedStruct.name, membNode->name, parsedStruct.name, membNode->name);
            //     if (membNode->isArray)
            //     {
            //         Str8ListPushLastFmt(arena, &output->impl.tables, ".isArray = true, .arrayLen = %lld,", membNode->arrayLength);
            //     }

            //     if (membNode->isPointer)
            //     {
            //         Str8ListPushLastFmt(arena, &output->impl.tables, ".isPointer = true,");
            //     }

            //     Str8ListPushLastFmt(arena, &output->impl.tables, "},\n");
            // }
            
            // Str8ListPushLastFmt(arena, &output->impl.tables, "\t\t},\n");
            // Str8ListPushLastFmt(arena, &output->impl.tables, "\t.len=%lld\n", parsedStruct.membs.len);
            // Str8ListPushLastFmt(arena, &output->impl.tables, "};\n");
        }
    }
}

Str8List metagenFindFilesToProcess(Arena *arena, str8 path)
{
    Str8List ret = {0};

    if (Str8EndsWith(path, STR8_LIT(".c"), STR_MATCHFLAGS_CASE_INSENSITIVE) ||
        Str8EndsWith(path, STR8_LIT(".h"), STR_MATCHFLAGS_CASE_INSENSITIVE))
    {
        Str8ListPushLast(arena, &ret, Str8PushCopy(arena, path));
        return ret;
    }

    typedef struct FindTask
    {
        str8 path;

        struct FindTask *next;
        struct FindTask *prev;
    }FindTask;

    FindTask initialTask = {.path = Str8PushFmt(arena, "%S", path)};

    FindTask *firstTask = &initialTask;
    FindTask *lastTask = &initialTask;

    for(FindTask *task = firstTask; task != null; task = task->next)
    {
        OSFileFindIter *iter = OSFindFileBegin(arena, Str8PushFmt(arena, "%S\\*", task->path), null);
        if (iter != null)
        {
            for(OSFileInfo fileInfo = {0}; OSFindFileNext(arena, iter, &fileInfo); )
            {
                if (fileInfo.attrs & OS_FILEATTR_DIR)
                {
                    if (!Str8EndsWith(fileInfo.name, STR8_LIT("metagen"), STR_MATCHFLAGS_CASE_INSENSITIVE) &&
                        !Str8EndsWith(fileInfo.name, METAGEN_DEFER_TEMP_FOLDER_NAME, STR_MATCHFLAGS_CASE_INSENSITIVE))
                    {
                        FindTask *t = arenaPushType(arena, FindTask);
                        t->path = Str8PushFmt(arena, "%S\\%S", task->path, fileInfo.name);

                        BaseDllNodePushLast(firstTask, lastTask, t);
                    }
                }
                else if (Str8EndsWith(fileInfo.name, STR8_LIT(".c"), STR_MATCHFLAGS_CASE_INSENSITIVE) ||
                         Str8EndsWith(fileInfo.name, STR8_LIT(".h"), STR_MATCHFLAGS_CASE_INSENSITIVE))
                {
                    Str8ListPushLast(arena, &ret, Str8PushFmt(arena, "%S\\%S", task->path, fileInfo.name));
                }
            }

            OSFindFileEnd(iter);
        }
    }

    return ret;
}

bool metagenTryGetAggregateTypeInfoForMemb(Arena *arena, MetagenCStructMemb memb, MetagenTypeDict *dict, MetagenCTypeInfo *info)
{
    info->alignment = 1;
    info->size = 0;

    if (memb.isUnion)
    {
        BASE_LIST_FOREACH(MetagenCStructMemb, unionMemb, memb.aggrMembs)
        {
            if (metagenTryGetTypeInfoForMemb(arena, *unionMemb, dict, &unionMemb->typeInfo))
            {
                info->alignment = BASE_CLAMP(BASE_MAX(info->alignment, unionMemb->typeInfo.alignment), 0, 8);
                info->size = BASE_MAX(info->size, unionMemb->typeInfo.size);

                unionMemb->offset = memb.offset;
            }
        }
    }
    else if (memb.isStruct)
    {
        BASE_LIST_FOREACH(MetagenCStructMemb, structMemb, memb.aggrMembs)
        {
            if(metagenTryGetTypeInfoForMemb(arena, *structMemb, dict, &structMemb->typeInfo))
            {
                if (structMemb->typeInfo.alignment > info->alignment)
                {
                    info->alignment = BASE_CLAMP(structMemb->typeInfo.alignment, 0, 8);
                }

                while(info->size % structMemb->typeInfo.alignment != 0)
                {
                    info->size++;
                }

                structMemb->offset = memb.offset + info->size;
                info->size += structMemb->typeInfo.size;
            }
        }

        while(info->size % info->alignment != 0)
        {
            info->size++;
        }
    }
    else if (memb.isArray)
    {
        bool success = false;

        MetagenCStruct *baseType = metagenTypeDictFindTypeByName(dict, memb.type);
        if (baseType != null)
        {
            success = metagenCheckType(arena, baseType, dict);
            *info = baseType->typeInfo;
        }
        else if (!metagenTryGetNonAggregateTypeInfoForMemb(memb, info))
        {
            success = metagenTryGetAggregateTypeInfoForMemb(arena, memb, dict, info);
        }

        info->size *= memb.arrayLength;
        return success;
    }
    else
    {
        return false;
    }

    return true;
}

bool metagenTryGetNonAggregateTypeInfoForMemb(MetagenCStructMemb memb, MetagenCTypeInfo *info)
{
    *info = (MetagenCTypeInfo){0};

    if (memb.isPointer)
    {
        info->size = 8;
        info->alignment = 8;
        return true;
    }
    else if (memb.isUnion)
    {
        return false;
    }
    else if (Str8Equals(STR8_LIT("u8"), memb.type, 0))
    {
        info->size = sizeof(u8);
        info->alignment = BASE_ALIGNOF(u8);
    }
    else if (Str8Equals(STR8_LIT("u16"), memb.type, 0))
    {
        info->size = sizeof(u16);
        info->alignment = BASE_ALIGNOF(u16);
    }
    else if (Str8Equals(STR8_LIT("u32"), memb.type, 0))
    {
        info->size = sizeof(u32);
        info->alignment = BASE_ALIGNOF(u32);
    }
    else if (Str8Equals(STR8_LIT("u64"), memb.type, 0))
    {
        info->size = sizeof(u64);
        info->alignment = BASE_ALIGNOF(u64);
    }
    else if (Str8Equals(STR8_LIT("i8"), memb.type, 0))
    {
        info->size = sizeof(i8);
        info->alignment = BASE_ALIGNOF(i8);
    }
    else if (Str8Equals(STR8_LIT("i16"), memb.type, 0))
    {
        info->size = sizeof(i16);
        info->alignment = BASE_ALIGNOF(i16);
    }
    else if (Str8Equals(STR8_LIT("i32"), memb.type, 0))
    {
        info->size = sizeof(i32);
        info->alignment = BASE_ALIGNOF(i32);
    }
    else if (Str8Equals(STR8_LIT("i64"), memb.type, 0))
    {
        info->size = sizeof(i64);
        info->alignment = BASE_ALIGNOF(i64);
    }
    else if (Str8Equals(STR8_LIT("f32"), memb.type, 0))
    {
        info->size = sizeof(f32);
        info->alignment = BASE_ALIGNOF(f32);
    }
    else if (Str8Equals(STR8_LIT("f64"), memb.type, 0))
    {
        info->size = sizeof(f64);
        info->alignment = BASE_ALIGNOF(f64);
    }
    else if (Str8Equals(STR8_LIT("bool"), memb.type, 0))
    {
        info->size = sizeof(bool);
        info->alignment = BASE_ALIGNOF(bool);
    }
    else if (Str8Equals(STR8_LIT("str8"), memb.type, 0))
    {
        info->size = sizeof(str8);
        info->alignment = BASE_ALIGNOF(str8);
    }
    else if (Str8Equals(STR8_LIT("int"), memb.type, 0))
    {
        info->size = sizeof(int);
        info->alignment = BASE_ALIGNOF(int);
    }
    else if (Str8Equals(STR8_LIT("float"), memb.type, 0))
    {
        info->size = sizeof(float);
        info->alignment = BASE_ALIGNOF(float);
    }
    else
    {
        return false;
    }

    return true;
}

bool metagenTryGetTypeInfoForMemb(Arena *arena, MetagenCStructMemb memb, MetagenTypeDict *dict, MetagenCTypeInfo *info)
{
    bool success = true;
    MetagenCStruct *membType = metagenTypeDictFindTypeByName(dict, memb.type);
    if (membType != null && !memb.isPointer && !memb.isUnion)
    {
        success = metagenCheckType(arena, membType, dict);
        *info = membType->typeInfo;
    }
    else if (!metagenTryGetNonAggregateTypeInfoForMemb(memb, info))
    {
        success = metagenTryGetAggregateTypeInfoForMemb(arena, memb, dict, info);
    }
    
    if (!success)
    {
        baseEPrintf("{r}Type '%S' cannot be found. If this is a struct/union please mark it as 'metagen_introspect'\n", memb.type);
    }

    return success;
}

u64 metagenGetTotalMembersIncludingAnonStructAndUnion(MetagenCStructMembList membs)
{
    u64 total = membs.len;
    BASE_LIST_FOREACH(MetagenCStructMemb, memb, membs)
    {
        if (memb->isUnion || memb->isStruct)
        {
            total += metagenGetTotalMembersIncludingAnonStructAndUnion(memb->aggrMembs);

            total -= 1; //minus 1 for the actual anonymous struct which is counted in membs.len
        }
    }

    return total;
}
void metagenFillFlattenedMemb(Arena *arena, MetagenCStruct *type, MetagenCStructMembList membs, bool first)
{
    if (first)
    {
        type->flattenedMembs.data = arenaPushArray(arena, MetagenCStructMemb, metagenGetTotalMembersIncludingAnonStructAndUnion(type->membs));
        type->flattenedMembs.len = 0;

        metagenFillFlattenedMemb(arena, type, type->membs, false);
    }
    else 
    {
        BASE_LIST_FOREACH(MetagenCStructMemb, memb, membs)
        {
            if (memb->isUnion || memb->isStruct)
            {
                metagenFillFlattenedMemb(arena, type, memb->aggrMembs, false);
            }
            else
            {
                type->flattenedMembs.data[type->flattenedMembs.len++] = *memb;
            }
        }
    }
}
bool metagenCheckType(Arena *arena, MetagenCStruct *type, MetagenTypeDict *dict)
{
    basePrintf("checking %S\n", type->name);
    // so we dont get divide by 0 errors later
    type->typeInfo.alignment = 1;
    switch (type->checkStatus)
    {
        case METAGEN_TYPECHECK_STATUS_NONE:
        {
            type->checkStatus = METAGEN_TYPECHECK_STATUS_CHECKING;
            BASE_LIST_FOREACH(MetagenCStructMemb, memb, type->membs)
            {
                if(metagenTryGetTypeInfoForMemb(arena, *memb, dict, &memb->typeInfo))
                {
                    if (memb->typeInfo.alignment > type->typeInfo.alignment)
                    {
                        type->typeInfo.alignment = BASE_CLAMP(memb->typeInfo.alignment, 0, 8);
                    }

                    while(type->typeInfo.size % memb->typeInfo.alignment != 0)
                    {
                        type->typeInfo.size++;
                    }
                    memb->offset = type->typeInfo.size;
                    type->typeInfo.size += memb->typeInfo.size;
                }
                else
                {
                    return false;
                }
            }

            while(type->typeInfo.size % type->typeInfo.alignment != 0)
            {
                type->typeInfo.size++;
            }

            metagenFillFlattenedMemb(arena, type, type->membs, true);
            type->checkStatus = METAGEN_TYPECHECK_STATUS_DONE;
            return true;
        }break;
        case METAGEN_TYPECHECK_STATUS_DONE:
        {
            return true;
        }break;
        case METAGEN_TYPECHECK_STATUS_CHECKING:
        {
            baseEPrintf("{r}Encountered cyclical type dependency.\n");
            return false;
        }break;
    }

    baseEPrintf("unhandled case in check type '.\n");
    return false;
}

void metagenMetadataPass(Arena *arena, str8 baseFolder, Str8List *inputPaths)
{
    basePrintf("{g}Beginning Metagen Metadata pass\n");

    DateTime currentTime = OSGetLocalTime();

    MetagenOutputList allOutputs = {0};
    MetagenCStructList allIntrospectedStructs = {0};

    BASE_LIST_FOREACH(Str8ListNode, pathNode, *inputPaths)
    {
        str8 path = pathNode->val;

        MetagenOutput *output = arenaPushType(arena, MetagenOutput);
        output->inputPath = path;
        output->header.path = Str8PushFmt(arena, "%S.gen.h", Str8ChopPast(path, STR8_LIT("."), STR_MATCHFLAGS_FIND_LAST));
        output->impl.path = Str8PushFmt(arena, "%S.gen.c", Str8ChopPast(path, STR8_LIT("."), STR_MATCHFLAGS_FIND_LAST));

        CLexerState clex = baseCLexerInitFromFile(arena, path);
        baseCLexerLexWholeBuffer(arena, &clex);

        // 1 2 3 4
        CTok prevTok = {0};

        for(u64 i = 0; i < clex.lexedToks.len; i++)
        {
            CTok tok = clex.lexedToks.data[i];
            CTokArray remaining = {.data = clex.lexedToks.data + i + 1, .len = clex.lexedToks.len - (i + 1)};

            if(tok.kind == CTOK_IDEN)
            {
                if (Str8Equals(tok.lexeme, gMetagenCmdKindStr8Table[METAGEN_CMD_EMBED_FILE], 0))
                {
                    metagenHandleEmbedFile(arena, output, remaining);
                }
                else if (Str8Equals(tok.lexeme, gMetagenCmdKindStr8Table[METAGEN_CMD_INTROSPECT], 0) &&
                         !Str8Equals(prevTok.lexeme, STR8_LIT("define"), 0))
                {
                    metagenHandleIntrospect(arena, output, remaining, &allIntrospectedStructs);
                }
            }

            prevTok = tok;
        }

        MetagenOutputListPushNodeLast(&allOutputs, output);
    }

    basePrintf("{g}Checking all introspected structs\n");
    BASE_LIST_FOREACH(MetagenCStruct, type, allIntrospectedStructs)
    {
        if(metagenCheckType(arena, type, &gMetagenTypeDict))
        {
            Str8ListPushLastFmt(arena, &type->ownerOutput->header.defines, "extern MetagenStructMembArray g%SMembDefsTable;\n", type->name);

            Str8ListPushLastFmt(arena, &type->ownerOutput->impl.tables, "extern MetagenStructMembArray g%SMembDefsTable=\n", type->name);
            Str8ListPushLastFmt(arena, &type->ownerOutput->impl.tables, "{\n");

            Str8ListPushLastFmt(arena, &type->ownerOutput->impl.tables, "\t.data=(MetagenStructMemb[%llu])\n", type->flattenedMembs.len);
            //.name = STR8_LIT_COMP_CONST("v"), .type
            Str8ListPushLastFmt(arena, &type->ownerOutput->impl.tables, "\t{\n");

            for(u64 i = 0; i < type->flattenedMembs.len; i++)
            {
                Str8ListPushLastFmt(
                    arena, 
                    &type->ownerOutput->impl.tables, 
                    "\t\t{.name = STR8_LIT_COMP_CONST(\"%S\"), .type = METAGEN_TYPE_%S, .size=%llu, .offset=%llu,", 
                    type->flattenedMembs.data[i].name,
                    type->flattenedMembs.data[i].type,
                    type->flattenedMembs.data[i].typeInfo.size,
                    type->flattenedMembs.data[i].offset
                );

                if (type->flattenedMembs.data[i].isArray)
                {
                    Str8ListPushLastFmt(arena, &type->ownerOutput->impl.tables, ".isArray=true, .arrayLen=%llu,", type->flattenedMembs.data[i].arrayLength);
                }

                if (type->flattenedMembs.data[i].isPointer)
                {
                    Str8ListPushLastFmt(arena, &type->ownerOutput->impl.tables, ".isPointer=true,");
                }

                Str8ListPushLastFmt(arena, &type->ownerOutput->impl.tables, "},\n");
            }

            Str8ListPushLastFmt(arena, &type->ownerOutput->impl.tables, "\t},\n");
            Str8ListPushLastFmt(arena, &type->ownerOutput->impl.tables, "\t.len=%llu,\n", type->flattenedMembs.len);
            Str8ListPushLastFmt(arena, &type->ownerOutput->impl.tables, "};\n");
        }
        else
        {
            baseEPrintf("{r}There was an error whilst introspecting type '%S', see previous error(s) if any\n", type->name);
        }

        basePrintf("type %S, %llu, %llu\n", type->name, type->typeInfo.size, type->typeInfo.alignment);
    }
    
    basePrintf("{g}Generating output\n");
    BASE_LIST_FOREACH(MetagenOutput, output, allOutputs)
    {
        if (BASE_ANY(output->header.embeds) || 
            BASE_ANY(output->header.defines) ||
            BASE_ANY(output->header.tables) ||
            BASE_ANY(output->header.typedefs))
        {
            OSHandle outputFile = OSFileOpen(output->header.path, false, OS_FILEACCESS_WRITE, OS_FILECREATION_CREATE_OVERRITE);
            OSFileWriteFmt(outputFile, "/**********************************************************************/\n");
            OSFileWriteFmt(outputFile, "// GENERATED FILE\n");
            OSFileWriteFmt(outputFile, "// Input: %S\n", output->inputPath);
            OSFileWriteFmt(outputFile, "// Date-Time: %d/%d/%d - %02d:%02d\n", currentTime.day, currentTime.month, currentTime.year, currentTime.hour, currentTime.min);
            OSFileWriteFmt(outputFile, "/**********************************************************************/\n\n");
            //OSFileWriteFmt(outputFile, "#include \"base\\baseCore.h\"\n\n");
            //OSFileWriteFmt(outputFile, "#include \"base\\baseStrings.h\"\n\n");
            OSFileWriteFmt(outputFile, "#include \"base/baseMetagen.h\"\n\n");
            OSFileWriteFmt(outputFile, "#include \"base/baseMetagenCommon.gen.h\"\n\n");

            BASE_LIST_FOREACH(Str8ListNode, node, output->header.defines)
            {
                OSFileWriteStr8(outputFile, node->val);
            }

            BASE_LIST_FOREACH(Str8ListNode, node, output->header.embeds)
            {
                OSFileWriteStr8(outputFile, node->val);
            }

            BASE_LIST_FOREACH(Str8ListNode, node, output->header.tables)
            {
                OSFileWriteStr8(outputFile, node->val);
            }

            OSFileClose(outputFile);

            Str8ListPushLast(arena, inputPaths, output->header.path);
        }

        if (BASE_ANY(output->impl.embeds) || 
            BASE_ANY(output->impl.tables))
        {
            str8 headerFileName = Str8SubStr8(output->header.path, Str8ChopPastLastSlash(output->header.path).len + 1, output->header.path.len);
            OSHandle outputFile = OSFileOpen(output->impl.path, false, OS_FILEACCESS_WRITE, OS_FILECREATION_CREATE_OVERRITE);
            OSFileWriteFmt(outputFile, "/**********************************************************************/\n");
            OSFileWriteFmt(outputFile, "// GENERATED FILE\n");
            OSFileWriteFmt(outputFile, "// Input: %S\n", output->inputPath);
            OSFileWriteFmt(outputFile, "// Date-Time: %d/%d/%d - %02d:%02d\n", currentTime.day, currentTime.month, currentTime.year, currentTime.hour, currentTime.min);
            OSFileWriteFmt(outputFile, "/**********************************************************************/\n\n");
            OSFileWriteFmt(outputFile, "#include \"%S\"\n\n", headerFileName);

            BASE_LIST_FOREACH(Str8ListNode, node, output->impl.embeds)
            {
                OSFileWriteStr8(outputFile, node->val);
            }

            BASE_LIST_FOREACH(Str8ListNode, node, output->impl.tables)
            {
                OSFileWriteStr8(outputFile, node->val);
            }

            OSFileClose(outputFile);

            Str8ListPushLast(arena, inputPaths, output->impl.path);
        }
    }

    if (BASE_ANY(gMetagenTypeDict))
    {
        str8 commonMetagenPath = Str8PushFmt(arena, "%S\\baseMetagenCommon.gen.h", baseFolder);
        OSHandle outputFile = OSFileOpen(commonMetagenPath, false, OS_FILEACCESS_WRITE, OS_FILECREATION_CREATE_OVERRITE);
        OSFileWriteFmt(outputFile, "/**********************************************************************/\n");
        OSFileWriteFmt(outputFile, "// GENERATED FILE\n");
        OSFileWriteFmt(outputFile, "// Date-Time: %d/%d/%d - %02d:%02d\n", currentTime.day, currentTime.month, currentTime.year, currentTime.hour, currentTime.min);
        OSFileWriteFmt(outputFile, "/**********************************************************************/\n\n");

        BASE_LIST_FOREACH(MetagenTypeDictSlotEntry, entryNode, gMetagenTypeDict)
        {
            OSFileWriteFmt(outputFile, "extern MetagenStructMembArray g%SMembDefsTable;\n", entryNode->type->name);
        }

        u64 i = 0;
        BASE_LIST_FOREACH_INDEX(MetagenTypeDictSlotEntry, entryNode, gMetagenTypeDict, i)
        {
            OSFileWriteFmt(outputFile, "#define METAGEN_TYPE_%S (METAGEN_TYPE_CUSTOM_BEGIN + %lld)\n", entryNode->type->name, i);
        }

        OSFileWriteFmt(outputFile, "#define METAGEN_PRINT_MEMB_CUSTOM \\\n");
        BASE_LIST_FOREACH(MetagenTypeDictSlotEntry, entryNode, gMetagenTypeDict)
        {
            OSFileWriteFmt(outputFile, "         case METAGEN_TYPE_%S: basePrintStructEx(((u8*)(member) + (size*i)), g%SMembDefsTable); break;\\\n", entryNode->type->name, entryNode->type->name, entryNode->type->name);
        }

        OSFileClose(outputFile);
    }

    basePrintf("{g}Ending Metagen Metadata pass\n");
}

CTok metagenGetNextNonWhitespaceTok(Arena *arena, Str8List *output, CLexerState *lex, bool outputWhitespace)
{
    CTok tok = {0};
    while((tok = baseCLexerNext(lex)).kind == CTOK_WHITESPACE)
    {
        if (outputWhitespace) Str8ListPushLastFmt(arena, output, "%S", tok.lexeme);
    }

    return tok;
}

str8 metagenDefersParseDefer(Arena *arena, Str8List *output, CLexerState *lex, MetagenScope *parent)
{
    if (Str8Equals(lex->tok.lexeme, gMetagenCmdKindStr8Table[METAGEN_CMD_DEFER], 0))
    {
        metagenGetNextNonWhitespaceTok(arena, output, lex, false);

        if (lex->tok.kind == '{')
        {
            Str8List blockStr = {0};
            metagenDefersProcessScope(arena, &blockStr, lex, parent, METAGEN_SCOPE_OWNER_OTHER);

            return Str8ListJoin(arena, &blockStr, null);
        }
        else
        {
            // todo handle for,if while, switch, dowhile etc statements
            // only handle single line statements for now;
            CTok start = lex->tok;
            CTok end = lex->tok;
            while (lex->tok.kind != ';')
            {
                metagenGetNextNonWhitespaceTok(arena, output, lex, true);
                end = lex->tok;
            }

            metagenGetNextNonWhitespaceTok(arena, output, lex, true);

            return baseStr8(start.lexeme.data, (end.lexeme.data +  end.lexeme.len) - start.lexeme.data);
        }
    }

    return STR8_EMPTY;
}

void metagenDefersEmitTabs(Arena *arena, Str8List *output, u64 amount)
{
    Str8ListPushLastFmt(arena, output, "\r");

    for(u64 t = 0; t < amount; t++)
    {
        Str8ListPushLastFmt(arena, output, "    ");
    }
}

void metagenDefersEmitReturnDefers(Arena *arena, Str8List *output, MetagenScope *scope)
{
    MetagenScope *curr = scope;
    while (curr != null)
    {
        BASE_LIST_REVFOREACH(Str8ListNode, node, curr->defers)
        {
            metagenDefersEmitTabs(arena, output, scope->nestLevel);

            Str8ListPushLastFmt(arena, output, "%S\n", node->val);

            metagenDefersEmitTabs(arena, output, scope->nestLevel);
        }

        curr = curr->parent;
    }
}
void metagenDefersEmitBreakContinueDefers(Arena *arena, Str8List *output, MetagenScope *scope)
{
    MetagenScope *curr = scope;
    while (curr != null)
    {
        BASE_LIST_REVFOREACH(Str8ListNode, node, curr->defers)
        {
            metagenDefersEmitTabs(arena, output, scope->nestLevel);

            Str8ListPushLastFmt(arena, output, "%S\n", node->val);

            metagenDefersEmitTabs(arena, output, scope->nestLevel);
        }

        if (curr->owner == METAGEN_SCOPE_OWNER_LOOP || curr->owner == METAGEN_SCOPE_OWNER_SWITCH)
        {
            break;
        }

        curr = curr->parent;
    }
}

void metagenDefersEmitDefersForNonBlockScopes(Arena *arena, Str8List *output, CLexerState *clex, MetagenScope *scope)
{
    if (Str8Equals(clex->tok.lexeme, STR8_LIT("return"), 0) ||
        Str8Equals(clex->tok.lexeme, STR8_LIT("break"), 0) ||
        Str8Equals(clex->tok.lexeme, STR8_LIT("continue"), 0))
    {
        bool isReturn = Str8Equals(clex->tok.lexeme, STR8_LIT("return"), 0);

        metagenDefersEmitTabs(arena, output, scope->nestLevel);
        Str8ListPushLastFmt(arena, output, "{\n");

        if (isReturn)
        {
            metagenDefersEmitReturnDefers(arena, output, scope);
        }
        else
        {
            metagenDefersEmitBreakContinueDefers(arena, output, scope);
        }

        while (clex->tok.kind != ';')
        {
            Str8ListPushLastFmt(arena, output, "%S", clex->tok.lexeme);
            metagenGetNextNonWhitespaceTok(arena, output, clex, true);
        }

        Str8ListPushLastFmt(arena, output, "%S", clex->tok.lexeme);
        metagenGetNextNonWhitespaceTok(arena, output, clex, true);

        metagenDefersEmitTabs(arena, output, scope->nestLevel);
        Str8ListPushLastFmt(arena, output, "}\n");
    }
}
bool metagenDefersProcessScope(Arena *arena, Str8List *output, CLexerState *clex, MetagenScope *parent, MetagenScopeOwnerKind ownerKind)
{
    MetagenScope scope = {.nestLevel = parent ? parent->nestLevel + 1 : 0, .parent = parent, .owner = ownerKind};
    u64 bracketsSeen = 0;

    bool foundDefers = false;
    if (clex->tok.kind == '{')
    {
        Str8ListPushLastFmt(arena, output, "%S", clex->tok.lexeme);
        bracketsSeen += 1;
        
        baseCLexerNext(clex);

        bool dontEmitAtEndOfScope = false;
        MetagenScopeOwnerKind newScopeOwner = METAGEN_SCOPE_OWNER_OTHER;

        while (bracketsSeen != 0 && clex->tok.kind != CTOK_END_INPUT)
        {
            // if (cond) return; <- eg
            if (clex->tok.kind == '{')
            {
                if(metagenDefersProcessScope(arena, output, clex, &scope, newScopeOwner))
                {
                    foundDefers = true;
                }

                newScopeOwner = METAGEN_SCOPE_OWNER_OTHER;
                continue;
            }
            else if (clex->tok.kind == '}')
            {
                bracketsSeen--;
            }
            else if (Str8Equals(clex->tok.lexeme, gMetagenCmdKindStr8Table[METAGEN_CMD_DEFER], 0))
            {
                str8 block = metagenDefersParseDefer(arena, output, clex, &scope);
                if (BASE_ANY(block))
                {
                    foundDefers = true;
                    Str8ListPushLast(arena, &scope.defers, block);
                }

                continue;
            }
            else if (Str8Equals(clex->tok.lexeme, STR8_LIT("do"), 0))
            {
            }
            else if (Str8Equals(clex->tok.lexeme, STR8_LIT("case"), 0))
            {
                // case
                Str8ListPushLastFmt(arena, output, "%S", clex->tok.lexeme);
                metagenGetNextNonWhitespaceTok(arena, output, clex, true);
                
                // iden
                Str8ListPushLastFmt(arena, output, "%S", clex->tok.lexeme);
                metagenGetNextNonWhitespaceTok(arena, output, clex, true);
                
                // :
                Str8ListPushLastFmt(arena, output, "%S", clex->tok.lexeme);
                metagenGetNextNonWhitespaceTok(arena, output, clex, true);

                if (Str8Equals(clex->tok.lexeme, STR8_LIT("return"), 0) ||
                    Str8Equals(clex->tok.lexeme, STR8_LIT("break"), 0) ||
                    Str8Equals(clex->tok.lexeme, STR8_LIT("continue"), 0))
                {
                    bool isReturn = Str8Equals(clex->tok.lexeme, STR8_LIT("return"), 0);

                    metagenDefersEmitTabs(arena, output, scope.nestLevel);
                    Str8ListPushLastFmt(arena, output, "{\n");

                    if (isReturn)
                    {
                        metagenDefersEmitReturnDefers(arena, output, &scope);
                    }
                    else
                    {
                        metagenDefersEmitBreakContinueDefers(arena, output, &scope);
                    }

                    while (clex->tok.kind != ';')
                    {
                        Str8ListPushLastFmt(arena, output, "%S", clex->tok.lexeme);
                        metagenGetNextNonWhitespaceTok(arena, output, clex, true);
                    }

                    Str8ListPushLastFmt(arena, output, "%S", clex->tok.lexeme);
                    metagenGetNextNonWhitespaceTok(arena, output, clex, true);

                    metagenDefersEmitTabs(arena, output, scope.nestLevel);
                    Str8ListPushLastFmt(arena, output, "}\n");
                }


                if (clex->tok.kind == '{')
                {
                    newScopeOwner = METAGEN_SCOPE_OWNER_SWITCH;
                }

                continue;
            }
            else if (Str8Equals(clex->tok.lexeme, STR8_LIT("return"), 0))
            {
                metagenDefersEmitReturnDefers(arena, output, &scope);
                dontEmitAtEndOfScope = true;
            }
            else if (Str8Equals(clex->tok.lexeme, STR8_LIT("break"), 0) ||
                     Str8Equals(clex->tok.lexeme, STR8_LIT("continue"), 0))
            {
                metagenDefersEmitBreakContinueDefers(arena, output, &scope);
                dontEmitAtEndOfScope = true;
            }
            else if (bracketsSeen == 0 && dontEmitAtEndOfScope == false)
            {
                BASE_LIST_REVFOREACH(Str8ListNode, node, scope.defers)
                {
                    metagenDefersEmitTabs(arena, output, scope.nestLevel);

                    Str8ListPushLastFmt(arena, output, "%S\n", node->val);

                    metagenDefersEmitTabs(arena, output, scope.nestLevel - 1);
                }
            }
            else
            {
                for(u64 i = 0; i < gMetagenHasBlockTable.len; i++)
                {
                    MetagenHasBlock item = gMetagenHasBlockTable.data[i];

                    if (Str8Equals(clex->tok.lexeme, item.name, 0))
                    {
                        CTok tok = clex->tok;

                        Str8ListPushLastFmt(arena, output, "%S", clex->tok.lexeme);
                        metagenGetNextNonWhitespaceTok(arena, output, clex, true);
                        
                        if (item.nextExpected == 0)
                        {
                            if (clex->tok.kind == '{')
                            {
                                newScopeOwner = item.kind;
                            }
                        }
                        else if (clex->tok.kind == item.nextExpected)
                        {
                            u64 pairCount = 1;
                            while (pairCount != 0 && clex->tok.kind != CTOK_END_INPUT)
                            {
                                Str8ListPushLastFmt(arena, output, "%S", clex->tok.lexeme);
                                metagenGetNextNonWhitespaceTok(arena, output, clex, true);
                                if (clex->tok.kind == item.nextExpected) pairCount++;
                                if (clex->tok.kind == item.skipUntil) pairCount--;
                            }

                            if (clex->tok.kind == item.skipUntil)
                            {
                                Str8ListPushLastFmt(arena, output, "%S", clex->tok.lexeme);
                                metagenGetNextNonWhitespaceTok(arena, output, clex, true);
                            }

                            metagenDefersEmitDefersForNonBlockScopes(arena, output, clex, &scope);

                            if (clex->tok.kind == '{')
                            {
                                newScopeOwner = item.kind;
                            }
                        }
                        else
                        {
                            baseEPrintf("%S isElse %S\n", clex->tok.lexeme, tok.lexeme);
                            baseEPrintf("for/if/while keywords require a bracket '(' after, '%S' (%llu, %llu)\n", clex->tok.pos.ownerLexer->filePath, clex->tok.pos.line, clex->tok.pos.col);
                        }

                        goto OUTER_LOOP_END;
                    }
                }
            }

            Str8ListPushLastFmt(arena, output, "%S", clex->tok.lexeme);
            baseCLexerNext(clex);

OUTER_LOOP_END:
            null;
        }
    }

    return foundDefers;
}
void metagenDefersPass(Arena *arena, Str8List inputPaths)
{
    basePrintf("{g}Starting defers pass\n");

    DateTime currentTime = OSGetLocalTime();

    BASE_LIST_FOREACH(Str8ListNode, inputPath, inputPaths)
    {
        CLexerState clex = baseCLexerInitFromFile(arena, inputPath->val);

        clex.allowWhitespace = true;
        baseCLexerLexWholeBuffer(arena, &clex);

        MetagenOutput output = {0};
        output.inputPath = inputPath->val;
        output.impl.path = Str8PushFmt(arena, "%S/%S", METAGEN_DEFER_TEMP_FOLDER_NAME, inputPath->val);

        bool foundDefer = false;
        while (clex.tok.kind != CTOK_END_INPUT)
        {
            if (clex.tok.kind == '{')
            {
                if(metagenDefersProcessScope(arena, &output.impl.raw, &clex, &(MetagenScope){.nestLevel = 0}, METAGEN_SCOPE_OWNER_OTHER))
                {
                    foundDefer = true;
                }
                continue;
            }

            Str8ListPushLastFmt(arena, &output.impl.raw, "%S", clex.tok.lexeme);
            baseCLexerNext(&clex);
        }

        if (foundDefer)
        {
            if (BASE_ANY(output.impl.raw))
            {
                OSHandle outputFile = OSFileOpen(output.impl.path, true, OS_FILEACCESS_WRITE, OS_FILECREATION_CREATE_OVERRITE);
                OSFileWriteFmt(outputFile, "/**********************************************************************/\n");
                OSFileWriteFmt(outputFile, "// GENERATED FILE\n");
                OSFileWriteFmt(outputFile, "// Input: %S\n", output.inputPath);
                OSFileWriteFmt(outputFile, "// Date-Time: %d/%d/%d - %02d:%02d\n", currentTime.day, currentTime.month, currentTime.year, currentTime.hour, currentTime.min);
                OSFileWriteFmt(outputFile, "/**********************************************************************/\n\n");
                
                OSFileWriteStr8(outputFile, Str8ListJoin(arena, &output.impl.raw, null));

                OSFileClose(outputFile);

                basePrintf("{b}Found defer, saving file '%S'\n", output.impl.path);
            }
        }
        else
        {
            OSFileWriteAll(output.impl.path, OSFileReadAll(arena, output.inputPath), true, true);
        }
    }

    basePrintf("{g}Ending defers pass\n");
}