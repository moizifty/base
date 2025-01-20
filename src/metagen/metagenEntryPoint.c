#include "base\base.h"
#include "os\os.h"
#include "metagen\metagen.h"

#include "base\base.c"
#include "os\os.c"
#include "metagen\metagen.c"

#include "os\core\osEntryPoint.c"

void ProgramMain(CmdLineHashMap *opts)
{
    if (opts->originalInputs.len != 1)
    {
        baseEPrintf("{r}Expected only one input, either a file or a directory to recusively collect files.\n");
        return;
    }

    BaseArena *arena = baseArenaAllocDefault();

    metagenInit(arena);

    str8 inputPath = opts->originalInputs.first->val;
    Str8List paths = metagenFindFilesToProcess(arena, inputPath);

    BASE_LIST_FOREACH(Str8ListNode, pathNode, paths)
    {
        str8 path = pathNode->val;

        MetagenOutput output = {0};
        output.header.path = baseStringsPushStr8Fmt(arena, "%S.gen.h", baseStringsStrChopPast(path, STR8_LIT("."), 0));
        output.impl.path = baseStringsPushStr8Fmt(arena, "%S.gen.c", baseStringsStrChopPast(path, STR8_LIT("."), 0));

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
                if (baseStringsStrEquals(tok.lexeme, gMetagenCmdKindStr8Table[METAGEN_CMD_EMBED_FILE], 0))
                {
                    metagenHandleEmbedFile(arena, &output, remaining);
                }
                else if (baseStringsStrEquals(tok.lexeme, gMetagenCmdKindStr8Table[METAGEN_CMD_INTROSPECT], 0) &&
                         !baseStringsStrEquals(prevTok.lexeme, STR8_LIT("define"), 0))
                {
                    metagenHandleIntrospect(arena, &output, remaining);
                }
            }

            prevTok = tok;
        }
        DateTime currentTime = OSGetLocalTime();

        if (BASE_ANY(output.header.embeds) || 
            BASE_ANY(output.header.defines) ||
            BASE_ANY(output.header.tables) ||
            BASE_ANY(output.header.typedefs))
        {
            OSHandle outputFile = OSFileOpen(output.header.path, false, OS_FILEACCESS_WRITE, OS_FILECREATION_CREATE_OVERRITE);
            OSFileWriteFmt(outputFile, "/**********************************************************************/\n");
            OSFileWriteFmt(outputFile, "/* GENERATED FILE\n");
            OSFileWriteFmt(outputFile, "/* Input: %S\n", path);
            OSFileWriteFmt(outputFile, "/* Date-Time: %d/%d/%d - %02d:%02d\n", currentTime.day, currentTime.month, currentTime.year, currentTime.hour, currentTime.min);
            OSFileWriteFmt(outputFile, "/**********************************************************************/\n\n");
            OSFileWriteFmt(outputFile, "#include \"base\\baseCore.h\"\n\n");
            OSFileWriteFmt(outputFile, "#include \"base\\baseStrings.h\"\n\n");
            OSFileWriteFmt(outputFile, "#include \"base\\baseMetagen.h\"\n\n");

            BASE_LIST_FOREACH(Str8ListNode, node, output.header.defines)
            {
                OSFileWriteStr8(outputFile, node->val);
            }

            BASE_LIST_FOREACH(Str8ListNode, node, output.header.embeds)
            {
                OSFileWriteStr8(outputFile, node->val);
            }

            BASE_LIST_FOREACH(Str8ListNode, node, output.header.tables)
            {
                OSFileWriteStr8(outputFile, node->val);
            }

            OSFileClose(outputFile);
        }

        if (BASE_ANY(output.impl.embeds) || 
            BASE_ANY(output.impl.tables))
        {
            str8 headerFileName = baseStringsStrSubStr8(output.header.path, baseStringsStrChopPastLastSlash(output.header.path).len + 1, output.header.path.len);
            OSHandle outputFile = OSFileOpen(output.impl.path, false, OS_FILEACCESS_WRITE, OS_FILECREATION_CREATE_OVERRITE);
            OSFileWriteFmt(outputFile, "/**********************************************************************/\n");
            OSFileWriteFmt(outputFile, "/* GENERATED FILE\n");
            OSFileWriteFmt(outputFile, "/* Input: %S\n", path);
            OSFileWriteFmt(outputFile, "/* Date-Time: %d/%d/%d - %02d:%02d\n", currentTime.day, currentTime.month, currentTime.year, currentTime.hour, currentTime.min);
            OSFileWriteFmt(outputFile, "/**********************************************************************/\n\n");
            OSFileWriteFmt(outputFile, "#include \"%S\"\n\n", headerFileName);

            BASE_LIST_FOREACH(Str8ListNode, node, output.impl.embeds)
            {
                OSFileWriteStr8(outputFile, node->val);
            }

            BASE_LIST_FOREACH(Str8ListNode, node, output.impl.tables)
            {
                OSFileWriteStr8(outputFile, node->val);
            }

            OSFileClose(outputFile);
        }
    }

    basePrintf("{g}Metagen Finished!");
}