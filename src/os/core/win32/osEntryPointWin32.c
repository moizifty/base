#include "osCoreWin32.h"

typedef struct CmdLineHashMap CmdLineHashMap;
void ProgramMain(CmdLineHashMap *);

#ifdef BASE_USE_EXCEPTION_HANDLER
#pragma comment(lib, "dbghelp.lib")
#pragma comment(lib, "Comctl32.lib")

// this is required for loading correct comctl32 dll file
#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include <DbgHelp.h>
inline LONG WINAPI BaseMainThreadExceptionHandler(struct _EXCEPTION_POINTERS *exceptionInfo)
{
    logProgErrorFmt("Program encountered an exception(0x%x).", exceptionInfo->ExceptionRecord->ExceptionCode);

    BaseArenaTemp temp = baseTempBegin(null, 0);
    {
        HANDLE process = GetCurrentProcess();
        HANDLE thread = GetCurrentThread();
        CONTEXT* context = exceptionInfo->ContextRecord;

        SymSetOptions(SYMOPT_EXACT_SYMBOLS | SYMOPT_FAIL_CRITICAL_ERRORS | SYMOPT_LOAD_LINES | SYMOPT_UNDNAME | SYMOPT_NO_PROMPTS);
        if(SymInitializeW(process, null, true))
        {
            STACKFRAME64 frame = {0};
            frame.AddrPC.Offset = context->Rip;
            frame.AddrPC.Mode = AddrModeFlat;
            frame.AddrFrame.Offset = context->Rbp;
            frame.AddrFrame.Mode = AddrModeFlat;
            frame.AddrStack.Offset = context->Rsp;
            frame.AddrStack.Mode = AddrModeFlat;

            for(u64 i = 0; ;i++)
            {
                const u64 maxStackFrames = 32;
                if(i >= maxStackFrames)
                {
                    logProgInfoFmt("...");
                    logProgInfoFmt("END OF STACK");
                    break;
                }

                // the machine type is obviously hardcoded here to intel x64s
                if(StackWalk64(IMAGE_FILE_MACHINE_AMD64,
                            process,
                            thread,
                            &frame,
                            context,
                            null,
                            SymFunctionTableAccess64,
                            SymGetModuleBase64,
                            null))
                {
                    SYMBOL_INFOW *symbol = baseArenaPush(temp.arena, sizeof(SYMBOL_INFOW) + (MAX_SYM_NAME * sizeof(WCHAR)));
                    symbol->SizeOfStruct = sizeof(SYMBOL_INFOW);
                    symbol->MaxNameLen = MAX_SYM_NAME;

                    i64 displacement = 0;
                    if(SymFromAddrW(process, frame.AddrPC.Offset, &displacement, symbol))
                    {
                        str16 name = baseStr16(symbol->Name, wcslen(symbol->Name));
                        str8 name8 = baseStr8FromFromStr16(temp.arena, name);

                        IMAGEHLP_LINEW64 line = {0};
                        line.SizeOfStruct = sizeof(line);
                        
                        DWORD lineDisplacement = 0;
                        if(SymGetLineFromAddrW64(process, frame.AddrPC.Offset, &lineDisplacement, &line))
                        {
                            str16 filename = baseStr16(line.FileName, wcslen(line.FileName));
                            str8 filename8 = baseStr8FromFromStr16(temp.arena, filename);
                            logProgErrorFmt("%S +%d, '%S' line %d", name8, displacement, filename8, line.LineNumber);
                        }
                        
                    }
                    else
                    {
                        IMAGEHLP_MODULEW64 module = {0};
                        module.SizeOfStruct = sizeof(module);
                        if(SymGetModuleInfoW64(process, frame.AddrPC.Offset, &module))
                        {
                            str16 name = baseStr16(module.ModuleName,wcslen(module.ModuleName));
                            logProgInfoFmt("%S", baseStr8FromFromStr16(temp.arena, name));
                        }
                    }
                }
                else
                {
                    break;
                }
            }

            TASKDIALOGCONFIG dialog = {0};
            dialog.cbSize = sizeof(dialog);
            dialog.dwFlags = TDF_SIZE_TO_CONTENT | TDF_ENABLE_HYPERLINKS | TDF_ALLOW_DIALOG_CANCELLATION;
            dialog.pszMainIcon = TD_ERROR_ICON;
            dialog.dwCommonButtons = TDCBF_CLOSE_BUTTON;
            dialog.pszWindowTitle = L"Fatal Exception";
            dialog.pszContent = L"Check log in application folder";
            TaskDialogIndirect(&dialog, 0, 0, 0);
        }

#ifdef BASE_USE_USER_DEFINED_EXCEPTION_HANDLER
        void BaseUserDefinedExceptionHandler(void *);
        BaseUserDefinedExceptionHandler(exceptionInfo);
#endif
    }
    baseTempEnd(temp);

    ExitProcess(EXIT_FAILURE);
}
#endif

#if BUILD_CONSOLE_APP
int main(int argc, char **argv)
{
#ifdef BASE_USE_EXCEPTION_HANDLER
    SetUnhandledExceptionFilter(BaseMainThreadExceptionHandler);
#endif
    BaseMainThreadEntry(ProgramMain, (u64)argc, argv);
    return 0;
}
#else
int WinMain(HINSTANCE instance, HINSTANCE prev_instance, LPSTR lp_cmd_line, int n_show_cmd)
{
#ifdef BASE_USE_EXCEPTION_HANDLER
    SetUnhandledExceptionFilter(BaseMainThreadExceptionHandler);
#endif
    BaseMainThreadEntry(ProgramMain, (u64)__argc, __argv);
    return 0;
}
#endif
