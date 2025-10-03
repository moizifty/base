#include "metagen\base\base.h"
#include "metagen\os\os.h"
#include "metagen\metagen.h"

#include "metagen\base\base.c"
#include "metagen\os\os.c"
#include "metagen\metagen.c"

#include "metagen\os\core\osEntryPoint.c"

void ProgramMain(CmdLineHashMap *opts)
{
    if (opts->originalInputs.len != 1)
    {
        baseEPrintf("{r}Expected only one input, either a file or a directory to recusively collect files.\n");
        return;
    }
    
    Arena *arena = arenaAllocDefault();

    metagenInit(arena);

    str8 inputPath = opts->originalInputs.first->val;
    Str8List paths = metagenFindFilesToProcess(arena, inputPath);

    str8 baseFolder = OSGetProgramDirectory(arena);
    baseFolder = Str8ChopPastLastSlash(Str8ChopPastLastSlash(baseFolder));
    baseFolder = Str8PushFmt(arena, "%S\\base\\", baseFolder);

    DateTime currentTime = OSGetLocalTime();

    MetagenOutputList allOutputs = {0};
    MetagenCStructList allIntrospectedStructs = {0};

    BASE_LIST_FOREACH(Str8ListNode, pathNode, paths)
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
    
    BASE_LIST_FOREACH(MetagenOutput, output, allOutputs)
    {
        if (BASE_ANY(output->header.embeds) || 
            BASE_ANY(output->header.defines) ||
            BASE_ANY(output->header.tables) ||
            BASE_ANY(output->header.typedefs))
        {
            OSHandle outputFile = OSFileOpen(output->header.path, false, OS_FILEACCESS_WRITE, OS_FILECREATION_CREATE_OVERRITE);
            OSFileWriteFmt(outputFile, "/**********************************************************************/\n");
            OSFileWriteFmt(outputFile, "/* GENERATED FILE\n");
            OSFileWriteFmt(outputFile, "/* Input: %S\n", output->inputPath);
            OSFileWriteFmt(outputFile, "/* Date-Time: %d/%d/%d - %02d:%02d\n", currentTime.day, currentTime.month, currentTime.year, currentTime.hour, currentTime.min);
            OSFileWriteFmt(outputFile, "/**********************************************************************/\n\n");
            //OSFileWriteFmt(outputFile, "#include \"base\\baseCore.h\"\n\n");
            //OSFileWriteFmt(outputFile, "#include \"base\\baseStrings.h\"\n\n");
            OSFileWriteFmt(outputFile, "#include \"base\\baseMetagen.h\"\n\n");

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
        }

        if (BASE_ANY(output->impl.embeds) || 
            BASE_ANY(output->impl.tables))
        {
            str8 headerFileName = Str8SubStr8(output->header.path, Str8ChopPastLastSlash(output->header.path).len + 1, output->header.path.len);
            OSHandle outputFile = OSFileOpen(output->impl.path, false, OS_FILEACCESS_WRITE, OS_FILECREATION_CREATE_OVERRITE);
            OSFileWriteFmt(outputFile, "/**********************************************************************/\n");
            OSFileWriteFmt(outputFile, "/* GENERATED FILE\n");
            OSFileWriteFmt(outputFile, "/* Input: %S\n", output->inputPath);
            OSFileWriteFmt(outputFile, "/* Date-Time: %d/%d/%d - %02d:%02d\n", currentTime.day, currentTime.month, currentTime.year, currentTime.hour, currentTime.min);
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
        }
    }

    if (BASE_ANY(gMetagenTypeDict))
    {
        str8 commonMetagenPath = Str8PushFmt(arena, "%S\\baseMetagenCommon.gen.h", baseFolder);
        OSHandle outputFile = OSFileOpen(commonMetagenPath, false, OS_FILEACCESS_WRITE, OS_FILECREATION_CREATE_OVERRITE);
        OSFileWriteFmt(outputFile, "/**********************************************************************/\n");
        OSFileWriteFmt(outputFile, "/* GENERATED FILE\n");
        OSFileWriteFmt(outputFile, "/* Date-Time: %d/%d/%d - %02d:%02d\n", currentTime.day, currentTime.month, currentTime.year, currentTime.hour, currentTime.min);
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
            OSFileWriteFmt(outputFile, "         case METAGEN_TYPE_%S: basePrintStruct(((u8*)(member) + (size*i)), g%SMembDefsTable); break;\\\n", entryNode->type->name, entryNode->type->name, entryNode->type->name);
        }

        OSFileClose(outputFile);
    }

    basePrintf("{g}Metagen Finished!");
}