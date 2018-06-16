.. Emterpreter:

==============================
Emterpreter
==============================

The **Emterpreter** is an option that compiles asm.js output from Emscripten into a binary bytecode. It also generates an interpreter ("Emscripten interpreter", hence *Emterpreter*) capable of executing that bytecode. This lets you compile your project, or parts of your project, into bytecode that will be interpreted, as opposed to asm.js that will be executed directly by the JavaScript engine.

Why does this option exist? To provide an alternative in situations where normal direct execution by the JavaScript engine has issues. The two main motivations are

 * JavaScript must be parsed and compiled before it is executed, which can take a long time in large codebases, whereas a binary bytecode is just data, so you can get to the point of *something* executing earlier. Executing in an interpreter might be slower, but it can start earlier. Or in other words, it can already be running (albeit slowly) before the code would normally be running at all.
 * JavaScript has high-level control flow (no gotos) and must be written as short-running events, not long-running synchronous code. However, sometimes you have code that is written in the latter form that you can't easily refactor. The Emterpreter can handle that, because running the code in an interpreter allows us to manually control the flow of execution, as well as pause and resume the entire call stack, letting us turn synchronous code into asynchronous code.

For more background on the Emterpreter, see
 * `Emterpreter startup time blogpost <https://blog.mozilla.org/research/2015/02/23/the-emterpreter-run-code-before-it-can-be-parsed/>`_
 * `Emterpreter synchronous execution blogpost <https://hacks.mozilla.org/2015/02/synchronous-execution-and-filesystem-access-in-emscripten/>`_

General Usage
=============

To use the Emterpreter, build with ``-s EMTERPRETIFY=1``. This runs all code in the interpreter by default. You can also use the ``EMTERPRETIFY_BLACKLIST`` option to specify the only methods **not** to be interpreted, or ``EMTERPRETIFY_WHITELIST`` to specify the only methods that **are** to be interpreted.

You can optionally use ``-s 'EMTERPRETIFY_FILE="data.binary"'`` to store the emterpreter bytecode in a file (called ``data.binary`` in this example). Otherwise, the default is to store the bytecode in the JS file itself, which is very inefficient for binary data. For small amounts of bytecode it is fine, but for large applications you should really use a file, otherwise code size and startup can be bad (you should receive a warning when building a large application without this option).

As usual, you can grep the ``tests/`` folder for examples of emterpreter usage in the test suite (search for ``EMTERPRETIFY``).

    .. note:: Setting ``EMTERPRETIFY=1`` forces ``js-opts=1``, because we convert to the emterpreter's binary format using a js-optimizer pass.

Specific Use Cases
==================

A couple of examples are given below of how the Emterpreter can be useful in general.

Improving Startup 1: Swapping
-----------------------------

As mentioned earlier, the Emterpreter and its binary bytecode load faster than JavaScript and asm.js can. Just building with the Emterpreter option gives you that, but it also makes the code run more slowly. A hybrid solution is to start up quickly in the Emterpreter, then switch to faster execution in full asm.js speed later. This is possible by **swapping** the asm.js module - first load the Emterpreted one, then load the fast one in the background and switch to it when it's ready.

To do this, build the project twice:

 * Once with the Emterpreter option enabled, and ``SWAPPABLE_ASM_MODULE``. This is the module you will start up with, and swap out when the fast one is ready.
 * Again to normal asm.js, then run ``tools/distill_asm.py infile.js outfile.js swap-in``. The output, ``outfile.js``, will be just the asm module itself. You can then load this in a script tag on the same page, and it will swap itself in when it is ready.

Improving Startup 2: Set Aside Cold Code
----------------------------------------

If you have a method that you know will only ever run exactly once, and doesn't need to be fast, you can run that specific method in the Emterpreter: As mentioned above, the JavaScript engine won't need to compile it, and the bytecode is smaller than asm.js, so both download and startup will be faster. Another example is exception-handling or assertion reporting code, something that should never run, and if it does, is ok to run at a slower speed.

To do this, simply use the whitelist option mentioned before, with a list of the methods you want to be run as bytecode.

Emterpreter-Async: Run Synchronous Code
=======================================

The emterpreter runs the code in an interpreter, which makes it feasible to manually control the call stack and so forth. To enable this support, build with ``-s EMTERPRETIFY_ASYNC=1``. You can then write synchronous-looking code, and it will "just work", e.g.

::

    while (1) {
      do_frame();
      emscripten_sleep(10);
    }

is a simple way to do a main loop, which typically you would refactor your code for and use ``emscripten_set_main_loop``. Instead, in the emterpreter the call to ``emscripten_sleep`` will save the execution state, including call stack, do a ``setTimeout`` for the specified amount of milliseconds, and after that delay, reconstruct the execution state exactly as it was before. From the perspective of the source code, it looks like synchronous sleep, but under the hood it is converted to a form that can work in a web browser asynchronously.

For a list of the APIs that can be used in this synchronous manner, see the `docs <http://kripken.github.io/emscripten-site/docs/api_reference/emscripten.h.html#emterpreter-async-functions>`_.

When using sleep in this manner, you can likely use the emterpreter whitelist very efficiently: Only things that can lead to a call to sleep (or another synchronous method) need to be emterpreted. In the example above, ``do_frame`` and everything that could call it should be in the whitelist, so that everything else runs at full asm.js speed. More specifically, for the interpreter to be able to save and later restore the state of execution, the current call stack must only contain emterpreted functions, not normal asm.js functions, and not functions from outside that are not compiled code. To save the state of execution, the interpreter records its current location and all variables on the stack, both of which we cannot do for code that is not run in the interpreter. Note that this means that when you use ``ccall`` to call code which does an asynchronous operation, you cannot treat this call like it was synchronous in your JS code. Instead you get back a JS Promise of the return value (see the `async option <https://kripken.github.io/emscripten-site/docs/api_reference/preamble.js.html#ccall>`_ on ccall).

Semantics
---------

``emscripten_sleep`` and other synchronous methods are meant to actually **be** synchronous. When they execute, control returns to the browser's main event loop (which allows rendering to show up), however, we attempt to block other asynchronous events. That is, if you do an ``emscripten_async_wget`` and then an ``emscripten_sleep``, the asynchronous wget will **not** execute during the sleep. The sleep must complete first. This keeps things in alignment with how synchronous code would work in C.

If you **do** want asynchronous events during sleep, use ``emscripten_sleep_with_yield``. This is not fully tested yet, however, and may need rethinking.

Deciding on which methods to Emterpret for async
------------------------------------------------

There are both static and dynamic tools that can help here.

Static Analysis
~~~~~~~~~~~~~~~

Building with ``EMTERPRETIFY_ADVISE`` will process the project and perform a static analysis to determine which methods should probably be run in the interpreter. This checks which methods *could* be on the stack underneath a call to a synchronous method, in which case they must be interpreted so that we can save and restore the stack later in an asynchronous way.

The analysis is pessimistic, in that it checks what *could* possibly be called, but might not in practice. For example, function pointers are hard to figure out: Even though the analysis takes into account the **type** of function pointer, if you call a ``void (int)`` method by a function pointer, then the analysis must assume that any ``void (int)`` method (that ever has its address taken, i.e., *could* be called via a function pointer) could be called there. For example, on Doom it suggests that 31% (!) of all methods should be interpreted, while in practice only 1% need to be (as is easy to verify by reading the code).

If you have written custom synchronous functions, use ``EMTERPRETIFY_SYNCLIST`` to specify them and ``EMTERPRETIFY_ADVISE`` will include them in its analysis along with the standard synchronous functions.

::

    -s EMTERPRETIFY_SYNCLIST='["_custom_func_a","_custom_func_b"]'

Dynamic Tools
~~~~~~~~~~~~~

By building with ``-s ASSERTIONS=1``, you can get runtime errors on not having methods interpreted that should be. This will catch only problems that occur **in practice**, so it is an optimistic approach (the opposite of the static analysis route). But by running your codecase on a representative workload, this approach should give you very useful results. Here is how you can do this:

First, make sure all synchronous execution works correctly when interpreting **everything**, by building with ``-s EMTERPRETIFY=1 -s EMTERPRETIFY_ASYNC=1``. Might be slow, but it should work!

Then, run only ``main()`` in the interpreter, by adding ``-s EMTERPRETIFY_WHITELIST='["_main"]'``, and building with assertions, ``-s ASSERTIONS=1,`` as well as preserving (not minifying) function names, ``--profiling-funcs``. Any synchronous execution not happening in main itself is in non-interpreted code, and therefore bad; in a build with assertions, this will trigger a runtime error you can view in the web console in your browser, and thanks to the profiling option, the error will have clearly readable function names, for example, you could see this on Doom::

    This error happened during an emterpreter-async save or load of the stack. Was there non-emterpreted code on the stack during save (which is unallowed)? This is what the stack looked like when we tried to save it:

    jsStackTrace@file:///home/alon/Dev/boon/boon.js:1:26546
    stackTrace@file:///home/alon/Dev/boon/boon.js:1:26729
    EmterpreterAsync.handle@file:///home/alon/Dev/boon/boon.js:1:196637
    _emscripten_sleep@file:///home/alon/Dev/boon/boon.js:1:196851
    _D_DoomLoop@file:///home/alon/Dev/boon/boon.js:12:188349
    _D_DoomMain@file:///home/alon/Dev/boon/boon.js:12:186664
    emterpret@file:///home/alon/Dev/boon/boon.js:11:10149
    _main@file:///home/alon/Dev/boon/boon.js:12:76663
    asm._main@file:///home/alon/Dev/boon/boon.js:19:985
    callMain@file:///home/alon/Dev/boon/boon.js:19:31106
    doRun@file:///home/alon/Dev/boon/boon.js:19:32001
    run/<@file:///home/alon/Dev/boon/boon.js:19:32169

You can see ``main()`` at the bottom (below it is how main is invoked, which you can ignore), then a call into the ``emterpret()`` function, which is how main invokes the interpreter for itself (``main()`` itself is just a little "trampoline" that jumps into the interpreter and tells it which bytecode to run). Above that, we can see ``_D_DoomMain`` and ``_D_DoomLoop``. Those two methods must be interpreted, since higher up on the stack trace you can see a call to emscripten_sleep(), which is synchronous.

Adding those methods to the whitelist of interpreted functions, you can then build and run the application again, and repeat this process until everything works properly. You should still carefully review your codebase and see what should be interpreted, but the semi-automatic process described here is easy to use and can be very effective in practice, if you test all relevant code paths.

**Warning**: The runtime checks that ASSERTIONS adds guards against compiled code that is not interpreted. But it does not protect you from non-compiled code. For example, if a compiled method calls a non-compiled method, which then calls back into compiled code, we cannot save and restore the stack: Even if the compiled methods are interpreted, the non-compiled one has no way for us to save its current execution state. If you try to run synchronous code in this incorrect manner, things will fail in potentially confusing ways: what happens is the emterpreted code returns immediately (in order to wait for the asynchronous callback), and your handwritten code underneath it will then continue to execute, not knowing that the code just returning has not yet completed.

Inlining
~~~~~~~~

A potentially confusing issue can arise through function inlining: If a parent method calls a method that will sleep, and another that won't, only the former of the two children needs to be interpreted (as well as the parent function). But, if both are inlined into the parent, then they are all now one function, which must be interpreted.

To obtain optimal performance, you may want to mark some non-interpreted methods called from interpreted methods as no-inline, using ``__attribute__((noinline))``. That avoids their code ending up running in the interpreter.

Note that this will only be a performance issue, not correctness - by inlining, a child's code ends up in the parent function, and since we need everything on the stack during a sync call to be interpreted, it is ok to inline among those, or even inline from non-interpreted functions as well. In other words, it just adds methods to be interpreted, which is always safe, at the cost of performance.

Comparison to ASYNCIFY
----------------------

ASYNCIFY is an earlier experiment on running synchronous code. It does a whole-program analysis in LLVM and modifies all relevant methods to they can be saved and resumed, by breaking them up and so forth. Comparing the two,

 * ASYNCIFY has a bad worst-case of large code size: If it needs to modify many methods, it can grow code size very significantly (even 10x more was seen). The emterpreter on the other hand has a guarantee of having smaller code size than normal emscripten output, simply because emterpreter bytecode is smaller than JS source. (Note: you should use ``EMTERPRETIFY_FILE`` to reduce code size, as without it the bytecode is stored in JS which is inefficient for binary data.)
 * ASYNCIFY is slower than normal emscripten output, but probably not hugely so, while the emterpreter can be much slower, because it interprets code. Using a whitelist or blacklist with the emterpreter, this can be mitigated.
 * There are some known bugs with ASYNCIFY on things like exceptions and setjmp. The emterpreter has not been tested on those feature yet, so it's unclear if it would work. Update: there are known issues with doing and async operation when there is a try-catch (llvm invoke) on the stack.
 * ASYNCIFY focused on a static analysis, while the Emterpreter-Async option has both a static analysis and dynamic tools to help figure out which methods should be treated in a special way to enable synchronous code.
 * As the emterpreter is useful for other things than synchronous code, it will likely continue to be worked on, while the ASYNCIFY option currently does not have activity.

Further reading
~~~~~~~~~~~~~~~

 * `DOSBox usage <http://dreamlayers.blogspot.com/2015/02/fixing-hard-problem-in-em-dosbox-using.html>`_

Debugging
=========

Stack traces when running the emterpreter can be a little confusing. Keep these things in mind:

 * When non-emterpreted code calls into emterpreted code, it has to go through a "trampoline", a little function that just calls ``emterpret()`` with the location of the code to execute. That's why you'll see ``main() -> emterpret()`` in your stack traces, ``main()`` is just a trampoline.
 * When calling between emterpreted code, there is an ``INTCALL`` opcode which does a direct call from ``emterpret()`` to another invocation of ``emterpret()``. That means that you do see a stack trace of the right size, but the names are all the same. Invoke emcc with ``--profiling-funcs`` or ``--profiling`` to have the emterpreter take a slower path of calling through trampolines all the time. This is useful for profiling.

Bytecode Design
===============

The bytecode is a simple register-based bytecode invented for this purpose, just enough to support the asm.js code that Emscripten emits. It is designed more for speed of execution and quick startup (no preprocessing necessary at all), than size.

It also has a bunch of "combo" opcodes for things like test+branch, etc. See ``tools/emterpretify.py`` for the list of opcodes.
