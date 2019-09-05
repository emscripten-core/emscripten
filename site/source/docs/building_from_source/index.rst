.. _installing-from-source:

===============================
Building Emscripten from Source
===============================

Building Emscripten yourself is an alternative to getting binaries using the emsdk.

Emscripten's core codebase, which is in the main "emscripten" repo, does not need to be compiled (it uses Python for most of the scripting that glues together all the tools). What do need to be compiled are LLVM (which in particular provides clang and wasm-ld) and Binaryen. After compiling them, simply edit the ``.emscripten`` file to point to the right place for each of those tools (if the file doesn't exist yet, run ``emcc`` for the first time).

Get the ``master`` branches, or check the `Packaging <https://github.com/emscripten-core/emscripten/blob/incoming/docs/process.md#packaging-emscripten>`_ instructions to identify precise commits in existing releases.


Building LLVM
-------------

For using the LLVM wasm backend (recommended), simply build normal upstream LLVM from the `monorepo <https://github.com/llvm/llvm-project>`_.
Include clang and wasm-ld (using something like ``-DLLVM_ENABLE_PROJECTS='lld;clang'``) and the wasm backend (which is included by default; just don't disable it), following `that project's instructions <http://llvm.org/docs/CMake.html>`_.
For example, something like this can work:

  ::

      mkdir build
      cd build/
      cmake ../llvm -DCMAKE_BUILD_TYPE=Release -DLLVM_ENABLE_PROJECTS='lld;clang' -DLLVM_TARGETS_TO_BUILD="host;WebAssembly" -DLLVM_INCLUDE_EXAMPLES=OFF -DLLVM_INCLUDE_TESTS=OFF
      cmake --build .

Then point LLVM_ROOT in ``.emscripten`` to ``<llvm_src>/build/bin`` (no need to install).

Please refer to the upstream docs for more detail.

For using the older fastcomp backend, see :ref:`the fastcomp docs <building-fastcomp-from-source>`.

Building Binaryen
-----------------

See the `Binaryen build instructions <https://github.com/WebAssembly/binaryen#building>`_.

.. toctree::
   :maxdepth: 1

   toolchain_what_is_needed
   building_fastcomp_manually_from_source
   configuring_emscripten_settings
   verify_emscripten_environment

