# Base

A bit of a stupid name, but this is the base library i use in all of my programs, its got a work in progress linux support and windows support (main)
it also includes a build script language (base/bss) and a metagen program that generates c code based on tags in C files. (base/metagen)

the support in windows is better right now as i have wrapped more win32 api then linux. But linux its getting there, most programs can compile fine for both, including amp

Theres 3 build scripts in this directory, to build the test .c file in base/tests , i tend to use unity builds (including all .c and .h files into one main file and only compiling that, NOT THE GAME ENGINE).

build.bs - is for linux
build.bat - is for windows
build.bss - is currently only for windows, but its written in my build script language (.bss), i kinda hate batch and bash script both, bss also has some cool features like chaining multiple bss files together similar to solution files including multiple proj files in visual studio, but i dont use any of these idek why i implemented them. So in the future i plan on getting rid of bss, its kinda poorly implemented anyway since ive ported it over many versions of my base library and the code is pretty old. 
