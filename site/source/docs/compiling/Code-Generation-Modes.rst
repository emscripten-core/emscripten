.. _emscripten-runtime-environment:

===================================================
Emscripten Runtime Environment (under-construction)
===================================================



.. _Code-Generation-Modes:

.. _typed-arrays:

.. _emscripten-memory-model:


Emscripten Memory Model
=======================

Emscripten's memory model is known as :term:`Typed Arrays Mode 2`. It represents memory using a single `typed array <https://developer.mozilla.org/en-US/docs/Web/JavaScript/Typed_arrays>`_, with different *views* providing access to different types (:js:data:`HEAPU32` for 32-bit unsigned integers, etc.)  

.. _typed-arrays-mode-2:

.. note:: *Typed Arrays Mode 2* is the *only* memory model supported by the :ref:`Fastcomp <LLVM-Backend>` compiler, and it is the *default* memory model for the :ref:`old compiler <original-compiler-core>`. 

	Compared to other models tried by the project, it can be used for a broad range of arbitrary compiled code, and is relatively fast.  

The model lays out items in memory in the same way as with normal C and C++, and as a result it uses the same amount of memory. 

We currently align the stack to 4-byte boundaries (this means that reading 8-byte values is slower as they must be read in two parts and then combined).

This model allows you to use code that violates the load-store consistency assumption. Since the different views show the same data, you can (say) write a 32-bit integer, then read a byte from the middle, and it will work just like in C or C++.


.. note:: ``SAFE_HEAP`` ignores load-store consistency violations, since they don't matter. Alignment of reads and writes will be checked, which is important since reading unaligned values can fail.

