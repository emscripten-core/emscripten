llvm's libc
-----------

These files are from the llvm-project based on release 21.1.8.

We maintain a local fork of llvm-project that contains any Emscripten
specific patches:

  https://github.com/emscripten-core/llvm-project

The current patch is based on the emscripten-libs-21 branch.

Currently in production, we do not use LLVM's libc directly yet, but libcxx uses
a subset of headers from libc. So libcxx directly depends on the following
directories:
- libc/hdr
- libc/include/llvm-libc-macros
- libc/include/llvm-libc-types
- libc/shared
- libc/src/__support

In addition, we are experimenting with using llvm-libc in overlay mode. You can
enable this by running with `EMCC_CFLAGS=-lllvmlibc`. See
https://libc.llvm.org/overlay_mode.html for more details.

Update Instructions
-------------------

Run `system/lib/update_libcxx.py path/to/llvm-project`
Run `system/lib/update_llvm_libc.py path/to/llvm-project`

Modifications
-------------

For a list of changes from upstream see the libcxx files that are part of:

https://github.com/llvm/llvm-project/compare/llvmorg-21.1.8...emscripten-core:emscripten-libs-21
