.. _installing-from-source:

===============================
Building Emscripten from Source
===============================

Building Emscripten yourself is an alternative to getting binaries using the
emsdk.

Emscripten itself is primarily written in Python and JavaScript. However,
after checkout you will need to run the top-level ``bootstrap.py`` script
before the toolchain is usable. This performs various steps including ``npm
install`` and building the native compiler frontend launcher (``emcc_native``),
which provides high-performance ``emcc`` and ``em++`` binaries.

Building ``emcc_native`` requires CMake 3.20+ and a C++20 host compiler
toolchain (which you already need for building LLVM and Binaryen). If you prefer
not to perform a native build, setting ``EMCC_NATIVE=0`` in your environment
before running ``./bootstrap.py`` instructs it to generate legacy Python launcher
scripts (e.g., ``.bat`` / ``.ps1`` files on Windows) via
``./tools/maint/create_entry_points.py``.

Emscripten comes with its own versions of some C/C++ system libraries which
``emcc`` builds automatically as and when needed (in the emsdk builds, these are
precompiled). You can also build them manually with the ``embuilder`` tool - see
``embuilder --help`` for more information.

In addition to the main emscripten repository you will also need to checkout and
build LLVM and Binaryen (as detailed below).  After compiling these, you will
need to edit your ``.emscripten`` file to point to their corresponding
locations.

Use the ``main`` branches of each of these repositories, or check the `Packaging
<https://github.com/emscripten-core/emscripten/blob/main/docs/packaging.md>`_
instructions to identify precise commits used in a specific release.


Building LLVM
-------------

Build LLVM from the `git repo <https://github.com/llvm/llvm-project>`_.
Include clang and wasm-ld (using something like ``-DLLVM_ENABLE_PROJECTS='lld;clang'``) and the Wasm backend (which is included by default; just don't disable it), following `that project's instructions <http://llvm.org/docs/CMake.html>`_.
For example, something like this can work:

  ::

      mkdir build
      cd build/
      cmake ../llvm -DCMAKE_BUILD_TYPE=Release -DLLVM_ENABLE_PROJECTS='lld;clang' -DLLVM_TARGETS_TO_BUILD="host;WebAssembly" -DLLVM_INCLUDE_EXAMPLES=OFF -DLLVM_INCLUDE_TESTS=OFF  # -DLLVM_ENABLE_ASSERTIONS=ON
      cmake --build .

Then set the environment variable ``EM_LLVM_ROOT`` to ``<llvm_src>/build/bin`` (no need to install).

If you need to match the emsdk releases of LLVM, `review the emscripten-release
build and test scripts <https://chromium.googlesource.com/emscripten-releases/+/refs/heads/main#build-and-test-scripts-in>`_.
Specifically `src/build.py <https://chromium.googlesource.com/emscripten-releases/+/refs/heads/main/src/build.py>`_.

Please refer to the upstream docs for more detail.

Building Binaryen
-----------------

See the `Binaryen build instructions <https://github.com/WebAssembly/binaryen#building>`_.

.. toctree::
   :maxdepth: 1

   toolchain_what_is_needed
   configuring_emscripten_settings
   verify_emscripten_environment

