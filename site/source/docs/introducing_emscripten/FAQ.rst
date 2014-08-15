.. _FAQ:

=================
FAQ (wiki-import)
=================
.. note:: This article was migrated from the wiki (Fri, 25 Jul 2014 04:21) and is now the "master copy" (the version in the wiki will be deleted). It may not be a perfect rendering of the original but we hope to fix that soon!

General
-------

-  **Q.** What does Emscripten do?

   **A.** Emscripten compiles LLVM bytecode into JavaScript, which then
   allows:

   -  Compiling C/C++ and other code that can be translated into LLVM,
      **directly** into JavaScript.
   -  Compiling the C/C++ **runtimes** of other languages into
      JavaScript, and then running code in those other languages in an
      *indirect* way. This works for languages like Python and Lua.

   Practically any **portable** C or C++ codebase can be run in
   JavaScript using Emscripten (but see
   :ref:`CodeGuidelinesAndLimitations`). That includes even things like
   games that need to render graphics, play sounds, load and process
   files, etc. etc. See the list of demos on the main wiki page to get
   an idea of what is possible.

-  **Q.** Why are you doing this?

   **A.** The web is standards-based, cross-platform, runs everywhere
   from PCs to iPads, and has numerous independent compatible
   implementations. It's arguably the best platform to develop for, for
   those reasons. But it could be even more developer-friendly: While
   JavaScript (when used well!) is an excellent language, lots of people
   want to code in other languages. By compiling to JavaScript, everyone
   is happy.

-  **Q.** What is the status of Emscripten?

   **A.** Emscripten is mature and has been used to port a very long
   list of real-world codebases to JavaScript, including large projects
   like CPython, Poppler and Bullet, as well as commercial projects like
   the Unreal Engine 3, Nebula 3 engine, etc.. You can see some examples
   `here <https://github.com/kripken/emscripten/wiki>`__.

-  **Q.** How fast will the compiled code be?

   **A.** Emscripten's default code generation mode is in
   `asm.js <http://asmjs.org>`__ format, which is a subset of JavaScript
   designed to make it possible for JavaScript engines to execute very
   quickly. See
   `here <http://arewefastyet.com/#machine=11&view=breakdown&suite=asmjs-ubench>`__
   for up-to-date benchmark results. In many cases, asm.js can get quite
   close to native speed.

   To run the emscripten benchmark suite yourself, do
   ``python tests/runner.py benchmark``.

-  **Q.** How big will the compiled code be?

   **A.** The effective size of the code will be about the same as
   native code. That is, if you gzip your code, it will be about the
   same size as gzipped native code. For more, see `this blog
   post <http://mozakai.blogspot.com/2011/11/code-size-when-compiling-to-javascript.html>`__.

   Note that **unoptimized** builds can be quite large. While in native
   code unoptimized and optimized builds can be much the same size, in
   JS unoptimized builds are much larger because optimization is
   required to cut down the size (using things like minification). So if
   you build your project at ``-O0`` and see a surprisingly large file,
   check the size when building with ``-O2``. Also, remember that you
   can run gzip on your webserver, so the amount actually transferred
   over the network is much smaller than even that.

-  **Q.** Where does Emscripten itself run?

   **A.** Emscripten is known to work on Windows, OS X and Linux. (There
   is a tendency to focus a little more on Linux since that's what one
   of the main devs uses, so that one might be slightly more stable.)

   That's for the compiler, of course, the generated code is valid
   JavaScript, so it will run anywhere JavaScript can run.

-  **Q.** What APIs/libraries does Emscripten support?

   **A.** libc and stdlibc++ support is very good. SDL support is
   sufficient to run quite a lot of code. OpenGL support is in very good
   shape for OpenGL ES 2.0-type code, and even some other types, see
   :ref:`OpenGL-support`.

-  **Q.** Is this really a compiler? Isn't it better described as a
   translator?

   **A.** Well, a *compiler* is usually defined as a program that
   transforms source code written in one programming language into
   another, which is what Emscripten does. A *translator* is a more
   specific term that is usually used for compilation between high-level
   languages, which isn't exactly applicable. On the other hand a
   *decompiler* is something that translates a low-level language to a
   higher-level one, so that might technically be a valid description,
   but it sounds odd since we aren't going back to the original language
   we compiled from (C/C++, most likely) but into something else
   (JavaScript).

-  **Q.** The name of the project sounds weird to me.

   **A.** I don't know why; it's a perfectly
   `cromulent <http://en.wikipedia.org/wiki/Lisa_the_Iconoclast>`__
   word!

Using Emscripten
----------------

-  **Q.** How do I compile code?

   **A.** See the :ref:`Tutorial`.

-  **Q.** I get lots of errors building the tests or even simple hello
   world type stuff?

   **A.** Try to run ``emcc -v``, which prints out some helpful
   information and runs some sanity checks. You can also try
   ``emcc --clear-cache`` to empty the compiler's internal cache, which
   will then be rebuilt next time it is used. If none of those help or
   give an idea of what could be the matter, then some common problems
   to check for are:

   -  Typos in the paths in ``~/.emscripten``.
   -  Using older versions of Node or JS engines. Use the versions
      mentioned in the :ref:`Tutorial`.
   -  Using older versions of LLVM. See the :ref:`Emscripten-SDK` page for
      which version of LLVM is supported. Using other builds might not
      work. If you are using fastcomp, see :ref:`LLVM-Backend` for the
      proper repos for LLVM and clang that you should be using.
   -  Not having ``python2`` in your system. For compatibility with
      systems that install python 2.x alongside 3.x (increasingly
      common), we look for ``python2``. If you only have python 2.x
      installed, make ``python2`` be a link to ``python``. Or, instead
      you can invoke our python scripts directly, for example
      ``python emcc`` instead of ``./emcc``.

   You might also want to go through the :ref:`Tutorial` again, if it's
   been a while since you have (we update it when things change).

-  **Q.** I tried something and it doesn't work?

   **A.** Emscripten has an extensive test suite. If you try something
   (a compiler flag, a libc function, etc.) and it fails, often the best
   thing is to ``grep -r`` in ``tests/`` to see if there is a test for
   it. On the master branch, **all** tests are known to pass. The tests
   can therefore both show you what works, and also they give concrete
   examples for how to use things, which can be helpful in addition to
   the docs.

-  **Q.** Can I compile my project using Emscripten? Do I need a new
   build system?

   **A.** You can in most cases very easily use your project's current
   build system with Emscripten. See :ref:`Building-Projects`.

-  **Q.** My code cannot access a file that is right there in the same
   directory?

   **A.** Emscripten-generated code cannot by default access local files
   on your machine like a native application. See :ref:`Filesystem-Guide`
   for details: you can in node allow such access, and in other
   platforms you can use preloading to work around lack of synchronous
   file IO.

-  **Q.** My code compiles slowly.

   **A.** Emscripten makes some tradeoffs that make the generated code
   faster and smaller, at the cost of longer compilation times. For
   example, we build parts of the standard library along with your code
   which enables some additional optimizations, but takes a little
   longer to compile.

   Emscripten can run some passes in parallel (specifically, the JS
   opts), and will do so automatically, so running on a machine with
   more cores can give you almost a linear speedup (so doubling the
   amount of cores can almost halve the amount of time it takes to build
   and so forth). To see details of how work is parallelized, compile
   with ``EMCC_DEBUG=1`` in the environment (note though that in that
   debug mode compilation takes longer than normal, because we print out
   a lot of intermediate steps to disk, by default to
   ``/tmp/emscripten_temp``, but it's still useful to see which stages
   are slowing you down). You can affect how many cores are using with
   ``EMCC_CORES=N``, which can be useful if you have many cores but
   relatively less memory.

   Note that optimization can in some cases be noticeably slower than
   unoptimized code, ``-O1`` is slower than ``-O0``, which in turn is
   slower than ``-O2`` (in return, though, they greatly improve the
   speed of the generated code). It might be useful to use ``-O0`` (or
   not specify an optimization level) during quick development
   iterations and to do fully optimized builds less frequently. Note
   that ``-O3`` is **especially** slow (you can mitigate the ``-O3``
   slowness by also enabling ``-s AGGRESSIVE_VARIABLE_ELIMINATION=1``,
   as it removes variables which makes the ``-O3`` regalloc easier).

   Currently builds with line-number debug info (where the source code
   was compiled with ``-g``) are slow, see issue #216. Stripping the
   debug info leads to much faster compile times.

-  **Q.** My code runs slowly.

   **A.** Make sure you build with ``-O2`` so code is optimized. **This
   is necessary both for each source file, and for the final stage of
   linking and compiling to JS**, see more details
   `here <https://github.com/kripken/emscripten/wiki/Building-Projects>`__.
   See :ref:`Optimizing-Code` for more info.

-  **Q.** My code is large.

   **A.** Make sure you build with ``-O2`` so code is optimized, which
   includes minification to reduce size. You should also set up gzip
   compression on your webserver, which all browsers now support.

   You can use closure compile to reduce code size even more with
   ``--closure 1``, however that will require that your code with with
   closure compiler advanced optimizations, including proper exports and
   so forth. It is usually not worth the effort over an optimized
   build+server gzip.

-  **Q.** When I compile code that should work, I get odd errors in
   Emscripten about various things. I get different errors (or it works)
   on another machine.

   **A.** Make sure you are using the Emscripten bundled system headers.
   Using ``emcc`` will do so by default, but if you compile into LLVM
   bitcode yourself, or you use your local system headers even with
   ``emcc``, problems can happen.

-  **Q.** My large codebase runs ok, but the time it takes to start up
   is significant. What can I do to improve that?

   **A.** First, make sure you are running an optimized build (which
   will be much smaller and faster to start up). Then, if the sheer code
   size is causing the slow startup, you can try the `outlining option
   in
   emscripten <http://mozakai.blogspot.com/2013/08/outlining-workaround-for-jits-and-big.html>`__.

   Another possible factor is network latency. If you load data off the
   network and also have a big codebase, try to put the file loading
   code in a separate script element - that is, run the file packager
   (``tools/file_packager.py``) and put its code in one script element,
   and in another script element after it put the generated codebase.
   That will allow the browser to start the network download
   immediately, so it can run in parallel to the browser parsing and
   starting up the codebase.

-  **Q.** My code fails to compile, the error includes something about
   inline assembly (or ``{"text":"asm"}``).

   **A.** Emscripten cannot compile inline assembly code, which is CPU
   specific, because Emscripten is not a CPU emulator.

   Many projects have build options that generate only
   platform-independent code, without inline assembly. That should be
   used for Emscripten. For example, the following might help (and are
   done automatically for you by ``emcc``):

   ::

       #undef __i386__
       #undef __x86_64__

   Since when no CPU-specific ``#define`` exists, many projects will not
   generate CPU specific code. In general though, you will need to find
   where inline assembly is generated, and how to disable that.

-  **Q.** How do I run an event loop?

   **A.** To run a C function repeatedly, use
   :c:func:`emscripten_set_main_loop`, see
   ``system/include/emscripten/emscripten.h``. The other functions in
   that file are also useful, they let you do things like add events
   that block the main loop, etc. Documentation for all of those
   functions is in that header file.

   To respond to browser events and so forth, use the SDL API normally.
   See the SDL tests for examples (look for SDL in ``tests/runner.py``).

   See also the next question.

-  **Q.** My HTML app hangs.

   **A.** Graphical C++ apps typically have a main loop that is an
   infinite loop, in which event handling is done, processing and
   rendering, then SDL\_Delay. However, in JS there is no way for
   SDL\_Delay to actually return control to the browser event loop. To
   do that, you must exit the current code. See
   :ref:`Emscripten-Browser-Environment`.

-  **Q.** My SDL app doesn't work.

   **A.** See the SDL automatic tests for working examples:
   ``python tests/runner.py browser``.

-  **Q.** How do I link against system libraries like SDL, boost, etc.?

   **A.** System libraries that are included with emscripten - libc,
   libc++ (C++ STL) and SDL - are automatically included when you
   compile (and just the necessary parts of them). You don't even need
   ``-lSDL``, unlike other compilers (but ``-lSDL`` won't hurt either).

   Other libraries not included with emscripten, like boost, you would
   need to compile yourself and link with your program, just as if they
   were a module in your project. For example, see how `BananaBread
   links in
   libz <https://github.com/kripken/BananaBread/blob/master/cube2/src/web/Makefile>`__.
   (Note that in the specific case of boost, if you only need the boost
   headers, you don't need to compile anything.)

   Another option for libraries not included is to implement them as a
   JS library, like emscripten does for libc (minus malloc) and SDL (but
   not libc++ or malloc). See ``--js-library`` in emcc.

-  **Q.** How can my compiled program access files?

   **A.** Emscripten uses a virtual file system that may be preloaded
   with data or linked to URLs for lazy loading. See the [[Filesystem
   Guide]] for more details.

-  **Q.** I get an error trying to access ``__tm_struct_layout`` (or
   another C structure used in libc).

   **A.** You may need to compile the source code with ``emcc -g``.
   ``-g`` tells the compiler to include debug info, which includes
   metadata about structures which is used to access those structures
   from Emscripten's JS libc implementation. (Adding ``-g`` is a
   workaround until we have a proper fix for this.)

-  **Q.** Functions in my C/C++ source code vanish when I compile to
   JavaScript, and/or I get ``No functions to process``..?

   **A.** By default Emscripten does dead code elimination to minimize
   code size. However, it might end up removing functions you want to
   call yourself, that are not called from the compiled code (so the
   LLVM optimizer thinks they are unneeded). You can run emcc with
   ``-s LINKABLE=1`` which will disable link-time optimizations and dead
   code elimination, but this makes the code larger and less optimized
   than it could be. Instead, you should prevent specific functions from
   being eliminated by adding them to EXPORTED\_FUNCTIONS (see
   ``src/settings.js``), for example, run emcc with something like
   ``-s EXPORTED_FUNCTIONS="['_main', '_my_func']"`` in order to keep
   my\_func from being removed/renamed (as well as ``main()``)). Note
   that this assumes my\_func is a C function, otherwise you would need
   to write out the C++ name mangling.

   It can be useful to compile with ``EMCC_DEBUG=1``
   (``EMCC_DEBUG=1 emcc ..``). Then the compilation steps are split up
   and saved in ``/tmp/emscripten_temp``. You can then see at what stage
   the code vanishes (you will need to do ``llvm-dis`` on the bitcode
   stages to read them, or ``llvm-nm``, etc.).

   In summary, the general procedure for making sure a function is
   accessible to be called from normal JS later is (1) make a C function
   interface (to avoid C++ name mangling), (2) run emcc with
   ``-s EXPORTED_FUNCTIONS="['_main', '_yourCfunc']"`` to make sure it
   is kept alive during optimization.

   If your function is used in other functions, LLVM may inline it and
   it will not show up. In this case you can define it with
   ``void EMSCRIPTEN_KEEPALIVE yourCfunc() {..}`` which will preserve it
   (see ``emscripten.h``).

   Another possible issue here is linking of ``.a`` files. ``.a`` files
   link only the internal object files needed by previous files on the
   command line, so the order of files matters, and this can be
   surprising. If you are linking ``.a`` files, make sure they are at
   the end of the list of files, and in the right order amongst
   themselves, or just use ``.so`` files instead in your project.

   Note: In LLVM 3.2 dead code elimination is significantly more
   aggressive. All functions not kept alive through EXPORTED\_FUNCTIONS
   will be potentially eliminated. Make sure to keep the things you need
   alive using one or both of those methods.

-  **Q.** The FS API is not available when I build with closure?

   **A.** Closure compiler will minify the FS API code. To write code
   that uses it, it must be optimized **with** the FS API code by
   closure. To do that, use emcc's ``--pre-js`` option, see
   ``emcc --help``.

-  **Q.** My code breaks with ``-O2 --closure 1``, giving odd errors..?

   **A.** The likely problem is that Closure Compiler minifies variable
   names. Names like ``i,j,xa`` can be generated, and if other code has
   such variables in the global scope, bad things can happen.

   To check if this is the problem, compile with ``-O2`` (without
   ``--closure 1``). If that works, name minification might be the
   problem. If so, wrapping the generated code in a closure should fix
   it. (Or, wrap your other code in a closure, or stop it from using
   small variable names in the global scope, you might be using such
   variables by mistake by forgetting a ``var`` and assigning to a
   variable - which makes it be in the global scope.)

   To 'wrap' code in a closure, do something like this:

::

    var CompiledModule = (function() {
      .. GENERATED CODE ..
      return Module;
    })();

-  **Q.** I get ``undefined is not a function`` or
   ``NAME is not a function``..?

   **A.** The likely cause is an undefined function - something that was
   referred to, but not implemented or linked in. If you get
   ``undefined``, look at the line number to see the function name.

   Emscripten by default does *not* give fatal errors on undefined
   symbols, so you can get runtime errors like these (because in
   practice in many codebases it is easiest to get them working without
   refactoring them to remove all undefined symbol calls). If you prefer
   compile-time notifications, run emcc with
   ``-s WARN_ON_UNDEFINED_SYMBOLS=1`` or
   ``-s ERROR_ON_UNDEFINED_SYMBOLS=1``.

   Aside from just forgetting to link in a necessary object file, one
   possible cause for this error is inline functions in headers. If you
   have a header with ``inline int my_func() { .. }`` then clang may not
   actually inline the function (since inline is just a hint), and also
   not generate code for it (since it's in a header), so the generated
   bitcode and js will not have that function implemented. One solution
   is to add ``static``, that is ``static inline int my_func() { .. }``
   which forces code to be generated in the object file.

-  **Q.** I get an odd python error complaining about libcxx.bc or
   libcxxabi.bc..?

   **A.** Possibly building libcxx or libcxxabi failed. Go to
   system/lib/libcxx (or libcxxabi) and do ``emmake make`` to see the
   actual error. Or, clean the emscripten cache
   (``~/.emscripten_cache``) and then compile your file with
   ``EMCC_DEBUG=1`` in the environment. libcxx will then be built in
   ``/tmp/emscripten_temp/libcxx``, and you can see ``configure*,make*``
   files that are the output of configure and make, etc.

   One possible cause of this error is the lack of ``make``, which is
   necessary to build these libraries. If you are on Windows, you need
   cygwin which supplies ``make``.

-  **Q.** Running LLVM bitcode generated by emcc through ``lli`` breaks
   with errors about ``impure_ptr`` stuff..?

   **A.** First of all, lli is not maintained (sadly) and has odd errors
   and crashes. However there is ``tools/nativize_llvm.py`` which
   compiles bitcode to a native executable. It will also hit the
   ``impure_ptr`` error though.

   The issue is that newlib uses that impure pointer stuff, while glibc
   uses something else. So bitcode build with the emscripten SDK (which
   emcc does) will not run locally, unless your machine uses newlib
   (which basically only embedded systems do). The impure\_ptr stuff is
   limited, however, it only applies to explicit use of ``stdout`` etc.
   So ``printf(..)`` will work, but ``fprintf(stdout, ..)`` will not. So
   often it is simple to modify your code to not hit this problem.

-  **Q.** I get a stack size error when optimizing
   (``RangeError: Maximum call stack size exceeded`` or similar)?

   **A.** You may need to increase the stack size for node. On linux and
   mac, you can just do ``NODE_JS = ['node', '--stack_size=8192']`` or
   such (in ``~/.emscripten``). On windows, you will also need
   ``--max-stack-size=8192``, and also to run
   ``editbin /stack:33554432 node.exe``.

-  **Q.** I get
   ``error: cannot compile this aggregate va_arg expression yet`` and it
   says ``compiler frontend failed to generate LLVM bitcode, halting``
   afterwards.

   **A.** This is a limitation of the le32 frontend in clang. You can
   use the x86 frontend instead by compiling with
   ``EMCC_LLVM_TARGET=i386-pc-linux-gnu`` in the environment (however
   you will lose the advantages of le32 which includes better alignment
   of doubles).

-  **Q.** I am building a library, and sometimes I get an error when
   I call a compiled function before the page fully loaded. How can
   I tell when is safe to call it?

   **A.** The easiest way to find out when loading is complete (which
   is asynchronous, as it often requires loading files, such as the
   .mem file or preloade files), is to just add a ``main()`` function. It
   will be called when it is safe to do so, after startup is complete,
   so that is a signal that it is safe to call any compiled method.
   You can make ``main()`` call your JS to notify it, for example if
   ``allReady()`` is a JS function you want called when everything is
   ready, you can do

   ::

       #include <emscripten.h>
       int main() {
         EM_ASM( allReady() );
       }

