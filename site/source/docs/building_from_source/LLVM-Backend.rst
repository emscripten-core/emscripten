.. _LLVM-Backend:

=========================
LLVM Backend ("Fastcomp")
=========================

This article introduces *Fastcomp*, Emscripten's LLVM + Clang implementation. It explains how you can obtain the tool, why it replaced the :ref:`original compiler core <original-compiler-core>`, and how you can turn off *Fastcomp* if needed. There is also a :ref:`fastcomp-faq` at the very end for troubleshooting *Fastcomp* problems.


Fastcomp overview
=================

*Fastcomp* is the default compiler core for Emscripten. Implemented as an :term:`LLVM backend`, its role is to convert the LLVM Intermediate Representation (IR) created by :term:`Clang` (from C/C++) into JavaScript.

*Fastcomp* has the following features:

- It is tightly integrated with LLVM (as an LLVM-backend)
- It has a core focus on **asm.js** code generation, which has been shown to give the best results.
- When compared to the previous compiler it is much faster (often 4x faster or more), uses less memory, and produces better code.

Fastcomp is maintained in two repositories:

- https://github.com/emscripten-core/emscripten-fastcomp (LLVM)
- https://github.com/emscripten-core/emscripten-fastcomp-clang (Clang)

Getting Fastcomp
================

*Fastcomp* (Clang) is part of the :ref:`Emscripten SDK <sdk-download-and-install>`, and the binaries are automatically provided during installation (except on Linux, where pre-built binaries are not supplied so the SDK builds them for you).

If you need to build from source you can:

- :ref:`Use the SDK <building-emscripten-from-source-using-the-sdk>` (these instructions show how to build the whole of Emscripten, including *Fastcomp*).
- :ref:`Build using a fully manual process <building-fastcomp-from-source>`.

.. warning:: The backend is still too new to be in the upstream LLVM repository. As such, builds from Linux distributions will **not** contain *Fastcomp*, and Emscripten will report an error if you try to use them.


.. _original-compiler-core:

Original compiler core (deprecated)
===================================

The original compiler supported dozens of different code generation modes (no-typed arrays, typed arrays in various modes, **asm.js** vs. **non-asm.js**, etc.), many of which were not very efficient. Over time, the compiler became harder to maintain and was susceptible to unpredictable compiler slow-downs.

*Fastcomp* was turned on by default in version 1.12.1. The original compiler is now "deprecated".

.. note:: While it is possible to manually disable Fastcomp and build the original compiler from source, this is discouraged.


Why did this change happen?
---------------------------

As a result of the problems with the original compiler, we developed *Fastcomp*, which is a much better compiler:

- It is much more streamlined than the original compiler. It focuses on **asm.js** code generation, which has been shown to give the best results.
- It is much faster and has more predictable performance (often 4x faster or more).
- It requires much less memory.
- It generates better code because, as an LLVM backend, it integrates more tightly with LLVM.


Are there downsides?
--------------------

The main downside is that Emscripten can no longer use a stock build of LLVM, because we have made changes that must be built with LLVM.

There are also a few features that were present in the original compiler that are not present in *Fastcomp* (see the next section).

.. note:: We hope that the new Emscripten backend will eventually become part of the upstream LLVM, and hence become available in stock builds.

Features not present in Fastcomp
--------------------------------

Some features that were present in the original compiler that are not present in *Fastcomp* include:

-  Various deprecated **settings.js** options (``FORCE_ALIGNMENT``, ``HEAP_INIT``, etc.) You should receive a compile-time error if you use a setting which is not supported.
-  Linking of **asm.js** shared modules. This is not deprecated, but may need to be reconsidered.

   .. note:: Normal static linking as used by almost all projects works fine; it is just specifically the options ``MAIN_MODULE`` and ``SIDE_MODULE`` that do not work.


How to disable Fastcomp
-----------------------

Fastcomp is now the only supported compiler and the old compiler has been removed from emscripten.


.. _fastcomp-faq:

FAQ
===

I see ``WARNING: Linking two modules of different target triples`` [..] ``'asmjs-unknown-emscripten' and 'le32-unknown-nacl'``..?
---------------------------------------------------------------------------------------------------------------------------------

You are linking together bitcode files compiled with the old compiler (or older versions of *Fastcomp*) with bitcode files from the new one. This may work in some cases but is dangerous and should be avoided. To fix it, just recompile all your bitcode with the new compiler.

