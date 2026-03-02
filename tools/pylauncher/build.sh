#!/bin/sh

SCRIPT_DIR=$(dirname "${BASH_SOURCE[0]}")
cd ${SCRIPT_DIR}

x86_64-w64-mingw32-gcc -Werror -static-libgcc -static-libstdc++ -s -Os pylauncher.c -o pylauncher.exe -lshlwapi -lshell32
