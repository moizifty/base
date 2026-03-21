#define BASE_USE_EXCEPTION_HANDLER
#include "metagen\base\base.h"
#include "metagen\os\os.h"
#include "metagen\metagen.h"

#include "metagen\base\base.c"
#include "metagen\os\os.c"
#include "metagen\metagen.c"

#include "metagen\os\core\osEntryPoint.c"

void ProgramMain(CmdLineHashMap *opts)
{
    if (opts->originalInputs.len == 0)
    {
        baseEPrintf("{r}Expected only one input, either a file or a directory to recusively collect files.\n");
        return;
    }

    bool processDefers = cmdlineGetFlag(opts, STR8_LIT("defers"));
    bool processMetadata = cmdlineGetFlag(opts, STR8_LIT("metadata"));
    bool clean = cmdlineGetFlag(opts, STR8_LIT("clean"));
    str8 inputPath = opts->originalInputs.first->val;

    Arena *arena = arenaAllocDefault();
    
    if (clean)
    {
        basePrintf("{g}Cleaning genned files\n");
        if(!OSPathDelete(METAGEN_DEFER_TEMP_FOLDER_NAME, true))
        {
            baseEPrintf("{r}Failed to delete metagen defer temp folder '%S'\n", METAGEN_DEFER_TEMP_FOLDER_NAME);
        }

        Str8List allGened = OSGetFilePaths(arena, inputPath, STR8_LIT("*.gen.*"), true);
        BASE_LIST_FOREACH(Str8ListNode, node, allGened)
        {
            OSPathDelete(node->val, true);
        }

        basePrintf("{g}Finished cleaning genned files\n");
    }

    metagenInit(arena);

    Str8List paths = metagenFindFilesToProcess(arena, inputPath);

    str8 baseFolder = OSGetProgramDirectory(arena);
    baseFolder = Str8ChopPastLastSlash(Str8ChopPastLastSlash(baseFolder));
    baseFolder = Str8PushFmt(arena, "%S\\base\\", baseFolder);


    if (processMetadata)
    {
        metagenMetadataPass(arena, baseFolder, &paths);
    }

    if (processDefers)
    {
        metagenDefersPass(arena, paths);
    }

    basePrintf("{g}Metagen Finished!");
}