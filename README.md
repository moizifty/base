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

```
src
|--- main.c 
|--- another.c <-- this has a defer statement
|--- another.h 
```

metagen if passed the `defers` arg will generate:

```
metagen_defers_temp
|--- src
     |--- main.c
     |--- another.c
     |--- another.h
```

And you then compiler `main.c` from `metagen_defers_temp` instead.
`defers` will generate defer statements at `return`, `break`, `goto`, `continue` statements aswell at the end of blocks if they werent already emited for the previous ones. It is pretty reliable.

# BSS
BSS is a interpreted language useful for writing small scripts (`src/bss`), its kinda similar to python in its semantics, but uses C syntax as that is what i prefer.

BSS only has 5 types:

- int: signed 64 bit integer
- bool: true or false
- string: utf8 string
- array: {n1, n2, n3}
- objects: are a little different but, objects are essentially just scopes which you can access, eg:
`{a = 90, b = {1, 2, 3}, c = {ca = 9, cb = 121}}`, objectes must define the inner variables and they cant be unnamed like with arrays.

# Cmdline
Base also provides a mechanism for parsing commandline arguments. `src/baseCmdline.h`. The functionality is inspired by golangs `flag` module which i find really cool. It defines commandline arguments inline as function calls, returning a ptr to the argument and when the args are parsed, the ptr will of course have its value updated.

Base builds on top of this a bit more and also combines the type introspection ability with the commandline args, see `cmdlineStruct`, each member in a struct can have a `metagen_introspectnote` attached to it, which gets added to the generated introspection data. `cmdlineStruct` parses this note to automatically create the args and assign them to the members of the struct. This allows you to create a gigantic struct which contains all the commandline args, and you have to do is pass it into `cmdlineStruct` and then call `cmdlineParse`.