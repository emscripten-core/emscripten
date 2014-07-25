.. _LLVM-Backend:

==========================
LLVM Backend (wiki-import)
==========================
.. note:: This article was migrated from the wiki (Fri, 25 Jul 2014 04:21) and is now the "master copy" (the version in the wiki will be deleted). It may not be a perfect rendering of the original but we hope to fix that soon!

LLVM Backend, aka "fastcomp"
============================

**Fastcomp** is a new compiler core for emscripten, replacing much of
the original compiler core in ``src/*.js``. This replaces **only** the
JS compiler itself, not the toolchain code nor library code
(``src/library*.js``) nor JS optimizer code. (For context, the core
compiler is a few thousand lines of code, to be replaced with a few
other thousand lines of code, whereas all the other stuff not being
replaced is far larger.)

**Fastcomp was turned on by default in version 1.12.1.** If you are
using that version or later, you are using fastcomp unless you manually
disable it (which is highly unrecommended).

See the FAQ at the bottom of this page if you are having problems.

Overview of Fastcomp
--------------------

Fastcomp is an **LLVM backend**. It is **not** in upstream LLVM yet, it
is far too new for that (but hopefully eventually will be). So you need
to use the emscripten fork of LLVM. You can either build it from source,
or get it as part of the emscripten SDK.

This means that if you use another build of LLVM - like an older one you
built yourself, or one from your linux distro's repos, etc. - it will
**not** contain fastcomp. Emscripten will give an error about this (you
can manually run those checks with ``emcc -v``), and briefly explain the
issue and link to this page (where, later down, you can see how to
disable fastcomp).

Getting Fastcomp
----------------

To use fastcomp, you need both emscripten (see the :ref:`Tutorial`) and the
emscripten LLVM code, either from the SDK or from source. Instructions
from source are as follows:

-  Clone the fastcomp LLVM repository:
   https://github.com/kripken/emscripten-fastcomp (doesn't matter where
   you do this, can be anywhere on your machine; you'll point to that
   directory when you edit ``~/.emscripten``, see later down)
-  Enter ``tools`` and clone the fastcomp Clang repository:
   https://github.com/kripken/emscripten-fastcomp-clang Note: **you must
   clone it into a dir named "clang"**, so that clang is present in
   ``tools/clang``! Use something like ``git clone ..repo.. clang``.
   (Another note: **this repo has changed**, earlier in fastcomp
   development we used another one, so make sure you are using this one
   now.)
-  Build it:
-  ``cd ..`` to get back to the root of the llvm checkout
-  ``mkdir build`` and then ``cd build`` (it is better to build into a
   separate build dir)
-  ``../configure --enable-optimized --disable-assertions --enable-targets=host,js``
-  (Alternatively, you can use CMake instead of configure:
   ``cmake .. -DCMAKE_BUILD_TYPE=Release -DLLVM_TARGETS_TO_BUILD="X86;JSBackend" -DLLVM_INCLUDE_EXAMPLES=OFF``
   ``-DLLVM_INCLUDE_TESTS=OFF -DCLANG_INCLUDE_EXAMPLES=OFF -DCLANG_INCLUDE_TESTS=OFF``,
   replace X86 if you are on something else.)
-  ``make -j 4`` (or whatever number of cores you want to use)
-  Set it up in ~/.emscripten (set the path to the llvm checkout +
   something like ``/build/Release/bin`` as LLVM\_ROOT, look for where
   the ``clang`` binary shows up under ``build/``)

Branches
~~~~~~~~

You should use the **same** branch in all 3 repos: emscripten,
emscripten-fastcomp and emscripten-fastcomp-clang (that is, in
emscripten, in emscripten's LLVM fork and in emscripten's clang fork).
That is, you can either use the master branch in all 3, or the incoming
branch in all 3. If you don't use the same branch in all 3, you may run
into errors (changes might land in emscripten incoming for example that
will not work on the master branches of the other two).

Run ``emcc -v`` to check if the branches are synchronized. Note that
this checks the code in the repos, not the build - make sure you rebuilt
LLVM+clang on the latest code you checked out.

Version numbers
^^^^^^^^^^^^^^^

Bisecting across multiple git trees can be hard. We use version numbers
to synchronize points between them, which helps.

-  tools/shared.py in emscripten
-  emscripten-version.txt in fastcomp (llvm)
-  emscripten-version.txt in fastcomp-clang (clang)

Version numbers are typically X.Y.Z where X is a major number (changes
very rarely), Y is a release number (changes each time we merge incoming
to master, so these numbers indicate points where all tests passed) and
Z is minor update that is just a sync point between the repos, or is
needed when libc changes in emscripten (version changes clear the
cache).

Compilation Notes
~~~~~~~~~~~~~~~~~

-  If you are building a large project, you will need a 64-bit build of
   llvm+clang, as compiling and optimizing can take more memory than a
   32-bit build can use.

-  To build 64 bit using cmake and visual studio, use the -G "Visual
   Studio 10 Win64" directive. Note: VS 11/12 don't work yet.

-  If you want to build with MinGW instead and have that in path,
   replace -G directive in above with "-G MinGW Makefiles", and run
   mingw32-make to build (not tested yet).

Backend code structure
----------------------

The backend is in the repo linked to above, and code is in
``lib/Target/JSBackend/``. The main file is ``JSBackend.cpp`` but the
the other files in that directory are important too.

Why did this change happen?
---------------------------

Fastcomp is much more streamlined than the original compiler - the
original compiler supports dozens of various code generation modes (no
typed arrays, typed arrays in various modes, asm.js vs non-asm.js,
etc.). Fastcomp on the other hand is directly focused on asm.js code
generation, which has proven to give the best results.

Fastcomp, as a C++ LLVM backend, is much faster than the original JS
compiler, often 4x faster or more. It also requires much less memory and
avoids unpredictable pathological compiler slowdowns that the old
compiler had.

Fastcomp also generates better code - by being an LLVM backend, it can
integrate more tightly with LLVM.

Are there downsides?
~~~~~~~~~~~~~~~~~~~~

The main downside is that Emscripten can no longer use a stock build of
LLVM, because we have changes that must be built with LLVM.

Note that you actually **can** still use a stock build, but only because
you can make emscripten use the original compiler (see next section) -
but this is not good, because you miss out on the benefits of the
backend (see above), and also you are running a code path that is not
recommended and less tested.

This will hopefully be a temporary issue because the new Emscripten
backend might get upstreamed to LLVM eventually, in which case a stock
build would contain it.

Status of Original Compiler (and how to disable fastcomp)
---------------------------------------------------------

You should **NOT** disable fastcomp. But, if you really, really must,
you should know that you will have the following downsides:

-  Not taking advantage of the benefits of fastcomp (much faster
   compilation, better generated code).
-  Using the old compiler which is deprecated and consequently less
   tested.

The original compiler is still present, and you may want to use it if
you need a feature not present in fastcomp. There should be very few
such features, as everything not deprecated or planned to be rewritten
has already been ported. However, if you do need one of those features,
you can use the old compiler, by building with

::

    EMCC_FAST_COMPILER=0 emcc [..]

so that ``EMCC_FAST_COMPILER`` is set in the environment to ``0``. This
will turn off fastcomp.

When you want to use fastcomp, you must be using a build from the
fastcomp repos (see below), so that the backend is present. When you
disable fastcomp on the other hand, you can use **either** a build from
the fastcomp repos, **or** a stock LLVM build. The latter is less
tested, but should work in principle: Disabling fastcomp does not use
anything new in the fastcomp repo (neither the new backend, nor the new
target triple).

You can check if fastcomp is on or off by looking at debug output. For
example, run ``EMCC_DEBUG=1 emcc tests/hello_world.c`` and if fastcomp
is on, then among the output will be

::

    DEBUG    root: emscript: llvm backend: ...
    DEBUG    root:   emscript: llvm backend took

That shows both the command used to run the backend, and how much time
it took. If fastcomp is off on the other hand, the old compiler is used,
and you will instead

::

    DEBUG    root: emscript: ll=>js
    DEBUG    root:   emscript: scan took ...
    ...
    DEBUG    root: emcc step "emscript (llvm=>js)" took ...

This shows that the old compiler (``ll=>js``) is called, as well as how
much time each step takes, and the total time. Again, this is the output
for the **old** compiler, so hopefully you will never see it :)

Some features not present in fastcomp are:

-  Various deprecated settings.js options (e.g. FORCE\_ALIGNMENT,
   HEAP\_INIT, etc.) have no effect. You should receive a compile-time
   error if you use a setting which is not yet supported, if it has not
   been missed.
-  :ref:`Linking` of asm.js shared modules (note that normal static linking
   as used by almost all projects works fine, it is just specifically
   the options MAIN\_MODULE and SIDE\_MODULE that do not work). This is
   not deprecated, but may need to be partially reconsidered, so it has
   not been ported to fastcomp.

FAQ
===

-  I see ``WARNING: Linking two modules of different target triples``
   [..] ``'asmjs-unknown-emscripten' and 'le32-unknown-nacl'``..?
-  You are linking together bitcode files compiled with the old compiler
   (or older versions of fastcomp) with bitcode files from the new one.
   This may work in some cases but is dangerous and should be avoided.
   To fix it, just recompile all your bitcode with the new compiler.

