.. _installing-from-source:

===============================
Building Emscripten from Source
===============================

Building Emscripten yourself is an alternative to getting binaries using the emsdk.

Emscripten's core codebase, which is in the main "emscripten" repo, does not need to be compiled (it uses Python for most of the scripting that glues together all the tools). What do need to be compiled are LLVM (which in particular provides clang and wasm-ld) and Binaryen. After compiling them, simply edit the ``.emscripten`` file to point to the right place for each of those tools (if the file doesn't exist yet, run ``emcc`` for the first time).

Building LLVM
-------------

For using the LLVM wasm backend (recommended), simply build normal upstream LLVM from the monorepo, including clang and wasm-ld (using something like ``-DLLVM_ENABLE_PROJECTS=lld;clang'``), following `that project's instructions <http://llvm.org/docs/CMake.html>`_. The build must not disable the WebAssembly backend (which is included by default).

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

