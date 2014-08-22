.. _FAQ:

==================================
FAQ (under-construction)
==================================

This FAQ has many frequently asked questions from IRC and the mailing list.

How do I compile code?
======================

See the :ref:`Tutorial` and :ref:`emccdoc`.

I get many errors building basic code and the tests?
====================================================

All the tests in the :ref:`Emscripten test suite <running-emscripten-tests>` are known to build and pass on the **master** branch, so it is likely that there is some problem with your environment.

First call ``./emcc -v``, which runs basic sanity checks and prints out useful environment information. If that doesn't help, follow the instructions in :ref:`verifying-the-emscripten-environment`.

You might also want to go through the :ref:`Tutorial` again, as this is updated as Emscripten changes.


I tried something and it doesn't work?
======================================

This site has comprehensive documentation. If you tried "something" and it fails (for example a :ref:`compiler flag <emccdoc>`, or a *libc* function), one of the first options should therefore be to search the site.

Another really good option is to see if there is a test for the failing functionality in the :ref:`Emscripten test suite <running-emscripten-tests>` (run ``grep -r`` in ``tests/``). **All** the tests are known to pass on the master branch, so they provide concrete "known-good" examples of how various options and code are used.


Do I need to change my build system to use Emscripten?
======================================================

In most cases you will be able to use your project's current build system with Emscripten. See :ref:`Building-Projects`.


My code cannot access a file in the same directory?
===================================================

Emscripten-generated code cannot by default access local files on your machine like a native application. See :ref:`Filesystem-Guide` for details: you can in node allow such access, and in other platforms you can use preloading to work around lack of synchronous file IO.


My code compiles slowly.
========================

Emscripten makes some tradeoffs that make the generated code faster and smaller, at the cost of longer compilation times. For example, we build parts of the standard library along with your code which enables some additional optimizations, but takes a little longer to compile.

Emscripten can run some passes in parallel (specifically, the JS opts), and will do so automatically, so running on a machine with more cores can give you almost a linear speedup (so doubling the amount of cores can almost halve the amount of time it takes to build and so forth). To see details of how work is parallelized, compile with ``EMCC_DEBUG=1`` in the environment (note though that in that debug mode compilation takes longer than normal, because we print out a lot of intermediate steps to disk, by default to ``/tmp/emscripten_temp``, but it's still useful to see which stages are slowing you down). You can affect how many cores are using with ``EMCC_CORES=N``, which can be useful if you have many cores but relatively less memory.

Note that optimization can in some cases be noticeably slower than unoptimized code, ``-O1`` is slower than ``-O0``, which in turn is slower than ``-O2`` (in return, though, they greatly improve the speed of the generated code). It might be useful to use ``-O0`` (or not specify an optimization level) during quick development iterations and to do fully optimized builds less frequently. Note that ``-O3`` is **especially** slow (you can mitigate the ``-O3`` slowness by also enabling ``-s AGGRESSIVE_VARIABLE_ELIMINATION=1``, as it removes variables which makes the ``-O3`` regalloc easier).

Currently builds with line-number debug info (where the source code was compiled with ``-g``) are slow, see issue #216. Stripping the debug info leads to much faster compile times.


My code runs slowly.
=====================

Make sure you build with ``-O2`` so code is optimized. **This is necessary both for each source file, and for the final stage of linking and compiling to JS**, see more details :ref:`Building-Projects`. See :ref:`Optimizing-Code` for more info.


My code is large.
===========================

Make sure you build with ``-O2`` so code is optimized, which includes minification to reduce size. You should also set up gzip compression on your webserver, which all browsers now support.

You can use closure compile to reduce code size even more with ``--closure 1``, however that will require that your code with with closure compiler advanced optimizations, including proper exports and so forth. It is usually not worth the effort over an optimized build+server gzip.


When I compile code that should work, I get odd errors in Emscripten about various things. I get different errors (or it works) on another machine.
==========================================================================================================================================================================

Make sure you are using the Emscripten bundled system headers. Using ``emcc`` will do so by default, but if you compile into LLVM bitcode yourself, or you use your local system headers even with ``emcc``, problems can happen.


My large codebase runs ok, but the time it takes to start up is significant. What can I do to improve that?
========================================================================================================================================

First, make sure you are running an optimized build (which will be much smaller and faster to start up). Then, if the sheer code size is causing the slow startup, you can try the `outlining option in emscripten <http://mozakai.blogspot.com/2013/08/outlining-workaround-for-jits-and-big.html>`__.

Another possible factor is network latency. If you load data off the network and also have a big codebase, try to put the file loading code in a separate script element - that is, run the file packager (``tools/file_packager.py``) and put its code in one script element, and in another script element after it put the generated codebase. That will allow the browser to start the network download immediately, so it can run in parallel to the browser parsing and starting up the codebase.


My code fails to compile, the error includes something about inline assembly (or ``{"text":"asm"}``).
========================================================================================================================================

Emscripten cannot compile inline assembly code, which is CPU specific, because Emscripten is not a CPU emulator.

Many projects have build options that generate only platform-independent code, without inline assembly. That should be used for Emscripten. For example, the following might help (and are done automatically for you by ``emcc``):

   ::

       #undef __i386__
       #undef __x86_64__

Since when no CPU-specific ``#define`` exists, many projects will not generate CPU specific code. In general though, you will need to find where inline assembly is generated, and how to disable that.


How do I run an event loop?
===========================

To run a C function repeatedly, use :c:func:`emscripten_set_main_loop` (**emscripten.h**). The other functions in that file are also useful, they let you do things like add events that block the main loop, etc. 

To respond to browser events and so forth, use the SDL API normally. See the SDL tests for examples (look for SDL in ``tests/runner.py``).

See also the next question.


My HTML app hangs.
==================

   **A.** Graphical C++ apps typically have a main loop that is an infinite loop, in which event handling is done, processing and rendering, then SDL_Delay. However, in JS there is no way for SDL_Delay to actually return control to the browser event loop. To do that, you must exit the current code. See
   :ref:`Emscripten-Browser-Environment`.

My SDL app doesn't work.
========================

See the SDL automatic tests for working examples: ``python tests/runner.py browser``.

How do I link against system libraries like SDL, boost, etc.?
==================================================================

System libraries that are included with Emscripten - libc, libc++ (C++ standard library) and SDL - are automatically included when you compile (and just the necessary parts of them). You don't even need ``-lSDL``, unlike other compilers (but ``-lSDL`` won't hurt either).

Other libraries not included with Emscripten, like boost, you would need to compile yourself and link with your program, just as if they were a module in your project. For example, see how `BananaBread links in libz <https://github.com/kripken/BananaBread/blob/master/cube2/src/web/Makefile>`_. (Note that in the specific case of boost, if you only need the boost headers, you don't need to compile anything.)

Another option for libraries not included is to implement them as a JS library, like emscripten does for libc (minus malloc) and SDL (but not libc++ or malloc). See ``--js-library`` in emcc.


How can my compiled program access files?
==================================================================

Emscripten uses a virtual file system that may be preloaded with data or linked to URLs for lazy loading. See the :ref:`Filesystem-Guide` for more details.


Functions in my C/C++ source code vanish when I compile to JavaScript, and/or I get ``No functions to process``..?
==================================================================================================================

   **A.** By default Emscripten does dead code elimination to minimize code size. However, it might end up removing functions you want to call yourself, that are not called from the compiled code (so the LLVM optimizer thinks they are unneeded). You can run emcc with ``-s LINKABLE=1`` which will disable link-time optimizations and dead code elimination, but this makes the code larger and less optimized than it could be. Instead, you should prevent specific functions from being eliminated by adding them to EXPORTED_FUNCTIONS (see ``src/settings.js``), for example, run emcc with something like ``-s EXPORTED_FUNCTIONS="['_main', '_my_func']"`` in order to keep my_func from being removed/renamed (as well as ``main()``)). Note that this assumes my_func is a C function, otherwise you would need to write out the C++ name mangling.

   It can be useful to compile with ``EMCC_DEBUG=1`` (``EMCC_DEBUG=1 emcc ..``). Then the compilation steps are split up and saved in ``/tmp/emscripten_temp``. You can then see at what stage the code vanishes (you will need to do ``llvm-dis`` on the bitcode  stages to read them, or ``llvm-nm``, etc.).

   In summary, the general procedure for making sure a function is accessible to be called from normal JS later is (1) make a C function interface (to avoid C++ name mangling), (2) run emcc with ``-s EXPORTED_FUNCTIONS="['_main', '_yourCfunc']"`` to make sure it is kept alive during optimization.

   If your function is used in other functions, LLVM may inline it and it will not show up. In this case you can define it with ``void EMSCRIPTEN_KEEPALIVE yourCfunc() {..}`` which will preserve it (see :c:type:`EMSCRIPTEN_KEEPALIVE`).

   Another possible issue here is linking of ``.a`` files. ``.a`` files link only the internal object files needed by previous files on the command line, so the order of files matters, and this can be surprising. If you are linking ``.a`` files, make sure they are at the end of the list of files, and in the right order amongst themselves, or just use ``.so`` files instead in your project.

   Note: In LLVM 3.2 dead code elimination is significantly more aggressive. All functions not kept alive through EXPORTED_FUNCTIONS will be potentially eliminated. Make sure to keep the things you need alive using one or both of those methods.

The FS API is not available when I build with closure?
=======================================================

Closure compiler will minify the FS API code. To write code that uses it, it must be optimized **with** the FS API code by closure. To do that, use emcc's ``--pre-js`` option, see ``emcc --help``.


My code breaks with ``-O2 --closure 1``, giving odd errors..?
====================================================================

The likely problem is that Closure Compiler minifies variable names. Names like ``i,j,xa`` can be generated, and if other code has such variables in the global scope, bad things can happen.
	
To check if this is the problem, compile with ``-O2`` (without ``--closure 1``). If that works, name minification might be the problem. If so, wrapping the generated code in a closure should fix it. (Or, wrap your other code in a closure, or stop it from using small variable names in the global scope, you might be using such variables by mistake by forgetting a ``var`` and assigning to a variable - which makes it be in the global scope.) 
	
To 'wrap' code in a closure, do something like this:

::

	var CompiledModule = (function() {
		.. GENERATED CODE ..
		return Module;
		})();

I get ``undefined is not a function`` or ``NAME is not a function``..?
=====================================================================================

The likely cause is an undefined function - something that was referred to, but not implemented or linked in. If you get ``undefined``, look at the line number to see the function name.

Emscripten by default does *not* give fatal errors on undefined symbols, so you can get runtime errors like these (because in practice in many codebases it is easiest to get them working without refactoring them to remove all undefined symbol calls). If you prefer compile-time notifications, run emcc with ``-s WARN_ON_UNDEFINED_SYMBOLS=1`` or ``-s ERROR_ON_UNDEFINED_SYMBOLS=1``.

Aside from just forgetting to link in a necessary object file, one possible cause for this error is inline functions in headers. If you have a header with ``inline int my_func() { .. }`` then clang may not actually inline the function (since inline is just a hint), and also not generate code for it (since it's in a header), so the generated bitcode and js will not have that function implemented. One solution is to add ``static``, that is ``static inline int my_func() { .. }`` which forces code to be generated in the object file.

I get an odd python error complaining about libcxx.bc or libcxxabi.bc..?
=====================================================================================


Possibly building libcxx or libcxxabi failed. Go to system/lib/libcxx (or libcxxabi) and do ``emmake make`` to see the actual error. Or, clean the emscripten cache (``~/.emscripten_cache``) and then compile your file with ``EMCC_DEBUG=1`` in the environment. libcxx will then be built in
   ``/tmp/emscripten_temp/libcxx``, and you can see ``configure*,make*`` files that are the output of configure and make, etc.

One possible cause of this error is the lack of ``make``, which is necessary to build these libraries. If you are on Windows, you need cygwin which supplies ``make``.

Running LLVM bitcode generated by emcc through ``lli`` breaks with errors about ``impure_ptr`` stuff..?
========================================================================================================

First of all, lli is not maintained (sadly) and has odd errors and crashes. However there is ``tools/nativize_llvm.py`` which compiles bitcode to a native executable. It will also hit the ``impure_ptr`` error though.

The issue is that newlib uses that impure pointer stuff, while glibc uses something else. So bitcode build with the emscripten SDK (which emcc does) will not run locally, unless your machine uses newlib (which basically only embedded systems do). The impure_ptr stuff is limited, however, it only applies to explicit use of ``stdout`` etc.  So ``printf(..)`` will work, but ``fprintf(stdout, ..)`` will not. So often it is simple to modify your code to not hit this problem.

I get a stack size error when optimizing (``RangeError: Maximum call stack size exceeded`` or similar)?
=======================================================================================================================================

You may need to increase the stack size for node. On linux and mac, you can just do ``NODE_JS = ['node', '--stack_size=8192']`` or such (in ``~/.emscripten``). On windows, you will also need ``--max-stack-size=8192``, and also to run ``editbin /stack:33554432 node.exe``.


I get ``error: cannot compile this aggregate va_arg expression yet`` and it says ``compiler frontend failed to generate LLVM bitcode, halting`` afterwards.
=============================================================================================================================================================================================

This is a limitation of the le32 frontend in clang. You can use the x86 frontend instead by compiling with ``EMCC_LLVM_TARGET=i386-pc-linux-gnu`` in the environment (however you will lose the advantages of le32 which includes better alignment of doubles).

.. _faq-when-safe-to-call-compiled-functions:

I am building a library, and sometimes I get an error when I call a compiled function before the page fully loaded. How can I tell when is safe to call it?
============================================================================================================================================================================

The easiest way to find out when loading is complete (which is asynchronous, as it often requires loading files, such as the .mem file or preloade files), is to just add a ``main()`` function. It will be called when it is safe to do so, after startup is complete, so that is a signal that it is safe to call any compiled method.

You can make ``main()`` call your JS to notify it, for example if ``allReady()`` is a JS function you want called when everything is ready, you can do

::

	#include <emscripten.h>

	int main() {
		EM_ASM( allReady() );
		}

		
The name of the project sounds weird to me
==========================================

I don't know why; it's a perfectly `cromulent <http://en.wikipedia.org/wiki/Lisa_the_Iconoclast>`_ word!
