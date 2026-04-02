#define BASE_USE_EXCEPTION_HANDLER
#include "metagen\base\base.h"
#include "metagen\os\os.h"
#include "metagen\metagen.h"

#include "metagen\base\base.c"
#include "metagen\os\os.c"
#include "metagen\metagen.c"

#include "metagen\os\core\osEntryPoint.c"

void ProgramMain(Str8List args)
{
    bool *defersArg = cmdlineBool(STR8_LIT("defers"), false, STR8_LIT("Process defers in all inputs."), CMDLINE_ARG_PRESENCE_OPTIONAL);
    bool *metadataArg = cmdlineBool(STR8_LIT("metadata"), false, STR8_LIT("Process metada in all inputs."), CMDLINE_ARG_PRESENCE_OPTIONAL);
    bool *cleanDeferArg = cmdlineBool(STR8_LIT("clean-defer"), false, STR8_LIT("Clean all generated defer code."), CMDLINE_ARG_PRESENCE_OPTIONAL);
    bool *cleanMetadataArg = cmdlineBool(STR8_LIT("clean-metadata"), false, STR8_LIT("Clean all generated metadata code"), CMDLINE_ARG_PRESENCE_OPTIONAL);
    Str8List *inputArgs = cmdlineTrailing(STR8_LIT("input"));

    if (!cmdlineParse(args))
    {
        cmdlineUsage();
        return;
    }

    if (inputArgs->len != 1)
    {
        basePrintf("{r}Expected one input.\n");
        cmdlineUsage();
        return;
    }

    Arena *arena = arenaAllocDefault();
    
    if (*cleanDeferArg)
    {
        basePrintf("{g}Cleaning defer genned files\n");
        if (OSPathExists(METAGEN_DEFER_TEMP_FOLDER_NAME))
        {
            if(!OSPathDelete(METAGEN_DEFER_TEMP_FOLDER_NAME, true))
            {
                baseEPrintf("{r}Failed to delete metagen defer temp folder '%S'\n", METAGEN_DEFER_TEMP_FOLDER_NAME);
            }
        }

        basePrintf("{g}Finished cleaning genned defer files\n");
    }
    if (*cleanMetadataArg)
    {
        basePrintf("{g}Cleaning metadata genned files\n");

        Str8List allGened = OSGetFilePaths(arena, inputArgs->first->val, STR8_LIT("*.gen.*"), true);
        BASE_LIST_FOREACH(Str8ListNode, node, allGened)
        {
            OSPathDelete(node->val, true);
        }

        basePrintf("{g}Finished cleaning genned metadata files\n");
    }

    metagenInit(arena);

    Str8List paths = metagenFindFilesToProcess(arena, inputArgs->first->val);

    str8 baseFolder = OSGetProgramDirectory(arena);
    baseFolder = Str8ChopPastLastSlash(Str8ChopPastLastSlash(baseFolder));
    baseFolder = Str8PushFmt(arena, "%S\\base\\", baseFolder);

    if (*metadataArg)
    {
        metagenMetadataPass(arena, baseFolder, &paths);
    }

    if (*defersArg)
    {
        metagenSourceEditPass(arena, paths, inputArgs->first->val);
    }

    basePrintf("{g}Metagen Finished!");
}