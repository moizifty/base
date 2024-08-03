#include "osCoreWin32.h"
#include "base\baseLog.h"

typedef struct CmdLineHashMap CmdLineHashMap;
void ProgramMain(CmdLineHashMap *);

#ifdef BASE_USE_EXCEPTION_HANDLER
#pragma comment(lib, "dbghelp.lib")
#pragma comment(lib, "Comctl32.lib")

// this is required for loading correct comctl32 dll file
#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include <DbgHelp.h>
inline LONG WINAPI BaseMainThreadExceptionHandler(EXCEPTION_POINTERS *exceptionInfo)
{
    logProgErrorFmt("Program encountered an exception(0x%x).", exceptionInfo->ExceptionRecord->ExceptionCode);

    BaseArenaTemp temp = baseTempBegin(null, 0);
    {
        HANDLE process = GetCurrentProcess();
        HANDLE thread = GetCurrentThread();
        CONTEXT* context = exceptionInfo->ContextRecord;

        logProgInfoFmt("=========== START OF STACK ===========");

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

                    DWORD64 displacement = 0;
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

            logProgInfoFmt("=========== END OF STACK ===========");
        }

#ifdef BASE_USE_USER_DEFINED_EXCEPTION_HANDLER

        //todo
        //right now its just null but i should
        //have a method called
        //OSTranslatePlatformExceptionInfo or something that returns this
        OSExceptionInfo *ex = null;
        void BaseUserDefinedExceptionHandler(OSExceptionInfo *);
        BaseUserDefinedExceptionHandler(ex);
#endif

        str8 logContent8 = logFlush(OSGetState()->thisProcState.processLog);
        str16 logContent = baseStr16FromFromStr8(temp.arena, logContent8);
        
        TASKDIALOGCONFIG dialog = {0};
        dialog.cbSize = sizeof(dialog);
        dialog.dwFlags = TDF_SIZE_TO_CONTENT | TDF_ENABLE_HYPERLINKS | TDF_ALLOW_DIALOG_CANCELLATION;
        dialog.pszMainIcon = TD_ERROR_ICON;
        dialog.dwCommonButtons = TDCBF_CLOSE_BUTTON;
        dialog.pszWindowTitle = L"Fatal Exception";
        dialog.pszContent = L"The program encountered a fatal exception, see the program log in the programs directory for more info.\n"
                            L"You can also expand the footer below to see a summary of the log.";
        dialog.pszCollapsedControlText = L"Expand for log details";
        dialog.pszExpandedControlText = L"Collapse log";
        dialog.pszExpandedInformation = logContent.data;
        TaskDialogIndirect(&dialog, 0, 0, 0);
    }
    baseTempEnd(temp);

    ExitProcess(EXIT_FAILURE);
}
#endif

#if !defined(BUILD_NOCONSOLE_APP)
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
