@echo off
echo Building bss

set baselib=%onedrive%\Documents\Programming\C\base

set flags=/Wall /wd4062 /wd4063 /wd4061 /wd4255 /wd4201 /wd4774 /wd4668 /wd4464 /wd5045 /wd4820 /DBUILD_CONSOLE_APP
set exePath="%baselib%\src\bss\builds\bss.exe"

set outputName=/Fe"%exePath%"
set objFolder=/Fo"%baselib%\src\bss\builds/"
set src=%baselib%\src\bss\bssEntryPoint.c

cl /Zi %src% /I"%baselib%\src" %outputName% %objFolder% %flags%

echo Built bss
echo Calling bss on %~dp0\build.bss

%baselib%\src\bss\builds\bss.exe build.bss %*

@echo on