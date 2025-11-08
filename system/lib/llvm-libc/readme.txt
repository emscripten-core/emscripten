llvm's libc
-----------

These files are from llvm-project's HEAD.

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
