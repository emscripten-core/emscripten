:layout: landing

.. title:: Main

.. _home-page:

.. image:: _static/emscripten_logo_full.svg
  :alt: Emscripten logo
  :align: center

.. rst-class:: lead

  Emscripten is a complete compiler toolchain to WebAssembly, using LLVM, with a special focus on speed, size, and the Web platform.

.. container:: buttons

  :doc:`Docs <docs/index>`
  `GitHub <https://github.com/emscripten-core/emscripten>`_

.. grid:: 1 1 2 3
  :gutter: 2

  .. grid-item-card:: Porting

    Compile your existing projects written in C or C++ -- or any language that uses `LLVM <https://llvm.org/>`_ -- to browsers, `Node.js <https://nodejs.org/>`_, or `wasm runtimes <https://v8.dev/blog/emscripten-standalone-wasm#running-in-wasm-runtimes>`_.

  .. grid-item-card:: APIs

    Emscripten converts OpenGL into WebGL, and has support for familiar APIs like SDL, pthreads, and POSIX, as well as Web APIs and JavaScript.

  .. grid-item-card:: Fast

    Thanks to the combination of LLVM, Emscripten, `Binaryen <https://github.com/WebAssembly/binaryen>`_, and `WebAssembly <https://webassembly.org/>`_, the output is compact and runs at near-native speed.

.. admonition:: Interested to learn more?

  Read our :doc:`About page <docs/introducing_emscripten/about_emscripten>`!

.. admonition:: Ready to get started?

  :doc:`Download and install the SDK <docs/getting_started/downloads>` and then proceed to the :doc:`Tutorial <docs/getting_started/Tutorial>`!

.. toctree::
  :hidden:

  docs/introducing_emscripten/index
  docs/getting_started/index
  docs/compiling/index
  docs/porting/index
  docs/api_reference/index
  docs/tools_reference/index
  docs/optimizing/Optimizing-Code
  docs/optimizing/Optimizing-WebGL
  docs/debugging/Sanitizers
  docs/building_from_source/index
  docs/contributing/index
  docs/optimizing/Profiling-Toolchain
  docs/site/about
  genindex
