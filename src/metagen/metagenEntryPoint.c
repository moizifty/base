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
    
    Arena *arena = arenaAllocDefault();
    metagenInit(arena);

    str8 inputPath = opts->originalInputs.first->val;
    Str8List paths = metagenFindFilesToProcess(arena, inputPath);

    str8 baseFolder = OSGetProgramDirectory(arena);
    baseFolder = Str8ChopPastLastSlash(Str8ChopPastLastSlash(baseFolder));
    baseFolder = Str8PushFmt(arena, "%S\\base\\", baseFolder);

    if (processDefers)
    {
        metagenDefersPass(arena, paths);
    }
    
    if (processMetadata)
    {
        metagenMetadataPass(arena, baseFolder, paths);
    }

    basePrintf("{g}Metagen Finished!");
}