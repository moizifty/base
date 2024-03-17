#include "osCoreWin32.h"

#include "..\..\..\base\baseCore.h"
#include "..\..\..\base\baseStrings.h"
#include "..\..\..\base\baseMemory.h"

void* OSReserveMemory(u64 size)
{
    return VirtualAlloc(NULL, size, MEM_RESERVE, PAGE_NOACCESS);
}
void OSCommitMemory(void *ptr, u64 size)
{
    VirtualAlloc(ptr, size, MEM_COMMIT, PAGE_READWRITE);
}
void OSDecommitMemory(void *ptr, u64 size)
{
    VirtualFree(ptr, size, MEM_DECOMMIT);
}
void OSFreeMemory(void *ptr, u64 size)
{
    VirtualFree(ptr, 0, MEM_RELEASE);
}

bool OSRunProcessEx(BaseArena *arena, str8 app, str8 args, void *peb, str8 *outStr, str8 *errStr)
{
    SECURITY_ATTRIBUTES sa = 
    {
         .nLength = sizeof(SECURITY_ATTRIBUTES), 
         .lpSecurityDescriptor = NULL, 
         .bInheritHandle = true, 
    };

    HANDLE stdoutPread, stdoutPwrite;
    HANDLE stderrPread, stderrPwrite;

    // Create named pipes for stdout and stderr
    if (!CreatePipe(&stdoutPread, &stdoutPwrite, &sa, 0) ||
        !CreatePipe(&stderrPread, &stderrPwrite, &sa, 0)) 
    {
        return false;
    }

    SetHandleInformation(stdoutPread, HANDLE_FLAG_INHERIT, 0);
    SetHandleInformation(stderrPread, HANDLE_FLAG_INHERIT, 0);

    STARTUPINFOA si = 
    { 
         .cb = sizeof(STARTUPINFOA),
         .hStdOutput = stdoutPwrite,
         .hStdError = stderrPwrite,
         .dwFlags = STARTF_USESTDHANDLES,
    };

    // Launch the child process
    PROCESS_INFORMATION pi = {0};
    if (!CreateProcessA(app.data, args.data, NULL, NULL, TRUE, 0, peb, NULL, &si, &pi)) 
    {
        return false;
    }
    
    // Close the read handles for the pipes
    CloseHandle(stdoutPwrite);
    CloseHandle(stderrPwrite);

    *outStr = STR8_LIT("");
    *errStr = STR8_LIT("");
    
    i64 totalStdoutSize = 0;
    i64 totalStderrSize = 0;

    Str8List outList = {0};
    Str8List errList = {0};
    
    DWORD exitCode = 0;

    while(true)
    {
        int readSize = 0;

        GetExitCodeProcess(pi.hProcess, &exitCode);
        DWORD waitR = WaitForSingleObject(pi.hProcess, 0);

        bool isRunning = (waitR != WAIT_OBJECT_0) && (exitCode == STILL_ACTIVE);
        
        DWORD availBytes = 0;
        if((PeekNamedPipe(stdoutPread, NULL, 0, NULL, &availBytes, NULL) && availBytes))
        {
            char buf[255];

            if(!ReadFile(stdoutPread, buf, BASE_ARRAY_SIZE(buf), &readSize, NULL) || (readSize == 0))
            {
                break;
            }

            if(readSize > BASE_ARRAY_SIZE(buf))
            {

                str8 s = {0};
                s.data = baseArenaPushNoZero(arena, readSize + 1);
                s.len = readSize;

                ReadFile(stdoutPread, s.data, readSize, &readSize, NULL);

                Str8ListPushLast(arena, &outList, s);
            }
            else
            {
                Str8ListPushLast(arena, &outList, baseStr8(buf, readSize));
            }

            totalStdoutSize += readSize;
        }

        if(PeekNamedPipe(stderrPread, NULL, 0, NULL, &availBytes, NULL) && availBytes)
        {
            char buf[255];

            if(!ReadFile(stderrPread, buf, BASE_ARRAY_SIZE(buf), &readSize, NULL) || (readSize == 0))
            {
                break;
            }

            if(readSize > BASE_ARRAY_SIZE(buf))
            {

                str8 s = {0};
                s.data = baseArenaPushNoZero(arena, readSize + 1);
                s.len = readSize;

                ReadFile(stderrPread, s.data, readSize, &readSize, NULL);

                Str8ListPushLast(arena, &errList, s);
            }
            else
            {
                Str8ListPushLast(arena, &errList, baseStr8(buf, readSize));
            }

            totalStderrSize += readSize;
        }
        
        if(!isRunning && availBytes == 0)
        {
            break;
        }
    }
    

    if(totalStdoutSize > 0)
    {
        *outStr = Str8ListJoin(arena, &outList, null);
    }
    if(totalStderrSize > 0)
    {
        *errStr = Str8ListJoin(arena, &errList, null);
    }

    CloseHandle(stdoutPread);
    CloseHandle(stderrPread);

    // Wait for the child process to exit
    WaitForSingleObject(pi.hProcess, INFINITE);

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return true;
}