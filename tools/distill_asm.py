'''
Gets the core asm module out of an emscripten output file.

By default it adds a ';' to end the

  var asm = ...

statement. You can add a third param to customize that. If the third param is 'swap-in', it will emit code to swap this asm module in, instead of the default one.

XXX this probably doesn't work with closure compiler advanced yet XXX
'''

import os, sys
import asm_module

infile = sys.argv[1]
outfile = sys.argv[2]
extra = sys.argv[3] if len(sys.argv) >= 4 else ';'

if extra == 'swap-in':
  # we do |var asm = | just like the original codebase, so that gets overridden anyhow (assuming global scripts).
  extra = r''' (Module.asmGlobalArg, Module.asmLibraryArg, Module['buffer']);
 // special fixups
 asm.stackRestore(Module['asm'].stackSave()); // if this fails, make sure the original was built to be swappable (-s SWAPPABLE_ASM_MODULE=1)
 // Finish swap
 Module['asm'] = asm;
 if (Module['onAsmSwap']) Module['onAsmSwap']();
'''

asm = asm_module.AsmModule(infile)
open(outfile, 'w').write(asm.asm_js + extra)

