.. _FAQ:

===
FAQ
===

This FAQ contains answers to many questions asked on IRC and the mailing list.

How do I compile code?
======================

See the :ref:`Emscripten Tutorial <Tutorial>` and :ref:`emcc <emccdoc>`.


Why do I get multiple errors building basic code and the tests?
===============================================================

All the tests in the :ref:`Emscripten test suite <emscripten-test-suite>` are known to build and pass on the **master** branch, so if these are failing it is likely that there is some problem with your environment.

First call ``emcc -v``, which runs basic sanity checks and prints out useful environment information. If that doesn't help, follow the instructions in :ref:`verifying-the-emscripten-environment`.

You might also want to go through the :ref:`Tutorial` again, as this is updated as Emscripten changes.


I tried something: why doesn’t it work?
=======================================

Some general steps that might help figure things out:

 * See if the problem happens without optimizations (`-O0`, or not specifying any optimization level). Without optimizations, emscripten enables many assertions at compile and runtime, which may catch a problem and display an error message with a suggestion for how to fix it.
 * Search the documentation on this site.
 * Check if there is a test for the failing functionality in the :ref:`Emscripten test suite <emscripten-test-suite>` (run ``grep -r`` in **tests/**). They should all pass (with only rare exceptions), so they provide concrete "known-good" examples of how various options and code are used.


Do I need to change my build system to use Emscripten?
======================================================

In most cases you will be able to use your project's current build system with Emscripten. See :ref:`Building-Projects`.



Why is code compilation slow?
=============================

Emscripten makes some trade-offs that make the generated code faster and smaller, at the cost of longer compilation times. For example, we build parts of the standard library along with your code, which enables some additional optimizations, but takes a little longer to compile.

.. note:: You can determine what compilation steps take longest by compiling with ``EMCC_DEBUG=1`` in the environment and then reviewing the debug logs (by default in ``/tmp/emscripten_temp``). Note that compiling in debug mode takes longer than normal, because we print out a lot of intermediate steps to disk.

The main tips for improving build time are:

- Create fully optimized builds less frequently — use ``-O0`` during frequent development iterations (or don't specify an optimization level).

  - Compiling with higher levels of optimization can in some cases be noticeably slower: ``-O2`` is slower than ``-O1``, which is in turn slower than ``-O0``.
  - Compiling with ``-O3`` is **especially** slow — this can be mitigated by also enabling ``-s AGGRESSIVE_VARIABLE_ELIMINATION=1`` (removing variables makes the ``-O3`` regalloc easier).

- Compile without :ref:`line number debug information <emcc-g>` (use ``-O1`` or ``-g0`` instead of ``-g``):

  - Currently builds with line-number debug information are slow (see issue `#216 <https://github.com/emscripten-core/emscripten/issues/216>`_). Stripping the debug information significantly improves compile times.

- Compile on a machine with more cores:

  - Emscripten can run some passes in parallel (specifically, the JavaScript optimisations). Increasing the number of cores results in an almost linear improvement.
  - Emscripten will automatically use more cores if they are available. You can control how many cores are used  with ``EMCC_CORES=N`` (this is useful if you have many cores but relatively less memory).

- Make sure that the native optimizer is being used, which greatly speeds up optimized builds as of 1.28.2. ``EMCC_DEBUG=1`` output should not report errors about the native optimizer failing to build or not being used because of a previous failed build (if it previously failed, do ``emcc --clear-cache`` then compile your file again, and the optimizer will be automatically rebuilt).

- When you have multiple bitcode files as inputs, put the largest file first (LLVM linking links the second and later ones into the first, so less copying is done on the first input to the linker).

- Having fewer bitcode files can be faster, so you might want to link files into larger files in parallel in your build system (you might already do this if you have logical libraries), and then the final command has fewer things to operate on.

- You don't need to link into a single bitcode file yourself, you can call the final ``emcc`` command that emits JS with a list of files. ``emcc`` can then defer linking and avoid an intermediary step, if possible (this optimization is disabled by LTO and by `EMCC_DEBUG=2`).


Why does my code run slowly?
============================

Make sure you optimize code by building with ``-O2`` (even more :ref:`aggressive optimization <emcc-O3>` is available, at the cost of significantly increased compilation time).

.. note: This is necessary both for each source file, and for the final stage of linking and compiling to JavaScript. For more information see :ref:`Building-Projects` and :ref:`Optimizing-Code`.


Why is my compiled code big?
============================

Make sure you build with ``-O3`` or ``-Os`` so code is fully optimized and minified. You should use the closure compiler, gzip compression on your webserver, etc., see the :ref:`section on code size in Optimizing code <optimizing-code-size>`.



Why does compiling code that works on another machine gives me errors?
======================================================================

Make sure you are using the Emscripten bundled system headers. Using :ref:`emcc <emccdoc>` will do so by default, but problems may occur if you use your local system headers with ``emcc`` or compile into LLVM bitcode yourself.


How can I reduce startup time?
==============================

Make sure that you are running an :ref:`optimized build <Optimizing-Code>` (smaller builds are faster to start up).

Network latency is also a possible factor in startup time. Consider putting the file loading code in a separate script element from the generated code so that the browser can start the network download in parallel to starting up the codebase (run the :ref:`file packager <packaging-files>` and put file loading code in one script element, and the generated codebase in a later script element).


What is "No WebAssembly support found. Build with -s WASM=0 to target JavaScript instead" or "no native wasm support detected"?
===============================================================================================================================

Those errors indicate that WebAssembly support is not present in the VM you are trying to run the code in. Compile with ``-s WASM=0`` to disable WebAssembly (and emit asm.js instead) if you want your code to run in such environments (all modern browsers support WebAssembly, but in some cases you may want to reach 100% of browsers, including legacy ones).


Why does my code fail to compile with an error message about inline assembly (or ``{"text":"asm"}``)?
=====================================================================================================

Emscripten cannot compile inline assembly code (because it is CPU specific, and Emscripten is not a CPU emulator).

You will need to find where inline assembly is used, and disable it or replace it with platform-independent code.

.. note:: Emscripten automatically unsets the following ``#define`` values, as these are commonly set in projects to enable platform dependent code (inline assembly):

  ::

    #undef __i386__
    #undef __x86_64__


.. _faq-my-html-app-hangs:

Why does my HTML app hang?
==========================

The browser event model uses *co-operative multitasking* — each event has a "turn" to run, and must then return control to the browser event loop so that other events can be processed. A common cause of HTML pages hanging is JavaScript that does not complete and return control to the browser.

Graphical C++ apps typically have an infinite main loop in which event handling, processing and rendering is done, followed by a delay to keep the frame-rate right (``SDL_DELAY`` in :term:`SDL` apps). As the main loop does not complete (is infinite) it cannot return control to the browser, and the app will hang.

Apps that use an infinite main loop should be re-coded to put the actions for a single iteration of the loop into a single "finite" function. In the native build this function can be run in an infinite loop as before. In the Emscripten build it is set as the :ref:`main loop function <faq-how-run-event-loop>` and will be called by the browser at a specified frequency.

There is more information on this topic in :ref:`emscripten-runtime-environment`.


.. _faq-how-run-event-loop:

How do I run an event loop?
===========================

To run a C function repeatedly, use :c:func:`emscripten_set_main_loop` (this is discussed in :ref:`emscripten-runtime-environment`). The related functions in :ref:`emscripten.h <emscripten-h-browser-execution-environment>` are also useful, allowing you to add events that block the main loop, etc.

To respond to browser events use the SDL API in the normal way. There are examples in the SDL tests (search for SDL in **tests/runner.py**).

See also: :ref:`faq-my-html-app-hangs`



Why doesn't my SDL app work?
=============================

See the :term:`SDL` automatic tests for working examples: ``python tests/runner.py browser``.


How do I link against system libraries like SDL, boost, etc.?
=============================================================

System libraries that are included with Emscripten are automatically linked when you compile (just the necessary parts). This includes *libc*, *libc++* (C++ standard library) and :term:`SDL`.

Libraries not included with Emscripten (like Boost) must be compiled and linked with the program just as if they were a module in the project.

There is a set of libraries ported to Emscripten for convenient use, Emscripten Ports. See :ref:`Building-Projects`

Another option is to implement needed C APIs as JavaScript libraries (see ``--js-library`` in :ref:`emcc <emcc-js-library>` and :ref:`implement-c-in-javascript`). Emscripten itself does this for *libc* (not including *malloc*) and :term:`SDL` (but not *libc++* or *malloc*).

.. note::

  - Unlike other compilers, you don't need ``-lSDL`` to include SDL (specifying it will do no harm).
  - In the specific case of *Boost*, if you only need the boost headers then you don't need to compile anything.


What are my options for audio playback?
=======================================

Emscripten has partial support for SDL1 and 2 audio, and OpenAL.

To use SDL1 audio, include it as ``#include <SDL/SDL_mixer.h>``. You can use it that way alongside SDL1, SDL2, or another library for platform integration.

To use SDL2 audio, include it as ``#include <SDL2/SDL_mixer.h>`` and use `-s USE_SDL_MIXER=2`. Format support is currently limited to OGG and WAV.

How can my compiled program access files?
=========================================

Emscripten uses a virtual file system that may be preloaded with data or linked to URLs for lazy loading. See the :ref:`file-system-overview` for more details.


Why can't my code access a file in the same directory?
======================================================

Emscripten-generated code running *in the browser* cannot access files in the local file system. Instead you can use :ref:`preloading <emcc-preload-file>` and :ref:`embedding <emcc-embed-file>` to work around the lack of synchronous file IO. See :ref:`file-system-overview` for more information.

It is possible to allow access to local file system for code running in *node.js*, use the :ref:`NODEFS <filesystem-api-nodefs>` filesystem option.


.. _faq-when-safe-to-call-compiled-functions:

How can I tell when the page is fully loaded and it is safe to call compiled functions?
=======================================================================================

(You may need this answer if you see an error saying something like ``you need to wait for the runtime to be ready (e.g. wait for main() to be called)``, which is a check enabled in ``ASSERTIONS`` builds.)

Calling a compiled function before a page has fully loaded can result in an error, if the function relies on files that may not be present (for example the :ref:`.mem <emcc-memory-init-file>` file and :ref:`preloaded <emcc-preload-file>` files are loaded asynchronously, and therefore if you just place some JS that calls compiled code in a ``--post-js``, that code will be called synchronously at the end of the combined JS file, potentially before the asynchronous event happens, which is bad).

The easiest way to find out when loading is complete is to add a ``main()`` function, and within it call a JavaScript function to notify your code that loading is complete.

.. note:: The ``main()`` function is called after startup is complete as a signal that it is safe to call any compiled method.

For example, if ``allReady()`` is a JavaScript function you want called when everything is ready, you can do:

::

  #include <emscripten.h>

  int main() {
    EM_ASM( allReady() );
  }

Another option is to define an ``onRuntimeInitialized`` function,

::

  Module['onRuntimeInitialized'] = function() { ... };

That method will be called when the runtime is ready and it is ok for you to call compiled code. In practice, that is exactly the same time at which ``main()`` would be called, so ``onRuntimeInitialized`` doesn't let you do anything new, but you can set it from JavaScript at runtime in a flexible way.

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

The crucial thing is that ``Module`` exists, and has the property ``onRuntimeInitialized``, before the script containing emscripten output (``my_project.js`` in this example) is loaded.

Another option is to use the ``MODULARIZE`` option, using ``-s MODULARIZE=1``. That will put all of the generated JavaScript in a function, which you can call to create an instance. The instance has a promise-like `.then()` method, so if you build with say ``-s MODULARIZE=1 -s 'EXPORT_NAME="MyCode"'`` (see details in settings.js), then you can do something like this:

::

    MyCode().then(function(Module) {
      // this is reached when everything is ready, and you can call methods on Module
    });

.. _faq-NO_EXIT_RUNTIME:

What does "exiting the runtime" mean? Why don't ``atexit()s`` run?
==================================================================

(You may need this answer if you see an error saying something like ``atexit() called, but EXIT_RUNTIME is not set`` or ``stdio streams had content in them that was not flushed. you should set EXIT_RUNTIME to 1``.)

By default Emscripten sets ``EXIT_RUNTIME=0``, which means that we don't include code to shut down the runtime. That means that when ``main()`` exits, we don't flush the stdio streams, or call the destructors of global C++ objects, or call ``atexit`` callbacks. This lets us emit smaller code by default, and is normally what you want on the web: even though ``main()`` exited, you may have something asynchronous happening later that you want to execute.

In some cases, though, you may want a more "commandline" experience, where we do shut down the runtime when ``main()`` exits. You can build with ``-s EXIT_RUNTIME=1``, and then we will call ``atexits`` and so forth. When you build with ``ASSERTIONS``, you should get a warning when you need this. For example, if your program prints something without a newline,

::

  #include <stdio.h>

  int main() {
    printf("hello"); // note no newline
  }

If we don't shut down the runtime and flush the stdio streams, "hello" won't be printed. In an ``ASSERTIONS`` build you'll get a notification saying ``stdio streams had content in them that was not flushed. you should set EXIT_RUNTIME to 1``.

.. _faq-dead-code-elimination:

Why do functions in my C/C++ source code vanish when I compile to JavaScript, and/or I get ``No functions to process``?
=======================================================================================================================

Emscripten does dead code elimination of functions that are not called from the compiled code. While this does minimize code size, it can remove functions that you plan to call yourself (outside of the compiled code).

To make sure a C function remains available to be called from normal JavaScript, it must be added to the `EXPORTED_FUNCTIONS <https://github.com/emscripten-core/emscripten/blob/1.29.12/src/settings.js#L388>`_ using the *emcc* command line. For example, to prevent functions ``my_func()`` and ``main()`` from being removed/renamed, run *emcc* with: ::

  emcc -s "EXPORTED_FUNCTIONS=['_main', '_my_func']"  ...

.. note::

   `_main` should be in the export list, as in that example, if you have a `main()` function. Otherwise, it will be removed as dead code; there is no special logic to keep `main()` alive by default.

.. note::

   `EXPORTED_FUNCTIONS` affects compilation to JavaScript. If you first compile to an object file,
   then compile the object to JavaScript, you need that option on the second command.

If your function is used in other functions, LLVM may inline it and it will not appear as a unique function in the JavaScript. Prevent inlining by defining the function with :c:type:`EMSCRIPTEN_KEEPALIVE`: ::

  void EMSCRIPTEN_KEEPALIVE yourCfunc() {..}

`EMSCRIPTEN_KEEPALIVE` also exports the function, as if it were on `EXPORTED_FUNCTIONS`.

.. note::

  - All functions not kept alive through ``EXPORTED_FUNCTIONS`` or :c:type:`EMSCRIPTEN_KEEPALIVE` will potentially be removed. Make sure you keep the things you need alive using one or both of those methods.

  - Exported functions need to be C functions (to avoid C++ name mangling).

  - Decorating your code with :c:type:`EMSCRIPTEN_KEEPALIVE` can be useful if you don't want to have to keep track of functions to export explicitly, and when these exports do not change. It is not necessarily suitable for exporting functions from other libraries — for example it is not a good idea to decorate and recompile the source code of the C standard library. If you build the same source in multiple ways and change what is exported, then managing exports on the command line is easier.

  - Running *emcc* with ``-s LINKABLE=1`` will also disable link-time optimizations and dead code elimination. This is not recommended as it makes the code larger and less optimized.

Another possible cause of missing code is improper linking of ``.a`` files. The ``.a`` files link only the internal object files needed by previous files on the command line, so the order of files matters, and this can be surprising. If you are linking ``.a`` files, make sure they are at the end of the list of files, and in the right order amongst themselves. Alternatively, just use ``.so`` files instead in your project.

.. tip:: It can be useful to compile with ``EMCC_DEBUG=1`` set for the environment (``EMCC_DEBUG=1 emcc ...`` on Linux, ``set EMMCC_DEBUG=1`` on Windows). This splits up the compilation steps and saves them in ``/tmp/emscripten_temp``. You can then see at what stage the code vanishes (you will need to do ``llvm-dis`` on the bitcode  stages to read them, or ``llvm-nm``, etc.).



Why is the File System API is not available when I build with closure?
======================================================================

The :term:`Closure Compiler` will minify the File Server API code. Code that uses the file system must be optimized **with** the File System API, using emcc's ``--pre-js`` :ref:`option <emcc-pre-js>`.


Why does my code break and gives odd errors when using ``-O2 --closure 1``?
===========================================================================

The :term:`Closure Compiler` minifies variable names, which results in very short variable names like ``i``, ``j``, ``xa``, etc. If other code declares variables with the same names in global scope, this can cause serious problems.

This is likely to be the cause if you can successfully run code compiled with ``-O2`` set and ``--closure`` unset.

One solution is to stop using small variable names in the global scope (often this is a mistake — forgetting to use ``var`` when assigning to a variable).

Another alternative is to wrap the generated code (or your other code) in a closure, as shown:

::

  var CompiledModule = (function() {
    .. GENERATED CODE ..
    return Module;
    })();

.. _faq-export-stuff:

Why do I get ``TypeError: Module.someThing is not a function``?
===============================================================

The ``Module`` object will contain exported methods. For something to appear there, you should add it to ``EXPORTED_FUNCTIONS`` for compiled code, or ``EXTRA_EXPORTED_RUNTIME_METHODS`` for a runtime method (like ``getValue``). For example,

 ::

  emcc -s "EXPORTED_FUNCTIONS=['_main', '_my_func']" ...

would export a C method ``my_func`` (in addition to ``main``, in this example). And

 ::

  emcc -s "EXTRA_EXPORTED_RUNTIME_METHODS=['ccall']" ...

will export ``ccall``. In both cases you can then access the exported function on the ``Module`` object.

.. note:: You can use runtime methods directly, without exporting them, if the compiler can see them used. For example, you can use ``getValue`` in ``EM_ASM`` code, or a ``--pre-js``, by calling it directly. The optimizer will not remove that JS runtime method because it sees it is used. You only need to use ``Module.getValue`` if you want to call that method from outside the JS code the compiler can see, and then you need to export it.

.. note:: Emscripten used to export many runtime methods by default. This increased code size, and for that reason we've changed that default. If you depend on something that used to be exported, you should see a warning pointing you to the solution, in an unoptimized build, or a build with ``ASSERTIONS`` enabled, which we hope will minimize any annoyance. See ``ChangeLog.md`` for details.

.. _faq-runtime-change:

Why does ``Runtime`` no longer exist? Why do I get an error trying to access ``Runtime.someThing``?
===================================================================================================

1.37.27 includes a refactoring to remove the ``Runtime`` object. This makes the generated code more efficient and compact, but requires minor changes if you used ``Runtime.*`` APIs. You just need to remove the ``Runtime.`` prefix, as those functions are now simple functions in the top scope (an error message in ``-O0`` or builds with assertions enabled with suggest this). In other words, replace

 ::

  x = Runtime.stackAlloc(10);

with

 ::

  x = stackAlloc(10);

.. note:: The above will work for code in a ``--pre-js`` or JS library, that is, code that is compiled together with the emscripten output. If you try to access ``Runtime.*`` methods from outside the compiled code, then you must export that function (using ``EXTRA_EXPORTED_RUNTIME_METHODS``), and use it on the Module object, see :ref:`that FAQ entry<faq-export-stuff>`.


Why do I get a ``NameError`` or ``a problem occurred in evaluating content after a "-s"`` when I use a ``-s`` option?
=====================================================================================================================

That may occur when running something like

::

  # this fails on most Linuxes
  emcc a.c -s EXTRA_EXPORTED_RUNTIME_METHODS=['addOnPostRun']

  # this fails on macOS
  emcc a.c -s EXTRA_EXPORTED_RUNTIME_METHODS="['addOnPostRun']"

You may need to quote things like this:

::

  # this works in the shell on most Linuxes and on macOS
  emcc a.c -s "EXTRA_EXPORTED_RUNTIME_METHODS=['addOnPostRun']"

  # or you may need something like this in a Makefile
  emcc a.c -s EXTRA_EXPORTED_RUNTIME_METHODS=\"['addOnPostRun']\"

The proper syntax depends on the OS and shell you are in, and if you are writing in a Makefile, etc.

Why do I get an odd python error complaining about libcxx.bc or libcxxabi.bc?
=============================================================================

A possible cause is that building *libcxx* or *libcxxabi* failed. Go to **system/lib/libcxx** (or libcxxabi) and do ``emmake make`` to see the actual error. Or, clean the Emscripten cache (``./emcc --clear-cache``) and then compile your file with ``EMCC_DEBUG=1`` in the environment. *libcxx* will then be built in **/tmp/emscripten_temp/libcxx**, and you can see ``configure*, make*`` files that are the output of configure and make, etc.

Another possible cause of this error is the lack of ``make``, which is necessary to build these libraries. If you are on Windows, you need *cmake*.


Why do I get an error mentioning Uglify and ``throw new JS_Parse_Error``?
=========================================================================

In ``-O2`` and above, emscripten will optimize the JS using Uglify1. If you added any JS (using ``--pre-js``/``--post-js``/``EM_ASM``/``EM_JS``) and it contains JS that Uglify1 can't parse - like recent ES6 features - then it will throw such a parsing error.

In the long term we hope to upgrade our internal JS parser. Meanwhile, you can move such code to another script tag on the page, that is, not pass it through the emscripten optimizer.

See also

 * https://github.com/emscripten-core/emscripten/issues/6000
 * https://github.com/emscripten-core/emscripten/issues/5700

Why does running LLVM bitcode generated by emcc through **lli** break with errors about ``impure_ptr``?
=======================================================================================================

.. note:: :term:`lli` is not maintained, and has odd errors and crashes. We do include **tools/nativize_llvm.py** (which compiles bitcode to a native executable) but it will also hit the ``impure_ptr`` error.

The issue is that *newlib* uses ``impure_ptr`` code, while *glibc* uses something else. The result is that bitcode built with the Emscripten will not run locally unless your machine uses *newlib* (basically, only embedded systems).

The ``impure_ptr`` error only occurs during explicit use of ``stdout`` etc., so ``printf(..)`` will work, but ``fprintf(stdout, ..)`` will not. **Usually it is simple to modify your code to avoid this problem.**


Why do I get a stack size error when optimizing: ``RangeError: Maximum call stack size exceeded`` or similar?
=============================================================================================================

You may need to increase the stack size for :term:`node.js`.

On Linux and Mac macOS, you can just do ``NODE_JS = ['node', '--stack_size=8192']`` in the :ref:`compiler-configuration-file`. On Windows, you will also need ``--max-stack-size=8192``, and also run ``editbin /stack:33554432 node.exe``.


Why do I get ``error: cannot compile this aggregate va_arg expression yet`` and it says ``compiler frontend failed to generate LLVM bitcode, halting`` afterwards?
==================================================================================================================================================================

This is a limitation of the asm.js target in :term:`Clang`. This code is not currently supported.


Why does building from source fail during linking (at 100%)?
============================================================

Building :ref:`Fastcomp from source <building-fastcomp-from-source>` (and hence the SDK) can fail at 100% progress. This is due to out of memory in the linking stage, and is reported as an error: ``collect2: error: ld terminated with signal 9 [Killed]``.

The solution is to ensure the system has sufficient memory. On Ubuntu 14.04.1 LTS 64bit, you should use at least 6Gb.


Why do I get odd rounding errors when using float variables?
============================================================

In asm.js, by default Emscripten uses doubles for all floating-point variables, that is, 64-bit floats even when C/C++ code contains 32-bit floats. This is simplest and most efficient to implement in JS as doubles are the only native numeric type. As a result, you may see rounding errors compared to native code using 32-bit floats, just because of the difference in precision between 32-bit and 64-bit floating-point values.

To check if this is the issue you are seeing, build with ``-s PRECISE_F32=1``. This uses proper 32-bit floating-point values, at the cost of some extra code size overhead. This may be faster in some browsers, if they optimize ``Math.fround``, but can be slower in others. See ``src/settings.js`` for more details on this option.

(This is not an issue for wasm, which has native float types.)


How do I pass int64_t and uint64_t values from js into wasm functions?
======================================================================

JS can't represent int64s, so what happens is that in exported functions (that you can call from JS) we "legalize" the types, by turning an i64 argument into two i32s (low and high bits), and an i64 return value becomes an i32, and you can access the high bits by calling a helper function called getTempRet0.


Can I use multiple Emscripten-compiled programs on one Web page?
================================================================

Emscripten output by default is just some code. When put in a script tag, that means the code is in the global scope. So multiple such modules on the same page can't work.

But by putting each module in a function scope, that problem is avoided. Emscripten even has a compile flag for this, ``MODULARIZE``, useful in conjunction with ``EXPORT_NAME`` (details in settings.js).

However, there are still some issues if the same Module object (that defines the canvas, text output area, etc.) is used among separate modules. By default Emscripten output even looks for Module in the global scope, but when using MODULARIZE, you get a function you must call with the Module as a param, so that problem is avoided. But note that each module will probably want its own canvas, text output area, etc.; just passing in the same Module object (e.g. from the default HTML shell) may not work.

So by using MODULARIZE and creating a proper Module object for each module, and passing those in, multiple modules can work fine.

Another option is to use an iframe, in which case the default HTML shell will just work, as each will have its own canvas, etc. But this is overkill for small programs, which can run modularly as described above.

Can I build JavaScript that only runs on the Web?
=================================================

Yes, you can see the `ENVIRONMENT` option in ``settings.js``. For example, building with ``emcc -s ENVIRONMENT=web`` will emit code that only runs on the Web, and does not include support code for Node.js and other environments.

This can be useful to reduce code size, and also works around issues like the Node.js support code using ``require()``, which Webpack will process and include unnecessary code for.

Why the weird name for the project?
===================================

I don't know why; it's a perfectly `cromulent <http://en.wikipedia.org/wiki/Lisa_the_Iconoclast>`_ word!

