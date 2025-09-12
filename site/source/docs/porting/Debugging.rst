.. _Debugging:

=========
Debugging
=========

One of the main advantages of debugging cross-platform Emscripten code is that
the same cross-platform source code can be debugged on either the native
platform or using the web browser's increasingly powerful toolset — including a
debugger, profiler, and other tools.

This article describes the main tools and settings provided by Emscripten for
debugging, organized by common developer use cases.


Overview: Emitting and Controlling Debug Information
====================================================
Debugging-related information comes in several forms: in Wasm object and binary
files (as DWARF sections or Wasm name section), side output files (as source
maps, symbol maps, or DWARF sidecar or package files), and even in the code
itself (as assertions or instrumentation, or JS whitespace and comments). For
information on DWARF, see :ref:`below <debugging-dwarf>`. In addition to DWARF,
wasm files may contain a `name section
<https://webassembly.github.io/spec/core/appendix/custom.html#name-section>`_
which includes names for each function; these function names are displayed by
browsers when they generate `stack traces
<https://webassembly.github.io/spec/web-api/index.html#conventions>`_ and in
developer tools. Source maps are also supported by Emscripten and by browser
DevTools (see :ref:`below <debugging-symbolization>`).

This document contains an overview of the flags used to emit and control
debugging behavior, and use-case-based examples.

Unlike traditional Unix-style C toolchains, flags must be passed at link time to
preserve or generate debug information (these defaults aim to avoid unintended
bloat in production builds). The most common of these are the :ref:`-g flags
<emcc-gN>`; see the flag documentation or the use cases below for more detail.

Flags that cause DWARF generation (e.g. ``-g3``, ``-gline-tables-only``) also
generate a name section in the binary and suppress minification of the JS glue
file (since most DWARF use cases are for interactive debugging or where the
binary will be stripped). Other flags (e.g. ``-g2``, ``-gsource-map``) should
affect only a specific behavior or type of debug info, and are generally
composable.


.. _debugging-interactive:

Interactive, Source-Level Debugging
=============================================

For stepping through C/C++ source code in a browser's debugger, you can use
debug information in either DWARF or source map format.

DWARF offers the best debugging experience and is supported in Chrome with an
`extension <https://goo.gle/wasm-debugging-extension>`_. See `here
<https://developer.chrome.com/blog/wasm-debugging-2020/>`_ for a detailed usage
guide. Source maps are more widely supported, but they provide only location
mapping and cannot be used easily to inspect variables.


.. _debugging-dwarf:

DWARF
-----

In a traditional Unix-style C toolchain, flags such as ``-g`` are passed to the
compiler, placing DWARF sections in the object files. This DWARF info is
combined by the linker and appears in the output, independently of any
optimization settings. In contrast, although :ref:`emcc <emccdoc>` supports many
of the common `clang flags
<https://clang.llvm.org/docs/ClangCommandLineReference.html#debug-information-generation>`_
to generate DWARF into the object files, final debug output is also controlled
by link-time flags, and is more affected by optimization. For example ``emcc``
strips out most of the debug information after linking if a debugging-related
flag is not provided at link time, even if the input object files contain DWARF.

DWARF can be produced at compile time with the *emcc* :ref:`-g flag <emcc-g>`.
Optimization levels above :ref:`-O1 <emcc-O1>` or :ref:`-Og <emcc-Og>`
increasingly degrade LLVM debug information (as with other architectures), and
optimization flags at link time also disable Emscripten's runtime
:ref:`ASSERTIONS <debugging-ASSERTIONS>` checks. Passing a ``-g`` flag at link
time also affects the generated JavaScript code (preserving white-space,
function names, and variable names, which makes the JavaScript debuggable).

The ``-g`` flag can also be specified with integer levels: :ref:`-g0 <emcc-g0>`,
:ref:`-g1 <emcc-g1>`, :ref:`-g2 <emcc-g2>`, and :ref:`-g3 <emcc-g3>` (equivalent
to ``-g``).  At compile time these flags control the amount of DWARF in the
object files. At link time, each adds sucessively more kinds of information in
the wasm and JS files (DWARF is only retained after linking when using ``-g`` or
``-g3``).

Example:

.. code-block:: bash

  emcc source.c -c -o source.o -g # source.o has DWARF sections emcc source.o -o
  program.js -g # program.wasm has DWARF and a name section


.. tip:: Even for medium-sized projects, DWARF debug information can be large.
  Debug information can be emitted in a separate file with the
  :ref:`-gseparate-dwarf <emcc-gseparate-dwarf>` option. To speed up linking,
  the :ref:`-gsplit-dwarf <emcc-gsplit-dwarf>` option can be used at compile
  time. See `this article
  <https://developer.chrome.com/blog/faster-wasm-debugging/#scalable_debugging>`_
  for more details on debugging large files, and see :ref:`the next section
  <debugging-symbolization>` for more ways to reduce debug info size.


.. note:: Because Binaryen optimization degrades the quality of DWARF info
  further, higher link-time optimization settings are
  not recommended. The ``-O1`` setting will skip running the Binaryen optimizer
  (``wasm-opt``) entirely unless required by other options. You can also add the
  ``-sERROR_ON_WASM_CHANGES_AFTER_LINK`` option if you want to ensure the debug
  info is preserved. See `Skipping Binaryen
  <https://developer.chrome.com/blog/faster-wasm-debugging/#skipping-binaryen>`_
  for more details.


.. _debugging-symbolization:

Symbolizing Production Crash Logs
=============================================

Even when not using an interactive debugger, it's valuable to have source
information for compiled code locations, particularly for stack traces or crash
logs. This is also true for fully-optimized production builds.

`Source maps <https://web.dev/articles/source-maps>`_ are commonly used for
languages that compile to JavaScript (mapping locations in the compiled JS
output to locations in the original source code), but WebAssembly is also
supported. Emscripten can emit source maps with the :ref:`-gsource-map
<emcc-gsource-map>` link-time flag. Source maps are preserved even with full
post-link optimizations, so they work well for this use case. Source maps are
generated by Emscripten from DWARF information. Therefore the linked object
files must have DWARF. The final linked output will not have DWARF unless `-g`
is also passed at link time.

DWARF can also be used for this purpose. Typically a binary containing DWARF
would be generated at build time, and then stripped. The stripped copy would be
served to users, and the original would be saved for symbolication purposes. For
this use case, full information about about types and variables from the sources
isn't needed; the `-gline-tables-only
<https://clang.llvm.org/docs/ClangCommandLineReference.html#cmdoption-clang-gline-tables-only>`_
compile-time flag causes clang to generate only the line table information,
saving DWARF size and compile/linking time.

Source maps are easier to parse and more widely supported by ecosystem tooling.
And as noted above, preserving DWARF inhibits some Binaryen optimizations.
However DWARF has the advantage that it includes information about inlining,
which can result in more accurate stack traces.

Examples:

.. code-block:: bash

  emcc source.c -c -o source.o -g # source.o has DWARF sections (-gsource-map also works here)
  emcc source.o -o program.js -gsource-map # program.wasm.map contains a source map

  emcc source.o -o program2.js -g # program2.wasm has DWARF
  llvm-strip program2.wasm -o program2_stripped.wasm # program2_stripped.wasm has no debug info

Emscripten includes a tool called ``emsymbolizer`` that can map wasm code
addresses to sources using several different kinds of debug info, including
DWARF (in wasm object or linked files) and source maps for line/column info, and
symbol maps (see :ref:`emcc-emit-symbol-map`), name sections and object file
symbol tables for function names.


Fast Edit+Compile with minimal debug information
================================================

When you want the fastest builds, you generally want to avoid generating large
debug information during compile, because it takes time to link into the final
binary. It is still worthwhile to use the ``-g2`` flag (at link time only)
because browsers understand the name section even when devtools are not in use,
resulting in more useful stack traces at minimal cost.

Example:

.. code-block:: bash

  emcc source.c -c -o source.o # source.o has no debug info
  emcc source.o -o program.js -g2 # program.wasm has a name section, program.js is unminified

Sometimes the use of the ``-O1`` or ``-Og`` flag at compile time can also result
in faster builds, because optimizations early in the pipeline can reduce the
amount of IR that is processed by later phases such as instruction selection and
linking. It also of course reduces test runtime.

.. _debugging-memory-safety:

Detecting Memory Errors and Undefined Behavior
==============================================

The best tools for detecting memory safety and undefined behavior issues are
Clang's sanitizers, such as the Undefined Behavior Sanitizer (UBSan) and the
Address Sanitizer (ASan). For more information, see :ref:`Sanitizers`.


Emscripten has several other compiler settings that can be useful for catching
errors at runtime. These are set using the :ref:`emcc -s<emcc-s-option-value>`
option. For example:

.. code-block:: bash

  emcc -O1 -sASSERTIONS test/hello_world.c

Some important settings are:

  -
    .. _debugging-ASSERTIONS:

    ``ASSERTIONS=1`` is used to enable runtime checks for many types of common
    errors. It also defines how Emscripten should handle errors in program flow.
    The value can be set to ``ASSERTIONS=2`` in order to run additional tests.
    ``ASSERTIONS=1`` is enabled by default at ``-O0`` and disabled at higher
    optimization levels, but can be overridden.

  -
    .. _debugging-SAFE-HEAP:

    ``SAFE_HEAP=1`` adds additional memory access checks with a Binaryen pass,
    and will give clear errors for problems like dereferencing 0 and memory
    alignment issues. You can also set ``SAFE_HEAP_LOG`` to log ``SAFE_HEAP``
    operations. :ref:`ASan<sanitizer_asan>` provides most of the functionality
    of this pass (plus some extras) and is generally preferred to try first
    unless :ref:`alignment issues<debugging-emscripten-specific-issues>` are
    important for your platform.

  -
    .. _debugging-STACK_OVERFLOW_CHECK:

    ``STACK_OVERFLOW_CHECK=1`` adds a runtime magic token value at the end of
    the stack, which is checked in certain locations to verify that the user
    code does not accidentally write past the end of the stack. While
    overrunning the Emscripten stack is not a security issue for JavaScript
    (which is unaffected), writing past the stack causes memory corruption in
    global data and dynamically allocated memory sections in the Emscripten
    HEAP, which makes the application fail in unexpected ways. The value
    ``STACK_OVERFLOW_CHECK=2`` enables slightly more detailed stack guard
    checks, which can give a more precise callstack at the expense of some
    performance. Default value is 1 if ``ASSERTIONS=1`` is set, and disabled
    otherwise.



A number of other useful debug settings are defined in `src/settings.js
<https://github.com/emscripten-core/emscripten/blob/main/src/settings.js>`_. For
more information, search that file for the keywords "check" and "debug".


.. _debugging-profiling:

Profiling Performance
=====================

Speed
-----

To profile your code for speed, build with :ref:`profiling info
<emcc-profiling>` using ``--profiling``, (which is currently the same as
:ref:`-g2 <emcc-g2>`), and then run the code in the browser's devtools profiler.
You should then be able to see in which functions most of the time is spent.

Memory
------

The browser's memory profiling tools generally only understand allocations at
the JavaScript level. From that perspective, the entire linear memory that the
emscripten-compiled application uses is a single big allocation (of a
``WebAssembly.Memory``). To get information about usage inside that object, you
need other tools:

* Emscripten supports the `mallinfo()
  <https://man7.org/linux/man-pages/man3/mallinfo.3.html>`_, API, which gives
  you information from ``dlmalloc`` about current allocations.
* Emscripten also has a ``--memoryprofiler`` option that displays memory usage
  in a visual manner. Note that you need to emit HTML (e.g. with a command like
  ``emcc test/hello_world.c --memoryprofiler -o page.html``) as the memory
  profiler output is rendered onto the page. To view it, load ``page.html`` in
  your browser (remember to use a :ref:`local webserver <faq-local-webserver>`).
  The display auto-updates, so you can open the devtools console and run a
  command like ``_malloc(1024 * 1024)``. That will allocate 1MB of memory, which
  will then show up on the memory profiler display.

.. _debugging-manual-debugging:


Manual print debugging
======================

You can also manually instrument the source code with ``printf()`` statements,
then compile and run the code to investigate issues. The output from the
`stdout` and `stderr` streams is copied to the browser console by default. Note
that ``printf()`` is line-buffered, so make sure to add ``\n`` to see output in
the console. The functions in the :ref:`console.h <console-h>` header can also
be used to access the console more directly.

.. _debugging-emscripten-specific-issues:

Emscripten-Specific Issues
==========================

Memory Alignment Issues
-----------------------

The :ref:`Emscripten memory representation <emscripten-memory-model>` is
compatible with C and C++. In WebAssembly, unaligned loads and stores will work;
each may be annotated with its expected alignment. However if the actual
alignment does not match, it may be very slow on some systems.

.. tip:: :ref:`SAFE_HEAP <debugging-SAFE-HEAP>` can be used to reveal memory alignment issues.

Generally it is best to avoid unaligned reads and writes. Often they occur as
the result of undefined behavior. In some cases, however, they are unavoidable —
for example if the code to be ported reads an ``int`` from a packed structure in
some pre-existing data format. In that case, to make it as fast as possible in
WebAssembly, you can make sure that the compiler knows the load or store is
unaligned. To do so you can:

- Manually read individual bytes and reconstruct the full value
- Use the :c:type:`emscripten_align* <emscripten_align1_short>` typedefs, which
  define unaligned versions of the basic types (``short``, ``int``, ``float``,
  ``double``). All operations on those types are not fully aligned (use the
  ``1`` variants in most cases, which mean no alignment whatsoever).

Function Pointer Issues
-----------------------

If you get an ``abort()`` from a function pointer call to ``nullFunc`` or ``b0``
or ``b1`` (possibly with an error message saying "incorrect function pointer"),
the problem is that the function pointer was not found in the expected function
pointer table when called.

.. note:: ``nullFunc`` is the function used to populate empty index entries in
  the function pointer tables (``b0`` and ``b1`` are shorter names used for
  ``nullFunc`` in more optimized builds).  A function pointer to an invalid
  index will call this function, which simply calls ``abort()``.

There are several possible causes:

- Your code is calling a function pointer that has been cast from another type
  (this is undefined behavior but it does happen in real-world code). In
  optimized Emscripten output, each function pointer type is stored in a
  separate table based on its original signature, so you *must* call a function
  pointer with that same signature to get the right behavior (see
  :ref:`portability-function-pointer-issues` in the code portability section for
  more information).
- Your code is calling a method on a ``NULL`` pointer or dereferencing 0. This
  sort of bug can be caused by any sort of coding error, but manifests as a
  function pointer error because the function can't be found in the expected
  table at runtime.


To debug these sorts of issues:

- Compile with ``-Werror`` (or otherwise fix warnings, many of which highlight
  undefined behavior).
- Use ``-sASSERTIONS=2`` to get some useful information about the function
  pointer being called, and its type.
- Look at the browser stack trace to see where the error occurs and which
  function should have been called.
- Enable clang warnings on dangerous function pointer casts using
  ``-Wcast-function-type``.
- Build with :ref:`SAFE_HEAP=1 <debugging-SAFE-HEAP>`.
- :ref:`Sanitizers` can help here, in particular UBSan.


Infinite loops
--------------

Infinite loops cause your page to hang. After a period the browser will notify
the user that the page is stuck and offer to halt or close it. If your code hits
an infinite loop, one easy way to find the problem code is to use a *JavaScript
profiler*. In the Firefox profiler, if the code enters an infinite loop you will
see a block of code doing the same thing repeatedly near the end of the profile.

.. note:: The :ref:`emscripten-runtime-environment-main-loop` may need to be
  re-coded if your application uses an infinite main loop.

.. _other-debugging-tools:

Debugging Emscripten
====================

.. _debugging-EMCC_DEBUG:

Debugging the compiler driver
-----------------------------

Compiling with the :ref:`emcc -v <emcc-verbose>` will cause emcc to output the
sub-commands that it runs as well as passes ``-v`` to Clang. The ``EMCC_DEBUG``
environment variable can be set to emit even more debug output and generate
intermediate files for the compiler's various stages.


.. _debugging-autodebugger:

AutoDebugger
------------

The *AutoDebugger* is the 'nuclear option' for debugging Emscripten code. It
will rewrite the output so it prints out each store to memory. This is useful
for comparing the output for different compiler settings in order to detect
regressions. To run the *AutoDebugger*, compile with the environment variable
``EMCC_AUTODEBUG=1`` set.

.. warning:: This option is primarily intended for Emscripten core developers.

The *AutoDebugger* will rewrite the output so it prints out each store to
memory. This is useful because you can compare the output for different compiler
settings in order to detect regressions.

The *AutoDebugger* can potentially find **any** problem in the generated code,
so it is strictly more powerful than the ``CHECK_*`` settings and ``SAFE_HEAP``.
One use of the *AutoDebugger* is to quickly emit lots of logging output, which
can then be reviewed for odd behavior. The *AutoDebugger* is also particularly
useful for :ref:`debugging regressions <debugging-autodebugger-regressions>`.

The *AutoDebugger* has some limitations:

-  It generates a lot of output. Using *diff* can be very helpful for
   identifying changes.
-  It prints out simple numerical values rather than pointer addresses (because
   pointer addresses change between runs, and hence can't be compared). This is
   a limitation because sometimes inspection of addresses can show errors where
   the pointer address is 0 or impossibly large. It is possible to modify the
   tool to print out addresses as integers in ``tools/autodebugger.py``.

To run the *AutoDebugger*, compile with the environment variable
``EMCC_AUTODEBUG=1`` set. For example:

.. code-block:: bash

  # Linux or macOS
  EMCC_AUTODEBUG=1 emcc test/hello_world.cpp -o hello.html
  # Windows
  set EMCC_AUTODEBUG=1
  emcc test/hello_world.cpp -o hello.html
  set EMCC_AUTODEBUG=0


.. _debugging-autodebugger-regressions:

AutoDebugger Regression Workflow
---------------------------------

Use the following workflow to find regressions with the *AutoDebugger*:

- Compile the working code with ``EMCC_AUTODEBUG=1`` set in the environment.
- Compile the code using ``EMCC_AUTODEBUG=1`` in the environment again, but this
  time with the settings that cause the regression. Following this step we have
  one build before the regression and one after.
- Run both versions of the compiled code and save their output.
- Compare the output using a *diff* tool.

Any difference between the outputs is likely to be caused by the bug.

.. note::
    You may want to use ``-sDETERMINISTIC`` which will ensure that timing
    and other issues don't cause false positives.



Useful Links
============

- `Links to Wasm debugging-related documents <https://web.dev/webassembly/#webassembly-debugging>`_


Need help?
==========

The :ref:`Emscripten Test Suite <emscripten-test-suite>` contains good examples
of almost all functionality offered by Emscripten. If you have a problem, it is
a good idea to search the suite to determine whether test code with similar
behavior is able to run.

If you've tried the ideas here and you need more help, please :ref:`contact`.
