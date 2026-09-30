llvm's OpenMP
-------------

These files are from llvm-project commit
9076414489edb1db74ec6f7bc22a715d1cd66f0c.

We maintain a local fork of llvm-project that contains any Emscripten
specific patches:

  https://github.com/emscripten-core/llvm-project

This snapshot is pinned to an LLVM 24 development commit until the changes are
available in a stable LLVM 24 release.

Update Instructions
-------------------

Run `system/lib/update_openmp.py path/to/llvm-project`

Modifications
-------------

For a list of changes from upstream see the OpenMP files that are part of:

https://github.com/llvm/llvm-project/compare/llvmorg-22.1.8...9076414489edb1db74ec6f7bc22a715d1cd66f0c
