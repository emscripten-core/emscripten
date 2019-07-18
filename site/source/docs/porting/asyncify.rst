.. Asyncify:

========================
Asyncify
========================

Asyncify lets synchronous C or C++ code do asynchronous Web API calls. For example, you can write ``emscripten_sleep(100)`` and it will return to the main browser event loop for 100ms, during which browser events can be handled, etc. (without returning to the event loop, no events will be dispatched). Asyncify does this by automatically transforming the compiled code into something that can be paused and resumed, and handles pausing and resuming for you.

See the `Asyncify introduction blogpost <https://kripken.github.io/blog/wasm/2019/07/16/asyncify.html>`_ for general background and details of how it works internally. The following expands on the Emscripten examples from that post.

.. note:: This post talks about Asyncify using the new LLVM wasm backend. There is also an older Asyncify implementation for the old fastcomp backend. The two algorithms and implementations are entirely separate, so if you are using fastcomp, these docs may not be accurate - you should upgrade to the wasm backend and new Asyncify!

Sleeping
========

Let's begin with the example from that blogpost:

::

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
      // "Infinite loop", synchronously poll for the timer.
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

.. note:: It's very important to optimize (``-O3`` here) when using Asyncify, as unoptimized builds are very large.

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

The code is written with an "infinite loop" that sleeps, which normally would not allow async events to be handled by the browser. With Asyncify, those sleeps actually exit to the browser's main event loop, and the timer can happen!

Waiting for Web APIs
====================

Aside from ``emscripten_sleep`` and the other standard sync APIs Asyncify supports, you can also add your own functions. To do so, you must create a JS function that is called from wasm (since Emscripten controls pausing and resuming the wasm from the JS runtime). One way to do that is with a JS library function; another is to use ``EM_JS``, which we'll use in this next example:

::

    // example.c
    #include <emscripten.h>
    #include <stdio.h>

    EM_JS(void, wait_for_click, (), {
      Asyncify.handleSleep(function(wakeUp) {
        console.log("waiting for a click");
        document.body.addEventListener("mousedown", function() {
          wakeUp();
        });
      });
    });

    int main() {
      puts("before");
      wait_for_click();
      puts("after");
    }

Here we print "before", then wait for the user to click on the document, and then print "after". Note how the C code in ``main()`` is all synchronous! The async operation happens in the ``EM_JS`` function ``wait_for_click()``, which calls ``Asyncify.handleSleep``. It gives that function the code to be run, and gets a ``wakeUp`` function that it calls in the asynchronous future at the right time. Here we add an event listener for a mouse button being pushed down on the document. After that event arrives asynchronously, calling ``wakeUp()`` lets the program resume normally, exactly as if it were paused while waiting. To see this, compile it with

::

    ./emcc example.c -O3 -o a.html -s ASYNCIFY -s 'ASYNCIFY_IMPORTS=["wait_for_click"]'

Note that must tell the compiler that ``wait_for_click()`` can do an asynchronous operation, using ``ASYNCIFY_IMPORTS``, otherwise it won't instrument the code to allow pausing and resuming. (That list must contain all such imports, so if you also use ``emscripten_sleep()`` then you must put it in that list as well.)

To run this, you must run a webserver (like say ``python -m SimpleHTTPServer``) and then browse to ``http://localhost:8000/a.html`` (the URL may depend on the port number in the server). You will see "before" printed. After you click on the document (like on the black canvas, or the textbox with "before") you will see it print "after" as expected.

