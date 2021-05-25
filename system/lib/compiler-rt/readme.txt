llvm's compiler-rt
------------------

These files are from the llvm-project based on release 10.0.0.

We maintain a local fork of llvm-project that contains any emscripten
specific patches:

  https://github.com/emscripten-core/llvm-project

The current patch is based on:

tag: llvmorg-10.0.0
git: d32170dbd5b0d54436537b6b75beaf44324e0c28

Update Instructions
-------------------

Run `system/lib/update_compiler_rt.py path/to/llvm-root`

Modifications
-------------

For a list of changes from upstream see the compiler-rt files that are part of:

https://github.com/llvm/llvm-project/compare/llvmorg-10.0.0...emscripten-core:emscripten-libs-10.0.0
