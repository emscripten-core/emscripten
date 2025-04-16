.. _stack-h:

=======
stack.h
=======

The functions defined in `<emscripten/stack.h>` allow inspecting
information about the WebAssembly data stack (sometimes called the
"user stack" or the "C stack"). This data stack is the data contained
within the linear memory (as opposed to the trusted call stack that
is managed by the VM, and which is not visible to the running program).

.. c:function:: uintptr_t emscripten_stack_get_base(void)

  Returns the starting address of the stack. This is the address
  that the stack pointer would point to when no bytes are in use on the
  stack.

.. c:function:: uintptr_t emscripten_stack_get_end(void)

  Returns the end address of the stack. This is the address that
  the stack pointer would point to when the whole stack is in use. (The
  address pointed to by the end is not part of the stack itself). Note
  that the stack grows down so the address returned by
  `emscripten_stack_get_end()` is smaller than
  :c:func:`emscripten_stack_get_base()`.

.. c:function:: void emscripten_stack_set_limits(void* base, void* end)

  Sets the internal values reported by :c:func:`emscripten_stack_get_base()`
  and :c:func:`emscripten_stack_get_end()`.  This should only be used by low
  level libraries such as asyncify fibers.

.. c:function:: uintptr_t emscripten_stack_get_current(void)

  Returns the current stack pointer.

.. c:function:: size_t emscripten_stack_get_free(void)

  Returns the number of free bytes left on the stack.  This is required
  to be fast so that it can be called frequently.
