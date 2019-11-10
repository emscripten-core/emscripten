#!/usr/bin/python2
# Copyright 2011 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

"""Small utility to execute some llvm bitcode.

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
"""

import os
import sys
from subprocess import check_call

__rootpath__ = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.append(__rootpath__)

from tools.shared import LLVM_OPT, LLVM_INTERPRETER  # noqa

check_call([LLVM_OPT, sys.argv[1], '-strip-debug', '-o', sys.argv[1] + '.clean.bc'])

# Execute with empty environment - just like the JS script will have
check_call([LLVM_INTERPRETER, sys.argv[1] + '.clean.bc'] + sys.argv[2:], env={'HOME': '.'})

# check_call([LLVM_COMPILER, '-march=c', sys.argv[1], '-o', sys.argv[1] + '.cbe.c'])
# check_call(['gcc', sys.argv[1]+'.cbe.c', '-lstdc++'])
# check_call(['./a.out'] + sys.argv[2:])
