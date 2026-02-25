#include "osCoreLinux.h"

typedef struct CmdLineHashMap CmdLineHashMap;
void ProgramMain(CmdLineHashMap *);

int main(int argc, char **argv)
{
    BaseMainThreadEntry(ProgramMain, (i64)argc, argv);
    return 0;
}