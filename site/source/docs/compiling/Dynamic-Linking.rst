.. _Dynamic-Linking:

===============
Dynamic Linking
===============

.. note:: This documentation is somewhat outdated and is in the process of being refreshed.

Emscripten supports linking object files (and ar archives that contain
object files) statically.  This lets most build systems work with Emscripten
with little or no changes (see :ref:`Building-Projects`).

In addition, Emscripten also has support for a form of **dynamic** linking of
WebAssembly modules.  This can add overhead, so for best performance static
linking should still be preferred.  However, this overhead can can be reduced
with the use of certain command line flags. See below for details.

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
separate code “modules” from your source code, and can link them at
runtime. The linking basically connects up the undefined symbols in
each module with the defined symbols in the others, in the simplest
of ways. It does not currently support some corner cases.

System libraries do utilize some more advanced linking features that
include such corner cases. For that reason, Emscripten tries to simplify
the problem as follows: There are two types of shared modules:

1. **Main modules**, which have system libraries linked in.
2. **Side modules**, which do not have system libraries linked in.

A project should contain **exactly one** main module. It can then be
linked at runtime to multiple side modules. This model also makes other
things simpler.  For example, only the singleton main module includes the
JavaScript environment and side modules are pure WebAssembly modules.

The one tricky aspect to this design is that a side module might depend on a
system library that the main module did not depend on. See the section on
system libraries, below, for how to handle that.

Note that the “main module” doesn’t need to contain the ``main()``
function. It could just as easily be in a side module. What makes the
main module the “main” module is that there is only one main module, and
only it has system libraries linked in.

(Note that system libraries are linked in to the main module
*statically*. We still have some optimizations from doing it that way,
even if we can’t dead code eliminate as well as we’d like.)

Practical Details
=================

If you want to jump to see running code, you can look in the test suite.
There are ``test_dylink_*`` tests that test dynamic linking in general, and
``test_dlfcn_*`` tests that test ``dlopen()`` specifically. Otherwise,
we describe the procedure now.

Load-time Dynamic Linking
-------------------------

Load-time dynamic linking refers to the case when the side modules are loaded
along with the main module, during startup and they are linked together
before your application starts to run.

-  Build one part of your code as the main module, linking it using
   ``-sMAIN_MODULE``.
-  Build other parts of your code as side modules, linking it using
   ``-sSIDE_MODULE``.

For the main module the output suffix should be ``.js`` (the WebAssembly
file will be generated alongside it just like normal).  For the side
module the output will be just a WebAssembly module we recommend the
output suffix ``.wasm`` or ``.so`` (which is the shared libraries suffix used by
UNIX systems).

In order to have the side modules loaded at startup you need to tell the
main module about their existence.  You can do this by specifying them on
the command line when you link the main module. e.g.

::

     emcc -sMAIN_MODULE main.c libsomething.wasm

At runtime, the JavaScript loading code will load ``libsomthing.wasm`` (along
with any other side modules) along with the main module before the application
starts to run.  The running application then can access code from any of the
modules linked together.

Runtime Dynamic Linking with ``dlopen()``
-----------------------------------------

Runtime dynamic linking can be performed by the calling the ``dlopen()``
function to load side modules after the program is already running. The
procedure begins in the same way, with the same flags used to build the main and
side modules.  The difference is that you do not specify the side modules on the
command line when linking the main module; instead, you must load the side
module into the filesystem, so that ``dlopen`` (or ``fopen``, etc.) can access
it (except for ``dlopen(NULL)`` which means to open the current executable,
which just works without filesystem integration). That’s basically it - you can
then use ``dlopen(), dlsym()``, etc. normally.

Code Size
=========

By default, main modules disable dead code elimination. That means that
all the code compiled remains in the output, including all system
libraries linked in, and also all the JS library code.

That is the default behavior since it is the least surprising. But it is
also possible to use normal dead code elimination, by building with
``-sMAIN_MODULE=2`` (instead of 1). In that mode, the main module is
built normally, with no special behavior for keeping code alive. It is
then your responsibility to make sure that code that side modules need
is kept alive. You can do this either by adding to ``EXPORTED_FUNCTIONS`` or
tagging the symbol ``EMSCRIPTEN_KEEPALIVE`` in the source code.
See ``other.test_minimal_dynamic`` for an example of this in action.

If you are doing load time dynamic linking then any symbols needed by
the side modules specified on the command line will be kept alive
automatically. For this reason we strongly recommend using ``MAIN_MODULE=2``
when doing load time dynamic linking.

There is also the corresponding ``-sSIDE_MODULE=2`` for side modules.

System Libraries
================

As mentioned earlier, system libraries are handled in a special way by the
Emscripten linker, and in dynamic linking, only the main module is linked
against system libraries. When linking the main module it is possible to pass
the side modules on the command line, in which case any system library
dependencies are automatically handled.

However when linking a main module without its side modules (Usually with
``-sMAIN_MODULE=1``) it is possible that required system libraries are not
included.  This section explains what to do to fix that by forcing the main
module to be linked against certain libraries.

You can build the main module with ``EMCC_FORCE_STDLIBS=1`` in the environment
to force inclusion of all standard libs.  A more refined approach is to name the
system libraries that you want to explicitly include.  For example, with
something like ``EMCC_FORCE_STDLIBS=libcxx,libcxxabi`` (if you need those two
libs).

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
``-sASSERTIONS`` can help some more.

Limitations
-----------

- Chromium does not support compiling >4kB WASM on the main thread, and that
  includes side modules; you can use ``--use-preload-plugins`` (in ``emcc`` or
  ``file_packager.py``) to make Emscripten compile them on startup
  `[doc] <https://emscripten.org/docs/porting/files/packaging_files.html#preloading-files>`__
  `[discuss] <https://groups.google.com/forum/#!topic/emscripten-discuss/cE3hUV3fDSw>`__.
- ``EM_ASM`` and ``EM_JS`` code defined within side modules depends on ``eval``
  support and are therefore incompatible with ``-sDYNAMIC_EXECUTION=0``.


Pthreads support
----------------

Dynamic linking + pthreads is is still experimental.  As such, linking with both
``MAIN_MODULE`` and ``-pthread`` will produce a warning.

While load-time dynamic linking works without any complications, runtime dynamic
linking via ``dlopen``/``dlsym`` can require some extra consideration.  The
reason for this is that keeping the indirection function pointer table in sync
between threads has to be done by emscripten library code.  Each time a new
library is loaded or a new symbol is requested via ``dlsym``, table slots can be
added and these changes need to be mirrored on every thread in the process.

Changes to the table are protected by a mutex, and before any thread returns
from ``dlopen`` or ``dlsym`` it will wait until all other threads are sync.  In
order to make this synchronization as seamless as possible, we hook into the
low level primitives of `emscripten_futex_wait` and `emscirpten_yield`.

For most use cases all this happens under hood and no special action is needed.
However, there there is one class of application that currently may require
modification.  If your applications busy waits, or directly uses the
``atomic.waitXX`` instructions (or the clang
``__builtin_wasm_memory_atomic_waitXX`` builtins) you maybe need to switch it
to use ``emscripten_futex_wait`` or order avoid deadlocks.  If you don't use
``emscripten_futex_wait`` while you block, you could potentially block other
threads that are calling ``dlopen`` and/or ``dlsym``.
