#!/bin/bash
clear &&

echo "Building"
warn="-Wall -Wextra -Wpedantic -Werror -Wunreachable-code -Warray-bounds -Wnull-dereference -Wundefined-bool-conversion -Wstrict-overflow -Wint-to-pointer-cast -Wpointer-arith -Wtype-limits -Wshadow -Wformat=2 -Wswitch-enum -Wmissing-noreturn -Wreturn-type -Wuninitialized -Wformat-extra-args -Wconversion -Wno-sign-compare -Wno-implicit-int-conversion -Wno-sign-conversion -ferror-limit=100 -Wno-missing-field-initializers -Wno-extern-initializer -Wno-gnu-zero-variadic-macro-arguments -Wno-switch-enum -Wno-switch -Wno-c23-extensions"
link="-lm"
other="-g -Isrc"

clang $warn $other $link "src/tests/main_linux.c" -obuilds/main

echo "Finished"