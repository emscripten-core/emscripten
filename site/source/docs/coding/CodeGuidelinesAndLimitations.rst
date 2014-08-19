.. _CodeGuidelinesAndLimitations:

=============================================
Code Guidelines And Limitations (wiki-import)
=============================================
.. note:: This article was migrated from the wiki (Fri, 25 Jul 2014 04:21) and is now the "master copy" (the version in the wiki will be deleted). It may not be a perfect rendering of the original but we hope to fix that soon!

Emscripten can work with pretty much any C/C++ code. However, it might be useful to know what types of code Emscripten compiles better than others, if you are rewriting part of the code you will be compiling.

Code that CANNOT be compiled
=============================

These issues are fundamental difficulties. We can get around them by emulation in theory, but it would be very slow.

-  Code that is multithreaded and uses shared state. JS has threads - web workers - but they cannot share state, instead they pass messages.
-  Nonportable code that relies on endianness is problematic. It can work with USE_TYPED_ARRAYS=2, but it will not be portable!
-  Nonportable code that relies on x86 alignment behavior. X86 allows unaligned reads and writes (so you can read a 16-bit value from a non-even address, i.e., which is only 8-bit aligned address, for example), but other archs do not: ARM will raise SIGILL, and when Emscripten generates JS you will get undefined behavior in this case. (If you build your code with ``SAFE_HEAP=1`` then you will get a clear runtime exception, see :ref:`Debugging`. Note: There is also a code generation mode, UNALIGNED\_MEMORY, which can support unaligned code like this - but it is very slow.)
-  Nonportable code that uses low-level features of the native environment, like native stack manipulation (e.g. in conjunction with setjmp/longjmp. We support normal setjmp/longjmp, but not with stack replacing etc.)
-  Nonportable code that scans registers or the stack, for example to do conservative garbage collection. A variable in a register or on the stack may be held in a JavaScript local variable, which cannot be scanned. (However, you can do conservative scanning when there is no other code on the stack, e.g. from an iteration of the main event loop.)

Function pointer issues
---------------------------------------------------------

There are two general issues with function pointers:

1. clang generates different code for C and C++ calls when a structure is passed **by value**. The two formats are incompatible in how we generate code in Emscripten, but each is compatible with itself. You should see a warning during compilation about this, and the workaround is to either not mix C and C++ in that location (just renaming a ``.c`` file to ``.cpp`` for example) or to pass the structure by reference. (More specifically, one convention is ``struct byval`` and the other is ``field a, field b``.)
2. In **asm.js** mode, there is an additional concern, with function tables. Without **asm.js**, all function pointers use a single table, but in **asm.js** each function pointer type has its own table (this allows the JS engine to know the exact type of each function pointer call, and optimize those calls much better than normally). As a consequence though, if you have a function that is say ``int (int)`` (return int, receive int) and you cast it to ``void (int)`` (no return, receive int), then the function pointer call will fail because we are looking in the wrong table. This is undefined behavior in C in any case, so it is recommended to refactor code to avoid this type of situation. You should see compilation warnings about these things. See :ref:`Asm-pointer-casts` for more information.
3. A related issue to do with function pointers is that in ``-O2`` and above we optimize the size of the separate function tables. That means that two functions can have the same function pointer so long as their type is different, and so potentially comparing function pointers of different types can give false positives. Also, it makes bugs with incorrect function pointers potentially more misleading, since there are fewer "holes" in function tables (holes would throw an error instead of running the wrong code). To check if this is causing issues, you can compile with ``-s ALIASING_FUNCTION_POINTERS=0``.

Code that DOES compile, but might be slower than expected
==========================================================

None of the issues here is a showstopper, but they might be slower than you expect. You probably don't need to worry about this stuff, but it might be useful to know about it.

-  64-bit ints. Bitwise operations on these are reasonable, but math (+, -, \*, /) is emulated, very slowly. JavaScript does not have native 64-bit ints so this is unavoidable.
-  32-bit multiplication that needs all 64 bits of precision must be emulated like 64-bit integer math (JS doubles work fine up to ~52 bits). By default precise 32-bit multiplication is off as it makes a lot of common 32-bit code very slow, but if you need it you can enable it with ``-s PRECISE_I32_MUL=1``, see **src/settings.js** for more.
-  32-bit floats will end up as 64-bit doubles in JS engines. That means they might be a little slower than expected, depending on your CPU.
-  memcpy/memset works, but JS engines don't optimize it very well. Native code will use SIMD on this, JS engines will not - yet.
-  Exceptions and longjmp. In JS such code generally makes the JS engine turn off various optimizations. For that reason exceptions are turned off by default in ``-O1`` and above (to re-enable them, run emcc with ``-s DISABLE_EXCEPTION_CATCHING=0``). setjmp also prevents relooping around it.

Code that MIGHT be useful to avoid
==========================================================

Avoiding these kinds of code might be nice to do, to prevent warnings and to enable additional optimizations. But unless it is very easy for you to avoid them, just ignore this section.

-  Unions. These will trigger ``SAFE_HEAP`` warnings (like *Valgrind*), and make the ``QUANTUM_SIZE=1`` optimization impossible.
-  Bitfields. Also ``SAFE_HEAP`` warnings.
-  Reusing memory, that is, using the same allocated block of memory once for one kind of object, later for another. This prevents the ``QUANTUM_SIZE=1`` optimization.

Libraries
=========

Networking
----------

-  Emscripten supports libc networking functions, but since JS networking is asynchronous, you must limit yourself to asynchronous (nonblocking) operations.

