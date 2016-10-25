.. Asyncify:

========================
Asyncify
========================

In general, you want to make your code as asynchronous-friendly as possible, e.g. by never calling sleep, having a single main loop and so on.

Sometimes refactoring a codebase with this in mind isn't feasible, so there are a couple of alternatives (each with downsides). Asyncify is covered here.

**Asyncify is experimental, and not recommended. See https://kripken.github.io/emscripten-site/docs/porting/emterpreter.html for a more recent option with similar functionality, that is currently supported.**

``ASYNCIFY`` allows you to use some asynchronous function in C, through several transformation of LLVM IR.

Intro
=====

If you call ``sleep()`` in C/C++, what kind of JavaScript code would you expect emscripten to produce?

::

    // test.c
    #include <stdio.h>
    #include <unistd.h>
    int main() {
      int i = 100;
      printf("Hello\n");
      sleep(1);
      printf("World %d!\n");
      return 0;
    }

Note that we cannot implement ``sleep`` like this::

    function sleep(ms) {
      var t = Date.now() + ms;
      while(Date.now() < t) ;
    }

because this would block the JavaScript engine, such that pending events cannot be processed.


A hand written counterpart in JavaScript would be

::

    function main() {
      var i = 100;
      console.log('Hello');
      setTimeout(function() {
        console.log('World ' + i + '!');
        async_return_value = 0;
      }, 1000);
    }

Specifically, a number of aspects should be taken into consideration:
 - Split the function when an async function is called, and the second function should be registered as the callback for the async function
 - Any function that calls an async function also becomes an async function.
 - Keep all local variables available to the callback
 - Closure cannot be used in order to make asm.js validated code. 
 - Take care of loops and branches
 - Make the return value available to the callee
 - Some functions could be both sync or async, depending on the input.
 
And the ``ASYNCIFY`` option does all above automatically, through a number of transformations on LLVM IR.


Usage
=====

Call ``emscripten_sleep()`` whenever you need to pause the program, and add ``-s ASYNCIFY=1`` to emscripten.

Sometimes it's a good replacement of ``emscripten_set_main_loop``, you may replace all ``sleep``-alike functions with ``emscripten_sleep``, instead of refactoring the whole main loop.

Also ``emscripten_sleep(1)`` can be used to 'interrupt' your code, such that the JavaScript engine can do the rendering and process events.

Extensions
==========

It is possible to implement more new async functions that appears to be sync in C.

- Implement the function normally in a JavaScript library, suppose the function name is ``func``.
- Add ``func`` into ``ASYNCIFY_FUNCTIONS``
- When ``func`` is called and finished, the program will NOT continue, instead it just save the context and exit.
- Call ``_emscripten_async_resume`` when you want to resume the program, usually in the callback functions of some async calls.

Please read ``src/library_async.js`` for details.

Limitations
===========

Code size increase should be expected, depending on the specific input.
``-Os`` (or ``-Oz`` for linking) is recommended when ``ASYNCIFY`` is turned on. 
E.g. usually the following loop is expanded to speed up::

    for(int i = 0; i < 3; ++i) {
      // do something
      emscripten_sleep(1000);
      // do something else
    }

However by expanding the loop, two more async calls are introduced, such that more callback functions will be produced during the asyncify transformation. 

**Asyncify can make performance much slower, if it ends up splitting a function which you need to be fast.**

``setjmp/longjmp`` and C++ exception are not working well when there are async function calls in the scope, but they still work when there's no async calls. E.g.

::

    try {
      // do something
      if(error) throw 0; // works
      emscripten_sleep(1000);
      // do something else
      if(error) throw 0; // does not work
    }

Currently all function pointer calls are considered as aync, and some functions might be recognized as async incorrectly. This can be corrected by manually setting the ``ASYNCIFY_WHITELIST`` option.


Other possible implementations
==============================

 - Closures (breaking asm.js)
 - Generators (too slow currently)
 - Blocking message (in workers)
