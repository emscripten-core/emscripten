.. _Best-Practices:

==============
Best Practices
==============

This guide provides recommendations and best practices for compiling C and C++
to the modern web using WebAssembly and Emscripten.

Following this guide when reporting bugs can also help speed up diagnosing and
fixing issues since you will be on the well-trodden path.

In some cases, Emscripten will guide you towards these best practices by
emitting warnings, but that is not always possible.


General Recommendations
=======================

- **Avoid high-frequency calls between WebAssembly and JavaScript**; for
  example, avoid per-pixel or per-sample calls in favor of operations that
  process entire memory regions or buffers.
- **Avoid shipping debug information in production**; see below for recommended
  release build flags.


Command-Line Style
==================

Keeping the command line flag usage minimal and consistent helps with codebase
understanding and avoids deviating from the well-lit path.

- **Don't pass settings that are enabled by default.** To keep command lines
  clean and concise, omit redundant options that are already default in modern
  Emscripten (such as ``-sWASM=1``).
- **Prefer standard compiler flags** over Emscripten-specific ones where
  possible (for example, prefer ``-pthread`` over ``-sUSE_PTHREADS`` and
  ``-m64`` over ``-sMEMORY64``).
- **Use simple comma-separated lists** for list-based settings (for example,
  ``-sEXPORTED_FUNCTIONS=_main,_malloc`` rather than JSON arrays like
  ``-sEXPORTED_FUNCTIONS=['_main','_malloc']``).
- **Avoid long lists on the command line**; use a response file (``@filename``)
  instead (for example, ``-sEXPORTED_FUNCTIONS=@exported_funcs.txt``).
- **Don't include the "=1" suffix for boolean flags.** For example, write
  ``-sSTRICT`` and ``-sALLOW_MEMORY_GROWTH`` rather than ``-sSTRICT=1`` or
  ``-sALLOW_MEMORY_GROWTH=1``.
- **Use separate compilation** by compiling ``.cpp`` sources to ``.o`` object
  files before linking rather than combining everything into a single monolithic
  compiler invocation.


Recommended Flags
=================

- ``-sSTRICT``: Opt into strict modern Emscripten behavior, disabling
  deprecated or legacy compatibility features.
- ``-sEXPORT_ES6``: Output a modern ES6 module (``module.mjs``). This option
  implies ``-sMODULARIZE`` so the generated code will be encapsulated and not
  impact the global namespace.
- ``-sENVIRONMENT=web``: Limit the runtime support to only the environments you
  are targeting. This reduces code size by, for example, omitting Node.js and
  compatibility code.
- ``-Werror -Wall``: Treat warnings as errors to catch C++ bugs and invalid
  compiler settings early.
- ``-O3``, ``-Os``, or ``-Oz``: For release builds, choose ``-O3`` when runtime
  performance is most critical, or ``-Os`` / ``-Oz`` when minimizing binary
  payload size is the priority.
- ``-flto``: Enable Link-Time Optimization (LTO) during both the compilation and
  linking steps of release builds for maximum runtime performance and size
  reduction.


Debug vs. Release Profiles
--------------------------

When configuring build profiles, keep compile and link flags consistent within
each configuration:

- **Release Builds:** Use ``-Oz`` or ``-Os`` (or ``-O3`` for CPU-bound tasks)
  combined with ``-flto``.  Add ``--closure=1`` to get minified JavaScript too,
  unless you plan to minify with an external tool.
- **Debug Builds:** Use ``-g`` when compiling and either ``-g``,
  ``-gline-tables-only``, or ``-gsource-map`` when linking. Avoid optimization
  flags (such as ``-O2`` or ``-O3``) or ``-flto`` during debug builds for
  faster compilation and accurate debugging.


Modern Web Workflows and Common Pitfalls
========================================

Asynchronous Code Execution and Main Thread Blocking
----------------------------------------------------

**Don't run long synchronous loops on the browser main thread.** The browser
uses cooperative multitasking; blocking the main UI thread prevents rendering
and freezes the web page.

- Restructure infinite loops to yield to the event loop using
  :c:func:`emscripten_set_main_loop`
  (see :ref:`emscripten-runtime-environment-howto-main-loop`).
- For synchronous-looking C++ code that must pause (or interact with
  asynchronous JavaScript APIs such as ``fetch()`` or Web Promises) without
  refactoring into callbacks, use :ref:`Asyncify <yielding_to_main_loop>`
  (``-sASYNCIFY``) or JavaScript Promise Integration (``-sJSPI``).
- Offload heavy compute or blocking operations to background workers using
  :doc:`multithreading and pthreads <../porting/pthreads>` (``-pthread``).

Virtual Filesystem and I/O
--------------------------

Standard C/C++ file operations (such as ``fopen`` or ``std::ifstream``) operate
on Emscripten's virtual in-memory filesystem (``MEMFS`` by default). See the
:ref:`file-system-overview` for an architectural overview.

- Do not assume direct access to the host file system.
- For small temporary files, ``MEMFS`` is sufficient.
- For persistent client-side data storage across browser sessions, use
  asynchronous storage backends such as :ref:`filesystem-api-idbfs` or the
  :ref:`Filesystem-API`.
