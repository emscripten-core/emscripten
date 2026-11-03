LLVM's libunwind
----------------

These files are from the llvm-project based on release 23.1.3.

We maintain a local fork of llvm-project that contains any Emscripten
specific patches:

  https://github.com/emscripten-core/llvm-project

The current patch is based on the emscripten-libs-23 branch.

Update Instructions
-------------------

Run `system/lib/update_libunwind.py path/to/llvm-project`

Modifications
-------------

For a list of changes from upstream see the libunwind files that are part of:

https://github.com/llvm/llvm-project/compare/llvmorg-23.1.3...emscripten-core:emscripten-libs-23
