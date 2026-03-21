## Base

A bit of a stupid name, but this is the base library i use in all of my programs, its got a work in progress linux support and windows support (main)
it also includes a build script language (base/bss) and a metagen program that generates c code based on tags in C files. (base/metagen) and can also generate defers!! (works pretty well)

the support in windows is better right now as i have wrapped more win32 api then linux. But linux its getting there, most programs can compile fine for both, including amp

Theres 3 build scripts in this directory, to build the test .c file in base/tests , i tend to use unity builds (including all .c and .h files into one main file and only compiling that, NOT THE GAME ENGINE).

- build.bs - is for linux
- build.bat - is for windows
- build.bss - is currently only for windows, but its written in my custom script language (.bss), i kinda hate batch and bash script both, bss is basically python with c like syntax. Its dynamically typed

# Metagen
Base library provides a metagenerator for C source code its under `src/metagen` and can be used to get reflection data for types at runtime, embedding files into headers and the coolest one!! it has a implementation for defer statements in C, it does this via tokensing all source code provided to it, and if it sees defer(s) it spits out code into a temp folder, which you then compile from. I tend to use unity builds (compiling only a single file which includes the other source files) so this approach works well.

Eg if you a structure like so:

src
|--- main.c 
|--- another.c <-- this has a defer statement
|--- another.h 

metagen if passed the `defers` arg will generate:

metagen_defers_temp
|--- src
     |--- main.c
     |--- another.c
     |--- another.h

And you then compiler `main.c` from `metagen_defers_temp` instead.

# BSS
BSS is a interpreted language useful for writing small scripts (`src/bss`), its kinda similar to python in its semantics, but used C syntax as that is what i prefer.

BSS only has 5 types:

- int: signed 64 bit integer
- bool: true or false
- string: utf8 string
- array: {n1, n2, n3}
- objects: are a little different but, objects are essentially just scopes which you can access, eg:
`{a = 90, b = {1, 2, 3}, c = {ca = 9, cb = 121}}`, objectes must define the inner variables and they cant be unnamed like with arrays.

