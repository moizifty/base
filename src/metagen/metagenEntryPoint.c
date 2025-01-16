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

    str8 inputPath = opts->originalInputs.first->val;
    Str8List paths = metagenFindFilesToProcess(arena, inputPath);
    
    MetagenOutput output = {0};
    output.header.path = baseStringsPushStr8Fmt(arena, "%S.gen.h", baseStringsStrChopPast(path, STR8_LIT("."), 0));

    CLexerState clex = baseCLexerInitFromFile(arena, path);
    baseCLexerLexWholeBuffer(arena, &clex);

    // 1 2 3 4
    for(u64 i = 0; i < clex.lexedToks.len; i++)
    {
        CTok tok = clex.lexedToks.data[i];
        CTokArray remaining = {.data = clex.lexedToks.data + i + 1, .len = clex.lexedToks.len - (i + 1)};

        if(tok.kind == CTOK_IDEN)
        {
            if (baseStringsStrEquals(tok.lexeme, STR8_LIT("metagen_embedfile"), 0))
            {
                metagenHandleEmbedFile(arena, &output, remaining);
            }
        }
    }

    OSHandle outputFile = OSFileOpen(output.header.path, false, OS_FILEACCESS_WRITE, OS_FILECREATION_CREATE_OVERRITE);
    
    DateTime currentTime = OSGetLocalTime();

    OSFileWriteFmt(outputFile, "/**********************************************************************/\n");
    OSFileWriteFmt(outputFile, "/* GENERATED FILE\n");
    OSFileWriteFmt(outputFile, "/* Input: %S\n", path);
    OSFileWriteFmt(outputFile, "/* Date-Time: %d/%d/%d - %02d:%02d\n", currentTime.day, currentTime.month, currentTime.year, currentTime.hour, currentTime.min);
    OSFileWriteFmt(outputFile, "/**********************************************************************/\n\n");
    OSFileWriteFmt(outputFile, "#include \"base\\baseCore.h\"\n\n");

    BASE_LIST_FOREACH(Str8ListNode, node, output.header.embeds)
    {
        OSFileWriteStr8(outputFile, node->val);
    }

    OSFileClose(outputFile);
}