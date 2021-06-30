llvm's libcxx
-------------

These files are from the llvm-project based on release 12.0.0.

We maintain a local fork of llvm-project that contains any emscripten
specific patches:

  https://github.com/emscripten-core/llvm-project

The current patch is based on:

tag: llvmorg-12.0.0
git: d28af7c654d8db0b68c175db5ce212d74fb5e9bc

Update Instructions
-------------------

Run `system/lib/update_libcxx.py path/to/llvm-project`

Modifications
-------------

For a list of changes from upstream see the libcxx files that are part of:

https://github.com/llvm/llvm-project/compare/llvmorg-12.0.0...emscripten-core:emscripten-libs-12.0.0
