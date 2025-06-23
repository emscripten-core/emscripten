llvm's libc
-----------

These files are from llvm-project's HEAD.

We maintain a local fork of llvm-project that contains any emscripten
specific patches:

  https://github.com/emscripten-core/llvm-project

The current patch is based on the emscripten-libs-20 branch.

Currently in producgtion, we do not use LLVM's libc directly yet, but libcxx uses a subset 
of headers from libc. So libcxx directly depends on the following directories:
- libc/hdr
- libc/include/llvm-libc-macros
- libc/include/llvm-libc-types
- libc/shared
- libc/src/__support

In addition, we are experimenting with using llvm-libc in overlay mode. You can enable this
by running with `EMCC_CFLAGS=-lllvmlibc`. See https://libc.llvm.org/overlay_mode.html for more
details.

Update Instructions
-------------------

Run `system/lib/update_libcxx.py path/to/llvm-project`
Run `system/lib/update_llvm_libc.py path/to/llvm-project`

Modifications
-------------

For a list of changes from upstream see the libc files that are part of:

https://github.com/llvm/llvm-project/compare/llvmorg-20.1.4...emscripten-core:emscripten-libs-20
