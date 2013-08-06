#!/usr/bin/python2

'''
Small utility to execute some llvm bitcode.

The use case is a Makefile that builds some executable
and runs it as part of the build process. With emmaken,
the Makefile will generate llvm bitcode, so we can't
just execute it directly. This script will get that
code into a runnable form, and run it.

We cannot just use lli, since code built with debug
symbols will crash it due to

  http://llvm.org/bugs/show_bug.cgi?id=6981

So we must get around that.

To use this, change the Makefile so that instead of
running

  /bin/sh THE_FILE PARAMS

it runs

  python $(EMSCRIPTEN_TOOLS)/exec_llvm.py THE_FILE PARAMS

An alternative solution to this problem is to compile
the .ll into native code, see nativize_llvm.py. That is
useful when this fails.
'''

import os, sys
from subprocess import Popen, PIPE, STDOUT

__rootpath__ = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
def path_from_root(*pathelems):
  return os.path.join(__rootpath__, *pathelems)
sys.path += [path_from_root('')]
from tools.shared import *

Popen([LLVM_OPT, sys.argv[1], '-strip-debug', '-o', sys.argv[1]+'.clean.bc']).communicate()[0]

# Execute with empty environment - just like the JS script will have
Popen([LLVM_INTERPRETER, sys.argv[1]+'.clean.bc'] + sys.argv[2:], env={'HOME': '.'}).communicate()[0]

#Popen([LLVM_COMPILER, '-march=c', sys.argv[1], '-o', sys.argv[1]+'.cbe.c']).communicate()[0]
#Popen(['gcc', sys.argv[1]+'.cbe.c', '-lstdc++']).communicate()[0]
#Popen(['./a.out'] + sys.argv[2:]).communicate()[0]

