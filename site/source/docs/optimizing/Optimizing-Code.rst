.. _Optimizing-Code:

=======================================
Optimizing Generated Code (wiki-import)
=======================================

By default Emscripten will compile code in a fairly safe way, and without all the possible optimizations. You should generally try this first, to make sure things work properly (if not, see the :ref:`Debugging` page). Afterwards, you may want to compile your code so it runs faster. This page explains how.

.. note:: You can also optimize the source code you are compiling into JavaScript, see :ref:`Optimizing-the-source-code`.

Using emcc
==========

The recommended way to optimize code is with :ref:`emcc <emccdoc>`. *Emcc* works like *gcc*, and basic usage is presented in the Emscripten :ref:`Tutorial`. As mentioned there, you can use *emcc* to optimize, for example

::

      emcc -O2 file.cpp

To see what the different optimizations levels do, run

::

      emcc --help

-  The meaning of ``-O1, -O2`` etc. in *emcc* are not identical to gcc/clang/other compilers, even though they have been chosen to be as familiar. They can't be, because optimizing JavaScript is very different than optimizing native code. See the ``--help`` as mentioned before for details.
-  If you compile several files into a single JavaScript output, be sure to specify the same optimization flags during all invocations of *emcc* — both when compiling sources into objects, and objects into JavaScript or HTML. See :ref:`Building-Projects` for more details.
-  Aside from ``-Ox`` options, there are ``--llvm-lto`` options that you can read about in ``emcc --help``.

How to optimize code
====================

The following procedure is how you should normally optimize your code. Note that all the flags here are for the link stage (when compiling bitcode to JavaScript), not to optimizing source files.

-  First, run emcc on your code without optimization. The default settings are set to be safe. Check that your code works (if not, see the :ref:`Debugging` page) before continuing.
-  Build with ``-O2`` to get an optimized build.
-  Build with ``-O0`` or ``-O1`` if you want faster iteration times, 0 is the fastest to build but slowest to run, while 1 is in the middle.
-  If you want an even more optimized build than ``-O2``, see the rest of this page.

Advanced compiler settings
==========================

There are several flags you can pass to the compiler to affect code generation, many of which can affect performance. Look in ``src/settings.js`` for which options are available and what they mean (and if you want to look under the hood, see ``apply_opt_level`` in ``tools/shared.py`` for how -O1,2,3 affect them), as well as ``emcc --help``. A few useful ones are:

- ``NO_EXIT_RUNTIME`` - When set, code can run but we never "shut down" the runtime environment, in the sense that no global destructors are run, no atexit calls are executed, etc. This is useful if your ``main()`` function finishes but you still want to execute code, and in fact necessary otherwise if you depend on a global structure it may have gone away - and for that reason, at runtime if we see that you call something like :c:func:`emscripten_set_main_loop` then we will never shut down the runtime. However, if you build with ``-s NO_EXIT_RUNTIME=1`` then we know at **compile** time that you want that, which lets the compiler statically get rid of atexit calls and global destructors, which can improve code size and startup speed.

Very large projects
===================

Very large projects can hit some issues that smaller ones will never see:

Memory initialization
---------------------

By default Emscripten emits the static memory initialization inside the **.js** file, for simplicity. If it is very large, it can slow down startup, or even cause issues in JavaScript engines with limits on array sizes (you may see ``Array initializer too large`` or ``Too much recursion`` for example), and it also leads to very large JavaScript files. To avoid that, Emscripten can emit a binary file on the side by running emcc with

``--memory-init-file 1``

This is the default in ``-O2`` and above in Emscripten 1.21.1 and above; in earlier versions, you can enable it manually. When utilized, a file with suffix ``.mem`` should appear, and it will be loaded by the JavaScript. See ``emcc --help`` for more details.

Optimization levels
-------------------

You might want to build some source files in your project with ``-Os`` or ``-Oz`` to reduce code size, and the rest using ``-O2`` which gives better performance (but increases code size). This allows you to keep files you know are less performance-sensitive at a minimal size, while keeping the files that need to be fast at maximal speed.

(Note that this only matters during the source to bitcode phase: during bitcode to JavaScript, ``-Os`` and ``-Oz`` are the same as ``-O2`` as there are currently no JavaScript specific optimization flags for ``-Os`` or ``-Oz``.)

Code size
---------

Tips for reducing code size include:

-  Memory init file as mentioned above.
-  Using -Os or -Oz, as also mentioned above.
-  Build bitcode to JavaScript with -O3 which runs the expensive variable reuse pass (registerizeHarder)
-  Use llvm LTO during bitcode to JavaScript ``-s INLINING_LIMIT=1 --llvm-lto 1`` (can break some code as the LTO code path is less tested)
-  That command also disables inlining. If sources were built with -Os or -Oz, it will avoid inlining anyhow for the most part, and you can try just ``--llvm-lto 1``
-  Use closure on the outside non-asm.js code ``--closure 1`` (can break some code)

Outlining
---------

``OUTLINING_LIMIT`` breaks up large functions into smaller ones, by "outlining" code. This helps startup speed as well as runtime speed in some cases, particularly when a codebase has huge functions, which confuse JavaScript engines. For more details see `this blog post <http://mozakai.blogspot.com/2013/08/outlining-workaround-for-jits-and-big.html>`_.

Aggressive Variable Elimination
-------------------------------

You can enable aggressive variable elimination with ``-s AGGRESSIVE_VARIABLE_ELIMINATION=1``. This will then attempt to remove variables whenever possible, even at the cost of increasing code size by duplicating expressions. This can improve speed in some cases where you have extremely large functions, for example it can make sqlite 7% faster (which has a huge interpreter loop with thousands of lines in it). However it can also he harmful in some cases, so test before using it.

Other optimization issues
=========================

Exception Catching
------------------

In ``-O1`` and above exception catching is disabled. This prevents the generation of try-catch blocks, which lets the code run much faster, and also makes the code smaller. To re-enable them, run emcc with ``-s DISABLE_EXCEPTION_CATCHING=0``.

Viewing code optimization passes
--------------------------------

If you run emcc with ``EMCC_DEBUG=1`` (so, something like ``EMCC_DEBUG=1 emcc``), then it will output all the intermediate steps after each optimization pass. The output will be in ``TEMP_DIR/emscripten_temp``, where ``TEMP_DIR`` is by default ``/tmp`` (and can be modified in ``~/.emscripten``). ``EMCC_DEBUG=2`` will output even more information, a separate file will be saved for each JavaScript optimization pass.

Inlining
--------

Inlining often generates large functions. These allow the compiler's optimizations to be more effective, but have downsides for JavaScript engines: They often do not try to optimize big functions for fear or long JIT times, or they do JIT them and it causes noticeable pauses. So ironically (or paradoxically) using -O1 or -O2, which inline by default, can actually decrease performance in some cases.

You can try to avoid this issue by disabling inlining (in specific files or everywhere), or by using the outliner feature, see `this blog post <http://mozakai.blogspot.com/2013/08/outlining-workaround-for-jits-and-big.html>`_.

Unsafe optimizations
--------------------

A few **UNSAFE** optimizations you might want to try are:

- ``-s FORCE_ALIGNED_MEMORY=1``: Makes all memory accesses fully aligned. This can break on code that actually requires unaligned accesses.
- ``-s PRECISE_I64_MATH=1``: When disabled, does shortcuts when implementing 64-bit addition etc., using doubles instead of full emulation. This will break on code that uses the full range of 64-bit numbers.
- ``--llvm-lto 1``: This enables LLVM's link-time opts, which can help in some cases but there are known issues with them as well, so use at your own risk. (There are btw a few modes aside from ``1``, see ``emcc --help``.)
- ``--closure 1``: This can help with reducing the size of the non-generated (support/glue) code, and with startup. However it can break if you do not do proper closure compiler annotations and exports.

Profiling
=========

Modern browsers have JavaScript profilers, which can help find the slower parts in your code. You should build your project with ``-profiling`` for this, that flag will leave the code in a readable-enough state for profiling purposes (``-profiling`` should be added in addition to your other optimization flags like ``-O1``, ``-O2`` or ``-O3``).

As each browser's profiler has limitations, it is highly recommended to profile in multiple browsers in order to get the best information. Also, in Firefox it is a good idea to profile both with and without asm.js optimizations enabled (can remove the ``'use asm'`` string to disable).

Troubleshooting Slowness
========================

If you get worse performance than you expect - you should get about 1/2 the speed of a native build - then aside from the tips above, here is a list of things to check:

-  Did you build with -O2 or -O3, **both** when compiling source code files **and** when generating JavaScript? The first is needed for LLVM optimizations, the latter for JavaScript optimizations, all of which are crucial (see :ref:`Building-Projects`).
-  Is performance ok on one browser, but not in another? Testing on multiple browsers is always good to understand where a bug or performance issue lies. Please file a bug on the browser where things are slow.
-  In firefox, does the code validate? Look for "Successfully compiled asm.js code in the web console. If instead you see a validation error, make sure you are running an up-to-date version of Firefox, and are building using an up-to-date version of Emscripten. If the problem exists with those, please file a bug on Emscripten.

