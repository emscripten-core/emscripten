.. _FAQ:

===
FAQ
===

This FAQ contains answers to many questions asked on IRC and the mailing list.


How do I compile code?
======================

See the :ref:`Emscripten Tutorial <Tutorial>` and :ref:`emcc <emccdoc>`.


Why do I get errors building basic code and the tests?
======================================================

All the tests in the :ref:`Emscripten test suite <emscripten-test-suite>` are
known to build and pass on our test infrastructure, so if you see failures
locally it is likely that there is some problem with your environment. (Rarely,
there may be temporary breakage, but never on a tagged release version.)

First call ``emcc --check``, which runs basic sanity checks and prints out
useful environment information. If that doesn't help, follow the instructions in
:ref:`verifying-the-emscripten-environment`.

You might also want to go through the :ref:`Tutorial` again, as it is updated as
Emscripten changes.

Also make sure that you have the necessary requirements for running Emscripten
as specified in the :ref:`SDK <sdk-download-and-install>` section, including
new-enough versions of the dependencies.


I tried something: why doesn’t it work?
=======================================

Some general steps that might help figure things out:

 * See if the problem happens without optimizations (`-O0`, or not specifying
   any optimization level). Without optimizations, emscripten enables many
   assertions at compile and runtime, which may catch a problem and display an
   error message with a suggestion for how to fix it.
 * Search the documentation on this site.
 * Check if there is a test for the failing functionality in the
   :ref:`Emscripten test suite <emscripten-test-suite>` (run ``grep -r`` in
   **test/**). They should all pass (with only rare exceptions), so they
   provide concrete "known-good" examples of how various options and code are
   used.


Do I need to change my build system to use Emscripten?
======================================================

In most cases you will be able to use your project's current build system with
Emscripten. See :ref:`Building-Projects`.


Why is code compilation slow?
=============================

Emscripten makes some trade-offs that make the generated code faster and
smaller, at the cost of longer link times. For example, we build parts of
the standard library with `-flto` (Link Time Optimization), which enables some
additional optimizations, but can take longer to build.  We also (in optimized
builds) run the Binaryen optimizer on the entire output even without LTO.

.. note:: You can determine what compilation steps take longest by compiling
   with ``EMCC_DEBUG=1`` in the environment and then reviewing the debug logs
   (by default in ``/tmp/emscripten_temp``). Note that compiling in debug mode
   takes longer than normal, because we print out a lot of intermediate steps to
   disk, so it's useful for debugging but not for actual compiling.

The main tips for improving build time are:

- Use ``-O0`` for fast iteration builds. You can still compile with higher
  optimization levels, but specifying ``-O0`` during link will make the link
  step much faster.

- Compile on a machine with more cores:

  - For compiling your source files, use a parallel build system (for example,
    in ``make`` you can do something like ``make -j8`` to run using 8 cores).
  - For the link step, Emscripten can run some optimizations in parallel
    (specifically, Binaryen optimizations for Wasm, and our JavaScript
    optimizations). Increasing the number of cores results in an almost linear
    improvement. Emscripten will automatically use more cores if they are
    available, but you can control that with ``EMCC_CORES=N`` in the environment
    (which is useful if you have many cores but relatively less memory).


Why does my code run slowly?
============================

Make sure you optimize code by building with ``-O2`` (even more :ref:`aggressive
optimization <emcc-O3>` is available, at the cost of significantly increased
compilation time).

.. note: This is necessary both when compiling each source file, and at link
   time, which is when Emscripten applies many of its optimizations.  For more
   information see :ref:`Building-Projects` and :ref:`Optimizing-Code`.


Why is my compiled code big?
============================

Make sure you build with ``-O3`` or ``-Os`` so code is fully optimized and
minified. You should use the closure compiler, gzip compression on your
webserver, etc., see the :ref:`section on code size in Optimizing code
<optimizing-code-size>`.


Why does compiling code that works on another machine gives me errors?
======================================================================

Make sure you are using the Emscripten bundled system headers. Using :ref:`emcc
<emccdoc>` will do so by default, but problems may occur if you use your local
system headers with ``emcc``.


How can I reduce startup time?
==============================

Make sure that you are running an :ref:`optimized build <Optimizing-Code>`
(smaller builds are faster to start up).

Network latency is also a possible factor in startup time. Consider putting the
file loading code in a separate script element from the generated code so that
the browser can start the network download in parallel to starting up the
codebase (run the :ref:`file packager <packaging-files>` and put file loading
code in one script element, and the generated codebase in a later script
element).


.. _faq-local-webserver:

How do I run a local webserver for testing / why does my program stall in "Downloading..." or "Preparing..."?
=============================================================================================================

That error can happen when loading the page using a ``file://`` URL, which works
in some browsers but not in others. Instead, it's best
to use a local webserver. For example, Python has one built in,
``python -m http.server`` in Python 3 or ``python -m SimpleHTTPServer``
in Python 2. After doing that, you can visit ``http://localhost:8000/``. You can
also use ``emrun FILENAME.html`` (which will run a python webserver for you).

When doing quick local testing, another option than a local webserver is to
bundle everything into a single file, using ``-sSINGLE_FILE`` (as then no XHRs
will be made to ``file://`` URLs).

Otherwise, to debug this, look for an error reported on the page itself, or in
the browser devtools (web console and network tab), or in your webserver's
logging.


Why do I get ``machine type must be wasm32`` or ``unknown file type`` during linking?
=====================================================================================

This means that one or more of this linker input files were not build by
Emscripten (or, more-specifically, not built for the correct target architecture).

Most often the file in question will be an ELF file or Mach-O file built for the
host machine.  You can run the ``file`` command-line utility to see what they
actually contain.

Common issues are:

* Attempting to link against libraries built for the host system.  For example,
  if you have something like ``-L/usr/lib`` in your link command that is almost
  always going to cause these errors since the libraries that exist in those
  system directories are almost certainly not built with/for Emscripten.
  This solution is to use Emscripten to build all the libraries that you depend
  on, and never use host libraries.
* Some libraries or object files in your project were built using the host
  compiler rather then the emscripten compiler.  If you are using autoconf
  or cmake make sure you use the emconfigure/emmake wrapper, see
  :ref:`Building-Projects`.
* LLVM IR from the old backend, if you built the project with a version before
  1.39.0 (which used the old backend by default), and are doing an incremental
  rebuild now. To fix that, do a complete rebuild from scratch of all your
  project's files, including libraries (this error often happens if you have
  prebuilt libraries from a third party; those must be recompiled too with the
  new backend).


Why does my code fail to compile with an error message about inline assembly (or ``{"text":"asm"}``)?
=====================================================================================================

Emscripten cannot compile inline assembly code (unless that assembly code
is specifically written to target WebAssembly).

You will need to find where inline assembly is used, and disable it or replace
it with platform-independent code.


.. _faq-my-html-app-hangs:

Why does my HTML app hang?
==========================

The browser event model uses *co-operative multitasking* — each event has a
"turn" to run, and must then return control to the browser event loop so that
other events can be processed. A common cause of HTML pages hanging is
JavaScript that does not complete and return control to the browser.

Graphical C++ apps typically have an infinite main loop in which event handling,
processing and rendering is done, followed by a delay to keep the frame-rate
right (``SDL_DELAY`` in :term:`SDL` apps). As the main loop does not complete
(is infinite) it cannot return control to the browser, and the app will hang.

Apps that use an infinite main loop should be re-coded to put the actions for a
single iteration of the loop into a single "finite" function. In the native
build this function can be run in an infinite loop as before. In the Emscripten
build it is set as the :ref:`main loop function <faq-how-run-event-loop>` and
will be called by the browser at a specified frequency.

There is more information on this topic in :ref:`emscripten-runtime-environment`.


.. _faq-how-run-event-loop:

How do I run an event loop?
===========================

To run a C function repeatedly, use :c:func:`emscripten_set_main_loop` (this is
discussed in :ref:`emscripten-runtime-environment`). The related functions in
:ref:`emscripten.h <emscripten-h-browser-execution-environment>` are also
useful, allowing you to add events that block the main loop, etc.

To respond to browser events use the SDL API in the normal way. There are
examples in the SDL tests (search for SDL in **test/runner.py**).

See also: :ref:`faq-my-html-app-hangs`


Why doesn't my SDL app work?
=============================

See the :term:`SDL` automatic tests for working examples: ``test/runner.py browser``.


How do I link against system libraries like SDL, boost, etc.?
=============================================================

System libraries that are included with Emscripten are automatically linked when
you compile (just the necessary parts). This includes *libc*, *libc++* (C++
standard library) and :term:`SDL`.

Libraries not included with Emscripten (like Boost) must be compiled and linked
with the program just as if they were a module in the project.

There is a set of libraries ported to Emscripten for convenient use, Emscripten
Ports. See :ref:`Building-Projects`

Another option is to implement needed C APIs as JavaScript libraries (see
``--js-library`` in :ref:`emcc <emcc-js-library>` and
:ref:`implement-c-in-javascript`). Emscripten itself does this for *libc* (not
including *malloc*) and :term:`SDL` (but not *libc++* or *malloc*).

.. note::

  - Unlike other compilers, you don't need ``-lSDL`` to include SDL (specifying
    it will do no harm).
  - In the specific case of *Boost*, if you only need the boost headers then you
    don't need to compile anything.


What are my options for audio playback?
=======================================

Emscripten has partial support for SDL1 and 2 audio, and OpenAL.

To use SDL1 audio, include it as ``#include <SDL/SDL_mixer.h>``. You can use it
that way alongside SDL1, SDL2, or another library for platform integration.

To use SDL2 audio, include it as ``#include <SDL2/SDL_mixer.h>`` and use
`-sUSE_SDL_MIXER=2`.  Format support is currently limited to OGG, WAV, MID, and
MOD.


How can my compiled program access files?
=========================================

Emscripten uses a virtual file system that may be preloaded with data or linked
to URLs for lazy loading. See the :ref:`file-system-overview` for more details.


Why can't my code access a file in the same directory?
======================================================

Emscripten-generated code running *in the browser* cannot access files in the
local file system. Instead you can use :ref:`preloading <emcc-preload-file>` and
:ref:`embedding <emcc-embed-file>` to work around the lack of synchronous file
IO. See :ref:`file-system-overview` for more information.

It is possible to allow access to local file system for code running in
*node.js*, use the :ref:`NODEFS <filesystem-api-nodefs>` filesystem option.


.. _faq-when-safe-to-call-compiled-functions:

How can I tell when the page is fully loaded and it is safe to call compiled functions?
=======================================================================================

(You may need this answer if you see an error saying something like ``native
function `x` called before runtime initialization``, which is a check enabled in
``ASSERTIONS`` builds.)

Calling a compiled function before a page has fully loaded can result in an
error, if the function relies on files that may not be present (for example
:ref:`preloaded <emcc-preload-file>` files are loaded asynchronously, and
therefore if you just place some JS that calls compiled code in a ``--post-js``,
that code will be called synchronously at the end of the combined JS file,
potentially before the asynchronous event happens, which is bad).

The easiest way to find out when loading is complete is to add a ``main()``
function, and within it call a JavaScript function to notify your code that
loading is complete.

.. note:: The ``main()`` function is called after startup is complete as a
   signal that it is safe to call any compiled method.

For example, if ``allReady()`` is a JavaScript function you want called when
everything is ready, you can do:

::

  #include <emscripten.h>

  int main() {
    EM_ASM( allReady() );
  }

Another option is to define an ``onRuntimeInitialized`` function,

::

  Module['onRuntimeInitialized'] = function() { ... };

That method will be called when the runtime is ready and it is ok for you to
call compiled code. In practice, that is exactly the same time at which
``main()`` would be called, so ``onRuntimeInitialized`` doesn't let you do
anything new, but you can set it from JavaScript at runtime in a flexible way.

Here is an example of how to use it:

::

    <script type="text/javascript">
      var Module = {
        onRuntimeInitialized: function() {
          Module._foobar(); // foobar was exported
        }
      };
    </script>
    <script type="text/javascript" src="my_project.js"></script>

The crucial thing is that ``Module`` exists, and has the property
``onRuntimeInitialized``, before the script containing emscripten output
(``my_project.js`` in this example) is loaded.

Another option is to use the ``MODULARIZE`` option, using ``-sMODULARIZE``.
That puts all of the generated JavaScript into a factory function, which you can
call to create an instance of your module. The factory function returns a
Promise that resolves with the module instance. The promise is resolved once
it's safe to call the compiled code, i.e. after the compiled code has been
downloaded and instantiated. For example, if you build with ``-sMODULARIZE -s
'EXPORT_NAME="createMyModule"'``, then you can do this:

::

    createMyModule(/* optional default settings */).then(function(Module) {
      // this is reached when everything is ready, and you can call methods on Module
    });

Note that in ``MODULARIZE`` mode we do not look for a global Module object for
default values. Default values must be passed as a parameter to the factory
function.  (see details in settings.js)


.. _faq-NO_EXIT_RUNTIME:

What does "exiting the runtime" mean? Why don't ``atexit()s`` run?
==================================================================

(You may need this answer if you see an error saying something like ``atexit()
called, but EXIT_RUNTIME is not set`` or ``stdio streams had content in them
that was not flushed. you should set EXIT_RUNTIME to 1``.)

By default Emscripten sets ``EXIT_RUNTIME=0``, which means that we don't include
code to shut down the runtime. That means that when ``main()`` exits, we don't
flush the stdio streams, or call the destructors of global C++ objects, or call
``atexit`` callbacks. This lets us emit smaller code by default, and is normally
what you want on the web: even though ``main()`` exited, you may have something
asynchronous happening later that you want to execute.

In some cases, though, you may want a more "commandline" experience, where we do
shut down the runtime when ``main()`` exits. You can build with ``-sEXIT_RUNTIME``,
and then we will call ``atexits`` and so forth. When you build
with ``ASSERTIONS``, you should get a warning when you need this. For example,
if your program prints something without a newline,

::

  #include <stdio.h>

  int main() {
    printf("hello"); // note no newline
  }

If we don't shut down the runtime and flush the stdio streams, "hello" won't be
printed. In an ``ASSERTIONS`` build you'll get a notification saying ``stdio
streams had content in them that was not flushed. you should set EXIT_RUNTIME to
1``.


.. _faq-dead-code-elimination:

Why do functions in my C/C++ source code vanish when I compile to WebAssembly?
==============================================================================

Emscripten does dead code elimination of functions that are not called from the
compiled code. While this does minimize code size, it can remove functions that
you plan to call yourself (outside of the compiled code).

To make sure a C function remains available to be called from normal JavaScript,
it must be added to the `EXPORTED_FUNCTIONS
<https://github.com/emscripten-core/emscripten/blob/1.29.12/src/settings.js#L388>`_
using the *emcc* command line. For example, to prevent functions ``my_func()``
and ``main()`` from being removed/renamed, run *emcc* with: ::

  emcc -sEXPORTED_FUNCTIONS=_main,_my_func  ...

.. note:: `_main` should be in the export list, as in that example, if you have
   a `main()` function. Otherwise, it will be removed as dead code; there is no
   special logic to keep `main()` alive by default.

.. note:: `EXPORTED_FUNCTIONS` affects compilation to JavaScript. If you first
   compile to an object file, then compile the object to JavaScript, you need
   that option on the second command.

If your function is used in other functions, LLVM may inline it and it will not
appear as a unique function in the JavaScript. Prevent inlining by defining the
function with :c:type:`EMSCRIPTEN_KEEPALIVE`: ::

  void EMSCRIPTEN_KEEPALIVE yourCfunc() {..}

`EMSCRIPTEN_KEEPALIVE` also exports the function, as if it were on
`EXPORTED_FUNCTIONS`.

.. note::

  - All functions not kept alive through ``EXPORTED_FUNCTIONS`` or
    :c:type:`EMSCRIPTEN_KEEPALIVE` will potentially be removed. Make sure you
    keep the things you need alive using one or both of those methods.

  - Exported functions need to be C functions (to avoid C++ name mangling).

  - Decorating your code with :c:type:`EMSCRIPTEN_KEEPALIVE` can be useful if
    you don't want to have to keep track of functions to export explicitly, and
    when these exports do not change. It is not necessarily suitable for
    exporting functions from other libraries — for example it is not a good idea
    to decorate and recompile the source code of the C standard library. If you
    build the same source in multiple ways and change what is exported, then
    managing exports on the command line is easier.

  - Running *emcc* with ``-sLINKABLE`` will also disable link-time
    optimizations and dead code elimination. This is not recommended as it makes
    the code larger and less optimized.

Another possible cause of missing code is improper linking of ``.a`` files. The
``.a`` files link only the internal object files needed by previous files on the
command line, so the order of files matters, and this can be surprising. If you
are linking ``.a`` files, make sure they are at the end of the list of files,
and in the right order amongst themselves. Alternatively, just use ``.so`` files
instead in your project.

.. tip:: It can be useful to compile with ``EMCC_DEBUG=1`` set for the
   environment (``EMCC_DEBUG=1 emcc ...`` on Linux, ``set EMCC_DEBUG=1`` on
   Windows). This splits up the compilation steps and saves them in
   ``/tmp/emscripten_temp``. You can then see at what stage the code vanishes
   (you will need to do ``llvm-dis`` on the bitcode  stages to read them, or
   ``llvm-nm``, etc.).


Why is the File System API is not available when I build with closure?
======================================================================

The :term:`Closure Compiler` will minify the File Server API code. Code that
uses the file system must be optimized **with** the File System API, using
emcc's ``--pre-js`` :ref:`option <emcc-pre-js>`.


Why does my code break and gives odd errors when using ``-O2 --closure 1``?
===========================================================================

The :term:`Closure Compiler` minifies variable names, which results in very
short variable names like ``i``, ``j``, ``xa``, etc. If other code declares
variables with the same names in global scope, this can cause serious problems.

This is likely to be the cause if you can successfully run code compiled with
``-O2`` set and ``--closure`` unset.

One solution is to stop using small variable names in the global scope (often
this is a mistake — forgetting to use ``var`` when assigning to a variable).

Another alternative is to wrap the generated code (or your other code) in a
closure, as shown:

::

  var CompiledModule = (function() {
    .. GENERATED CODE ..
    return Module;
    })();


.. _faq-export-stuff:

Why do I get ``TypeError: Module.someThing is not a function``?
===============================================================

The ``Module`` object will contain exported methods. For something to appear
there, you should add it to ``EXPORTED_FUNCTIONS`` for compiled code, or
``EXPORTED_RUNTIME_METHODS`` for a runtime method (like ``getValue``). For
example,

 ::

  emcc -sEXPORTED_FUNCTIONS=_main,_my_func ...

would export a C method ``my_func`` (in addition to ``main``, in this example). And

 ::

  emcc -sEXPORTED_RUNTIME_METHODS=ccall ...

will export ``ccall``. In both cases you can then access the exported function on the ``Module`` object.

.. note:: You can use runtime methods directly, without exporting them, if the
   compiler can see them used. For example, you can use ``getValue`` in
   ``EM_ASM`` code, or a ``--pre-js``, by calling it directly. The optimizer
   will not remove that JS runtime method because it sees it is used. You only
   need to use ``Module.getValue`` if you want to call that method from outside
   the JS code the compiler can see, and then you need to export it.

.. note:: Emscripten used to export many runtime methods by default. This
   increased code size, and for that reason we've changed that default. If you
   depend on something that used to be exported, you should see a warning
   pointing you to the solution, in an unoptimized build, or a build with
   ``ASSERTIONS`` enabled, which we hope will minimize any annoyance. See
   ``ChangeLog.md`` for details.


.. _faq-runtime-change:

Why does ``Runtime`` no longer exist? Why do I get an error trying to access ``Runtime.someThing``?
===================================================================================================

1.37.27 includes a refactoring to remove the ``Runtime`` object. This makes the
generated code more efficient and compact, but requires minor changes if you
used ``Runtime.*`` APIs. You just need to remove the ``Runtime.`` prefix, as
those functions are now simple functions in the top scope (an error message in
``-O0`` or builds with assertions enabled with suggest this). In other words,
replace

 ::

  x = Runtime.stackAlloc(10);

with

 ::

  x = stackAlloc(10);

.. note:: The above will work for code in a ``--pre-js`` or JS library, that is,
   code that is compiled together with the emscripten output. If you try to
   access ``Runtime.*`` methods from outside the compiled code, then you must
   export that function (using ``EXPORTED_RUNTIME_METHODS``), and use it on the
   Module object, see :ref:`that FAQ entry<faq-export-stuff>`.


Why do I get a ``NameError`` or ``a problem occurred in evaluating content after a "-s"`` when I use a ``-s`` option?
=====================================================================================================================

This can occur if you have non-trivial strings in ``-s`` argument and are having
trouble getting the shell quoting / escaping correct.

Using the simpler list form (without quotes, spaces or square brackets) can
sometimes help:

::

  emcc a.c -sEXPORTED_RUNTIME_METHODS=foo,bar

It is also possible to use a **response file**, that is,

::

  emcc a.c -sEXPORTED_RUNTIME_METHODS=@extra.txt

with ``extra.txt`` being a plain text file that contains ``foo`` and ``bar`` on
separate lines.

How do I specify ``-s`` options in a CMake project?
===================================================

Simple things like this should just work in a ``CMakeLists.txt`` file:

::

  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -sUSE_SDL=2")

However, some ``-s`` options may require quoting, or the space between ``-s``
and the next argument may confuse CMake, when using things like
``target_link_options``. To avoid those problems, you can use ``-sX=Y``
notation, that is, without spaces and without square brackets or quotes:

::

  # same as before but no space after -s
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -sUSE_SDL=2")
  # example of target_link_options with a list of names
  target_link_options(example PRIVATE "-sEXPORTED_FUNCTIONS=_main")

Note also that ``_main`` does not need to be quoted, even though it's a string
name (``emcc`` knows that the argument to ``EXPORTED_FUNCTIONS`` is a list of
strings, so it accepts ``a`` or ``a,b`` etc.).


Why do I get a Python ``SyntaxError: invalid syntax`` on ``file=..`` or on a string starting with ``f'..'``?
============================================================================================================

Emscripten requires a recent-enough version of Python. An older Python version,
like ``2.*``, will not support the print statement by default, so it will error on
syntax like ``print('..', file=..)``. And an older ``3.*`` Python may not support
f-strings, which look like ``f'..'``.

Make sure that you have a new enough version of Python installed, as specified
in the :ref:`SDK <sdk-download-and-install>` instructions, and that it is used by emcc (for example by
running ``emcc.py`` using that Python).

In a CI environment you may need to specify the Python version to use, if the
default is not new enough. For example,
`on Netlify <https://github.com/emscripten-core/emscripten/issues/12896>`_
you can use ``PYTHON_VERSION``.


Why do I get a stack size error when optimizing: ``RangeError: Maximum call stack size exceeded`` or similar?
=============================================================================================================

You may need to increase the stack size for :term:`node.js`.

On Linux and Mac macOS, you can just do ``NODE_JS = ['/path/to/node',
'--stack_size=8192']`` in the :ref:`compiler-configuration-file`. On Windows
(for node versions older than v19), you will also need
``--max-stack-size=8192``, and also run ``editbin /stack:33554432 node.exe``.


How do I pass int64_t and uint64_t values from js into Wasm functions?
======================================================================

If you build using the `-sWASM_BIGINT` flag, then `int64_t` and `uint64_t` will
be represented as `bigint` values in JS. Without the `-sWASM_BIGINT` flag, the
values will be represented as `number` in JS which can't represent int64s, so
what happens is that in exported functions (that you can call from JS) we
"legalize" the types, by turning an i64 argument into two i32s (low and high
bits), and an i64 return value becomes an i32, and you can access the high bits
by calling a helper function called getTempRet0.


Can I use multiple Emscripten-compiled programs on one Web page?
================================================================

Emscripten output by default is just some code. When put in a script tag, that
means the code is in the global scope. So multiple such modules on the same page
can't work.

But by putting each module in a function scope, that problem is avoided.
Emscripten even has a compile flag for this, ``MODULARIZE``, useful in
conjunction with ``EXPORT_NAME`` (details in settings.js).

However, there are still some issues if the same Module object (that defines the
canvas, text output area, etc.) is used among separate modules. By default
Emscripten output even looks for Module in the global scope, but when using
``MODULARIZE``, you get a function you must call with the Module as a param, so
that problem is avoided. But note that each module will probably want its own
canvas, text output area, etc.; just passing in the same Module object (e.g.
from the default HTML shell) may not work.

So by using ``MODULARIZE`` and creating a proper Module object for each module,
and passing those in, multiple modules can work fine.

Another option is to use an iframe, in which case the default HTML shell will
just work, as each will have its own canvas, etc. But this is overkill for small
programs, which can run modularly as described above.


Can I build JavaScript that only runs on the Web?
=================================================

Yes, you can use the `ENVIRONMENT` option in ``settings.js``. For example,
building with ``emcc -sENVIRONMENT=web`` will emit code that only runs on the
Web, and does not include support code for Node.js and other environments.

This can be useful to reduce code size, and also works around issues like the
Node.js support code using ``require()``, which Webpack will process and include
unnecessary code for.


Why the weird name for the project?
===================================

I don't know why; it's a perfectly `cromulent <http://en.wikipedia.org/wiki/Lisa_the_Iconoclast>`_ word!
