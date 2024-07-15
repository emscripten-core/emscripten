.. Pthreads support:

==============================
Pthreads support
==============================

.. note:: Browsers `that have implemented and enabled <https://webassembly.org/roadmap/>`_ SharedArrayBuffer, are gating it behind Cross Origin Opener Policy (COOP) and Cross Origin Embedder Policy (COEP) headers. Pthreads code will not work in deployed environment unless these headers are correctly set. For more information click `this <https://web.dev/coop-coep>`_

Emscripten has support for multithreading using SharedArrayBuffer in browsers. That API allows sharing memory between the main thread and web workers as well as atomic operations for synchronization, which enables Emscripten to implement support for the Pthreads (POSIX threads) API. This support is considered stable in Emscripten.

Compiling with pthreads enabled
===============================

By default, support for pthreads is not enabled. To enable code generation for pthreads, the following command line flags exist:

- Pass the compiler flag ``-pthread`` when compiling any .c/.cpp files, AND when linking to generate the final output .js file.
- Optionally, pass the linker flag ``-sPTHREAD_POOL_SIZE=<expression>`` to specify a predefined pool of web workers to populate at page ``preRun`` time before application ``main()`` is called. This is important because if the workers do not already exist then we may need to wait for the next browser event iteration for certain things, see below. ``<expression>`` can be any valid JavaScript expression, including integers like ``8`` for a fixed number of threads or, say, ``navigator.hardwareConcurrency`` to create as many threads as there are CPU cores.

There should be no other changes required. In C/C++ code, the preprocessor check ``#ifdef __EMSCRIPTEN_PTHREADS__`` can be used to detect whether Emscripten is currently targeting pthreads.

.. note:: It is not possible to build one binary that would be able to leverage
    multithreading when available and fall back to single threaded when not. The
    best you can do is two separate builds, one with and one
    without threads, and pick between them at runtime.

Additional flags
================

- ``-sPROXY_TO_PTHREAD``: In this mode your original ``main()`` is replaced by
  a new one that creates a pthread and runs the original ``main()`` on it. As a
  result, your application's ``main()`` is run off the browser main (UI) thread,
  which is good for responsiveness. The browser main thread does still run code
  when things are proxied to it, for example to handle events, rendering, etc.
  The main thread also does things like create pthreads for you, so that you
  can depend on them synchronously.

Note that Emscripten has the
``--proxy-to-worker`` :ref:`linker flag <proxy-to-worker>` which sounds similar
but is unrelated. That flag does not use pthreads or SharedArrayBuffer, and
instead uses a plain Web Worker to run your main program (and postMessage to
proxy messages back and forth).

Proxying
========

The Web allows certain operations to only happen from the main browser thread,
like interacting with the DOM. As a result, various operations are proxied to
the main browser thread if they are called on a background thread. See
`bug 3495 <https://github.com/emscripten-core/emscripten/issues/3495>`_ for
more information and how to try to work around this until then. To check which
operations are proxied, you can look for the function's implementation in
the JS library (``src/library_*``) and see if it is annotated with
``__proxy: 'sync'`` or ``__proxy: 'async'``; however, note that the browser
itself proxies certain things (like some GL operations), so there is no
general way to be safe here (aside from not blocking on the main browser
thread).

In addition, Emscripten currently has a simple model of file I/O only happening
on the main application thread (as we support JS plugin filesystems, which
cannot share memory); this is another set of operations that are proxied.

Proxying can cause problems in certain cases, see the section on blocking below.

Blocking on the main browser thread
===================================

Note that in most cases the "main browser thread" is the same as the "main
application thread". The main browser thread is where web pages start to run
JavaScript, and where JavaScript can access the DOM (a page can also create a Web
Worker, which would no longer be on the main thread). The main application
thread is the one on which you started up the application (by loading the main
JS file emitted by Emscripten). If you started it on the main browser thread -
by it being a normal HTML page - then the two are identical. However, you can
also start a multithreaded application in a worker; in that case the main
application thread is that worker, and there is no access to the main browser
thread.

The Web API for atomics does not allow blocking on the main thread
(specifically, ``Atomics.wait`` doesn't work there). Such blocking is
necessary in APIs like ``pthread_join`` and anything that uses a futex wait
under the hood, like ``usleep()``, ``emscripten_futex_wait()``, or
``pthread_mutex_lock()``. To make them work, we use a busy-wait on the main
browser thread, which can make the browser tab unresponsive, and also wastes
power. (On a pthread, this isn't a problem as it runs in a Web Worker, where
we don't need to busy-wait.)

Busy-waiting on the main browser thread in general will work despite the
downsides just mentioned, for things like waiting on a lightly-contended mutex.
However, things like ``pthread_join`` and ``pthread_cond_wait``
are often intended to block for long periods of time, and if that
happens on the main browser thread, and while other threads expect it to
respond, it can cause a surprising deadlock. That can happen because of
proxying, see the previous section. If the main thread blocks while a worker
attempts to proxy to it, a deadlock can occur.

The bottom line is that on the Web it is bad for the main browser thread to
wait on anything else. Therefore by default Emscripten warns if
``pthread_join`` and ``pthread_cond_wait`` happen on the main browser thread,
and will throw an error if ``ALLOW_BLOCKING_ON_MAIN_THREAD`` is zero
(whose message will point to here).

To avoid these problems, you can use ``PROXY_TO_PTHREAD``, which as
mentioned earlier moves your ``main()`` function to a pthread, which leaves
the main browser thread to focus only on receiving proxied events. This is
recommended in general, but may take some porting work, if the application
assumed ``main()`` was on the main browser thread.

Another option is to replace blocking calls with nonblocking ones. For example
you can replace ``pthread_join`` with ``pthread_tryjoin_np``. This may require
your application to be refactored to use asynchronous events, perhaps through
:c:func:`emscripten_set_main_loop` or :ref:`Asyncify`.

Special considerations
======================

The Emscripten implementation for the pthreads API should follow the POSIX standard closely, but some behavioral differences do exist:

- When ``pthread_create()`` is called, if we need to create a new Web Worker,
  then that requires returning the main event loop. That is, you cannot call
  ``pthread_create`` and then keep running code synchronously that expects the
  worker to start running - it will only run after you return to the event loop.
  This is a violation of POSIX behavior and will break common code which creates
  a thread and immediately joins it or otherwise synchronously waits to observe
  an effect such as a memory write. There are several solutions to this:

  1. Return to the main event loop (for example, use
     ``emscripten_set_main_loop``, or Asyncify).
  2. Use the linker flag ``-sPTHREAD_POOL_SIZE=<expression>``. Using a pool
     creates the Web Workers before main is called, so they can just be used
     when ``pthread_create`` is called.
  3. Use the linker flag ``-sPROXY_TO_PTHREAD``, which will run ``main()`` on
     a worker for you. When doing so, ``pthread_create`` is proxied to the
     main browser thread, where it can return to the main event loop as needed.

- The Emscripten implementation does not support `POSIX signals <http://man7.org/linux/man-pages/man7/signal.7.html>`_, which are sometimes used in conjunction with pthreads. This is because it is not possible to send signals to web workers and pre-empt their execution. The only exception to this is pthread_kill() which can be used as normal to forcibly terminate a running thread.

- The Emscripten implementation does also not support multiprocessing via ``fork()`` and ``join()``.

- For web security purposes, there exists a fixed limit (by default 20) of threads that can be spawned when running in Firefox Nightly. `#1052398 <https://bugzilla.mozilla.org/show_bug.cgi?id=1052398>`_. To adjust the limit, navigate to about:config and change the value of the pref "dom.workers.maxPerDomain".

- Some of the features in the pthreads specification are unsupported since the upstream musl library that Emscripten utilizes does not support them, or they are marked optional and a conformant implementation need not support them. Such unsupported features in Emscripten include prioritization of threads, and pthread_rwlock_unlock() is not performed in thread priority order. The functions pthread_mutexattr_set/getprotocol(), pthread_mutexattr_set/getprioceiling() and pthread_attr_set/getscope() are no-ops.

- One particular note to pay attention to when porting is that sometimes in existing codebases the callback function pointers to pthread_create() and pthread_cleanup_push() omit the void* argument, which strictly speaking is undefined behavior in C/C++, but works in several x86 calling conventions. Doing this in Emscripten will issue a compiler warning, and can abort at runtime when attempting to call a function pointer with incorrect signature, so in the presence of such errors, it is good to check the signatures of the thread callback functions.

- Note that the function emscripten_num_logical_cores() will always return the value of navigator.hardwareConcurrency, i.e. the number of logical cores on the system, even when shared memory is not supported. This means that it is possible for emscripten_num_logical_cores() to return a value greater than 1, while at the same time emscripten_has_threading_support() can return false. The return value of emscripten_has_threading_support() denotes whether the browser has shared memory support available.

- Pthreads + memory growth (``ALLOW_MEMORY_GROWTH``) is especially tricky, see `Wasm design issue #1271 <https://github.com/WebAssembly/design/issues/1271>`_. This currently causes JS accessing the Wasm memory to be slow - but this will likely only be noticeable if the JS does large amounts of memory reads and writes (Wasm runs at full speed, so moving work over can fix this). This also requires that your JS be aware that the HEAP* views may need to be updated - JS code embedded with ``--js-library`` etc will automatically be transformed to use the ``GROWABLE_HEAP_*`` helper functions where ``HEAP*`` are used, but external code that uses ``Module.HEAP*`` directly may encounter problems with views being smaller than memory.

.. _Allocator_performance:

Allocator performance
=====================

The default system allocator in Emscripten, ``dlmalloc``, is very efficient in a
single-threaded program, but it has a single global lock which means if there is
contention on ``malloc`` then you can see overhead. You can use
`mimalloc <https://github.com/microsoft/mimalloc>`_
instead by using ``-sMALLOC=mimalloc``, which is a more sophisticated allocator
tuned for multithreaded performance. ``mimalloc`` has separate allocation
contexts on each thread, allowing performance to scale a lot better under
``malloc/free`` contention.

Note that ``mimalloc`` is larger in code size than ``dlmalloc``, and also uses
more memory at runtime (so you may need to adjust ``INITIAL_MEMORY`` to a higher
value), so there are tradeoffs here.

Running code and tests
======================

Any code that is compiled with pthreads support enabled will currently only work in the Firefox Nightly channel, since the SharedArrayBuffer specification is still in an experimental research stage before standardization. There exists two test suites that can be used to verify the behavior of the pthreads API implementation in Emscripten:

- The Emscripten unit test suite contains several pthreads-specific tests in the "browser." suite. Run any of the tests named browser.test_pthread_*.

- An Emscripten-specialized version of the `Open POSIX Test Suite <http://posixtest.sourceforge.net/>`_ is available at `juj/posixtestsuite <https://github.com/juj/posixtestsuite>`_ GitHub repository. This suite contains about 300 tests for pthreads conformance. To run this suite, the pref dom.workers.maxPerDomain should first be increased to at least 50.

Please check these first in case of any issues. Bugs can be reported to the Emscripten bug tracker as usual.
