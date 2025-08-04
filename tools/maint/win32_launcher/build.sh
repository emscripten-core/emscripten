#!/bin/sh

SCRIPT_DIR=$(dirname "${BASH_SOURCE[0]}")
cd ${SCRIPT_DIR}

x86_64-w64-mingw32-gcc -municode -static-libgcc -static-libstdc++ -s -O2 launcher.c -o launcher.exe -lshlwapi -lshell32
