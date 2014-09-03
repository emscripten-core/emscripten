.. _Optimizing-the-source-code:

========================================
Optimizing Source Code (wiki-import)
========================================
.. note:: This article was migrated from the wiki (Fri, 25 Jul 2014 04:21) and is now the "master copy" (the version in the wiki will be deleted). It may not be a perfect rendering of the original but we hope to fix that soon!

In general, normal C and C++ code will compile and run at good speeds.
However, there are some fundamental differences between JavaScript and
native code, that make certain types of source code much faster in one
or the other. Typical C and C++ code has been optimized for native code
obviously, so some tweaking might help for compiling to JavaScript.

-  ``#define ABS(x) (x > 0 ? x : -x)`` and similar things are common
   macros in C and C++ code. The assumption is that using ``math.h``'s
   abs/fabs etc. will not be faster. However, when compiling to
   JavaScript, the condition in this macro will be much slower: Doing
   ``define ABS(x) abs(x)``, that is, using ``math.h``, would be faster,
   since then it will be compiled by Emscripten into a single operation,
   a call to ``Math.abs``. ``Math.abs`` has been optimized by JS
   engines, and will run faster than any JavaScript code that emulates
   the same behavior, whereas in native code, the conditional macro
   might be just as fast, or faster, and simpler in that ``math.h`` is
   not needed - so what makes sense in native code makes less sense for
   JavaScript. In this case, the solution is simple, just switch the
   macro to use ``abs()``.

