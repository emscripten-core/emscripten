.. _about-emscripten:

================
About Emscripten
================

Emscripten is an :ref:`Open Source <emscripten-license>` compiler to WebAssembly. Using Emscripten you can:

- Compile C and C++ code, or any other language that uses LLVM, into WebAssembly.
- Compile the C/C++ **runtimes** of other languages into WebAssembly, and then
  run code in those other languages in an *indirect* way (for example, this has
  been done for
  `Python <https://github.com/iodide-project/pyodide>`_ and
  `Lua <https://daurnimator.github.io/lua.vm.js/lua.vm.js.html>`_).

.. tip:: Emscripten makes native code immediately available on the Web: a platform that is standards-based, has numerous independent compatible implementations, and runs everywhere from PCs to iPads.

  With Emscripten, C/C++ developers don't have the high cost of porting code manually to the Web.
  Web developers also benefit, as they can use the many thousands of pre-existing native utilities and libraries in their sites.

Practically any **portable** C or C++ codebase can be compiled into JavaScript using Emscripten, ranging from high performance games that need to render graphics, play sounds, and load and process files, through to application frameworks like Qt. Emscripten has already been used to convert a `very long list <https://github.com/emscripten-core/emscripten/wiki/Porting-Examples-and-Demos>`_ of real-world codebases to JavaScript, including large projects like *CPython*, `Poppler <https://github.com/coolwanglu/emscripten/tree/master/tests/poppler#readme>`_ and the `Bullet Physics Engine <http://kripken.github.io/ammo.js/examples/new/ammo.html>`_, as well as commercial projects like the `Unreal Engine 4 <https://blog.mozilla.org/blog/2014/03/12/mozilla-and-epic-preview-unreal-engine-4-running-in-firefox/>`_ and the `Unity <http://www.unity3d.com>`_ engine. Here are two demos using Unity:

.. figure:: angrybots.png
  :alt: Angrybots game logo
  :target: http://beta.unity3d.com/jonas/AngryBots/
  :align: left

.. figure:: DEAD-TRIGGER-2-Icon1.png
  :alt: Dead Trigger 2 Game logo
  :target: http://beta.unity3d.com/jonas/DT2/
  :align: left

.. raw:: html

  <div style="clear:both;"></div>

For more demos, see the `list on the wiki <https://github.com/emscripten-core/emscripten/wiki/Porting-Examples-and-Demos>`_.

Emscripten generates fast code! Its default output format is
`WebAssembly <http://webassembly.org/>`_ , a highly optimizable executable
format, that runs almost as fast as native code, while being portable and safe.

.. _about-emscripten-toolchain:

Emscripten Toolchain
====================

A high level view of the Emscripten toolchain is given below. The main tool is the :ref:`emccdoc`. This is a drop-in replacement for a standard compiler like *gcc* or *clang*.

.. image:: EmscriptenToolchain.png

*Emcc* uses :term:`Clang` and LLVM to compile to wasm or asm.js. Emscripten emits JavaScript that can run that compiled code and provide the necessary runtime support. That JavaScript can be executed by :term:`node.js`, or from within HTML in a browser.

The :ref:`emsdk` is used to manage multiple SDKs and tools, and to specify the particular SDK/set of tools currently being used to compile code (the :term:`Active Tool/SDK`). It can even "install" (download and build) the latest toolchain from GitHub!

*Emsdk* writes the "active" configuration to the :ref:`compiler-configuration-file`. This file is used by *emcc* to get the correct current toolchain for building.

A number of other tools are not shown — for example, Java can optionally be used by *emcc* to run the :term:`closure compiler`, which can further decrease code size.

The whole toolchain is delivered in the :ref:`Emscripten SDK <sdk-download-and-install>`, and can be used on Linux, Windows or macOS.

.. _about-emscripten-porting-code:

Porting code to use Emscripten
==============================

Emscripten support for **portable** C/C++ code is fairly comprehensive. Support for the C standard library, C++ standard library, C++ exceptions, etc. is very good. `SDL <https://www.libsdl.org/>`_ support is sufficient to run quite a lot of code. :ref:`OpenGL-support` support is excellent for OpenGL ES 2.0-type code, and acceptable for other types.

There are differences between the native and :ref:`emscripten-runtime-environment`, which mean some changes usually need to be made to the native code. That said, many applications will only need to change the way they define their main loop, and also modify their :ref:`file handling <file-system-overview>` to adapt to the limitations of the browser/JavaScript.

There are also limitations that can make some code easier to port — read :ref:`code-portability-guidelines` to determine where you may need to spend more effort.


