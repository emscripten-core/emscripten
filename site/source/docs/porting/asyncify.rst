.. Asyncify:

========================
Asyncify
========================

Asyncify lets **synchronous** C or C++ code interact with **asynchronous**
JavaScript. This allows things like:

 * A synchronous call in C that yields to the event loop, which
   allows browser events to be handled.
 * A synchronous call in C that waits for an asynchronous operation in JS to
   complete.

Asyncify automatically transforms your compiled code into a form that can be
paused and resumed, and handles pausing and resuming for you, so that it is
asynchronous (hence the name "Asyncify") even though you wrote it in a normal
synchronous way.

See the
`Asyncify introduction blogpost <https://kripken.github.io/blog/wasm/2019/07/16/asyncify.html>`_
for general background and details of how it works internally. The following
expands on the Emscripten examples from that post.

.. note:: This post talks about Asyncify using the new LLVM wasm backend.
          There was an older Asyncify implementation for the old fastcomp
          backend. The two algorithms and implementations are entirely separate,
          so if you are using fastcomp, these docs may not be accurate - you
          should upgrade to the wasm backend and new Asyncify!

Sleeping / yielding to the event loop
#####################################

Let's begin with the example from that blogpost:

.. code-block:: cpp

    // example.cpp
    #include <emscripten.h>
    #include <stdio.h>

    // start_timer(): call JS to set an async timer for 500ms
    EM_JS(void, start_timer, (), {
      Module.timer = false;
      setTimeout(function() {
        Module.timer = true;
      }, 500);
    });

    // check_timer(): check if that timer occurred
    EM_JS(bool, check_timer, (), {
      return Module.timer;
    });

    int main() {
      start_timer();
      // Continuously loop while synchronously polling for the timer.
      while (1) {
        if (check_timer()) {
          printf("timer happened!\n");
          return 0;
        }
        printf("sleeping...\n");
        emscripten_sleep(100);
      }
    }

You can compile that with

::

    emcc -O3 example.cpp -s ASYNCIFY

.. note:: It's very important to optimize (``-O3`` here) when using Asyncify, as
          unoptimized builds are very large.

And you can run it with

::

    nodejs a.out.js

You should then see something like this:

::

    sleeping...
    sleeping...
    sleeping...
    sleeping...
    sleeping...
    timer happened!

The code is written with a straightforward loop, which does not exit while
it is running, which normally would not allow async events to be handled by the
browser. With Asyncify, those sleeps actually yield to the browser's main event
loop, and the timer can happen!

Making async Web APIs behave as if they were synchronous
########################################################

Aside from ``emscripten_sleep`` and the other standard sync APIs Asyncify
supports, you can also add your own functions. To do so, you must create a JS
function that is called from wasm (since Emscripten controls pausing and
resuming the wasm from the JS runtime). One way to do that is with a JS library
function; another is to use ``EM_JS``, which we'll use in this next example:

.. code-block:: cpp

    // example.c
    #include <emscripten.h>
    #include <stdio.h>

    EM_JS(void, do_fetch, (), {
      Asyncify.handleSleep(function(wakeUp) {
        out("waiting for a fetch");
        fetch("a.html").then(response => {
          out("got the fetch response");
          // (normally you would do something with the fetch here)
          wakeUp();
        });
      });
    });

    int main() {
      puts("before");
      do_fetch();
      puts("after");
    }


The async operation happens in the ``EM_JS`` function ``do_fetch()``, which
calls ``Asyncify.handleSleep``. It gives that function the code to be run, and
gets a ``wakeUp`` function that it calls in the asynchronous future at the right
time. After we call ``wakeUp()`` the compiled C code resumes normally.

In this example the async operation is a ``fetch``, which means we need to wait
for a Promise. While that is async, note how the C code in ``main()`` is
completely synchronous!

To run this example, first compile it with

::

    ./emcc example.c -O3 -o a.html -s ASYNCIFY -s 'ASYNCIFY_IMPORTS=["do_fetch"]'

Note that you must tell the compiler that ``do_fetch()`` can do an
asynchronous operation, using ``ASYNCIFY_IMPORTS``, otherwise it won't
instrument the code to allow pausing and resuming; see more details later down.

To run this, you must run a webserver (like say ``python -m SimpleHTTPServer``)
and then browse to ``http://localhost:8000/a.html`` (the URL may depend on the
port number in the server). You will see something like this:

::

    before
    waiting for a fetch
    got the fetch response
    after

That shows that the C code only continued to execute after the async JS
completed.

More on ``ASYNCIFY_IMPORTS``
############################

As in the above example, you can add JS functions that do an async operation but
look synchronous from the perspective of C. The key thing is to add such methods
to ``ASYNCIFY_IMPORTS``, regardless of whether the JS function is from a JS
library or ``EM_JS``. That list of imports is the list of imports to the wasm
module that the Asyncify instrumentation must be aware of. Giving it that list
tells it that all other JS calls will **not** do an async operation, which lets
it not add overhead where it isn't needed.

The ``ASYNCIFY_IMPORTS`` list must contain **all** relevant imports, not just
ones you add yourself, so it must contain things like ``emscripten_sleep()``
if you call them (by default the list will contain them, so you must only add
them if you change the list).

Returning values
################

You can also return values from async JS functions. Here is an example:

.. code-block:: cpp

    // example.c
    #include <emscripten.h>
    #include <stdio.h>

    EM_JS(int, get_digest_size, (const char* str), {
      // Note how we return the output of handleSleep() here.
      return Asyncify.handleSleep(function(wakeUp) {
        const text = UTF8ToString(str);
        const encoder = new TextEncoder();
        const data = encoder.encode(text);
        out("ask for digest for " + text);
        window.crypto.subtle.digest("SHA-256", data).then(digestValue => {
          out("got digest of length " + digestValue.byteLength);
          // Return the value by sending it to wakeUp(). It will then be returned
          // from handleSleep() on the outside.
          wakeUp(digestValue.byteLength);
        });
      });
    });

    int main() {
      const char* silly = "some silly text";
      printf("%s's digest size is: %d\n", silly, get_digest_size(silly));
      return 0;
    }

You can build this with

::

    ../emcc example.c -s ASYNCIFY=1 -s 'ASYNCIFY_IMPORTS=["get_digest_size"]' -o a.html -O2

This example calls the Promise-returning ``window.crypto.subtle()`` API (the
example is based off of
`this MDN example <https://developer.mozilla.org/en-US/docs/Web/API/SubtleCrypto/digest#Basic_example>`_
). Note how we pass the value to be returned into ``wakeUp()``. We must also
return the value returned from ``handleSleep()``. The calling C code then
gets it normally, after the Promise completes.

Optimizing
##########

As mentioned earlier, unoptimized builds with Asyncify can be large and slow.
Build with optimizations (say, ``-O3``) to get good results.

Asyncify adds overhead, both code size and slowness, because it instruments
code to allow unwinding and rewinding. That overhead is usually not extreme,
something like 50% or so. Asyncify achieves that by doing a whole-program
analysis to find functions need to be instrumented and which do not -
basically, which can call something that reaches one of
``ASYNCIFY_IMPORTS``. That analysis avoids a lot of unnecessary overhead,
however, it is limited by **indirect calls**, since it can't tell where
they go - it could be anything in the function table (with the same type).

If you know that indirect calls are never on the stack when unwinding, then
you can tell Asyncify to ignore indirect calls using
``ASYNCIFY_IGNORE_INDIRECT``.

If you know that some indirect calls matter and others do not, then you
can provide a manual list of functions to Asyncify:

* ``ASYNCIFY_BLACKLIST`` is a list of functions that do not unwind the stack.
  Asyncify will do it's normal whole-program analysis under the assumption
  that those do not unwind.
* ``ASYNCIFY_WHITELIST`` is a list of the **only** functions that can unwind
  the stack. Asyncify will instrument those and no others.

For more details see ``settings.js``. Note that the manual settings
mentioned here are error-prone - if you don't get things exactly right,
your application can break. If you don't absolutely need maximal performance,
it's usually ok to use the defaults.

Potential problems
##################

Stack overflows
***************

If you see an exception thrown from an ``asyncify_*`` API, then it may be
a stack overflow. You can increase the stack size with the
``ASYNCIFY_STACK_SIZE`` option.

Reentrancy
**********

While waiting on an asynchronous operation browser events can happen. That
is often the point of using Asyncify, but unexpected events can happen too.
For example, if you just want to pause for 100ms then you can call
``emscripten_sleep(100)``, but if you have any event listeners, say for a
keypress, then if a key is pressed the handler will fire. If that handler
calls into compiled code, then it can be confusing, since it starts to look
like coroutines or multithreading, with multiple executions interleaved.

It is *not* safe to start an async operation while another is already running.
The first must complete before the second begins.

Such interleaving may also break assumptions in your codebase. For example,
if a function uses a global and assumes nothing else can modify it until it
returns, but if that function sleeps and an event causes other code to
change that global, then bad things can happen.

Migrating from older APIs
#########################

If you have code using the Emterpreter-Async API, or the old Asyncify, then the
new API is somewhat different, and you may need some minor changes:

 * The Emterpreter has "yielding" as a concept, but it isn't needed in Asyncify.
   You can replace ``emscripten_sleep_with_yield()`` calls with ``emscripten_sleep()``.
 * The JS API is different. See notes above on ``Asyncify.handleSleep()``, and
   see ``src/library_async.js`` for more examples.

