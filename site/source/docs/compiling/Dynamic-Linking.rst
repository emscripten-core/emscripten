.. _Dynamic-Linking:

===============
Dynamic Linking
===============

.. note:: Dynamic linking in emscripten is still somewhat experimental and subject to change.  This documentation is also curently somewhat outdated the in the process of being refreshed.

Emscripten supports linking object files (and ar archives that contain
object files) statically.  This lets most build systems work with Emscripten
with little or no changes (see :ref:`Building-Projects`).

In addition, Emscripten also has support for a form of **dynamic** linking of
WebAssembly modules.  This can add overhead, so for best performance static
linking should still be prefered.  However, this overhead can can be reduced
with the use of certain command line flags, see below for details.

Background
==========

Before we get to dynamic linking, let’s talk about static linking.
Emscripten’s linking model is a little different than most native
platforms. To understand it, consider that native linking models work in
a setting where the following facts are true:

1. The application runs directly on the local system, and has access to
   local system libraries, like C and C++ standard libraries, and
   others.
2. Code size is not a big concern. In part this is because the system
   libraries already exist on the system, so “hello world” in C++ can be
   small, even if it uses a large amount of iostream code in the C++
   standard library. But also, code size is perhaps a matter that
   influences cold startup times, in that more code takes longer to load
   from disk, but the cost is general not significant, and modern OSes
   mitigate it in various ways, like caching apps they expect to be
   loaded.

In Emscripten’s case, code is typically going to run on the web. That
means the following:

1. The application is running in a sandbox. It has no local system
   libraries to dynamically link to; it must ship its own system library
   code.
2. Code size is a major concern, as the application’s code is being
   downloaded over the internet, which is many orders of magnitude
   slower than an installed native app on one’s local machine.

For that reason, Emscripten automatically handles system libraries for
you and automatically does dead code elimination etc. to do the best
possible job it can at getting them small.

An additional factor here is that Emscripten has “js libraries” - system
libraries written in JavaScript. Such system libraries are the way we
access APIs on the web. It’s also a convenient way for people to connect
compiled code and handwritten code on the same page. This is another
reason for Emscripten to handle system libraries in a special way, and
in particular, in a way that lets it strip out as much of those js
libraries as it can, leaving only what is actually used, and again, that
works best in the context of statically linking a standalone app with no
external dependencies.

Overview of Dynamic Linking
===========================

Emscripten’s dynamic linking is fairly simple: you build several
separate code “modules” containing JavaScript, and can link them at
runtime. The linking basically connects up the unresolved symbols in
each module with the implemented symbols in the others, in the simplest
of ways. It does not currently support some corner cases.

System libraries do utilize some more advanced linking features that
include such corner cases. For that reason, Emscripten tries to simplify
the problem as follows: There are two types of shared modules:

1. **Main modules**, which have system libraries linked in.
2. **Side modules**, which do not have system libraries linked in.

A project should contain **exactly one** main module. It can then be
linked at runtime to multiple side modules. This model also makes other
things simpler, like only the singleton main module has the general
JavaScript environment setup code to connect to the web page and so
forth; side modules contain just the pure compiled WebAssembly and
nothing more.

The one tricky aspect to this design is that a side module might need a
system library that the main doesn’t know about. See the section on
system libraries, below, for how to handle that.

Note that the “main module” doesn’t need to contain the ``main()``
function. It could just as easily be in a side module. What makes the
main module the “main” module is just that there is only one main
module, and only it has system libs linked in.

(Note that system libraries are linked in to the main module
*statically*. We still have some optimizations from doing it that way,
even if we can’t dead code eliminate as well as we’d like.)

Practical Details
=================

If you want to jump to see running code, you can look in the test suite.
There are ``test_dylink_*`` tests that test general dynamic linking, and
``test_dlfcn_*`` tests that test ``dlopen()`` specifically. Otherwise,
we describe the procedure now.

General Dynamic Linking
-----------------------

-  Build one part of your code as the main module, using
   ``-s MAIN_MODULE=1``.
-  Build other parts of your code as side modules, using
   ``-s SIDE_MODULE=1``.

Important: since 1.38.16 you need to set ``-s EXPORT_ALL=1`` (for
SIDE_MODULEs, as well as the MAIN_MODULE if it exposes functions to the
modules). Alternatively, use ``-s EXPORTED_FUNCTIONS`` to declare the
exported functions. Without either of them, modules are useless.

Note that both should have suffix ``.js`` or ``.wasm`` (``emcc`` uses
suffixes to know what to emit). If you want, you can then rename the
side modules to ``.so`` or such (but it is just a superficial change.)

You then need to tell the main module to load the sides. You can do that
using the ``Module`` object, with something like

::

     Module.dynamicLibraries = ['libsomething.js'];

At runtime, when you run the main module, if it sees
``dynamicLibraries`` on ``Module``, then it loads them one by one and
links them. The running application then can access code from any of the
modules linked together.

``dlopen()`` Dynamic Linking
----------------------------

``dlopen()`` is slightly simpler than general dynamic linking. The
procedure begins in the same way, with the same flags used to build the
main and side modules. The difference is that you do not use
``Module.dynamicLibraries``; instead, you must load the side module into
the filesystem, so that ``dlopen`` (or ``fopen``, etc.) can access it
(except for ``dlopen(NULL)`` which means to open the current executable,
which just works without filesystem integration). That’s basically it -
you can then use ``dlopen(), dlsym()``, etc. normally.

System Libraries
================

As mentioned earlier, system libraries are handled in a special way by
the Emscripten linker, and in dynamic linking, only the main module is
linked against system libraries. A possible issue is if a side module
needs a system library that the main does not. If so, you’ll get a
runtime error. This section explains what to do to fix that.

To get around this, you can build the main module with
``EMCC_FORCE_STDLIBS=1`` in the environment to force inclusion of all
standard libs. A more refined approach is to build the side module with
``-v`` in order to see which system libs are actually needed - look for
``including lib[...]`` messages - and then building the main module with
something like ``EMCC_FORCE_STDLIBS=libcxx,libcxxabi`` (if you need
those two libs).

Code Size
=========

By default, main modules disable dead code elimination. That means that
all the code compiled remains in the output, including all system
libraries linked in, and also all the JS library code.

That is the default behavior since it is the least surprising. But it is
also possible to use normal dead code elimination, by building with
``-s MAIN_MODULE=2`` (instead of 1). In that mode, the main module is
built normally, with no special behavior for keeping code alive. It is
then your responsibility to make sure that code that side modules need
is kept alive. You can do this in the usual ways, like adding to
``EXPORTED_FUNCTIONS``. See ``other.test_minimal_dynamic`` for an
example of this in action. There is also the corresponding
``-s SIDE_MODULE=2`` for side modules.

Miscellaneous Notes
===================

Dynamic Checks
--------------

Native linkers generally only run code when all symbols are resolved.
Emscripten’s dynamic linker hooks up symbols to unresolved references to
those symbols **dynamically**. As a result, we don’t check if any
symbols remain unresolved, and code can start to run even if there are.
It will run successfully if they are not called in practice. If they
are, you will get a runtime error. What went wrong should be clear from
the stack trace (in an unminified build); building with
``-s ASSERTIONS=1`` can help some more.

Limitations
-----------

-  Chromium does not support compiling >4kB WASM on the main thread, and
   that includes side modules; you can use ``--use-preload-plugins`` (in
   ``emcc`` or ``file_packager.py``) to make Emscripten compile them on
   startup
   `[doc] <https://emscripten.org/docs/porting/files/packaging_files.html#preloading-files>`__
   `[discuss] <https://groups.google.com/forum/#!topic/emscripten-discuss/cE3hUV3fDSw>`__.
-  See also `webAssembly standalone <https://github.com/emscripten-core/emscripten/wiki/WebAssembly-Standalone>`_` for more on side modules in this context.

Pthreads support
----------------

Dynamic linking + pthreads is not supported. It would require new wasm
spec features (a way to share the Table), or some serious workarounds in
the toolchain.
