llvm's libc
-----------

These files are from llvm-project based on release 20.1.4.

We maintain a local fork of llvm-project that contains any emscripten
specific patches:

  https://github.com/emscripten-core/llvm-project

The current patch is based on the emscripten-libs-20 branch.

We do not use LLVM's libc directly, but libcxx uses a subset of headers from
libc. So we only import these directories that libcxx depend on:
- libc/hdr
- libc/include/llvm-libc-macros
- libc/include/llvm-libc-types
- libc/shared
- libc/src/__support

Update Instructions
-------------------

Run `system/lib/update_libcxx.py path/to/llvm-project`

Modifications
-------------

For a list of changes from upstream see the libc files that are part of:

https://github.com/llvm/llvm-project/compare/llvmorg-20.1.4...emscripten-core:emscripten-libs-20
