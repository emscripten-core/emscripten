.. _Debugging:

=======================
Debugging (wiki-import)
=======================
.. note:: This article was migrated from the wiki (Fri, 25 Jul 2014 04:21) and is now the "master copy" (the version in the wiki will be deleted). It may not be a perfect rendering of the original but we hope to fix that soon!

Debugging
=========

For a quick overview of debugging Emscripten-generated code, see `these
slides <http://people.mozilla.org/~lwagner/gdc-pres/gdc-2014.html#/20>`__

Limitation or Bug?
------------------

Emscripten can compile almost but not all C/C++ code out there. Some
limitations exist, see :ref:`CodeGuidelinesAndLimitations`.

Aside from these limitations, it's possible you ran into a bug in
Emscripten, such as:

-  Missing functionality. For example, a library function which hasn't
   yet been implemented in Emscripten. Possible solutions here are to
   implement the function (see ``library.js``) or to compile it from
   C++.
-  An actual mistake in Emscripten. Please report it!

Optimizations
-------------

Try to build without optimizations (no ``-O1`` etc.). If that has an
effect, you can try to disable LLVM optimizations specifically using
``--llvm-opts 0``, see ``emcc --help``.

Build with ``EMCC_DEBUG=1`` to get intermediate files for the compiler's
various stages (output to ``/tmp/emscripten_temp``). ``EMCC_DEBUG=2``
will emit more intermediate files (one for each JS optimizer pass).

Useful Compilation Settings
---------------------------

As already mentioned, some useful settings appear in
``src/settings.js``. Change the settings there and then recompile the
code to have them take effect. When code isn't running properly, you
should compile with ``ASSERTIONS``, and if that doesn't clear things up,
then ``SAFE_HEAP``. ``ASSERTIONS`` adds various runtime checks, and
``SAFE_HEAP`` adds even more (slow) memory access checks like
dereferencing 0 and memory alignment issues.

Memory Alignment Issues
-----------------------

``SAFE_HEAP`` will reveal memory alignment issues - where your code is
assuming a higher alignment than appears in practice. Unaligned reads
and writes can lead to incorrect results, as our typed array model of
memory does not support them.

The best solution is to avoid unaligned reads and writes. Generally they
occur as the result of undefined behavior.

In some cases, however, you may need unaligned reads and writes, for
example to read an int from a packed structure in some pre-existing data
format. To force an unaligned read or write (which will be slower, but
work properly), you can either manually read individual bytes and
reconstruct the full value, or use the ``emscripten_align*`` typedefs in
``emscripten.h``, which define unaligned versions of the basic types
(short, int, float, double). All operations on those types are not fully
aligned (use the ``1`` variants in most cases, which mean no alignment
whatsoever). They are slower due to lack of alignment, though, so be
sure to only use them when absolutely necessary.

Function Pointer Issues
-----------------------

If you get an abort() from a function pointer call (nullFunc or b0 or b1
or such, possibly with an error message saying "incorrect function
pointer"), the issue is that a function pointer was called but it is
invalid in that type.

It is undefined behavior to cast a function pointer to another type and
call that (e.g., cast away the last parameter), but this does happen in
real-world code, and is one possible cause for this error. In optimized
emscripten output, each function pointer type has a different table of
entries, so you must call with the correct type to get the right
behavior.

Another possible cause is a dereference of 0, like calling a method on a
NULL pointer or such. That can be a bug in the code caused by any
reason, but shows itself as a function pointer error (as just reading or
writing to a NULL pointer will work, unlike in native builds - it is
just function pointers that will always fail when NULL).

Building with ``-Werror`` (turn warnings into errors) can help here, as
some cases of undefined behavior show warnings. If you can get your
codebase to build with ``-Werror`` that might help.

Use ``-s ASSERTIONS=2`` to get some useful information about the
function pointer being called, and its type. Also useful is to look at
the stack trace (may want to disable asm.js optimizations in firefox to
see the best trace information) to see where in your code the error
happens, then see which function should be called but isn't.

SAFE\_HEAP is also useful when debugging issues like this, as is
disabling function pointer aliasing using
ALIASING\_FUNCTION\_POINTERS=0. When you build with
``-s SAFE_HEAP=1 -s ALIASING_FUNCTION_POINTERS=0`` then it should be
impossible for a function pointer to be called with the wrong type
without an error showing up.

Another possible problem with function pointers is that what appears to
be the wrong function is called. Again, ``SAFE_HEAP`` can help with this
as it detects some possible errors with function table accesses.

Inspecting the Generated Code
-----------------------------

See the slides linked to before for the ``-g`` options.

Another thing you might find useful is to not run JS optimizations, to
leave inline source code hints. You can try something like

::

    /emcc -O2 --js-opts 0 -g4 tests/hello_world_loop.cpp

which applies only LLVM opts, and basic JS opts but not the JS
optimizer, which retains debug info, giving

::

    function _main() {
     var label = 0;
     var $puts=_puts(((8)|0)); //@line 4 "tests/hello_world.c"
     return 1; //@line 5 "tests/hello_world.c"
    }

Infinite loops
--------------

If your code hits an infinite loop, one easy way to see where that
happens is to use a JS profiler. In the Firefox profiler for example it
is easy to see which code ran at what time, so if the code enters an
infinite loop for a while (before the browser shows the slow script
dialog and you quit it), you will see a block of code doing the same
thing near the end of the profile.

Debugging Emscripten Issues
===========================

If you think you may have hit an Emscripten codegen bug, there are a few
tools to help you.

The AutoDebugger
----------------

The 'nuclear option' when debugging is to use the **autodebugger tool**.
The autodebugger will rewrite the LLVM bitcode so it prints out each
store to memory. You can then run the exact same LLVM bitcode in the
LLVM interpreter (lli) and JavaScript, and compare the output (``diff``
is useful if the output is large). For how to use the autodebugger tool,
see the ``autodebug`` test.

The autodebugger can potentially find **any** problem in the generated
code, so it is strictly more powerful than the ``CHECK_*`` settings and
``SAFE_HEAP``. However, it has some limitations:

-  The autodebugger generates a lot of output. Using ``diff`` can be
   very helpful here.
-  The autodebugger doesn't print out pointer values, just simple
   numerical values. The reason is that pointer values change from run
   to run, so you can't compare them. However, on the one hand this may
   miss potential problems, and on the other, a pointer may be converted
   into an integer and stored, in which case it would be shown but it
   should be ignored. (You can modify this, look in
   ``tools/autodebugger.py``.)

One use of the autodebugger is to quickly emit lots of logging output.
You can then take a look and see if something weird pops up. Another use
is for regressions, see below.

AutoDebugger Regression Workflow
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Fixing regressions is pretty easy with the autodebugger, using the
following workflow:

-  Compile the code using ``EMCC_AUTODEBUG=1`` in the environment.
-  Compile the code using ``EMCC_AUTODEBUG=1`` in the environment,
   again, but with a difference emcc setting etc., so that you now have
   one build before the regression and one after.
-  Run both versions, saving their output, then do a diff and
   investigate that. Any difference is likely the bug (other false
   positives could be things like the time, if something like
   ``clock()`` is called, which differs slightly between runs).

(You can also make the second build a native one using the llvm
nativizer tool mentioned above - run it on the autodebugged .ll file,
which EMCC\_DEBUG=1 will emit in ``/tmp/emscripten_temp``. This helps
find bugs in general and not just regressions, but has the same issues
with the nativizer tool mentioned earlier.)

Debug Info
----------

It can be very useful to compile the C/C++ files with ``-g`` flag to get
debugging into - Emscripten will add source file and line number to each
line in the generated code. Note, however, that attempting to interpret
code compiled with ``-g`` using ``lli`` may cause crashes. So you may
need to build once without ``-g`` for ``lli``, then build again with
``-g``. Or, use ``tools/exec_llvm.py`` in Emscripten, which will run lli
after cleaning out debug info.

Additional Tips
---------------

You can also do something similar to what the autodebugger does,
manually - modify the original source code with some ``printf()``\ s,
then compile and run that, to investigate issues.

Another useful tip is if you have a good idea of what line is
problematic in generated .js, you can add ``print(new Error().stack)``
to get a stack trace there. There is also :js:func:`stackTrace` which emits a
stack trace and also tries to demangle C++ function names.

Useful Links
------------

`Blogpost about reading compiler
output <http://mozakai.blogspot.com/2014/06/looking-through-emscripten-output.html>`__

Additional Help
---------------

Of course, you can also ask the Emscripten devs for help. :) See links
to IRC and the Google Group on the main project page.
