.. _Optimizing-Code:

===============
Optimizing Code
===============

Generally you should first compile and run your code without optimizations (the default). Once you are sure that the code runs correctly, you can use the techniques in this article to make it load and run faster.

How to optimize code
====================

Code is optimized by specifying :ref:`optimization flags <emcc-compiler-optimization-options>` when running :ref:`emcc <emccdoc>`. The levels include: :ref:`-O0 <emcc-O0>` (no optimization), :ref:`-O1 <emcc-O1>`, :ref:`-O2 <emcc-O2>`, :ref:`-Os <emcc-Os>`, :ref:`-Oz <emcc-Oz>`, and :ref:`-O3 <emcc-O3>`.

For example, to compile with optimization level ``-O2``:

.. code-block:: bash

  emcc -O2 file.cpp

The higher optimization levels introduce progressively more aggressive optimization, resulting in improved performance and code size at the cost of increased compilation time. The levels can also highlight different issues related to undefined behavior in code.

The optimization level you should use depends mostly on the current stage of development:

- When first porting code, run *emcc* on your code using the default settings (without optimization). Check that your code works and :ref:`debug <Debugging>` and fix any issues before continuing.
- Build with lower optimization levels during development for a shorter compile/test iteration cycle (``-O0`` or ``-O1``).
- Build with ``-O2`` to get a well-optimized build.
- Building with ``-O3`` or ``-Os`` can produce an ever better build than ``-O2``, and are worth considering for release builds. ``-O3`` builds are even more optimized than ``-O2``, but at the cost of significantly longer compilation time and potentially larger code size. ``-Os`` is similar in increasing compile times, but focuses on reducing code size while doing additional optimization. It's worth trying these different optimization options to see what works best for your application.
- Other optimizations are discussed in the following sections.

.. note::

  -  The meanings of the *emcc* optimization flags (``-O1, -O2`` etc.) are similar to *gcc*, *clang*, and other compilers, but also different because optimizing asm.js and WebAssembly includes some additional types of optimizations. The mapping of the *emcc* levels to the LLVM bitcode optimization levels is documented in the reference.

How Emscripten optimizes
========================

Compiling source files to object files works as you'd expect in a native build system that uses clang and LLVM. When linking object files to the final executable, Emscripten does additional optimizations as well depending on the optimization level:

- For wasm, the Binaryen optimizer is run. Binaryen does both general-purpose optimizations to the wasm that LLVM does not, and also does some whole-program optimization. (Note that Binaryen's whole-program optimizations may do things like inlining, which can be surprising in some cases as LLVM IR attributes like ``noinline`` have been lost at this point.)
- For asm.js, the Emscripten asm.js optimizer is run.
- JavaScript is generated at this phase, and is optimized by Emscripten's JS optimizer. Optionally you can also run :ref:`the closure compiler <emcc-closure>`, which is highly recommended for code size.
- Emscripten also optimizes the combined wasm+JS, by minifying imports and exports between them, and by running meta-dce which removes unused code in cycles that span the two worlds.

To skip extra optimization work at link time, link with ``-O0`` (or no optimization level), which works regardless of how the source files were compiled and optimized. Linking in this way with ``-O0`` is useful for fast iteration builds, while final release builds may want something like ``-O3 --closure 1``.


Advanced compiler settings
==========================

There are several flags you can :ref:`pass to the compiler <emcc-s-option-value>` to affect code generation, which will also affect performance — for example :ref:`DISABLE_EXCEPTION_CATCHING <optimizing-code-exception-catching>`. These are documented in `src/settings.js <https://github.com/emscripten-core/emscripten/blob/master/src/settings.js>`_. Some of these will be directly affected by the optimization settings (you can find out which ones by searching for ``apply_opt_level`` in `tools/shared.py <https://github.com/emscripten-core/emscripten/blob/1.29.12/tools/shared.py#L958>`_).

WebAssembly
===========

Emscripten will emit WebAssembly by default. You can switch that off with ``-s WASM=0`` (and then emscripten emits asm.js), which is necessary if you want the output to run in places where wasm support is not present yet, but the downside is larger and slower code.

.. _optimizing-code-size:

Code size
=========

This section describes optimisations and issues that are relevant to code size. They are useful both for small projects or libraries where you want the smallest footprint you can achieve, and in large projects where the sheer size may cause issues (like slow startup speed) that you want to avoid.

.. _optimizing-code-oz-os:

Trading off code size and performance
-------------------------------------

You may wish to build the less performance-sensitive source files in your project using :ref:`-Os <emcc-Os>` or :ref:`-Oz <emcc-Oz>` and the remainder using :ref:`-O2 <emcc-O2>` (:ref:`-Os <emcc-Os>` and :ref:`-Oz <emcc-Oz>` are similar to :ref:`-O2 <emcc-O2>`, but reduce code size at the expense of performance. :ref:`-Oz <emcc-Oz>` reduces code size more than :ref:`-Os <emcc-Os>`.)

Separately, you can do the final link/build command with ``-Os`` or ``-Oz`` to make the compiler focus more on code size when generating WebAssembly/asm.js.

Miscellaneous code size tips
----------------------------

In addition to the above, the following tips can help to reduce code size:

- Use :ref:`the closure compiler <emcc-closure>` on the non-compiled code: ``--closure 1``. This can hugely reduce the size of the support JavaScript code, and is highly recommended. However, if you add your own additional JavaScript code (in a ``--pre-js``, for example) then you need to make sure it uses `closure annotations properly <https://developers.google.com/closure/compiler/docs/api-tutorial3>`_.
- `Floh's blogpost on this topic <http://floooh.github.io/2016/08/27/asmjs-diet.html>`_ is very helpful.
- Make sure to use gzip compression on your webserver, which all browsers now support.

The following compiler settings can help (see ``src/settings.js`` for more details):

- Disable inlining when possible, using ``-s INLINING_LIMIT=1``. Compiling with -Os or -Oz generally avoids inlining too. (Inlining can make code faster, though, so use this carefully.)
- You can use the ``-s FILESYSTEM=0`` option to disable bundling of filesystem support code (the compiler should optimize it out if not used, but may not always succeed). This can be useful if you are building a pure computational library, for example.
- The ``ENVIRONMENT`` flag lets you specify that the output will only run on the web, or only run in node.js, etc. This prevents the compiler from emitting code to support all possible runtime environments, saving ~2KB.

LTO
===

Link Time Optimization (LTO) lets the compiler do more optimizations, as it can
inline across separate compilation units, and even with system libraries.
LTO is enabled by compiling objects files with ``-flto``.  The effect of this
flag is to emit LTO object files (technically this means emitting bitcode).  The
linker can handle a mix wasm object files and LTO object files.  Passing
``-flto`` at link time will also trigger LTO system libraries to be used.

Thus, to allow maximal LTO opportunities with the LLVM wasm backend, build all
source files with ``-flto`` and also link with ``flto``.

Very large codebases
====================

The previous section on reducing code size can be helpful on very large codebases. In addition, here are some other topics that might be useful.

Running by itself
-----------------

If you hit memory limits in browsers, it can help to run your project by itself, as opposed to inside a web page containing other content. If you open a new web page (as a new tab, or a new window) that contains just your project, then you have the best chance at avoiding memory fragmentation issues.


Other optimization issues
=========================

.. _optimizing-code-exception-catching:

C++ exceptions
--------------

Catching C++ exceptions (specifically, emitting catch blocks) is turned off by default in ``-O1`` (and above). Due to how asm.js/wasm currently implement exceptions, this makes the code much smaller and faster (eventually, wasm should gain native support for exceptions, and not have this issue).

To re-enable exceptions in optimized code, run *emcc* with ``-s DISABLE_EXCEPTION_CATCHING=0`` (see `src/settings.js <https://github.com/emscripten-core/emscripten/blob/master/src/settings.js>`_).

.. note:: When exception catching is disabled, a thrown exception terminates the application. In other words, an exception is still thrown, but it isn't caught.

.. note:: Even with catch blocks not being emitted, there is some code size overhead unless you build your source files with ``-fno-exceptions``, which will omit all exceptions support code (for example, it will avoid creating proper C++ exception objects in errors in std::vector, and just abort the application if they occur)

C++ RTTI
--------

C++ run-time type info support (dynamic casts, etc.) adds overhead that is sometimes not needed. For example, in Box2D neither rtti nor exceptions are needed, and if you build the source files with ``-fno-rtti -fno-exceptions`` then it shrinks the output by 15% (!).

Memory Growth
-------------

Building with ``-s ALLOW_MEMORY_GROWTH=1`` allows the total amount of memory used to change depending on the demands of the application. This is useful for apps that don't know ahead of time how much they will need, but it disables asm.js optimizations. In WebAssembly, however, there should be little or no overhead.

Viewing code optimization passes
--------------------------------

Enable :ref:`debugging-EMCC_DEBUG` to output files for each compilation phase, including the main optimization operations.

.. _optimizing-code-unsafe-optimisations:

Unsafe optimizations
====================

A few **UNSAFE** optimizations you might want to try are:

- ``--closure 1``: This can help with reducing the size of the non-generated (support/glue) JS code, and with startup. However it can break if you do not do proper :term:`Closure Compiler` annotations and exports. But it's worth it!

.. _optimizing-code-profiling:

Profiling
=========

Modern browsers have JavaScript profilers that can help find the slower parts in your code. As each browser's profiler has limitations, profiling in multiple browsers is highly recommended.

To ensure that compiled code contains enough information for profiling, build your project with :ref:`profiling <emcc-profiling>` as well as optimization and other flags:

.. code-block:: bash

  emcc -O2 --profiling file.cpp


Troubleshooting poor performance
================================

Emscripten-compiled code can currently achieve approximately half the speed of a native build. If the performance is significantly poorer than expected, you can also run through the additional troubleshooting steps below:

-  :ref:`Building-Projects` is a two-stage process: compiling source code files to LLVM **and** generating JavaScript from LLVM. Did you build using the same optimization values in **both** steps (``-O2`` or ``-O3``)?
-  Test on multiple browsers. If performance is acceptable on one browser and significantly poorer on another, then :ref:`file a bug report <bug-reports>`, noting the problem browser and other relevant information.
- Does the code *validate* in Firefox (look for "Successfully compiled asm.js code" in the web console). If you see a validation error when using an up-to-date version of Firefox and Emscripten then please :ref:`file a bug report <bug-reports>`.

