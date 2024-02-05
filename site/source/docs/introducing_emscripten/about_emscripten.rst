.. _about-emscripten:

================
About Emscripten
================

Emscripten is a complete :ref:`Open Source <emscripten-license>` compiler
toolchain to WebAssembly. Using Emscripten you can:

- Compile C and C++ code, or any other language that uses LLVM, into WebAssembly,
  and run it on the Web, Node.js, or other Wasm runtimes.
- Compile the C/C++ **runtimes** of other languages into WebAssembly, and then
  run code in those other languages in an *indirect* way (for example, this has
  been done for
  `Python <https://github.com/iodide-project/pyodide>`_ and
  `Lua <https://daurnimator.github.io/lua.vm.js/lua.vm.js.html>`_).

Practically any **portable** C or C++ codebase can be compiled into WebAssembly
using Emscripten, ranging from high-performance games that need to render
graphics, play sounds, and load and process files, through to application
frameworks like Qt. Emscripten has already been used to convert a very long list
of real-world codebases to WebAssembly, including commercial codebases like the
`Unreal Engine 4 <https://blog.mozilla.org/blog/2014/03/12/mozilla-and-epic-preview-unreal-engine-4-running-in-firefox/>`_
and the `Unity <https://blogs.unity3d.com/2018/08/15/webassembly-is-here/>`_ engine.
For examples and demos, see the
`community-maintained list on the wiki <https://github.com/emscripten-core/emscripten/wiki/Porting-Examples-and-Demos>`_.

Emscripten generates small and fast code! Its default output format is
`WebAssembly <http://webassembly.org/>`_ , a highly optimizable executable
format, that runs almost as fast as native code, while being portable and safe.
Emscripten does a lot of optimization work for you automatically, by careful
integration with LLVM,
`Binaryen <https://github.com/WebAssembly/binaryen>`_,
`Closure Compiler <https://developers.google.com/closure/compiler>`_, and other
tools.

.. _about-emscripten-toolchain:

Emscripten Toolchain
====================

A high level view of the Emscripten toolchain is given below. The main tool is
the :ref:`emccdoc`. This is a drop-in replacement for a standard compiler like *gcc* or *clang*.

*Emcc* uses :term:`Clang` and LLVM to compile to WebAssembly. Emcc also
emits JavaScript that provides API support to the compiled code. That JavaScript
can be executed by :term:`Node.js`, or from within HTML in a browser.

The :ref:`Emscripten SDK <sdk-download-and-install>` is used to install the entire toolchain, including emcc and
LLVM and so forth. The :ref:`emsdk` can be used on Linux, Windows or MacOS.

.. _about-emscripten-porting-code:

Porting code to use Emscripten
==============================

Emscripten support for **portable** C/C++ code is fairly comprehensive.
Support for the C standard library, C++ standard library, C++ exceptions, etc.
is very good, as is `SDL2 <https://www.libsdl.org/>`_ and other APIs.
:ref:`OpenGL-support`
support is excellent for OpenGL ES 2.0-type code, and acceptable for other types.

There are differences between the native and :ref:`emscripten-runtime-environment`,
which mean some changes usually need to be made to the native code. That said,
many applications will only need to change the way they define their main loop,
and also modify their :ref:`file handling <file-system-overview>` to adapt to
the limitations of the browser/JavaScript.

There are also limitations that can make some code easier to port — read
:ref:`code-portability-guidelines` to determine where you may need to spend more
effort.
