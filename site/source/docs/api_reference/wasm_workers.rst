.. _wasm_workers:

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
    printf("Hello from wasm worker!\n");
  }

  int main()
  {
    emscripten_wasm_worker_t worker = emscripten_malloc_wasm_worker(/*stack size: */1024);
    emscripten_wasm_worker_post_function_v(worker, run_in_worker);
  }

Build the code by passing the Emscripten flag ``-sWASM_WORKERS=1`` at both compile
and link steps. The example code creates a new Worker on the main browser thread,
which shares the same WebAssembly.Module and WebAssembly.Memory object. Then a
``postMessage()`` is passed to the Worker to ask it to execute the function
``run_in_worker()`` to print a string.

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
cross-compiled to multiple platforms, like to a native Linux x64 executable and an
Emscripten WebAssembly based program.

Pthreads API in Emscripten seeks to carefully emulate compatibility and the features that
the native Pthreads platforms already provide. This helps porting large C/C++ codebases
over to WebAssembly.

Wasm Workers API on the other hand seeks to provide a more "direct mapping" to the web
multithreading primitives as they exist on the web. If an application is only developed to
target WebAssembly, and portability is not a concern, then using Wasm Workers can provide
great benefits in the form of simpler compiled output, less complexity, smaller code size
and better performance.

However this benefit might not be an obvious win. The Pthreads API was designed to be useful
from the synchronous C/C++ language, whereas Web Workers are designed to be useful from
asynchronous JavaScript. WebAssembly C/C++ programs can find themselves somewhere in
the middle.

To further understand the differences between Pthreads and Wasm Workers, refer to the following
table.


.. raw:: html

  <table style='border:solid 2px;' cellpadding=10>
    <tr><td class='cellborder'>Feature</td>
    <td class='cellborder'>Pthreads</td>
    <td class='cellborder'>Wasm Workers</td> </tr>

    <tr><td class='cellborder'>Workers vs threads</td>
    <td class='cellborder'>Worker is either <i>dormant</i> or <i>hosts</i> an active thread.</td>
    <td class='cellborder'>Each Worker <b>is</b> an active thread (colloquially synonymous).</td> </tr>

    <tr><td class='cellborder'>Worker pooling</td>
    <td class='cellborder'>Dormant and active Workers reside in a system pool.<br>Worker returns to pool after its hosted thread terminates.</td>
    <td class='cellborder'>Not pooled (implement pooling yourself)</td> </tr>

    <tr><td class='cellborder'>Thread startup</td>
    <td class='cellborder'>Threads start <i>synchronously</i> and <i>fast</i> if enough dormant Workers available in pool to host, <i>asynchronously</i> and <i>slow</i> otherwise.</td>
    <td class='cellborder'>Workers always start <i>asynchronously</i> and <i>slow</i>.</td></tr>

    <tr><td class='cellborder'>Thread entry point</td>
    <td class='cellborder'>Thread starts with execution of an entry point function, returning from that function (by default) exits the thread.</td>
    <td class='cellborder'>No concept of a thread entry point, created Workers are idle after creation, not executing any user code until functions are posted to them.</td></tr>

    <tr><td class='cellborder'>Thread termination</td>
    <td class='cellborder'>Thread terminates by returning from entry point, or by calling <pre>pthread_exit(code)</pre>or by main thread calling <pre>pthread_kill(code)</pre></td>
    <td class='cellborder'>Worker cannot terminate itself, parent thread terminates by calling <pre>emscripten_terminate_wasm_worker(worker)</pre></td></tr>

    <tr><td class='cellborder'>Dynamic memory (malloc) utilization</td>
    <td class='cellborder'>Requires dynamic memory allocator, allocates memory for internal operation.</td>
    <td class='cellborder'>Dynamic memory allocator not necessary, manual placement allocation possible.</td></tr>

    <tr><td class='cellborder'>Code size overhead</td>
    <td class='cellborder'>Few hundred KBs</td>
    <td class='cellborder'>Few KBs</td></tr>

    <tr><td class='cellborder'>Thread stack size</td>
    <td class='cellborder'>Specify in pthread_attr_t structure.</td>
    <td class='cellborder'>Manage thread stack area explicitly with <pre>emscripten_create_wasm_worker_*_tls()</pre> functions, or
      <br>automatically allocate stack with <pre>emscripten_malloc_wasm_worker()</pre> API.</td></tr>

    <tr><td class='cellborder'>Thread Local Storage (TLS)</td>
    <td class='cellborder'>Supported transparently.</td>
    <td class='cellborder'>Supported either explicitly with <pre>emscripten_create_wasm_worker_*_tls()</pre> functions, or
      <br>automatically via <pre>emscripten_malloc_wasm_worker()</pre> API.</td></tr>

    <tr><td class='cellborder'>Thread ID</td>
    <td class='cellborder'>Creating a pthread obtains its ID. Call <pre>pthread_self()</pre> to acquire ID of calling thread.</td>
    <td class='cellborder'>Creating a Worker obtains its ID. Call <pre>emscripten_wasm_worker_self_id()</pre> acquire ID of calling thread.</td></tr>

    <tr><td class='cellborder'>emscripten_get_now()</td>
    <td class='cellborder'>All pthreads are synchronized to the same wallclock time base, so emscripten_get_now() return values across threads are comparable.</td>
    <td class='cellborder'>Workers each have their own wallclock time base, emscripten_get_now() is not synchronized across them.</td></tr>

    <tr><td class='cellborder'>Synchronous blocking on main thread</td>
    <td class='cellborder'>Synchronization primitives internally fall back to busy spin loops.</td>
    <td class='cellborder'>Explicit spin vs sleep synchronization primitives.</td></tr>

    <tr><td class='cellborder'>Futex API</td>
    <td class='cellborder'><pre>emscripten_futex_wait</pre><pre>emscripten_futex_wake</pre> in emscripten/threading.h</td>
    <td class='cellborder'><pre>emscripten_wasm_wait_i32</pre><pre>emscripten_wasm_wait_i64</pre><pre>emscripten_wasm_notify</pre> in emscripten/wasm_workers.h</td></tr>

    <tr><td class='cellborder'>Asynchronous futex wait</td>
    <td class='cellborder'>N/A</td>
    <td class='cellborder'><pre>emscripten_atomic_wait_async()</pre><pre>emscripten_*_async_acquire()</pre>However these are a difficult footgun, read <a href='https://github.com/WebAssembly/threads/issues/176'>WebAssembly/threads issue #176</a></td></tr>

    <tr><td class='cellborder'>C/C++ Function Proxying</td>
    <td class='cellborder'>emscripten/threading.h API for proxying function calls to other threads.</td>
    <td class='cellborder'>Use emscripten_wasm_worker_post_function_*() API to message functions to other threads. These messages follow event queue semantics rather than proxy queue semantics.</td></tr>

    <tr><td class='cellborder'>JS Library Main Thread Proxying</td>
    <td class='cellborder'>Use the foo__proxy: 'sync'/'async' directive to specify a JS function to be run on the main thread context.</td>
    <td class='cellborder'>N/A. JS code is always run on the calling thread context. Functions with __proxy directive will abort at runtime if called in a Worker.</td></tr>

    <tr><td class='cellborder'>Proxied EM_ASM</td>
    <td class='cellborder'>Use MAIN_THREAD_EM_ASM() and MAIN_THREAD_ASYNC_EM_ASM() to proxy EM_ASM code blocks to the main thread.</td>
    <td class='cellborder'>N/A. Only calling thread EM_ASM() function blocks are possible.</td></tr>

    <tr><td class='cellborder'>Build flags</td>
    <td class='cellborder'>Compile and link with -pthread</td>
    <td class='cellborder'>Compile and link with -sWASM_WORKERS=1</td></tr>

    <tr><td class='cellborder'>Preprocessor directives</td>
    <td class='cellborder'>__EMSCRIPTEN_PTHREADS__=1 and __EMSCRIPTEN_SHARED_MEMORY__=1 are active</td>
    <td class='cellborder'>__EMSCRIPTEN_PTHREADS__=1, __EMSCRIPTEN_SHARED_MEMORY__=1 and __EMSCRIPTEN_WASM_WORKERS__=1 are active</td></tr>

    <tr><td class='cellborder'>JS library directives</td>
    <td class='cellborder'>USE_PTHREADS=1 and SHARED_MEMORY=1 are active</td>
    <td class='cellborder'>USE_PTHREADS=1, SHARED_MEMORY=1 and WASM_WORKERS=1 are active</td></tr>

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
    <td class='cellborder'><pre>Supported with -sOFFSCREEN_FRAMEBUFFER=1</pre></td>
    <td class='cellborder'><pre>Not supported.</pre></td></tr>

  </table>

Limitations and TODOs
=====================

Currently it is not possible to simultaneously use pthreads and Wasm Workers in the same application, but this may change in the future.

Also, the following build options are not supported at the moment with Wasm Workers:

- -sSINGLE_FILE=1
- Dynamic linking (-sLINKABLE=1, -sMAIN_MODULE=1, -sSIDE_MODULE=1)
- -sPROXY_TO_WORKER=1
- -sPROXY_TO_PTHREAD=1

Example Code
============

See the directory tests/wasm_workers/ for code examples on different Wasm Workers API functionality.
