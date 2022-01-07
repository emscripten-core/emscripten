llvm's compiler-rt
------------------

These files are from the llvm-project based on release 13.0.0.

We maintain a local fork of llvm-project that contains any emscripten
specific patches:

  https://github.com/emscripten-core/llvm-project

The current patch is based on:

tag: llvmorg-13.0.0
git: d7b669b3a30345cfcdb2fde2af6f48aa4b94845d

Update Instructions
-------------------

Run `system/lib/update_compiler_rt.py path/to/llvm-root`

Modifications
-------------

For a list of changes from upstream see the compiler-rt files that are part of:

https://github.com/llvm/llvm-project/compare/llvmorg-13.0.0...emscripten-core:emscripten-libs-13.0.0
