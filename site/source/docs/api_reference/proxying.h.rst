.. _proxying-h:

==========
proxying.h
==========

The Emscripten `proxying.h`_
API provides a mechanism for dispatching work to be executed by a target thread
and optionally blocking until that work has been completed. This is particularly
useful in a Web context where certain JS APIs can only be called on particular
threads; any thread can access those APIs by proxying the API calls to the
correct threads. Proxying is also useful for wrapping asynchronous JS tasks with
a synchronous interface. The calling thread can proxy the asynchronous task to a
dedicated worker thread and wait until the worker thread eventually marks the
task as complete, possibly after returning multiple times to the JS event loop.

On the target thread, queued work can be executed in two ways. First, the user
may explicitly call ``emscripten_proxy_execute_queue`` to execute any tasks that
have been queued for the current thread. Alternatively, queued work will execute
automatically if the target thread returns to the JS event loop, for example via
``emscripten_exit_with_live_runtime``.

See `test_pthread_proxying.c`_ and `test_pthread_proxying_cpp.cpp`_ for examples
of how to use the proxying API.

API Reference
=============

Types
-----

.. c:type:: em_proxying_queue

  An opaque handle to a set of thread-local work queues (one per thread) to
  which work can be asynchronously or synchronously proxied from other threads.

  Proxied work can only be completed on live thread runtimes, so users must
  ensure either that all proxied work is completed before a thread exits or that
  the thread exits with a live runtime, e.g. via
  ``emscripten_exit_with_live_runtime`` to avoid dropped work.

.. c:type:: em_proxying_ctx

  An opaque handle to a currently-executing proxied task, used to signal the end
  of the task.

Functions
---------

.. c:function:: em_proxying_queue* em_proxying_queue_create()

  Allocate a new proxying queue.

.. c:function:: void em_proxying_queue_destroy(em_proxying_queue* q)

  Free a proxying queue. The queue should not have any remaining queued work.

.. c:function:: em_proxying_queue* emscripten_proxy_get_system_queue()

  Get the queue used for proxying low-level runtime work. Work on this queue may
  be processed at any time inside system functions, so it must be nonblocking
  and safe to run at any time, similar to a native signal handler. User code
  should generally not use this function.

.. c:function:: void emscripten_proxy_execute_queue(em_proxying_queue* q)

  Execute all the tasks enqueued for the current thread on the given queue. New
  tasks that are enqueued concurrently with this execution will be executed as
  well. This function returns once it observes an empty queue.

.. c:function:: void emscripten_proxy_finish(em_proxying_ctx* ctx)

  Signal the end of a task proxied with ``emscripten_proxy_sync_with_ctx``.

.. c:function:: int emscripten_proxy_async(em_proxying_queue* q, pthread_t target_thread, void (*func)(void*), void* arg)

  Enqueue ``func`` to be called with argument ``arg`` on the given queue and
  thread then return immediately without waiting for ``func`` to be executed.
  Returns 1 if the work was successfully enqueued or 0 otherwise.

.. c:function:: int emscripten_proxy_sync(em_proxying_queue* q, pthread_t target_thread, void (*func)(void*), void* arg)

  Enqueue ``func`` to be called with argument ``arg`` on the given queue and
  thread then wait for ``func`` to be executed synchronously before returning.
  Returns 1 if the ``func`` was successfully completed and 0 otherwise,
  including if the target thread is canceled or exits before the work is
  completed.

.. c:function:: int emscripten_proxy_sync_with_ctx(em_proxying_queue* q, pthread_t target_thread, void (*func)(em_proxying_ctx*, void*), void* arg)

  The same as ``emscripten_proxy_sync`` except that instead of waiting for the
  proxied function to return, it waits for the proxied task to be explicitly
  marked finished with ``emscripten_proxy_finish``. ``func`` need not call
  ``emscripten_proxy_finish`` itself; it could instead store the context pointer
  and call ``emscripten_proxy_finish`` at an arbitrary later time.

.. c:function:: int emscripten_proxy_callback(em_proxying_queue* q, pthread_t target_thread, void (*func)(void*), void (*callback)(void*), void (*cancel)(void*), void* arg)

  Enqueue ``func`` on the given queue and thread. Once (and if) it finishes
  executing, it will asynchronously proxy ``callback`` back to the current
  thread on the same queue, or if the target thread dies before the work can be
  completed, ``cancel`` will be proxied back instead. All three function will
  receive the same argument, ``arg``. Returns 1 if ``func`` was successfully
  enqueued and the target thread notified or 0 otherwise.

.. c:function:: int emscripten_proxy_callback_with_ctx(em_proxying_queue* q, pthread_t target_thread, void (*func)(em_proxying_ctx*, void*), void (*callback)(void*), void (*cancel)(void*), void* arg)

  Enqueue ``func`` on the given queue and thread. Once (and if) it finishes the
  task by calling ``emscripten_proxy_finish`` on the given ``em_proxying_ctx``,
  it will asynchronously proxy ``callback`` back to the current thread on the
  same queue, or if the target thread dies before the work can be completed,
  ``cancel`` will be proxied back instead. All three function will receive the
  same argument, ``arg``. Returns 1 if ``func`` was successfully enqueued and
  the target thread notified or 0 otherwise.

C++ API
-------

This C++ API is provided by proxying.h when compiling with C++11 or later. It is
defined within namespace ``emscripten``.

.. cpp:type:: ProxyingQueue

  A thin C++ wrapper around an ``em_proxying_queue*``.

  .. cpp:type:: ProxyingCtx

  A thin C++ wrapper around an ``em_proxying_ctx*``.

    .. cpp:member:: em_proxying_ctx* ctx

    The wrapped ``em_proxying_ctx*``.

    .. cpp:member:: void finish()

    Calls ``emscripten_proxy_finish`` on the wrapped ``em_proxying_ctx*``.

  .. cpp:member:: void execute()

    Calls ``emscripten_proxy_execute_queue`` on the wrapped ``em_proxying_queue*``.

  .. cpp:member:: bool proxyAsync(pthread_t target, std::function<void()>&& func)

    Calls ``emscripten_proxy_async`` to execute ``func``, returning ``true`` if the
    function was successfully enqueued and ``false`` otherwise.

  .. cpp:member:: bool proxySync(const pthread_t target, const std::function<void()>& func)

    Calls ``emscripten_proxy_sync`` to execute ``func``, returning ``true`` if the
    function was successfully completed or ``false`` otherwise.

  .. cpp:member:: bool proxySyncWithCtx(const pthread_t target, const std::function<void(ProxyingCtx)>& func)

    Calls ``emscripten_proxy_sync_with_ctx`` to execute ``func``, returning
    ``true`` if the function was successfully marked done with
    ``emscripten_proxy_finish`` or ``ProxyingCtx::finish`` and ``false``
    otherwise.

  .. cpp:member:: bool proxyCallback(pthread_t target, std::function<void()>&& func, std::function<void()>&& callback, std::function<void()>&& cancel)

    Calls ``emscripten_proxy_callback`` to execute ``func`` and schedule either
    ``callback`` or ``cancel``, returning ``true`` if the function was
    successfully enqueued and ``false`` otherwise.

  .. cpp:member:: bool proxyCallbackWithCtx(pthread_t target, std::function<void(ProxyingCtx)>&& func, std::function<void()>&& callback, std::function<void()>&& cancel)

    Calls ``emscripten_proxy_callback_with_ctx`` to execute ``func`` and
    schedule either ``callback`` or ``cancel``, returning ``true`` if the
    function was successfully enqueued and ``false`` otherwise.

.. _proxying.h: https://github.com/emscripten-core/emscripten/blob/main/system/include/emscripten/proxying.h
.. _test_pthread_proxying.c: https://github.com/emscripten-core/emscripten/blob/main/test/pthread/test_pthread_proxying.c
.. _test_pthread_proxying_cpp.cpp: https://github.com/emscripten-core/emscripten/blob/main/test/pthread/test_pthread_proxying_cpp.cpp
