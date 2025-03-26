.. _fiber-h:

=======
fiber.h
=======

`Fibers <https://en.wikipedia.org/wiki/Fiber_(computer_science)>`_ are light, co-operative threads of execution. The `fiber.h <https://github.com/emscripten-core/emscripten/blob/main/system/include/emscripten/fiber.h>`_ header defines a low-level API for manipulating Fibers in Emscripten. Fibers are implemented with :ref:`asyncify section`, so you must link your program with :ref:`ASYNCIFY` if you intend to use them.

Fibers are intended as a building block for asynchronous control flow constructs,
such as coroutines. They supersede the legacy coroutine API that was available
in the fastcomp backend. This API is similar to, but distinct from, POSIX
`ucontext <https://en.wikipedia.org/wiki/Setcontext>`_.

.. contents:: Table of Contents
    :local:
    :depth: 3

API Reference
=============

Types
-----

.. c:type:: emscripten_fiber_t

  This structure represents a Fiber context `continuation <https://en.wikipedia.org/wiki/Continuation>`_. The runtime does not keep references to these objects, they only contain information needed to perform the context switch. The switch operation updates some of the contents, however.

  .. c:member:: void *stack_base

    Upper limit of the C stack region. The stack grows down, so this is the initial position of the stack pointer. Must be at least 16-byte aligned.

  .. c:member:: void *stack_limit

    Lower limit of the C stack region. Must be below :c:member:`emscripten_fiber_t.stack_base`.

  .. c:member:: void *stack_ptr

    Current position of the stack pointer. Must be between :c:member:`emscripten_fiber_t.stack_base` and :c:member:`emscripten_fiber_t.stack_limit`.

  .. c:member:: em_arg_callback_func entry

    Entry point. If not NULL, this function will be called when the fiber is switched into. Otherwise, :c:member:`emscripten_fiber_t.asyncify_data` is used to rewind the call stack.

  .. c:member:: void *user_data

    Opaque pointer, passed as-is to :c:member:`emscripten_fiber_t.entry`.

  .. c:member:: asyncify_data_t asyncify_data

    Asyncify data structure. Used to unwind and rewind the call stack when switching fibers.

.. c:type:: asyncify_data_t

  .. c:member:: void *stack_ptr

    Current position of the Asyncify stack pointer.

    The Asyncify stack is distinct from the C stack. It contains the call stack as well as the state of WASM locals. Unlike the C stack, it grows up.

  .. c:member:: void *stack_limit

    Upper limit of the Asyncify stack region.

  .. c:member:: int rewind_id

    Opaque handle to the function that needs to be called in order to rewind the call stack. This value is only meaningful to the runtime.

    .. warning:: Rewind IDs are currently thread-specific. This makes it impossible to resume a fiber that has been started from a different thread.


Functions
---------

.. c:function:: void emscripten_fiber_init(emscripten_fiber_t *fiber, em_arg_callback_func entry_func, void *entry_func_arg, void *c_stack, size_t c_stack_size, void *asyncify_stack, size_t asyncify_stack_size)

  Initializes a fiber context. It can then be entered by calling :c:func:`emscripten_fiber_swap`.

  :param emscripten_fiber_t* fiber: Pointer to the fiber structure.
  :param em_arg_callback_func entry_func: Entry point function, called when the fiber is entered for the first time.
  :param void* entry_func_arg: Opaque pointer passed to `entry_func`.
  :param void* c_stack: Pointer to memory region to use for the C stack. Must be at least 16-byte aligned. This points to the lower bound of the stack, regardless of growth direction.
  :param size_t c_stack_size: Size of the C stack memory region, in bytes.
  :param void* asyncify_stack: Pointer to memory region to use for the Asyncify stack. No special alignment requirements.
  :param size_t asyncify_stack_size: Size of the Asyncify stack memory region, in bytes.

  .. note:: If `entry_func` returns, the entire program will end, as if `main` had returned. To avoid this, you can use :c:func:`emscripten_fiber_swap` to jump to another fiber.

.. c:function:: void emscripten_fiber_init_from_current_context(emscripten_fiber_t *fiber, void *asyncify_stack, size_t asyncify_stack_size)

  Partially initializes a fiber based on the currently active context. This is needed in order to switch back from a fiber into the thread's original context.

  This function sets up :c:member:`emscripten_fiber_t.stack_base` and :c:member:`emscripten_fiber_t.stack_limit` to refer to the current stack boundaries, sets :c:member:`emscripten_fiber_t.entry` to `NULL`, and makes :c:member:`emscripten_fiber_t.asyncify_data` refer to the provided Asyncify stack memory. Other fields are not changed.

  Fibers initialized by this function are not complete. They are only suitable to pass as the first argument to :c:func:`emscripten_fiber_swap`. Doing that completes the continuation, making it possible to switch back to the original context with another :c:func:`emscripten_fiber_swap`, as with a normal fiber.

  :param emscripten_fiber_t* fiber: Pointer to the fiber structure.
  :param void* asyncify_stack: Pointer to memory region to use for the Asyncify stack. No special alignment requirements.
  :param size_t asyncify_stack_size: Size of the Asyncify stack memory region, in bytes.

.. c:function:: void emscripten_fiber_swap(emscripten_fiber_t *old_fiber, emscripten_fiber_t *new_fiber)

  Performs a fiber context switch.

  :param emscripten_fiber_t* old_fiber: Fiber representing the current context. It will be partially updated, such as that switching back into it via another call to :c:func:`emscripten_fiber_swap` would appear to return from the original call.
  :param emscripten_fiber_t* new_fiber: Fiber representing the target context. If the fiber has an entry point, it will be called in the new context and set to `NULL`. Otherwise, :c:member:`emscripten_fiber_t.asyncify_data` is used to rewind the call stack. If the fiber is invalid or incomplete, the behavior is undefined.
