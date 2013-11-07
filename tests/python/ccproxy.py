#!/usr/bin/env python2

from __future__ import with_statement

import sys
import os
import subprocess

CC='..LLVM_GCC_DIR../llvm-gcc'
CC_ARG_SKIP = ['-g', '-O1', '-O2', '-O3']
CC_ADDITIONAL_ARGS = ['-m32', '-U__i386__', '-U__x86_64__', '-UX87_DOUBLE_ROUNDING', '-UHAVE_GCC_ASM_FOR_X87']

#CC='llvm-gcc'
#CC_ARG_SKIP = ['-g', '-O1', '-O2', '-O3']
#CC_ADDITIONAL_ARGS = ['-U__i386__', '-U__x86_64__']

LINK='..LLVM_DIR../llvm-link'
ALLOWED_LINK_ARGS = ['-f', '-help', '-o', '-print-after', '-print-after-all', '-print-before',
                     '-print-before-all', '-time-passes', '-v', '-verify-dom-info', '-version' ]  
#LINK_ARG_SKIP = ['-pthread', '-DNDEBUG', '-g', '-O3', '-Wall', '-Wstrict-prototypes',
#                 '-lpthread', '-ldl', '-lutil', '-Xlinker', '-export-dynamic', '-lm', '-shared']

# ----------------  End configs -------------

# no-op if called as ranlibproxy.py
if os.path.basename(sys.argv[0])=='ranlibproxy.py':
    sys.exit(0)


use_linker = True
#use_linker = False

opts = []
files = []
for arg in sys.argv[1:]:
    if arg.startswith('-'):
        opts.append(arg)
    else:
        files.append(arg)
        if arg.endswith('.c'):
            use_linker = False
            
if '--version' in opts:
    use_linker = False

if use_linker:
    call = LINK
    newargs = []
    found_o = False
    for arg in sys.argv[1:]:
        if os.path.basename(sys.argv[0])=='arproxy.py':
            if arg.endswith('.a'):
                newargs.append('-o=%s' % arg)
            elif arg.endswith('.o'):
                newargs.append(arg)
            else:
                pass
            continue
        if found_o:
            newargs.append('-o=%s' % arg)
            found_o = False
            continue
        if arg.startswith('-'):
            if arg == '-o':
                found_o = True
                continue
            prefix = arg.split('=')[0]
            if prefix in ALLOWED_LINK_ARGS:
                newargs.append(arg)
        else:
            # not option, so just append
            newargs.append(arg)
else:
    call = CC
    newargs = [ arg for arg in sys.argv[1:] if arg not in CC_ARG_SKIP ] + CC_ADDITIONAL_ARGS
    if 'conftest.c' not in files:
        newargs.append('-emit-llvm')
        if CC=='llvm-gcc':
            newargs.append('-c') 
    
with open('ccproxy.log', 'a') as f:
    f.write('## Called with %s\n' % ' '.join(sys.argv))
    f.write('** Calling %s %s\n\n' % (call, ' '.join(newargs)))
    f.flush()

print "Running:", call, newargs

subprocess.call([call] + newargs)
