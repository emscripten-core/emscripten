.. _asyncify section:

=================
Asynchronous Code
=================

Emscripten supports two ways (Asyncify and JSPI) that let **synchronous** C or
C++ code interact with **asynchronous** JavaScript. This allows things like:

 * A synchronous call in C that yields to the event loop, which
   allows browser events to be handled.
 * A synchronous call in C that waits for an asynchronous operation in JS to
   complete.


In general the two options are very similar, but rely on different underlying
mechanisms to work.

  * `Asyncify` - Asyncify automatically transforms your compiled code into a
    form that can be paused and resumed, and handles pausing and resuming for
    you, so that it is asynchronous (hence the name "Asyncify") even though you
    wrote it in a normal synchronous way. This works in most environments, but
    can cause the Wasm output to be much larger.
  * `JSPI` (experimental) - Uses the VM's support for JavaScript Promise
    Integration (JSPI) for interacting with async JavaScript. The code size will
    remain the same, but support for this feature is still experimental.

For more on Asyncify see the
`Asyncify introduction blogpost <https://kripken.github.io/blog/wasm/2019/07/16/asyncify.html>`_
for general background and details of how it works internally (you can also view
`this talk about Asyncify <https://www.youtube.com/watch?v=qQOP6jqZqf8>`_).
The following expands on the Emscripten examples from that post.

.. _yielding_to_main_loop:

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

You can compile that using either `-sASYNCIFY` or `-sJSPI`

::

    emcc -O3 example.cpp -s<ASYNCIFY or JSPI>

.. note:: It's very important to optimize (``-O3`` here) when using Asyncify, as
          unoptimized builds are very large.

And you can run it with

::

    nodejs a.out.js

Or with JSPI

::

    nodejs --experimental-wasm-stack-switching a.out.js

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
browser. With Asyncify/JSPI, those sleeps actually yield to the browser's main event
loop, and the timer can happen!

Making async Web APIs behave as if they were synchronous
########################################################

Aside from ``emscripten_sleep`` and the other standard sync APIs Asyncify
supports, you can also add your own functions. To do so, you must create a JS
function that is called from Wasm (since Emscripten controls pausing and
resuming the Wasm from the JS runtime).

One way to do that is with a JS library function. Another is to use
``EM_ASYNC_JS``, which we'll use in this next example:

.. code-block:: cpp

    // example.c
    #include <emscripten.h>
    #include <stdio.h>

    EM_ASYNC_JS(int, do_fetch, (), {
      out("waiting for a fetch");
      const response = await fetch("a.html");
      out("got the fetch response");
      // (normally you would do something with the fetch here)
      return 42;
    });

    int main() {
      puts("before");
      do_fetch();
      puts("after");
    }

In this example the async operation is a ``fetch``, which means we need to wait
for a Promise. While that operation is async, note how the C code in ``main()``
is completely synchronous!

To run this example, first compile it with

::

    emcc example.c -O3 -o a.html -s<ASYNCIFY or JSPI>

To run this, you must run a :ref:`local webserver <faq-local-webserver>`
and then browse to ``http://localhost:8000/a.html``.
You will see something like this:

::

    before
    waiting for a fetch
    got the fetch response
    after

That shows that the C code only continued to execute after the async JS
completed.

Ways to use Asyncify APIs in older engines
##########################################

If your target JS engine doesn't support the modern ``async/await`` JS
syntax, you can desugar the above implementation of ``do_fetch`` to use Promises
directly with ``EM_JS`` and ``Asyncify.handleAsync`` instead:

.. code-block:: cpp

    EM_JS(int, do_fetch, (), {
      return Asyncify.handleAsync(function () {
        out("waiting for a fetch");
        return fetch("a.html").then(function (response) {
          out("got the fetch response");
          // (normally you would do something with the fetch here)
          return 42;
        });
      });
    });

When using this form, the compiler doesn't statically know that ``do_fetch`` is
asynchronous anymore. Instead, you must tell the compiler that ``do_fetch()``
can do an asynchronous operation using ``ASYNCIFY_IMPORTS``, otherwise it won't
instrument the code to allow pausing and resuming (see more details later down):

::

    emcc example.c -O3 -o a.html -sASYNCIFY -sASYNCIFY_IMPORTS=do_fetch

Finally, if you can't use Promises either, you can desugar the example to use
``Asyncify.handleSleep``, which will pass a ``wakeUp`` callback to your
function implementation. When this ``wakeUp`` callback is invoked, the C/C++
code will resume:

.. code-block:: cpp

    EM_JS(int, do_fetch, (), {
      return Asyncify.handleSleep((wakeUp) => {
        out("waiting for a fetch");
        fetch("a.html").then(function (response) {
          out("got the fetch response");
          // (normally you would do something with the fetch here)
          wakeUp(42);
        });
      });
    });

Note that when using this form, you can't return a value from the function itself.
Instead, you need to pass it as an argument to the ``wakeUp`` callback and
propagate it by returning the result of ``Asyncify.handleSleep`` in ``do_fetch``
itself.

More on ``ASYNCIFY_IMPORTS``
############################

As in the above example, you can add JS functions that do an async operation but
look synchronous from the perspective of C. If you don't use ``EM_ASYNC_JS``,
it's vital to add such methods to ``ASYNCIFY_IMPORTS``. That list of imports is
the list of imports to the Wasm module that the Asyncify instrumentation must be
aware of. Giving it that list tells it that all other JS calls will **not** do
an async operation, which lets it not add overhead where it isn't needed.

.. note:: If the import is not inside ``env`` the full path must be specified, for example, ``ASYNCIFY_IMPORTS=wasi_snapshot_preview1.fd_write``

Asyncify with Dynamic Linking
#############################

If you want to use Asyncify in dynamic libraries, those methods which are imported
from other linked modules (and that will be on the stack in an async operation)
should be listed in ``ASYNCIFY_IMPORTS``.

.. code-block:: cpp

    // sleep.cpp
    #include <emscripten.h>

    extern "C" void sleep_for_seconds() {
      emscripten_sleep(100);
    }

In the side module, you can compile sleep.cpp in the ordinal emscripten dynamic
linking manner:

::

    emcc sleep.cpp -O3 -o libsleep.wasm -sASYNCIFY -sSIDE_MODULE

.. code-block:: cpp

    // main.cpp
    #include <emscripten.h>

    extern "C" void sleep_for_seconds();

    int main() {
      sleep_for_seconds();
      return 0;
    }

In the main module, the compiler doesn’t statically know that ``sleep_for_seconds`` is
asynchronous. Therefore, you must add ``sleep_for_seconds`` to the ``ASYNCIFY_IMPORTS``
list.

::

    emcc main.cpp libsleep.wasm -O3 -sASYNCIFY -sASYNCIFY_IMPORTS=sleep_for_seconds -sMAIN_MODULE

Usage with Embind
#################

If you're using :ref:`Embind<embind-val-guide>` for interaction with JavaScript
and want to ``await`` a dynamically retrieved ``Promise``, you can call an
``await()`` method directly on the ``val`` instance:

.. code-block:: cpp

    val my_object = /* ... */;
    val result = my_object.call<val>("someAsyncMethod").await();

In this case you don't need to worry about ``ASYNCIFY_IMPORTS`` or
``JSPI_IMPORTS``, since it's an internal implementation detail of ``val::await``
and Emscripten takes care of it automatically.

Note that when using Embind exports, Asyncify and JSPI behave differently. When
Asyncify is used with Embind and the code is invoked from JavaScript, then the
function will return a ``Promise`` if the export calls any suspending functions,
otherwise the result will be returned synchronously. However, with JSPI, the
parameter ``emscripten::async()`` must be used to mark the function as
asynchronous and the export will always return a ``Promise`` regardless if the
export suspended.

.. code-block:: cpp

   #include <emscripten/bind.h>
   #include <emscripten.h>

   static int delayAndReturn(bool sleep) {
     if (sleep) {
       emscripten_sleep(0);
     }
     return 42;
   }

   EMSCRIPTEN_BINDINGS(example) {
     // Asyncify
     emscripten::function("delayAndReturn", &delayAndReturn);
     // JSPI
     emscripten::function("delayAndReturn", &delayAndReturn, emscripten::async());
   }

Build with
::

    emcc -O3 example.cpp -lembind -s<ASYNCIFY or JSPI>

Then invoke from JavaScript (using Asyncify)

.. code-block:: javascript

   let syncResult = Module.delayAndReturn(false);
   console.log(syncResult); // 42
   console.log(await syncResult); // also 42 because `await` is no-op

   let asyncResult = Module.delayAndReturn(true);
   console.log(asyncResult); // Promise { <pending> }
   console.log(await asyncResult); // 42

In contrast to JavaScript ``async`` functions which always return a ``Promise``,
the return value is determined at run time, and a ``Promise`` is only returned
if Asyncify calls are encountered (such as ``emscripten_sleep()``,
``val::await()``, etc).

If the code path is undetermined, the caller may either check if the returned
value is an ``instanceof Promise`` or simply ``await`` on the returned value.

When using JSPI the return values will always be a ``Promise`` as seen below

.. code-block:: javascript

   let syncResult = Module.delayAndReturn(false);
   console.log(syncResult); // Promise { <pending> }
   console.log(await syncResult); // 42

   let asyncResult = Module.delayAndReturn(true);
   console.log(asyncResult); // Promise { <pending> }
   console.log(await asyncResult); // 42


Usage with ``ccall``
####################

To make use of an Asyncify-using Wasm export from Javascript, you can use the
``Module.ccall`` function and pass ``async: true`` to its call options object.
``ccall`` will then return a Promise, which will resolve with the result of the
function once the computation completes.

In this example, a function "func" is called which returns a Number.

.. code-block:: javascript

    Module.ccall("func", "number", [], [], {async: true}).then(result => {
      console.log("js_func: " + result);
    });


Differences Between Asyncify and JSPI
#####################################

Besides using different underlying mechanisms, Asyncify and JSPI also handle
async imports and exports differently. Asyncify will automatically determine
what exports will become async based on what could potentially call an
an async import (``ASYNCIFY_IMPORTS``). However, with JSPI, the async imports
and exports must be explicitly set using ``JSPI_IMPORTS`` and ``JSPI_EXPORTS``
settings.

.. note:: ``<JSPI/ASYNCIFY>_IMPORTS`` and ``JSPI_EXPORTS`` aren't needed when
          using various helpers mentioned above such as: ``EM_ASYNC_JS``,
          Embind's Async support, ``ccall``, etc...

Optimizing Asyncify
###################

.. note:: This section does not apply to JSPI.

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

* ``ASYNCIFY_REMOVE`` is a list of functions that do not unwind the stack.
  As Asyncify processes the call tree, functions in this list will be removed,
  and neither they nor their callers will be instrumented (unless their callers
  need to be instrumented for other reasons.)
* ``ASYNCIFY_ADD`` is a list of functions that do unwind the stack, and will be
  processed like the imports. This is mostly useful
  if you use ``ASYNCIFY_IGNORE_INDIRECT`` but want to also mark some additional
  functions that need to unwind. If the ``ASYNCIFY_PROPAGATE_ADD`` setting is
  disabled however, then this list will only be added after the whole-program
  analysis. If ``ASYNCIFY_PROPAGATE_ADD`` is disabled then you must also add
  their callers, their callers' callers, and so on.
* ``ASYNCIFY_ONLY`` is a list of the **only** functions that can unwind
  the stack. Asyncify will instrument exactly those and no others.

You can enable the ``ASYNCIFY_ADVISE`` setting, which will tell the compiler to
output which functions it is currently instrumenting and why. You can then
determine whether you should add any functions to ``ASYNCIFY_REMOVE`` or
whether it would be safe to enable ``ASYNCIFY_IGNORE_INDIRECT``. Note that this
phase of the compiler happens after many optimization phases, and several
functions maybe be inlined already. To be safe, run it with `-O0`.

For more details see ``settings.js``. Note that the manual settings
mentioned here are error-prone - if you don't get things exactly right,
your application can break. If you don't absolutely need maximal performance,
it's usually ok to use the defaults.

Potential problems
##################

Stack overflows (Asyncify)
**************************

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

Starting to rewind with compiled code on the stack (Asyncify)
*************************************************************

The examples above show `wakeUp()` being called from JS (after a callback,
typically), and without any compiled code on the stack. If there *were* compiled
code on the stack, then that could interfere with properly rewinding and
resuming execution, in confusing ways, and therefore an assertion will be
thrown in a build with ``ASSERTIONS``.

(Specifically, the problem there is that while rewinding will work properly,
if you later unwind again, that unwinding will also unwind through that extra
compiled code that was on the stack - causing a later rewind to behave badly.)

A simple workaround you may find useful is to do a setTimeout of 0, replacing
``wakeUp()`` with ``setTimeout(wakeUp, 0);``. That will run ``wakeUp`` in a
later callback, when nothing else is on the stack.

Migrating from older Asyncify APIs
##################################

If you have code uses the old Emterpreter-Async API, or the old Asyncify, then
almost everything should just work when you replace ``-sEMTERPRETIFY`` usage
with ``-sASYNCIFY``. In particular all the things like ``emscripten_wget``
should just work as they did before.

Some minor differences include:

 * The Emterpreter had "yielding" as a concept, but it isn't needed in Asyncify.
   You can replace ``emscripten_sleep_with_yield()`` calls with ``emscripten_sleep()``.
 * The internal JS API is different. See notes above on
   ``Asyncify.handleSleep()``, and see ``src/library_async.js`` for more
   examples.
