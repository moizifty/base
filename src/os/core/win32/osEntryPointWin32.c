#include "osCoreWin32.h"

void ProgramMain(void);

#if BUILD_CONSOLE_APP
int main(int argc, char **argv)
{
 BaseMainThreadEntry(ProgramMain, (u64)argc, argv);
 return 0;
}
#else
int WinMain(HINSTANCE instance, HINSTANCE prev_instance, LPSTR lp_cmd_line, int n_show_cmd)
{
 BaseMainThreadEntry(ProgramMain, (u64)__argc, __argv);
 return 0;
}
#endif
