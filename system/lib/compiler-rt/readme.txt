These files are from llvm-project/compiler-rt, release 10.0.0.

tag: llvmorg-10.0.0
git: d32170dbd5b0d54436537b6b75beaf44324e0c28

Not all files are included. The list of included files from lib/builtins/ is in
filelist.txt.

Update Instructions
-------------------

Run `system/lib/update_compiler_rt.py path/to/llvm-root`

Local Change
------------

lib/builtins/int_lib.h: __asmjs__ added to list of supported platforms.
lib/builtins/powitf2.c: enable for wasm as well as PPC
