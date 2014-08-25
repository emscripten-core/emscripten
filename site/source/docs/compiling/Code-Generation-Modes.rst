.. _Code-Generation-Modes:

===================================
Code Generation Modes (wiki-import)
===================================
.. note:: This article was migrated from the wiki (Fri, 25 Jul 2014 04:21) and is now the "master copy" (the version in the wiki will be deleted). It may not be a perfect rendering of the original but we hope to fix that soon!

Code Generation Modes
=====================

One of the reasons for writing Emscripten itself in JavaScript was to be able to easily test different methods of generating code. As a consequence, Emscripten currently supports various code generation modes, each with different features.

.. _typed-arrays:

Typed Arrays
------------

Typed arrays are currently enabled by default (in mode 2, see below), because that has the greatest support for arbitrary code.

There are two modes for typed arrays in Emscripten, 1 and 2. To explain them, first recall how Emscripten manages memory **without** typed arrays: It uses ``HEAP`` to represent memory, where ``HEAP`` is a normal JavaScript array. We write to the first address of  multi-byte values to keep things fast, so writing a 32-bit integer will end up as ``[value, 0, 0, 0]`` - we do not write anything to the upper 3 bytes. We will also read just from the first location. This is much faster than splitting things up into bytes and vice versa, but requires that we have "load-store consistency", which is that if we read a value, we wrote the same type there. (Otherwise, if we write a 32-bit int and read the second byte, we won't get what we expect.) ``SAFE_HEAP`` when enabled will check for violations of load-store consistency.

Disabling typed arrays can be done with ``USE_TYPED_ARRAYS=0``. This is safer in the sense that not all browsers support typed arrays. However, typed arrays can be optimized very well in some cases, and can support code that does not have load-store consistency.

Typed Arrays Mode 1
+++++++++++++++++++

With ``USE_TYPED_ARRAYS=1``, two typed arrays will represent memory. One will contain integer values, the other float values. When we write an integer to memory, we write to the first, and when we write a float, we write to the second. (LLVM is typed, so we always know what values we are writing/reading.) Otherwise, this mode is identical to without typed arrays: We still require load-store consistency and so forth.

The benefit of this mode is that memory is implemented as two typed arrays, which JavaScript engines can often optimize very well. The downsides, though are 

-  In things like memcpy, where we don't know the types, we must copy **both** arrays.
-  We can take more memory than without typed arrays, since we have two arrays now for memory, one of which uses 32 bits for each memory address, and one of which uses 64.
-  We cannot dynamically resize these arrays in an easy way. If you try to use more memory than was originally set up, we need to create new typed arrays and copy everything into them, which is slow. Setting the total size of memory (``TOTAL_MEMORY``) can avoid this, but only if you know in advance the maximum you will need.
-  Not all LLVM optimizations are possible, and not all C/C++ code is compatible with this mode.

This mode is generally much faster than mode 0, but often slower then mode 2.

.. _typed-arrays-mode-2:

Typed Arrays Mode 2
+++++++++++++++++++

This is the default mode.

With ``USE_TYPED_ARRAYS=2``, one typed array buffer will represent memory, while different views into it will allow us to access different types (:js:data:`HEAPU32` for 32-bit unsigned integers, for example). Because of that, it is possible to violate the load-store consistency assumption: The different views show the same data. You can write a 32-bit integer, then read a byte from the middle, and it will work just like in C or C++.

The main benefit of this mode is that it uses as much memory on the heap as C and C++ normally do. We are literally laying things out in memory as they do. This mode is also quite fast in many cases. Also, you can successfully run code that does not have load-store consistency. There are two downsides though,

-  Because of how typed array views work, we must do pointer shifts: To access a 32-bit value at memory location X, we need to read from the 32-bit view at index X divided by 4 (since it looks like an array, whose indexes each represent 4 bytes). So the code ends up filled with ``<<2``, ``>>2`` and so forth. We can optimize this some more, but it will remain an overhead. (However, other binary data APIs are coming to JavaScript which can help here.)
-  We currently align the stack and so forth to 4-byte boundaries. This means that reading 8-byte values is slower (we must read them in two parts, and combine them, see ``I64_MODE`` and ``DOUBLE_MODE``,    below).

Note that ``SAFE_HEAP``, in this mode, will ignore load-store consistency violations, since they don't matter. Alignment of reads and writes, though, will be checked, which is important since reading unaligned values will fail as mentioned earlier.

64-bit Integer Modes
--------------------

As mentioned earlier, 64-bit integers cannot be represented directly in JavaScript, whose numbers are doubles, which cannot contain all 64-bit integer values. In typed arrays mode 2, we implement 64-bit integers as pairs of 32-bit values, but we do not emulate addition and multiplication, instead we perform that as doubles - which is faster. Bit operations like and and or, however, are done with full precision.

Note that in modes other than typed arrays mode 2, we implement 64-bit integers entirely as doubles. If you need 64-bit integers to work, use typed arrays mode 2.

Double Mode
-----------

In every mode but typed arrays mode 2, doubles work fine in a simple manner. However, in typed arrays mode 2 we cannot read unaligned doubles, because of the typed array API. Therefore the default behavior here is ``DOUBLE_MODE=1``, in which we carefully read doubles as two 32-bit values, to not hit alignment problems. This is slower than mode 0, obviously.

It should be possible to modify clang to always align doubles to 64-bit boundaries (gcc has ``-malign-double``), this should be investigated - it would mean we no longer need mode 1 here.

Exceptions
----------

Exceptions can work in code compiled by emscripten, however due to how JS engines work they are typically very slow (all JS engines currently disable most optimizations inside ``try {} catch() {}`` blocks...). For that reason, in ``-O1`` and above emscripten disables exception catching by default, because often LLVM generates code that can catch exceptions even when it isn't needed (in C++, exception catching often has zero cost unless an exception actually happens, so there is little downside there, but in JS there is).

To get full exception handling support in ``-O1`` and above, run emcc with ``-s DISABLE_EXCEPTION_CATCHING=0``. Note that enabling exceptions will make the code larger due to the additional try-catch blocks (which cannot be minified very well).

.. _code-generation-modes-quantum-size:

Memory Compression (a.k.a QUANTUM_SIZE == 1)
---------------------------------------------

By default, Emscripten sets ``QUANTUM_SIZE`` to 4, which means that a 'normal' element - like an int or a pointer - takes 4 'bytes'. That is the normal behavior on 32-bit systems, and allows Emscripten to properly compile a great deal of C/C++ code. However, the downside is that each int or pointer takes 4 places in the HEAP array, which is how Emscripten models the normal C/C++ memory space. This makes HEAP much bigger than it needs to be, which takes more memory, and also slows things down (for example, memset ends up clearing larger areas of memory).

By setting QUANTUM_SIZE to 1, memory is 'compressed' - it is not longer isomorphic to normal C/C++ memory. So for example a structure with an int, a double, and a pointer would have a size of 3 (one for each variable), instead of the normal 16 (4 for the int and pointer, 8 for the double). This leads to significantly more optimized code, both in terms of speed and memory use.

However, this is risky, since with QUANTUM\_SIZE of 1, Emscripten rewrites the .ll, trying to fix all the hard-coded places where it assumes the normal memory space. So consider this experimental for now.

Notes:

-  With QUANTUM_SIZE == 1, Emscripten will warn you about ``ptrtoint`` and ``inttoptr`` operations (in the generated .js file). You should make sure there are no problems in each of those cases, and perhaps modify the original C/C++ to avoid generating that kind of code (the changes are often simple).
-  You can **not** use ``QUANTUM_SIZE == 1`` with typed arrays mode 2. The reason is that typed arrays mode 2 is C-like memory layout, whereas in ``QUANTUM_SIZE == 1`` we radically change the layout (ints take 1 memory address instead of 4, etc.), and the two can't be mixed.

The second version of the Bullet demo in Emscripten uses ``QUANTUM_SIZE == 1``. It appears to give a speedup of around 25%.



.. todo:: **HamishW** In :ref:`CodeGuidelinesAndLimitations` it mentions a code generation mode UNALIGNED_MEMORY - perhaps add?

.. todo:: **HamishW** SAFE_HEAP gets mentioned. Anything we can link to?
