.. _wasm workers section:

================
Wasm Workers API
================

The Wasm Workers API enables C/C++ code to leverage Web Workers and shared
WebAssembly.Memory (SharedArrayBuffer) to build multithreaded programs
via a direct web-like programming API.

Quick Example
=============

.. code-block:: cpp

  #include <emscripten/wasm_worker.h>
  #include <stdio.h>

  void run_in_worker()
  {
    printf("Hello from Wasm Worker!\n");
  }

  int main()
  {
    emscripten_wasm_worker_t worker = emscripten_malloc_wasm_worker(/*stackSize: */1024);
    emscripten_wasm_worker_post_function_v(worker, run_in_worker);
  }

Build the code by passing the Emscripten flag ``-sWASM_WORKERS`` at both compile
and link steps. The example code creates a new Worker on the main browser thread,
which shares the same WebAssembly.Module and WebAssembly.Memory object. Then a
``postMessage()`` is passed to the Worker to ask it to execute the function
``run_in_worker()`` to print a string.

To explicitly control the memory allocation placement when creating a worker,
use the ``emscripten_create_wasm_worker()`` function. This function takes a
region of memory that must be large enough to hold both the stack and the TLS
data for the worker.  You can use ``__builtin_wasm_tls_size()`` to find out at
runtime how much space is required for the program's TLS data.

Introduction
============

In WebAssembly programs, the Memory object that contains the application state can be
shared across multiple Workers. This enables direct, high performance (and if explicit
care is not taken, racy!) access to synchronously share data state between multiple
Workers (shared state multithreading).

Emscripten supports two multithreading APIs to leverage this web feature:
 - POSIX Threads (Pthreads) API, and
 - Wasm Workers API.

The Pthreads API has a long history with native C programming and the POSIX standard,
while Wasm Workers API is unique to Emscripten compiler only.

These two APIs provide largely the same feature set, but have important differences,
which this documentation seeks to explain to help decide which API one should target.

Pthreads vs Wasm Workers: Which One to Use?
===========================================

The intended audience and use cases of these two multithreading APIs are slightly
different.

The focus on Pthreads API is on portability and cross-platform compatibility. This API
is best used in scenarios where portability is most important, e.g. when a codebase is
cross-compiled to multiple platforms, like building both a native Linux x64 executable and an
Emscripten WebAssembly based web site.

Pthreads API in Emscripten seeks to carefully emulate compatibility and the features that
the native Pthreads platforms already provide. This helps porting large C/C++ codebases
over to WebAssembly.

Wasm Workers API on the other hand seeks to provide a "direct mapping" to the web
multithreading primitives as they exist on the web, and call it a day. If an application
is only developed to target WebAssembly, and portability is not a concern, then using Wasm
Workers can provide great benefits in the form of simpler compiled output, less complexity,
smaller code size and possibly better performance.

However this benefit might not be an obvious win. The Pthreads API was designed to be useful
from the synchronous C/C++ language, whereas Web Workers are designed to be useful from
asynchronous JavaScript. WebAssembly C/C++ programs can find themselves somewhere in
the middle.

Pthreads and Wasm Workers share several similarities:

 * Both can use emscripten_atomic_* Atomics API,
 * Both can use GCC __sync_* Atomics API,
 * Both can use C11 and C++11 Atomics APIs,
 * Both types of threads have a local stack.
 * Both types of threads have thread-local storage (TLS) support via ``thread_local`` (C++11),
   ``_Thread_local`` (C11) and ``__thread`` (GNU11) keywords.
 * Both types of threads support TLS via explicitly linked in Wasm globals (see
   ``test/wasm_worker/wasm_worker_tls_wasm_assembly.c/.S`` for example code)
 * Both types of threads have a concept of a thread ID (``pthread_self()`` for pthreads,
   ``emscripten_wasm_worker_self_id()`` for Wasm Workers)
 * Both types of threads can perform an event-based and an infinite loop programming model.
 * Both can use ``EM_ASM`` and ``EM_JS`` API to execute JS code on the calling thread.
 * Both can call out to JS library functions (linked in with ``--js-library`` directive) to
   execute JS code on the calling thread.
 * Neither pthreads nor Wasm Workers can be used in conjunction with ``-sSINGLE_FILE`` linker flag.

However, the differences are more notable.

Pthreads can proxy JS functions
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Only pthreads can use the ``MAIN_THREAD_EM_ASM*()`` and ``MAIN_THREAD_ASYNC_EM_ASM()`` functions and
the ``foo__proxy: 'sync'/'async'`` proxying directive in JS libraries.

Wasm Workers on the other hand do not provide a built-in JS function proxying facility. Proxying a JS
function with Wasm Workers can be done by explicitly passing the address of that function to the
``emscripten_wasm_worker_post_function_*`` API.

If you need to synchronously wait for the posted function to finish from within a Worker, use one of
the ``emscripten_wasm_worker_*()`` thread synchronization functions to sleep the calling thread until
the callee has finished the operation.

Note that Wasm Workers cannot 

Pthreads have cancellation points
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

At the expense of performance and code size, pthreads implement a notion of **POSIX cancellation
points** (``pthread_cancel()``, ``pthread_testcancel()``).

Wasm Workers are more lightweight and performant by not enabling that concept.

Pthreads may start up synchronously - Wasm Workers always start asynchronously
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Creating new Workers can be slow. Spawning a Worker in JavaScript is an asynchronous operation. In order
to support synchronous pthread startup (for applications that need it) and to improve thread startup
performance, pthreads are hosted in a cached Emscripten runtime managed Worker pool.

Wasm Workers omit this concept, and as result Wasm Workers will always start up asynchronously.
If you need to detect when a Wasm Worker has started up, post a ping-pong function and reply pair
manually between the Worker and its creator. If you need to spin up new threads quickly, consider
managing a pool of Wasm Workers yourself.

Pthread topology is flat - Wasm Workers are hierarchical
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

On the web, if a Worker spawns a child Worker of its own, it will create a nested Worker hierarchy
that the main thread cannot directly access. To sidestep portability issues stemming from this kind
of topology, pthreads flatten the Worker creation chain under the hood so that only the main browser thread
ever spawns threads.

Wasm Workers do not implement this kind of topology flattening, and creating a Wasm Worker in a
Wasm Worker will produce a nested Worker hierarchy. If you need to create Wasm Workers from within
a Wasm Worker, consider which type of hierarchy you would like, and if necessary, flatten the
hierarchy manually by posting the Worker creation over to the main thread yourself.

Note that support for nested Workers varies across browsers. As of 02/2022, nested Workers are `not
supported in Safari <https://bugs.webkit.org/show_bug.cgi?id=22723>`_. See `here 
<https://github.com/johanholmerin/nested-worker>`_ for a polyfill.

Pthreads can use the Wasm Worker synchronization API, but not vice versa
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The multithreading synchronization primitives offered in ``emscripten/wasm_worker.h``
(``emscripten_lock_*``, ``emscripten_semaphore_*``, ``emscripten_condvar_*``) can be freely invoked
from within pthreads if one so wishes, but Wasm Workers cannot utilize any of the synchronization
functionality in the Pthread API (``pthread_mutex_*``, ``pthread_cond_``, ``pthread_rwlock_*``, etc),
since they lack the needed pthread runtime.

Pthreads have a "thread main" function and atexit handlers
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The startup/execution model of pthreads is to start up executing a given thread entry point function.
When that function exits, the pthread will also (by default) quit, and the Worker hosting that pthread
will return to the Worker pool to wait for another thread to be created on it.

Wasm Workers instead implement the direct web-like model, where a newly created Worker sits idle in its
event loop, waiting for functions to be posted to it. When those functions finish, the Worker will
return to its event loop, waiting to receive more functions (or worker scope web events) to execute.
A Wasm Worker will only quit with a call to ``emscripten_terminate_wasm_worker(worker_id)`` or
``emscripten_terminate_all_wasm_workers()``.

Pthreads allow one to register thread exit handlers via ``pthread_atexit``, which will be called when
the thread quits. Wasm Workers do not have this concept.

Pthreads have a per-thread incoming proxy message queue, Wasm Workers do not
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

In order to enable flexible synchronous execution of code on other threads, and to implement support
APIs for example for MEMFS filesystem and Offscreen Framebuffer (WebGL emulated from a Worker) features,
main browser thread and each pthread have a system-backed "proxy message queue" to receive messages.

This enables user code to call API functions, ``emscripten_sync_run_in_main_runtime_thread()``,
``emscripten_async_run_in_main_runtime_thread()``, ``emscripten_dispatch_to_thread()``, etc. from
``emscripten/threading.h`` to perform proxied calls.

Wasm Workers do not provide this functionality. If needed, such messaging should be implemented manually
by users via regular multithreaded synchronized programming techniques (mutexes, futexes, semaphores, etc.)

Pthreads synchronize wallclock times
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Another portability aiding emulation feature that Pthreads provide is that the time values returned by
``emscripten_get_now()`` are synchronized to a common time base across all threads.

Wasm Workers omit this concept, and it is recommended to use the function ``emscripten_performance_now()``
for high performance timing in a Wasm Worker, and avoid comparing resulting values across Workers, or
manually synchronize them.

Input events API backproxies only to pthreads
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The multithreaded input API provided in ``emscripten/html5.h`` only works with the pthread API. When
calling any of the functions ``emscripten_set_*_callback_on_thread()``, one can choose the target
pthread to be the recipient of the received events.

With Wasm Workers, if desired, "backproxying" events from the main browser thread to a Wasm Worker
should be implemented manually e.g. by using the ``emscripten_wasm_worker_post_function_*()`` API family.

However note that backproxying input events has a drawback that it prevents security sensitive operations,
like fullscreen requests, pointer locking and audio playback resuming, since handling the input event
is detached from the event callback context executing the initial operation.

Pthread vs emscripten_lock implementation differences
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The mutex implementation from ``pthread_mutex_*`` has a few different creation options, one being a
"recursive" mutex.

The lock implemented by ``emscripten_lock_*`` API is not recursive (and does not provide an option).

Pthreads also offer a programming guard against a programming error that one thread would not release
a lock that is owned by another thread. ``emscripten_lock_*`` API does not track lock ownership.

Memory requirements
^^^^^^^^^^^^^^^^^^^

Pthreads have a fixed dependency to dynamic memory allocation, and perform calls to ``malloc`` and ``free``
to allocate thread specific data, stacks and TLS slots.

With the exception of the helper function ``emscripten_malloc_wasm_worker()``, Wasm Workers are not dependent
on a dynamic memory allocator. Memory allocation needs are met by the caller at Worker creation time, and
can be statically placed if desired.

Generated code size
^^^^^^^^^^^^^^^^^^^

The disk size overhead from pthreads is on the order of a few hundred KBs. Wasm Workers runtime on the other
hand is optimized for tiny deployments, just a few hundred bytes on disk.

API Differences
^^^^^^^^^^^^^^^

To further understand the different APIs available between Pthreads and Wasm Workers, refer to the following
table.

.. raw:: html

  <table style='border:solid 2px;' cellpadding=10>
    <tr><td class='cellborder'>Feature</td>
    <td class='cellborder'>Pthreads</td>
    <td class='cellborder'>Wasm Workers</td> </tr>

    <tr><td class='cellborder'>Thread termination</td>
    <td class='cellborder'>Thread calls <pre>pthread_exit(status)</pre>or main thread calls <pre>pthread_kill(code)</pre></td>
    <td class='cellborder'>Worker cannot terminate itself, parent thread terminates by calling <pre>emscripten_terminate_wasm_worker(worker)</pre></td></tr>

    <tr><td class='cellborder'>Thread stack</td>
    <td class='cellborder'>Specify in pthread_attr_t structure.</td>
    <td class='cellborder'>Manage thread stack area explicitly with <pre>emscripten_create_wasm_worker_*_tls()</pre> functions, or
      <br>automatically allocate stack+TLS area with <pre>emscripten_malloc_wasm_worker()</pre> API.</td></tr>

    <tr><td class='cellborder'>Thread Local Storage (TLS)</td>
    <td class='cellborder'>Supported transparently.</td>
    <td class='cellborder'>Supported either explicitly with <pre>emscripten_create_wasm_worker_*_tls()</pre> functions, or
      <br>automatically via <pre>emscripten_malloc_wasm_worker()</pre> API.</td></tr>

    <tr><td class='cellborder'>Thread ID</td>
    <td class='cellborder'>Creating a pthread obtains its ID. Call <pre>pthread_self()</pre> to acquire ID of calling thread.</td>
    <td class='cellborder'>Creating a Worker obtains its ID. Call <pre>emscripten_wasm_worker_self_id()</pre> acquire ID of calling thread.</td></tr>

    <tr><td class='cellborder'>High resolution timer</td>
    <td class='cellborder'>``emscripten_get_now()``</td>
    <td class='cellborder'>``emscripten_performance_now()``</td></tr>

    <tr><td class='cellborder'>Synchronous blocking on main thread</td>
    <td class='cellborder'>Synchronization primitives internally fall back to busy spin loops.</td>
    <td class='cellborder'>Explicit spin vs sleep synchronization primitives.</td></tr>

    <tr><td class='cellborder'>Futex API</td>
    <td class='cellborder'><pre>emscripten_futex_wait</pre><pre>emscripten_futex_wake</pre> in emscripten/threading.h</td>
    <td class='cellborder'><pre>emscripten_atomic_wait_u32</pre><pre>emscripten_atomic_wait_u64</pre><pre>emscripten_atomic_notify</pre> in emscripten/atomic.h</td></tr>

    <tr><td class='cellborder'>Asynchronous futex wait</td>
    <td class='cellborder'>N/A</td>
    <td class='cellborder'><pre>emscripten_atomic_wait_async()</pre><pre>emscripten_*_async_acquire()</pre>However these are a difficult footgun, read <a href='https://github.com/WebAssembly/threads/issues/176'>WebAssembly/threads issue #176</a></td></tr>

    <tr><td class='cellborder'>C/C++ Function Proxying</td>
    <td class='cellborder'>emscripten/threading.h API for proxying function calls to other threads.</td>
    <td class='cellborder'>Use emscripten_wasm_worker_post_function_*() API to message functions to other threads. These messages follow event queue semantics rather than proxy queue semantics.</td></tr>

    <tr><td class='cellborder'>Build flags</td>
    <td class='cellborder'>Compile and link with -pthread</td>
    <td class='cellborder'>Compile and link with -sWASM_WORKERS</td></tr>

    <tr><td class='cellborder'>Preprocessor directives</td>
    <td class='cellborder'>__EMSCRIPTEN_SHARED_MEMORY__=1 and __EMSCRIPTEN_PTHREADS__=1 are active</td>
    <td class='cellborder'>__EMSCRIPTEN_SHARED_MEMORY__=1 and __EMSCRIPTEN_WASM_WORKERS__=1 are active</td></tr>

    <tr><td class='cellborder'>JS library directives</td>
    <td class='cellborder'>USE_PTHREADS and SHARED_MEMORY are active</td>
    <td class='cellborder'>USE_PTHREADS, SHARED_MEMORY and WASM_WORKER are active</td></tr>

    <tr><td class='cellborder'>Atomics API</td>
    <td colspan=2>Supported, use any of <a href="https://gcc.gnu.org/onlinedocs/gcc/_005f_005fatomic-Builtins.html">__atomic_* API</a>, <a href="https://llvm.org/docs/Atomics.html#libcalls-sync">__sync_* API</a> or <a href="https://en.cppreference.com/w/cpp/atomic/atomic">C++11 std::atomic API</a>.</td></tr>

    <tr><td class='cellborder'>Nonrecursive mutex</td>
    <td class='cellborder'><pre>pthread_mutex_*</pre></td>
    <td class='cellborder'><pre>emscripten_lock_*</pre></td></tr>

    <tr><td class='cellborder'>Recursive mutex</td>
    <td class='cellborder'><pre>pthread_mutex_*</pre></td>
    <td class='cellborder'>N/A</td></tr>

    <tr><td class='cellborder'>Semaphores</td>
    <td class='cellborder'>N/A</td>
    <td class='cellborder'><pre>emscripten_semaphore_*</pre></td></tr>

    <tr><td class='cellborder'>Condition Variables</td>
    <td class='cellborder'><pre>pthread_cond_*</pre></td>
    <td class='cellborder'><pre>emscripten_condvar_*</pre></td></tr>

    <tr><td class='cellborder'>Read-Write locks</td>
    <td class='cellborder'><pre>pthread_rwlock_*</pre></td>
    <td class='cellborder'>N/A</td></tr>

    <tr><td class='cellborder'>Spinlocks</td>
    <td class='cellborder'><pre>pthread_spin_*</pre></td>
    <td class='cellborder'><pre>emscripten_lock_busyspin*</pre></td></tr>

    <tr><td class='cellborder'>WebGL Offscreen Framebuffer</td>
    <td class='cellborder'><pre>Supported with -sOFFSCREEN_FRAMEBUFFER</pre></td>
    <td class='cellborder'><pre>Not supported.</pre></td></tr>

  </table>

Wasm Workers stack size considerations
======================================

When instantiating a Wasm Worker, one has to create a memory array for the LLVM
data stack for the created Worker. This data stack will generally consist only
of local variables that have been "spilled" by LLVM into memory, e.g. to contain
large arrays, structs, or other variables that are referenced by a memory
address. This stack will not contain control flow information.

Since WebAssembly does not support virtual memory, the size of the LLVM data
stack that is defined both for Wasm Workers but also the main thread will not be
possible to grow at runtime. So if the Worker (or the main thread) runs out of
stack space, the program behavior will be undefined. Use the Emscripten linker
flag -sSTACK_OVERFLOW_CHECK=2 to emit runtime stack overflow checks into the
program code to detect these situations during development.

Note that to avoid the need to perform two separate allocations, the TLS memory
for the Wasm Worker will be located at the bottom end (low memory address) of
the Wasm Worker stack space.

Wasm Workers vs the earlier Emscripten Worker API
=================================================

Emscripten provides a second Worker API as part of the emscripten.h header. This Worker API predates the advent of SharedArrayBuffer, and is quite distinct from Wasm Workers API, just the naming of these two APIs is similar due to historical reasons.

Both APIs allow one to spawn Web Workers from the main thread, though the semantics are different.

With the Worker API, the user will be able to spawn a Web Worker from a custom URL. This URL can point to a completely separate JS file that was not compiled with Emscripten, to load up Workers from arbitrary URLs. With Wasm Workers, a custom URL is not specified: Wasm Workers will always spawn a Web Worker that computes in the same WebAssembly+JavaScript context as the main program.

The Worker API does not integrate with SharedArrayBuffer, so interaction with the loaded Worker will always be asynchronous. Wasm Workers howerer is built on top of SharedArrayBuffer, and each Wasm Worker shares and computes in the same WebAssembly Memory address space of the main thread.

Both the Worker API and Wasm Workers API provide the user with ability to postMessage() function calls to the Worker. In Worker API, this message posting is restricted to need to originate/initiate from the main thread towards the Worker (using the API ``emscripten_call_worker()`` and ``emscripten_worker_respond()`` in ``<emscripten.h>``). With Wasm Workers however one can also postMessage() function calls to their parent (owning) thread.

If posting function calls with the Emscripten Worker API, it is required that the target Worker URL points to an Emscripten compiled program (so it has the ``Module`` structure to locate function names). Only functions that have been exported to the ``Module`` object are callable. With Wasm Workers, any C/C++ function may be posted, and does not need to be exported.

Use the Emscripten Worker API when:
 - you want to easily spawn a Worker from a JS file that was not built using Emscripten
 - you want to spawn as Worker a single separate compiled program than the main thread program represents, and the main thread and Worker programs do not share common code
 - you do not want to require the use of SharedArrayBuffer, or setting up COOP+COEP headers
 - you only need to communicate with the Worker asynchronously using postMessage() function calls

Use the Wasm Workers API when:
 - you want to create one or more new threads that synchronously compute in the same Wasm Module context
 - you want to spawn multiple Workers from the same codebase and save memory by sharing the WebAssembly Module (object code) and Memory (address space) across the Workers
 - you want to synchronously coordinate communication between threads by using atomic primitives and locks
 - your web server has been configured with the needed COOP+COEP headers to enable SharedArrayBuffer capabilities on the site

Limitations
===========

The following build options are not supported at the moment with Wasm Workers:

- -sSINGLE_FILE
- Dynamic linking (-sLINKABLE, -sMAIN_MODULE, -sSIDE_MODULE)
- -sPROXY_TO_WORKER
- -sPROXY_TO_PTHREAD

Example Code
============

See the directory ``test/wasm_workers/`` for code examples on different Wasm Workers API functionality.
