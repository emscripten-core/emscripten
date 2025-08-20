.. _Debugging:

=========
Debugging
=========

One of the main advantages of debugging cross-platform Emscripten code is that the same cross-platform source code can be debugged on either the native platform or using the web browser's increasingly powerful toolset — including a debugger, profiler, and other tools.

This article describes the main tools and settings provided by Emscripten for debugging, organized by common developer use cases.

Overview of Debugging Flags
===========================

Emscripten offers a variety of flags to control the generation of debug information. Here is a summary of the most common ones:

.. list-table::
   :header-rows: 1
   :widths: 20 60 20
   :class: wrap-table-content

   * - Flag
     - Primary Use Case
     - More Info
   * - ``-g``
     - Interactive, source-level debugging with full DWARF information. May disable some optimizations.
     - :ref:`emcc-g`
   * - ``-gsource-map``
     - Symbolicating production crash logs with source maps. Designed to work with optimizations.
     - :ref:`emcc-gsource-map`
   * - ``-fsanitize=address``
     - Detecting memory errors (buffer overflows, use-after-free, memory leaks).
     - `Clang ASan docs <https://clang.llvm.org/docs/AddressSanitizer.html>`_
   * - ``-fsanitize=undefined``
     - Detecting undefined behavior (e.g., null pointer dereferences, integer overflow).
     - `Clang UBSan docs <https://clang.llvm.org/docs/UndefinedBehaviorSanitizer.html>`_
   * - ``-sSAFE_HEAP=1``
     - Checking for memory access errors like null pointer dereferences and unaligned access.
     - :ref:`SAFE_HEAP`
   * - ``-sASSERTIONS=1``
     - Enabling runtime checks for common errors and incorrect program flow.
     - :ref:`ASSERTIONS`
   * - ``--profiling``
     - Building with information for speed profiling in the browser's devtools.
     - :ref:`emcc-profiling`
   * - ``--memoryprofiler``
     - Embedding a visual memory allocation tracker in the generated HTML.
     - :ref:`emcc-profiling`



Emitting and controlling debug information
==========================================
Debugging-related information comes in several forms: in Wasm object and binary files (DWARF 
sections, Wasm name section), side output files (source maps, symbol maps, DWARF sidecar and package files),
and even in the code itself (assertions and instrumentation, whitespace).
For information on DWARF, see :ref:`below <debugging-dwarf>`.
In addition to DWARF, wasm files may contain a name section (TODO link) which includes names for each
function; these function names are displayed by browsers when they generate stack traces and in
developer tools. (TODO more info?). Source maps are also supported (see :ref:`below <debugging-symbolization>`).

This document contains an overview of the flags used to emit and control debugging behavior, and
use-case-based examples.

DWARF:
Amount of debug information generated: ``-g``, ``-g<level>``, ``-gline-tables-only``
Type of debug information in the binary: ``-gdwarf-5`` (others?)
Where DWARF is written: ``-gsplit-dwarf``, ``-gseparate-dwarf``

Type of debug information generated: (dwarf flags), ``-gname``, ``--profiling-funcs``, ``--profiling``
Type of debug information generated alongside: ``-gsource-maps``, ``--emit-symbol-map``

JS Minification: ``--profiling``, ``--minify=0``

Runtime safety and bug detection: ``-fsanitize=address|undefined|leak``, ``-sASSERTIONS``

Flags that cause DWARF generation also generate a name section in the binary and suppress
minification of the JS glue file (since most DWARF use cases are for interactive debugging).
Other flags should affect only a specific behavior or type of debug info, and are generally
composable.



Interactive, Source-Level Debugging
=============================================

For stepping through C/C++ source code in a browser's debugger, you can use debug information in either DWARF or source map format.

DWARF offers the best debugging experience and is supported in Chrome with an
`extension <https://goo.gle/wasm-debugging-extension>`_.
See `here <https://developer.chrome.com/blog/wasm-debugging-2020/>`_ for a detailed usage guide.
Source maps are more widely supported, but they provide only location mapping
and cannot be used easily to inspect variables.


.. _debugging-dwarf:

DWARF
-----

In a traditional Unix-style C toolchain, flags such as ``-g`` are passed to the compiler, placing
DWARF sections in the object files. This DWARF info is combined by the linker and appears in the
output, independently of any optimization settings.
In contrast, although :ref:`Emcc <emccdoc>` supports many of the common
`clang flags <https://clang.llvm.org/docs/ClangCommandLineReference.html#debug-information-generation>`_ to generate DWARF into
the object files, final debug output is also controlled by link-time flags, and is more affected
by optimization.
For example ``emcc`` strips out most of the debug information after linking if a debugging-related
flag is not provided at link time, even if the input object files contain DWARF.

DWARF can be produced at compile time with the *emcc* :ref:`-g flag <emcc-g>`. Optimization levels above
:ref:`-O1 <emcc-O1>` or :ref:`-Og <emcc-Og>` increasingly remove LLVM debug information (as with other architectures),
and optimization flags at link time also disable Emscripten's runtime :ref:`ASSERTIONS <debugging-ASSERTIONS>` checks.
Passing a ``-g`` flag at link time also affects the generated JavaScript code (preserving white-space, function names, and variable names).

The ``-g`` flag can also be specified with integer levels: :ref:`-g0 <emcc-g0>`, :ref:`-g1 <emcc-g1>`, :ref:`-g2 <emcc-g2>`,
and :ref:`-g3 <emcc-g3>` (default with ``-g``).  At compile time these flags control the amount of DWARF in the object files.
At link time, each adds sucessively more kinds of information in the wasm and JS files (DWARF is only retained after linking
when using ``-g`` or ``-g3``).

.. tip:: Even for medium-sized projects, DWARF debug information can be large. Debug information can be emitted in a
  separate file with the :ref:`-gseparate-dwarf <emcc-gseparate-dwarf>` option. To speed up linking,
  the :ref:`-gsplit-dwarf <emcc-gsplit-dwarf>` option can be used at compile time.
  See `this article <https://developer.chrome.com/blog/faster-wasm-debugging/#scalable_debugging>`_
  for more details on debugging large files, and see
  :ref:`the next section <debugging-symbolization>` for more ways to reduce debug info size.

.. note:: Because Binaryen optimization degrades the quality of DWARF info further, higher link-time optimization settings are
  not recommended. The ``-O1`` setting will skip running the Binaryen
  optimizer (``wasm-opt``) entirely unless required by other options. You can also add the 
  ``-sERROR_ON_WASM_CHANGES_AFTER_LINK`` option if you want to ensure the debug info is preserved.
  See `Skipping Binaryen <https://developer.chrome.com/blog/faster-wasm-debugging/#skipping-binaryen>`_ for more details.


.. _debugging-symbolization:

Symbolizing Production Crash Logs
=============================================

Even when not using an interactive debugger, it's valuable to have source information for compiled
code locations, particularly for stack traces or crash logs. This is also true for fully-optimized
production builds.

`Source maps <https://web.dev/articles/source-maps>`_ are commonly used for langauges that compile
to JavaScript (mapping locations in the compiled JS output to locations in the original source
code), but WebAssembly is also supported. Emscripten can emit source maps with
the :ref:`-gsource-map <emcc-gsource-map>` link-time flag. Source maps are preserved even with
full post-link optimizations, so they work well for this use case.

DWARF can also be used for this purpose. Typically a binary containing DWARF would be generated
at build time, and then stripped. The stripped copy would be served to users, and the original
would be saved for symbolication purposes. For this use case, full information about about types
and variables from the sources isn't needed; the 
`-gline-tables-only <https://clang.llvm.org/docs/ClangCommandLineReference.html#cmdoption-clang-gline-tables-only>`_
compile-time flag causes clang to generate only the line table information, saving DWARF size and compile/linking time.

Source maps are easier to parse and more widely supported by ecosystem tooling. And as noted
above, preserving DWARF inhibits some Binaryen optimizations. However DWARF has the advantage
that it includes information about inlining, which can result in more accurate stack traces.

(TODO: -g1 at compile time on native generates DWARF but not for emscripten)

Emscripten includes a tool called ``emsymbolizer`` that can map wasm code addresses to sources
using several different kinds of debug info, including DWARF (in wasm object or linked files)
and source maps for line/column info, and symbol maps (see :ref:`emcc-emit-symbol-map`),
name sections and object file symbol tables for function names.


Fast Edit+Compile with minimal debug information
================================================

When you want the fastest builds, you generally want to avoid generating large debug information
during compile, because it takes time to link into the final binary. It is still worthwhile to use
the ``--profiling`` (TODO gnames?)
flag (at link time only) because browsers understand the name section even when devtools are not 
in use, resulting in more useful stack traces at minimal cost.



Detecting Memory Errors and Undefined Behavior
==============================================

The best tools for detecting memory safety and undefined behavior issues. are Clang's sanitizers,
such as the Undefined Behaviour Sanitizer (UBSan) and the Address Sanitizer (ASan).
For more information, see :ref:`Sanitizers`.


Emscripten has several other compiler settings that can be useful for catching errors at runtime.
These are set using the :ref:`emcc -s<emcc-s-option-value>` option, and will override any optimization flags (TODO is this true?). For example:

.. code-block:: bash

  emcc -O1 -sASSERTIONS test/hello_world.c

Some important settings are:

  -
    .. _debugging-ASSERTIONS:

    ``ASSERTIONS=1`` is used to enable runtime checks for many types of common errors. It also
    defines how Emscripten should handle errors in program flow. The value can be set to 
    ``ASSERTIONS=2`` in order to run additional tests. ``ASSERTIONS=1`` is enabled by default at
    ``-O0``.

  -
    .. _debugging-SAFE-HEAP:

    ``SAFE_HEAP=1`` adds additional memory access checks with a Binaryen pass, and will give clear
    errors for problems like dereferencing 0 and memory alignment issues.
    You can also set ``SAFE_HEAP_LOG`` to log ``SAFE_HEAP`` operations. (TODO: any advantages over ASan?)

  -
    .. _debugging-STACK_OVERFLOW_CHECK:

    ``STACK_OVERFLOW_CHECK=1`` adds a runtime magic
    token value at the end of the stack, which is checked in certain locations
    to verify that the user code does not accidentally write past the end of the
    stack. While overrunning the Emscripten stack is not a security issue for
    JavaScript (which is unaffected), writing past the stack causes memory
    corruption in global data and dynamically allocated memory sections in the
    Emscripten HEAP, which makes the application fail in unexpected ways. The
    value ``STACK_OVERFLOW_CHECK=2`` enables slightly more detailed stack guard
    checks, which can give a more precise callstack at the expense of some
    performance. Default value is 1 if ``ASSERTIONS=1`` is set, and disabled
    otherwise.



A number of other useful debug settings are defined in `src/settings.js <https://github.com/emscripten-core/emscripten/blob/main/src/settings.js>`_. For more information, search that file for the keywords "check" and "debug".


.. _debugging-profiling:

Profiling Performance
=====================

Speed
-----

To profile your code for speed, build with :ref:`profiling info <emcc-profiling>`,
then run the code in the browser's devtools profiler. You should then be able to
see in which functions is most of the time spent.

Memory
------

The browser's memory profiling tools generally only understand
allocations at the JavaScript level. From that perspective, the entire linear
memory that the emscripten-compiled application uses is a single big allocation
(of a ``WebAssembly.Memory``).
To get information about usage inside that object, you need other tools:

* Emscripten supports the `mallinfo() <https://man7.org/linux/man-pages/man3/mallinfo.3.html>`_,
  API, which gives you information from ``dlmalloc`` about current allocations.
* Emscripten also has a ``--memoryprofiler`` option that displays memory usage in a visual manner.
  Note that you need to emit HTML (e.g. with a command like
  ``emcc test/hello_world.c --memoryprofiler -o page.html``) as the memory profiler
  output is rendered onto the page. To view it, load ``page.html`` in your
  browser (remember to use a :ref:`local webserver <faq-local-webserver>`). The display
  auto-updates, so you can open the devtools console and run a command like
  ``_malloc(1024 * 1024)``. That will allocate 1MB of memory, which will then show
  up on the memory profiler display.

.. _other-debugging-tools:

Other Debugging Tools and Techniques
====================================

.. _debugging-EMCC_DEBUG:

Debugging the compiler driver
-----------------------------

Compiling with the :ref:`emcc -v <emcc-verbose>` will cause emcc to output
the sub-commands that it runs as well as passes ``-v`` to Clang.
The ``EMCC_DEBUG`` environment variable can be set to emit even more debug
output and generate intermediate files for the compiler's various stages.

.. _debugging-manual-debugging:

Manual print debugging
----------------------

You can also manually instrument the source code with ``printf()`` statements,
then compile and run the code to investigate issues. The output from the `stdout` and `stderr`
streams is copied to the browser console by default. Note that ``printf()`` is
line-buffered, make sure to add ``\n`` to see output in the console. The functions
in the :ref:`console.h <console-h>` header can also be used to access the console
more directly.

.. _debugging-autodebugger:

AutoDebugger
------------

The *AutoDebugger* is the 'nuclear option' for debugging Emscripten code. It will rewrite the
output so it prints out each store to memory. This is useful for comparing the output for
different compiler settings in order to detect regressions. To run the *AutoDebugger*, compile
with the environment variable ``EMCC_AUTODEBUG=1`` set.

.. warning:: This option is primarily intended for Emscripten core developers.

.. _debugging-emscripten-specific-issues:

Emscripten-Specific Issues
==========================

Memory Alignment Issues
-----------------------

The :ref:`Emscripten memory representation <emscripten-memory-model>` is compatible with C and C++.
However, when undefined behavior is involved you may see differences with native architectures:

- In asm.js, unaligned loads and stores can fail silently (i.e. access the wrong address).
- In WebAssembly, unaligned loads and stores will work; each may be annotated with its expected
  alignment. If the actual alignment does not match, it may be very slow on some systems.

.. tip:: :ref:`SAFE_HEAP <debugging-SAFE-HEAP>` can be used to reveal memory alignment issues.

Generally it is best to avoid unaligned reads and writesoften they occur as the result of
undefined behavior, as mentioned above. In some cases, however, they are unavoidable — for example
if the code to be ported reads an ``int`` from a packed structure in some pre-existing data format.
In that case, to make things work properly in asm.js, and be fast in WebAssembly, you must be sure
that the compiler knows the load or store is unaligned. To do so you can:

- Manually read individual bytes and reconstruct the full value
- Use the :c:type:`emscripten_align* <emscripten_align1_short>` typedefs, which define unaligned
  versions of the basic types (``short``, ``int``, ``float``, ``double``). All operations on those
  types are not fully aligned (use the ``1`` variants in most cases, which mean no alignment
  whatsoever).

Function Pointer Issues
-----------------------

If you get an ``abort()`` from a function pointer call to ``nullFunc`` or ``b0`` or ``b1`` (possibly with an error message saying "incorrect function pointer"), the problem is that the function pointer was not found in the expected function pointer table when called.

.. note:: ``nullFunc`` is the function used to populate empty index entries in the function pointer tables (``b0`` and ``b1`` are shorter names used for ``nullFunc`` in more optimized builds).  A function pointer to an invalid index will call this function, which simply calls ``abort()``.

There are several possible causes:

- Your code is calling a function pointer that has been cast from another type (this is undefined behavior but it does happen in real-world code). In optimized Emscripten output, each function pointer type is stored in a separate table based on its original signature, so you *must* call a function pointer with that same signature to get the right behavior (see :ref:`portability-function-pointer-issues` in the code portability section for more information).
- Your code is calling a method on a ``NULL`` pointer or dereferencing 0. This sort of bug can be caused by any sort of coding error, but manifests as a function pointer error because the function can't be found in the expected table at runtime.


To debug these sorts of issues:

- Compile with ``-Werror`` (or otherwise fix warnings, many of which highlight undefined behavior).
- Use ``-sASSERTIONS=2`` to get some useful information about the function pointer being called, and its type.
- Look at the browser stack trace to see where the error occurs and which function should have been called.
- Enable clang warnings on dangerous function pointer casts using ``-Wcast-function-type``.
- Build with :ref:`SAFE_HEAP=1 <debugging-SAFE-HEAP>`.
- :ref:`Sanitizers` can help here, in particular UBSan.


Infinite loops
--------------

Infinite loops cause your page to hang. After a period the browser will notify the user that the page is stuck and offer to halt or close it.
If your code hits an infinite loop, one easy way to find the problem code is to use a *JavaScript profiler*. In the Firefox profiler, if the code enters an infinite loop you will see a block of code doing the same thing repeatedly near the end of the profile.
.. note:: The :ref:`emscripten-runtime-environment-main-loop` may need to be re-coded if your application uses an infinite main loop.

Useful Links
============

- `Links to Wasm debugging-related documents <https://web.dev/webassembly/#webassembly-debugging>`_


Need help?
==========

The :ref:`Emscripten Test Suite <emscripten-test-suite>` contains good examples of almost all functionality offered by Emscripten. If you have a problem, it is a good idea to search the suite to determine whether test code with similar behavior is able to run.

If you've tried the ideas here and you need more help, please :ref:`contact`.
