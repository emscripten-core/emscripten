.. _Optimizing-Code:

===============
Optimizing Code
===============

Generally you should first compile and run your code without optimizations,
which is the default when you just run ``emcc`` without specifying an
optimization level. Such unoptimized builds contain some checks and assertions
that can be very helpful in making sure that your code runs correctly. Once it
does, it is highly recommended to optimize the builds that you ship, for
several reasons: First, optimized builds are much smaller and faster, so they
load quickly and run more smoothly, and second, **un**-optimized builds contain
debug information such as the names of files and functions, code comments in
JavaScript, etc. (which aside from increasing size may also contain things you
do not want to ship to your users).

The rest of this page explains how to optimize your code.

How to optimize code
====================

Code is optimized by specifying :ref:`optimization flags <emcc-compiler-optimization-options>` when running :ref:`emcc <emccdoc>`. The levels include: :ref:`-O0 <emcc-O0>` (no optimization), :ref:`-O1 <emcc-O1>`, :ref:`-O2 <emcc-O2>`, :ref:`-Os <emcc-Os>`, :ref:`-Oz <emcc-Oz>`, :ref:`-Og <emcc-Og>`, and :ref:`-O3 <emcc-O3>`.

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

  -  The meanings of the *emcc* optimization flags (``-O1, -O2`` etc.) are similar to *gcc*, *clang*, and other compilers, but also different because optimizing WebAssembly includes some additional types of optimizations. The mapping of the *emcc* levels to the LLVM bitcode optimization levels is documented in the reference.

How Emscripten optimizes
========================

Compiling source files to object files works as you'd expect in a native build system that uses clang and LLVM. When linking object files to the final executable, Emscripten does additional optimizations as well depending on the optimization level:

- The Binaryen optimizer is run. Binaryen does both general-purpose optimizations to the Wasm that LLVM does not, and also does some whole-program optimization. (Note that Binaryen's whole-program optimizations may do things like inlining, which can be surprising in some cases as LLVM IR attributes like ``noinline`` have been lost at this point.)
- JavaScript is generated at this phase, and is optimized by Emscripten's JS optimizer. Optionally you can also run :ref:`the closure compiler <emcc-closure>`, which is highly recommended for code size.
- Emscripten also optimizes the combined Wasm+JS, by minifying imports and exports between them, and by running meta-dce which removes unused code in cycles that span the two worlds.

Link Times
==========

To skip extra optimization work at link time, link with ``-O0`` or ``-O1``. In
those modes Emscripten focuses on faster iteration times. (Note that it is ok
to link with those flags even if the source files were compiled with a different
optimization level.)

To also skip non-optimization work at link time, link with ``-sWASM_BIGINT``.
Enabling BigInt support removes the need for Emscripten to "legalize" the Wasm
to handle ``i64`` values on the JS/Wasm boundary (as with BigInts ``i64`` values
are legal, and require no extra processing).

Some link flags add additional work at the link stage that can slow things down.
For example ``-g`` enables DWARF support, flags like ``-sSAFE_HEAP`` will require
JS post-processing, and flags like ``-sASYNCIFY`` will require Wasm
post-processing. To ensure your flags allow the fastest possible link, in which
the Wasm is not modified after ``wasm-ld``, build with
``-sERROR_ON_WASM_CHANGES_AFTER_LINK``. With that option you will get an error
during link if Emscripten must perform changes to the Wasm. For example, if you
didn't pass ``-sWASM_BIGINT`` then it will tell you that legalization forces
it to change the Wasm. You will also get an error if you build with ``-O2`` or
above, as the Binaryen optimizer would normally be run.


Advanced compiler settings
==========================

There are several flags you can :ref:`pass to the compiler <emcc-s-option-value>` to affect code generation, which will also affect performance — for example :ref:`DISABLE_EXCEPTION_CATCHING <optimizing-code-exception-catching>`. These are documented in `src/settings.js <https://github.com/emscripten-core/emscripten/blob/main/src/settings.js>`_.

WebAssembly
===========

Emscripten emits WebAssembly by default. You can switch that off with ``-sWASM=0`` (in which case emscripten will emit JavaScript), which is necessary if you want the output to run in places where Wasm support is not present yet, but the downside is larger and slower code.

.. _optimizing-code-size:

Code size
=========

This section describes optimisations and issues that are relevant to code size. They are useful both for small projects or libraries where you want the smallest footprint you can achieve, and in large projects where the sheer size may cause issues (like slow startup speed) that you want to avoid.

.. _optimizing-code-oz-os:

Trading off code size and performance
-------------------------------------

You may wish to build the less performance-sensitive source files in your project using :ref:`-Os <emcc-Os>` or :ref:`-Oz <emcc-Oz>` and the remainder using :ref:`-O2 <emcc-O2>` (:ref:`-Os <emcc-Os>` and :ref:`-Oz <emcc-Oz>` are similar to :ref:`-O2 <emcc-O2>`, but reduce code size at the expense of performance. :ref:`-Oz <emcc-Oz>` reduces code size more than :ref:`-Os <emcc-Os>`.)

Separately, you can do the final link/build command with ``-Os`` or ``-Oz`` to make the compiler focus more on code size when generating WebAssembly module.

Miscellaneous code size tips
----------------------------

In addition to the above, the following tips can help to reduce code size:

- Use :ref:`the closure compiler <emcc-closure>` on the non-compiled code: ``--closure 1``. This can hugely reduce the size of the support JavaScript code, and is highly recommended. However, if you add your own additional JavaScript code (in a ``--pre-js``, for example) then you need to make sure it uses `closure annotations properly <https://developers.google.com/closure/compiler/docs/api-tutorial3>`_.
- `Floh's blogpost on this topic <http://floooh.github.io/2016/08/27/asmjs-diet.html>`_ is very helpful.
- Make sure to use gzip compression on your webserver, which all browsers now support.

The following compiler settings can help (see ``src/settings.js`` for more details):

- Disable inlining when possible, using ``-sINLINING_LIMIT``. Compiling with -Os or -Oz generally avoids inlining too. (Inlining can make code faster, though, so use this carefully.)
- You can use the ``-sFILESYSTEM=0`` option to disable bundling of filesystem support code (the compiler should optimize it out if not used, but may not always succeed). This can be useful if you are building a pure computational library, for example.
- The ``ENVIRONMENT`` flag lets you specify that the output will only run on the web, or only run in node.js, etc. This prevents the compiler from emitting code to support all possible runtime environments, saving ~2KB.

LTO
===

Link Time Optimization (LTO) lets the compiler do more optimizations, as it can
inline across separate compilation units, and even with system libraries.
LTO is enabled by compiling objects files with ``-flto``.  The effect of this
flag is to emit LTO object files (technically this means emitting bitcode).  The
linker can handle a mix Wasm object files and LTO object files.  Passing
``-flto`` at link time will also trigger LTO system libraries to be used.

Thus, to allow maximal LTO opportunities with the LLVM Wasm backend, build all
source files with ``-flto`` and also link with ``flto``.

EVAL_CTORS
==========

Building with ``-sEVAL_CTORS`` will evaluate as much code as possible at
compile time. That includes both the "global ctor" functions (functions LLVM
emits that run before ``main()``) as well as ``main()`` itself. As much as can
be evaluated will be, and the resulting state is then "snapshotted" into the
wasm. Then when the program is run it will begin from that state, and not need
to execute that code, which can save time.

This optimization can either reduce or increase code size. If a small amount
of code generates many changes in memory, for example, then overall size may
increase. It is best to build with this flag and then measure code and startup
speed and see if the tradeoff is worthwhile in your program.

You can make an effort to write EVAL_CTORS-friendly code, by deferring things
that cannot be evalled as much as possible. For example, calls to imports stop
this optimization, and so if you have a game engine that creates a GL context
and then does some pure computation to set up unrelated data structures in
memory, then you could reverse that order. Then the pure computation could run
first, and be evalled away, and the GL context creation call to an import would
not prevent that. Other things you can do are to avoid using ``argc/argv``, to
avoid using ``getenv()``, and so forth.

Logging is shown when using this option so that you can see whether things can
be improved. Here is an example of output from ``emcc -sEVAL_CTORS``:

::

  trying to eval __wasm_call_ctors
    ...partial evalling successful, but stopping since could not eval: call import: wasi_snapshot_preview1.environ_sizes_get
         recommendation: consider --ignore-external-input
    ...stopping

The first line indicates an attempt to eval LLVM's function that runs global
ctors. It evalled some of the function but then it stopped on the WASI import
``environ_sizes_get``, which means it is trying to read from the environment.
As the output says, you can tell ``EVAL_CTORS`` to ignore external input, which
will ignore such things. You can enable that with mode ``2``, that is, build
with ``emcc -sEVAL_CTORS=2``:

::

  trying to eval __wasm_call_ctors
    ...success on __wasm_call_ctors.
  trying to eval main
    ...stopping (in block) since could not eval: call import: wasi_snapshot_preview1.fd_write
    ...stopping

Now it has succeeded to eval ``__wasm_call_ctors`` completely. It then moved on
to ``main``, where it stopped because of a call to WASI's ``fd_write``, that is,
a call to print something.

Very large codebases
====================

The previous section on reducing code size can be helpful on very large codebases. In addition, here are some other topics that might be useful.

Running by itself
-----------------

If you hit memory limits in browsers, it can help to run your project by itself, as opposed to inside a web page containing other content. If you open a new web page (as a new tab, or a new window) that contains just your project, then you have the best chance at avoiding memory fragmentation issues.

Module Splitting
----------------

If your module is large enough that the time to download and instantiate it is noticeably affecting your application's startup performance, it may be worth splitting the module and deferring the loading of code that is not necessary to bring up the application. See :ref:`Module-Splitting` for a guide on how to do this. *Note that module splitting is an experimental feature and subject to change.*


Other optimization issues
=========================

.. _optimizing-code-exception-catching:

C++ exceptions
--------------

Catching C++ exceptions (specifically, emitting catch blocks) is turned off by default in ``-O1`` (and above). Due to how WebAssembly currently implement exceptions, this makes the code much smaller and faster (eventually, Wasm should gain native support for exceptions, and not have this issue).

To re-enable exceptions in optimized code, run *emcc* with ``-sDISABLE_EXCEPTION_CATCHING=0`` (see `src/settings.js <https://github.com/emscripten-core/emscripten/blob/main/src/settings.js>`_).

.. note:: When exception catching is disabled, a thrown exception terminates the application. In other words, an exception is still thrown, but it isn't caught.

.. note:: Even with catch blocks not being emitted, there is some code size overhead unless you build your source files with ``-fno-exceptions``, which will omit all exceptions support code (for example, it will avoid creating proper C++ exception objects in errors in std::vector, and just abort the application if they occur)

C++ RTTI
--------

C++ run-time type info support (dynamic casts, etc.) adds overhead that is sometimes not needed. For example, in Box2D neither rtti nor exceptions are needed, and if you build the source files with ``-fno-rtti -fno-exceptions`` then it shrinks the output by 15% (!).

Memory Growth
-------------

Building with ``-sALLOW_MEMORY_GROWTH`` allows the total amount of memory used to change depending on the demands of the application. This is useful for apps that don't know ahead of time how much they will need.

Viewing code optimization passes
--------------------------------

Enable :ref:`debugging-EMCC_DEBUG` to output files for each compilation phase, including the main optimization operations.

.. _optimizing-code-unsafe-optimisations:

Allocation
----------

The default ``malloc/free`` implementation used is ``dlmalloc``. You can also
pick ``emmalloc`` (``-sMALLOC=emmalloc``) which is smaller but less fast, or
``mimalloc`` (``-sMALLOC=mimalloc``) which is larger but scales better in a
multithreaded application with contention on ``malloc/free`` (see
:ref:`Allocator_performance`).

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

Emscripten-compiled code can often be close to the speed of a native build. If the performance is significantly poorer than expected, you can also run through the additional troubleshooting steps below:

-  :ref:`Building-Projects` is a two-stage process: compiling source code files to LLVM **and** generating JavaScript from LLVM. Did you build using the same optimization values in **both** steps (``-O2`` or ``-O3``)?
-  Test on multiple browsers. If performance is acceptable on one browser and significantly poorer on another, then :ref:`file a bug report <bug-reports>`, noting the problem browser and other relevant information.
