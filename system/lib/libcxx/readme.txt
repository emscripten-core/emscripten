llvm's libcxx
-------------

These files are from the llvm-project based on release 19.1.6.

We maintain a local fork of llvm-project that contains any emscripten
specific patches:

  https://github.com/emscripten-core/llvm-project

The current patch is based on the emscripten-libs-19 branch.

Update Instructions
-------------------

Run `system/lib/update_libcxx.py path/to/llvm-project`

Modifications
-------------

For a list of changes from upstream see the libcxx files that are part of:

https://github.com/llvm/llvm-project/compare/llvmorg-19.1.6...emscripten-core:emscripten-libs-19
