.. _GC:

================
GC (wiki-import)
================
.. note:: This article was migrated from the wiki (Fri, 25 Jul 2014 04:21) and is now the "master copy" (the version in the wiki will be deleted). It may not be a perfect rendering of the original but we hope to fix that soon!

Emscripten has support for a Boehm-like API for garbage collection in
C/C++ programs. This is useful if your application uses the Boehm API or
if you are compiling a language like Java into C/C++ with the goal of
then translating it to JavaScript.

API Changes From Boehm
----------------------

There is no way to run another thread that interrupts the program at
intervals in order to GC. So you need to call ``GC_MAYBE_COLLECT`` at
reasonable intervals, which will do a collection if enough allocations
happened since the last one. If you compile to HTML, a timer will be
spun for this automatically and you don't need to do anything.

You can also call ``GC_FORCE_COLLECT`` which forces a collection. This
is useful if there are times in your app when responsiveness is not
important (for example, right before starting a new level in a game).

Mechanism
---------

Static allocations are scanned, but the C and JS stacks are not. We can
add such scanning at the cost of runtime performance, however, you don't
need stack scanning if you compile for browsers and call
``GC_MAYBE_COLLECT`` from a ``setInterval``, since the interval will run
when there is no relevant C or JS stack to scan (as mentioned before,
such a timer will be run for you automatically if you are compiling to
HTML).

Pitfalls
--------

The main risk with the current implementation is that we do not force a
collection directly from allocations. So if you allocate a lot in
between collections, you can run out of memory. You can call
``GC_FORCE_COLLECT`` to prevent this in some cases if you know relevant
parts in your code.

Relevant Files
--------------

-  ``system/include/gc.h``
-  ``src/library_gc.js``

