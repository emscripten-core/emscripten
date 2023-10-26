.. _installing-from-source:

===============================
Building Emscripten from Source
===============================

Building Emscripten yourself is an alternative to getting binaries using the
emsdk.

Emscripten itself is written in Python and JavaScript so it does not need to be
compiled.  However, after checkout you will need to perform various steps
before it can be used (e.g. ``npm install``).  The ``bootstrap`` script in the
top level of the repository takes care of running these steps and ``emcc`` will
error out if it detects that ``bootstrap`` needs to be run.

In addition to the main emscripten repository you will also need to checkout
and build LLVM and Binaryen (as detailed below).  After compiling these, you
will need to edit your ``.emscripten`` file to point to their corresponding
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

