.. _threading-h:

===========
threading.h
===========

The C++ API in `threading.h <https://github.com/kripken/emscripten/blob/master/system/include/emscripten/threading.h>`_ allow to interract with threads.

.. contents:: Table of contents
    :local:
    :depth: 1


Threads support
===============

Theses functions gives information about the support of threads.


Functions
---------

.. c:function:: int emscripten_has_threading_support(void)

    Returns if the browser is able to spawn threads with ``pthread_create()``, and the compiled page was built with threading support enabled. If this returns 0, calls to ``pthread_create()`` will fail with return code ``EAGAIN``.

    :return: Return true if the current browser is abble to spawn thread and the page was build with ``-s USE_PTHREADS=1``
    :rtype: int

.. c:function:: int emscripten_num_logical_cores(void)

    Returns the number of logical cores on the system.

    :return: The number of cores on the system.
    :rtype: int

.. c:function:: void emscripten_force_num_logical_cores(int cores)

    Configures the number of logical cores on the system. This can be called at startup to specify the number of cores ``emscripten_num_logical_cores()`` reports. The Emscripten system itself does not use this value anywhere, it is just a hint to developers have a single access point ``emscripten_num_logical_cores()`` to query the number of cores in the system.

    :param cores: The number of core to specifie.
    :type cores: int


Atomic operations
=================

All atomics operations available.


Functions
---------

.. c:function:: uint8_t emscripten_atomic_exchange_u8(void* addr, uint8_t newVal)

    Atomicaly stores the given value to the memory location, and returns the values that was prior to the store.

    :param addr: The address of the memory location.
    :type addr: void*
    :param newVal: The new value for the memory location.
    :type newVal: uint8_t
    :return: The old value of the memory location.
    :rtype: uint8_t

.. c:function:: uint16_t emscripten_atomic_exchange_u16(void* addr, uint16_t newVal)

    Atomicaly stores the given value to the memory location, and returns the values that was prior to the store.

    :param addr: The address of the memory location.
    :type addr: void*
    :param newVal: The new value for the memory location.
    :type newVal: uint16_t
    :return: The old value of the memory location.
    :rtype: uint16_t

.. c:function:: uint32_t emscripten_atomic_exchange_u32(void* addr, uint32_t newVal)

    Atomicaly stores the given value to the memory location, and returns the values that was prior to the store.

    :param addr: The address of the memory location.
    :type addr: void*
    :param newVal: The new value for the memory location.
    :type newVal: uint32_t
    :return: The old value of the memory location.
    :rtype: uint32_t

.. c:function:: uint64_t emscripten_atomic_exchange_u64(void* addr, uint64_t newVal)

    Atomicaly stores the given value to the memory location, and returns the values that was prior to the store.

    :param addr: The address of the memory location.
    :type addr: void*
    :param newVal: The new value for the memory location.
    :type newVal: uint64_t
    :return: The old value of the memory location.
    :rtype: uint64_t

    .. warning: This function is very slow because is emulated with locks.


.. c:function:: uint8_t emscripten_atomic_cas_u8(void* addr, uint8_t newVal)

    Returns the *old* value that was in the memory location before the operation took place.
    That is, if the return value when calling this function equals to ``oldVal``, then the operation succeed, otherwise it was ignored.

    :param addr: The address of the memory location.
    :type addr: void*
    :param oldVal: The old value of the memory location.
    :type oldVal: uint8_t
    :param newVal: The new value for the memory location.
    :type newVal: uint8_t
    :return: The old value of the memory location, if equals to ``oldVal`` operation succeeded.
    :rtype: uint8_t

.. c:function:: uint16_t emscripten_atomic_cas_u16(void* addr, uint16_t newVal)

    Returns the *old* value that was in the memory location before the operation took place.
    That is, if the return value when calling this function equals to ``oldVal``, then the operation succeed, otherwise it was ignored.

    :param addr: The address of the memory location.
    :type addr: void*
    :param oldVal: The old value of the memory location.
    :type oldVal: uint16_t
    :param newVal: The new value for the memory location.
    :type newVal: uint16_t
    :return: The old value of the memory location, if equals to ``oldVal`` operation succeeded.
    :rtype: uint16_t

.. c:function:: uint32_t emscripten_atomic_cas_u32(void* addr, uint32_t newVal)

    Returns the *old* value that was in the memory location before the operation took place.
    That is, if the return value when calling this function equals to ``oldVal``, then the operation succeed, otherwise it was ignored.

    :param addr: The address of the memory location.
    :type addr: void*
    :param oldVal: The old value of the memory location.
    :type oldVal: uint32_t
    :param newVal: The new value for the memory location.
    :type newVal: uint32_t
    :return: The old value of the memory location, if equals to ``oldVal`` operation succeeded.
    :rtype: uint32_t

.. c:function:: uint64_t emscripten_atomic_cas_u64(void* addr, uint64_t newVal)

    Returns the *old* value that was in the memory location before the operation took place.
    That is, if the return value when calling this function equals to ``oldVal``, then the operation succeed, otherwise it was ignored.

    :param addr: The address of the memory location.
    :type addr: void*
    :param oldVal: The old value of the memory location.
    :type oldVal: uint64_t
    :param newVal: The new value for the memory location.
    :type newVal: uint64_t
    :return: The old value of the memory location, if equals to ``oldVal`` operation succeeded.
    :rtype: uint64_t

    .. warning: This function is very slow because is emulated with locks.


.. c:function:: uint8_t emscripten_atomic_load_u8(void* addr)

    Loads the value from the memory location.

    :param addr: The address the memory location.
    :type addr: void*
    :return: The value of the memory location.
    :rtype: uint8_t

.. c:function:: uint16_t emscripten_atomic_load_u16(void* addr)

    Loads the value from the memory location.

    :param addr: The address the memory location.
    :type addr: void*
    :return: The value of the memory location.
    :rtype: uint16_t

.. c:function:: uint32_t emscripten_atomic_load_u32(void* addr)

    Loads the value from the memory location.

    :param addr: The address the memory location.
    :type addr: void*
    :return: The value of the memory location.
    :rtype: uint32_t

.. c:function:: float emscripten_atomic_load_f32(void* addr)

    Loads the value from the memory location.

    :param addr: The address the memory location.
    :type addr: void*
    :return: The value of the memory location.
    :rtype: float

.. c:function:: uint64_t emscripten_atomic_load_u64(void* addr)

    Loads the value from the memory location.

    :param addr: The address the memory location.
    :type addr: void*
    :return: The value of the memory location.
    :rtype: uint64_t

    .. warning: This function is very slow because is emulated with locks.

.. c:function:: double emscripten_atomic_load_f64(void* addr)

    Loads the value from the memory location.

    :param addr: The address the memory location.
    :type addr: void*
    :return: The value of the memory location.
    :rtype: double

    .. warning: This function is very slow because is emulated with locks.


.. c:function:: uint8_t emscripten_atomic_store_u8(void* addr, uint8_t val)

    Stores the value in the memory location and return the value that was stored (i.e. ``val``).

    :param addr: The address of the memory location.
    :type addr: void*
    :param val: The new value for the memory location.
    :type val: uint8_t
    :return: The old value of the memory location.
    :rtype: uint8_t

.. c:function:: uint16_t emscripten_atomic_store_u16(void* addr, uint16_t val)

    Stores the value in the memory location and return the value that was stored (i.e. ``val``).

    :param addr: The address of the memory location.
    :type addr: void*
    :param val: The new value for the memory location.
    :type val: uint16_t
    :return: The old value of the memory location.
    :rtype: uint16_t

.. c:function:: uint32_t emscripten_atomic_store_u32(void* addr, uint32_t val)

    Stores the value in the memory location and return the value that was stored (i.e. ``val``).

    :param addr: The address of the memory location.
    :type addr: void*
    :param val: The new value for the memory location.
    :type val: uint32_t
    :return: The old value of the memory location.
    :rtype: uint32_t

.. c:function:: float emscripten_atomic_store_f32(void* addr, float val)

    Stores the value in the memory location and return the value that was stored (i.e. ``val``).

    :param addr: The address of the memory location.
    :type addr: void*
    :param val: The new value for the memory location.
    :type val: float
    :return: The old value of the memory location.
    :rtype: float

.. c:function:: uint64_t emscripten_atomic_store_u64(void* addr, uint64_t val)

    Stores the value in the memory location and return the value that was stored (i.e. ``val``).

    :param addr: The address of the memory location.
    :type addr: void*
    :param val: The new value for the memory location.
    :type val: uint64_t
    :return: The old value of the memory location.
    :rtype: uint64_t

    .. warning: This function is very slow because is emulated with locks.

.. c:function:: double emscripten_atomic_store_f64(void* addr, double val)

    Stores The value in the memory location and return the value that was stored (i.e. ``val``).

    :param addr: The address of the memory location.
    :type addr: void*
    :param val: The new value for the memory location.
    :type val: double
    :return: The old value of the memory location.
    :rtype: double

    .. warning: This function is very slow because is emulated with locks.

.. c:function:: void emscripten_atomic_fence(void)

    Put an std::atomic_thread_fence with memory_order_seq_cst

.. c:function:: uint8_t emscripten_atomic_add_u8(void* addr, uint8_t val)

    Add the value to the memory location and then return the result.

    :param addr: The address of the memory location.
    :type addr: void*
    :param val: The value to add to the value of the memory location.
    :type val: uint8_t
    :return: The old value of the memory location.
    :rtype: uint8_t

.. c:function:: uint16_t emscripten_atomic_add_u16(void* addr, uint16_t val)

    Add the value to the memory location and then return the result.

    :param addr: The address of the memory location.
    :type addr: void*
    :param val: The value to add to the value of the memory location.
    :type val: uint16_t
    :return: The old value of the memory location.
    :rtype: uint16_t

.. c:function:: uint32_t emscripten_atomic_add_u32(void* addr, uint32_t val)

    Add the value to the memory location and then return the result.

    :param addr: The address of the memory location.
    :type addr: void*
    :param val: The value to add to the value of the memory location.
    :type val: uint32_t
    :return: The old value of the memory location.
    :rtype: uint32_t

.. c:function:: uint64_t emscripten_atomic_add_u64(void* addr, uint64i_t val)

    Add the value to the memory location and then return the result.

    :param addr: The address of the memory location.
    :type addr: void*
    :param val: The value to add to the value of the memory location.
    :type val: uint64_t
    :return: The old value of the memory location.
    :rtype: uint64_t

    .. warning: This function is very slow because is emulated with locks.

.. c:function:: uint8_t emscripten_atomic_sub_u8(void* addr, uint8_t val)

    Substract the value to the memory location and then return the result.

    :param addr: The address of the memory location.
    :type addr: void*
    :param val: The value to substract to the value of the memory location.
    :type val: uint8_t
    :return: The old value of the memory location.
    :rtype: uint8_t

.. c:function:: uint16_t emscripten_atomic_sub_u16(void* addr, uint16_t val)

    Substract the value to the memory location and then return the result.

    :param addr: The address of the memory location.
    :type addr: void*
    :param val: The value to substract to the value of the memory location.
    :type val: uint16_t
    :return: The old value of the memory location.
    :rtype: uint16_t

.. c:function:: uint32_t emscripten_atomic_sub_u32(void* addr, uint32_t val)

    Substract the value to the memory location and then return the result.

    :param addr: The address of the memory location.
    :type addr: void*
    :param val: The value to substract to the value of the memory location.
    :type val: uint32_t
    :return: The old value of the memory location.
    :rtype: uint32_t

.. c:function:: uint64_t emscripten_atomic_sub_u64(void* addr, uint64_t val)

    Substract the value to the memory location and then return the result.

    :param addr: The address of the memory location.
    :type addr: void*
    :param val: The value to substract to the value of the memory location.
    :type val: uint64_t
    :return: The old value of the memory location.
    :rtype: uint64_t

    .. warning: This function is very slow because is emulated with locks.

.. c:function:: uint8_t emscripten_atomic_and_u8(void* addr, uint8_t val)

    Do a logicaly and with ``val`` and the value to the memory location.

    :param addr: The address of the memory location.
    :type addr: void*
    :param val: The value with which do the logical and.
    :type val: uint8_t
    :return: The old value of the memory location.
    :rtype: uint8_t

.. c:function:: uint16_t emscripten_atomic_and_u16(void* addr, uint16_t val)

    Do a logicaly and with ``val`` and the value to the memory location.

    :param addr: The address of the memory location.
    :type addr: void*
    :param val: The value with which do the logical and.
    :type val: uint16_t
    :return: The old value of the memory location.
    :rtype: uint16_t

.. c:function:: uint32_t emscripten_atomic_and_u32(void* addr, uint32_t val)

    Do a logicaly and with ``val`` and the value to the memory location.

    :param addr: The address of the memory location.
    :type addr: void*
    :param val: The value with which do the logical and.
    :type val: uint32_t
    :return: The old value of the memory location.
    :rtype: uint32_t

.. c:function:: uint64_t emscripten_atomic_and_u64(void* addr, uint64_t val)

    Do a logicaly and with ``val`` and the value to the memory location.

    :param addr: The address of the memory location.
    :type addr: void*
    :param val: The value with which do the logical and.
    :type val: uint64_t
    :return: The old value of the memory location.
    :rtype: uint64_t

    .. warning: This function is very slow because is emulated with locks.

.. c:function:: uint8_t emscripten_atomic_or_u8(void* addr, uint8_t val)

    Do a logicaly or with ``val`` and the value to the memory location.

    :param addr: The address of the memory location.
    :type addr: void*
    :param val: The value with which do the logical or.
    :type val: uint8_t
    :return: The old value of the memory location.
    :rtype: uint8_t

.. c:function:: uint16_t emscripten_atomic_or_u16(void* addr, uint16_t val)

    Do a logicaly or with ``val`` and the value to the memory location.

    :param addr: The address of the memory location.
    :type addr: void*
    :param val: The value with which do the logical or.
    :type val: uint16_t
    :return: The old value of the memory location.
    :rtype: uint16_t

.. c:function:: uint32_t emscripten_atomic_or_u32(void* addr, uint32_t val)

    Do a logicaly or with ``val`` and the value to the memory location.

    :param addr: The address of the memory location.
    :type addr: void*
    :param val: The value with which do the logical or.
    :type val: uint32_t
    :return: The old value of the memory location.
    :rtype: uint32_t

.. c:function:: uint64_t emscripten_atomic_or_u64(void* addr, uint64_t val)

    Do a logicaly or with ``val`` and the value to the memory location.

    :param addr: The address of the memory location.
    :type addr: void*
    :param val: The value with which do the logical or.
    :type val: uint64_t
    :return: The old value of the memory location.
    :rtype: uint64_t

    .. warning: This function is very slow because is emulated with locks.

.. c:function:: uint8_t emscripten_atomic_xor_u8(void* addr, uint8_t val)

    Do a logicaly xor with ``val`` and the value to the memory location.

    :param addr: The address of the memory location.
    :type addr: void*
    :param val: The value with which do the logical xor.
    :type val: uint8_t
    :return: The old value of the memory location.
    :rtype: uint8_t

.. c:function:: uint16_t emscripten_atomic_xor_u16(void* addr, uint16_t val)

    Do a logicaly xor with ``val`` and the value to the memory location.

    :param addr: The address of the memory location.
    :type addr: void*
    :param val: The value with which do the logical xor.
    :type val: uint16_t
    :return: The old value of the memory location.
    :rtype: uint16_t

.. c:function:: uint32_t emscripten_atomic_xor_u32(void* addr, uint32_t val)

    Do a logicaly xor with ``val`` and the value to the memory location.

    :param addr: The address of the memory location.
    :type addr: void*
    :param val: The value with which do the logical xor.
    :type val: uint32_t
    :return: The old value of the memory location.
    :rtype: uint32_t

.. c:function:: uint64_t emscripten_atomic_xor_u64(void* addr, uint64_t val)

    Do a logicaly xor with ``val`` and the value to the memory location.

    :param addr: The address of the memory location.
    :type addr: void*
    :param val: The value with which do the logical xor.
    :type val: uint64_t
    :return: The old value of the memory location.
    :rtype: uint64_t

    .. warning: This function is very slow because is emulated with locks.


Synchronisation
===============

Defines
-------

.. c:macro:: EM_QUEUED_CALL_MAX_ARGS 8

.. c:macro:: EM_FUNC_SIGNATURE int

Structures
----------

.. c:type:: em_variant_val

    This is an union to pass values.

    .. c:member:: int i

    .. c:member:: float f

    .. c:member:: void* vp

    .. c:member:: char* cp

.. c:type:: em_queued_call

    .. c:member:: int functionEnum

    .. c:member:: void* functionPtr

    .. c:member:: int operationDone

    .. c:member:: em_variant_val args[EM_QUEUED_CALL_MAX_ARGS]

    .. c:member:: em_variant_val returnValue

    .. c:member:: int calleeDelete

        If true, the caller has "detached" itselt from this call object and the Emscripten main runtime thread should free up this em_queued_call object after it has been executed. If false, the caller is in control of memory.


Callbak functions
-----------------

.. c:type:: void (*em_func_v)(void)
.. c:type:: void (*em_func_vi)(int)
.. c:type:: void (*em_func_vii)(int, int)
.. c:type:: void (*em_func_viii)(int, int, int)
.. c:type:: int (*em_func_i)(void)
.. c:type:: int (*em_func_ii)(int)
.. c:type:: int (*em_func_iii)(int, int)
.. c:type:: int (*em_func_iiii)(int, int, int)


Functions
---------


.. c:function:: int emscripten_futex_wait(volatile void* addr, uint32_t val, double maxWaitMilliseconds)

    If the values stored at ``addr`` is ``val``, sleep for ``maxWaitMilliseconds`` or until be awaken.

    :param addr: The memory location.
    :type addr: void*
    :param val: The value to compare.
    :type val: uint32_t
    :param maxWaitMilliseconds: The maximum amount of time to wait.
    :type maxWaitMilliseconds: double
    :return: the status of why it was awaken
    :rtype: int

.. c:function:: int emscripten_futex_wake(volatile void* addr, int count)

    Wake up a ``count`` number of threads waiting on the address addr.

    :param addr: The memory location.
    :type addr: void*
    :param count: The number of threads to wake up
    :type count: int
    :return:
    :rtype: int

.. c:function:: int emscripten_futex_wake_or_requeue(volatile void* addr, int count, volatile void* addr2, int cmpValue)

    If the value stored at the address ``addr`` is ``cmpValue``, wakes ``count`` threads waiting on the address ``addr`` and enqueue ``count`` threads waiting and the address ``addr`` to now wait to the new address ``addr2``.

    :param addr: The memory location.
    :type addr: void*
    :param count: The number of threads to wake up
    :param addr2: The second memory location to compare
    :type addr2: void*
    :param cmpValue: The value to compare with
    :type cmpValue: int
    :type count: int
    :return:
    :rtype: int

.. c:function:: void emscripten_sync_run_in_main_thread(em_queued_call *call)

    Runs synchronously the ``call`` in the main thread.

    :param call: the function to call in the main thread
    :type call: em_queued_call

.. c:function:: void* emscripten_sync_in_main_thread_0(int function)

    Runs synchronously the ``function`` in the main thread. The function takes no argument.

    :param function: The function to call in the main thread.
    :type function: int

.. c:function:: void* emscripten_sync_in_main_thread_1(int function, void* arg1)

    Runs synchronously the ``function`` in the main thread. The funciton takes one argument.

    :param function: The function to call in the main thread.
    :type function: int
    :param arg1: The argument for the function.
    :type arg1: void*

.. c:function:: void* emscripten_sync_in_main_thread_2(int function, void* arg1, void* arg2)

    Runs synchronously the ``function`` in the main thread. The funciton takes two arguments.

    :param function: The function to call in the main thread.
    :type function: int
    :param arg1: The argument first for the function.
    :type arg1: void*
    :param arg2: The second argument for the function.
    :type arg2: void*

.. c:function:: void* emscripten_sync_in_main_thread_3(int function, void* arg1, void* arg2, void* arg3)

    Runs synchronously the ``function`` in the main thread. The funciton takes three arguments.

    :param function: The function to call in the main thread.
    :type function: int
    :param arg1: The argument first for the function.
    :type arg1: void*
    :param arg2: The second argument for the function.
    :type arg2: void*
    :param arg3: The third argument for the function.
    :type arg3: void*

.. c:function:: void* emscripten_sync_in_main_thread_7(int function, void* arg1, void* arg2, void* arg3, void* arg4, void* arg5, void* arg6, void* arg7)

    Runs synchronously the ``function`` in the main thread. The funciton takes seven arguments.

    :param function: The function to call in the main thread.
    :type function: int
    :param arg1: The argument first for the function.
    :type arg1: void*
    :param arg2: The second argument for the function.
    :type arg2: void*
    :param arg3: The third argument for the function.
    :type arg3: void*
    :param arg4: The fourth argument for the function.
    :type arg4: void*
    :param arg5: The fifth argument for the function.
    :type arg5: void*
    :param arg6: The sixth argument for the function.
    :type arg6: void*
    :param arg7: The seventh argument for the function.
    :type arg7: void*

.. c:function:: int emscripten_sync_run_in_main_runtime_thread(EM_FUNC_SIGNATURE sig, void* func_ptr, ...)

    Runs the given function synchronously on the main Emscripten runtime thread.
    If this thread is the main thread, the operation is immediately performed, and the result is retourned.
    If the current thread is not the main Emscripten runtime thread (but a pthread), the function will be proxied to be called by the main thread.

    :param sig: The signature of the funcion.
    :type sig: EM_FUNC_SIGNATURE
    :param func_ptr: A pointer to the function to call.
    :type func_ptr: void*
    :param ...: All the arguments to the function.
    :type ...: void*

    .. note::
        - Calling this function requires that the application was compiled with pthreads support enabled (-s USE_PTHREADS=1/2) and that the browser supports SharedArrayByffer specification.

        
.. c:function:: void emscripten_async_run_in_main_runtime_thread(EM_FUNC_SIGNATURE sig, void* func_ptr, ...)

    Runs the given function asynchronously on the main Emscripten runtime thread.
    The call is placed to the command queue of the main Emscripten runtime thread, but is completion is not waited for. As a result if a funciton did have a return value, the return value is not received.

    :param sig: The signature of the funcion.
    :type sig: EM_FUNC_SIGNATURE
    :param func_ptr: A pointer to the function to call.
    :type func_ptr: void*
    :param ...: All the arguments to the function.
    :type ...: void*

    .. note::
        - Calling this function requires that the application was compiled with pthreads support enabled (-s USE_PTHREADS=1/2) and that the browser supports SharedArrayByffer specification.
        - Multiple asunchronous commands from a single pthread/Worker are guaranteed to be executed on the main thread in the program order they where called in.


.. c:function:: em_queued_call* emscripten_async_waitable_run_in_main_runtime_thread(EM_FUN_SIGNATURE sig, void* func_ptr, ...)

    Runs the given function asynchronously on the main Emscripten runtime thread.
    The call is placed to the command queue of the main Emscripten runtime thread, and the result is then later waited upon to receive the return value

    :param sig: The signature of the funcion.
    :type sig: EM_FUNC_SIGNATURE
    :param func_ptr: A pointer to the function to call.
    :type func_ptr: void*
    :param ...: All the arguments to the function.
    :type ...: void*
    :return: An object where the result will be put when the function called will returned.
    :rtype: em_queued_call*

    .. note::
        - Calling this function requires that the application was compiled with pthreads support enabled (-s USE_PTHREADS=1/2) and that the browser supports SharedArrayByffer specification.
        - Multiple asunchronous commands from a single pthread/Worker are guaranteed to be executed on the main thread in the program order they where called in.
        - The object returned by this function call is dynamically allocated, and shoulf be freed up via call to ``emscripten_async_waitable_close()`` after the wait has been performed.


.. c:function:: EMSCRIPTEN_RESULT emscripten_wait_for_call_v(em_queued_call *call, double timeoutMSecs)

    Wait the call is performed or untill timeoutMSec passed.

    :param call: The call to performed.
    :type call: em_queued_call*
    :param timeoutMSecs: The time to wait in milliseconds before it timeout.
    :type timoutMSecs: double
    :return: EMSCRIPTEN_RESULT_SUCCESS if the function succeed.
    :rtype: EMSCRIPTEN_RESULT

.. c:function:: EMSCRIPTEN_RESULT emscripten_wait_for_call_i(em_queued_call *call, double timeoutMSecs, int* outResult)

    Wait the call is performed or untill timeoutMSec passed, and set the funciton result in ``outResult``.

    :param call: The call to performed.
    :type call: em_queued_call*
    :param timeoutMSecs: The time to wait in milliseconds before it timeout.
    :type timoutMSecs: double
    :param outResult: The result of the function.
    :type outResult: int*
    :return: EMSCRIPTEN_RESULT_SUCCESS if the function succeed.
    :rtype: EMSCRIPTEN_RESULT

.. c:function:: void emscripten_async_waitable_close(em_queued_call* call)

    This function freed the object ``call`` that was created by the function ``emscripten_async_waitable_run_in_main_runtime_thread()`` function.

    :param call: The object to freed.
    :type call: em_queued_call*


Threads Attributes
==================


Defines
-------

.. c:macro:: EM_THREAD_STATUS int

    This type is used to know and set the status of a thread. Possible values are listed below.

    .. c:macro:: EM_THREAD_STATUS_NOTSTARTED 0

    .. c:macro:: EM_THREAD_STATUS_RUNNING 1

    .. c:macro:: EM_THREAD_STATUS_SLEEPING 2

        Performing an unconfitional sleep (unsleep, etc.)

    .. c:macro:: EM_THREAD_STATUS_WAITFUTEX 3

        Waiting for an explicit low-level futex (emscripten_futex_wait)

    .. c:macro:: EM_THREAD_STATUS_WAITMUTEX 4

        Waiting for a pthread_mutex_t

    .. c:macro:: EM_THREAD_STATUS_WAIT_PROXY 5

        Waiting for a proxied operation to finish

    .. c:macro:: EM_THREAD_STATUS_FINISHED 6

    .. c:macro:: EM_THREAD_STATUS_NUMFILED 7


Structures
----------

.. c:type:: thread_profiler_block

    .. c:member:: int threadStatus

        One of THREAD_STATUS_*

    .. c:member:: double currentStatusStartTime

        Wallclock time denoting when the curren state was entered in.

    .. c:member:: double timeSpentInStatus[EM_THREAD_STATUS_NUMFILEDS]

        Accumulated duration tims denoting how muche time has been spent in each state, in msecs.

    .. c:member:: char name[32]

        A human-readable name for this thread.


Functions
---------

.. c:function:: int emscripten_is_main_runtime_thread(void)

    Returns 1 if the current thread is the thread that hosts the Emscripten runtime.

    :return: 1 if the current thread is the thread that hosts the Emscripten rutime, 0 otherwise.
    :rtype: int

.. c:function:: int emscripten_is_main_browser_thread(void)

    Return 1 if the current thread is the main browser thread.

    :return: 1 id the current thread is the main browser thread, 0 otherwise.
    :rtype: int

.. c:function:: void emscriptent_main_thread_process_queued_calls(void)

    Call this in the body of all lock-free atomic (cas) loops that the main thread might enter which don't otherwise call to any pthead api calls (mutexes) or C runtime functions that are consifered cancellation points.

    .. note::
        This is a temporary workaround to this `issue <https://github.com/kripken/emscripten/issues/3495>`_.


.. c:function:: int emscripten_syscall(int syscall, void* args)

    Direct syscall access, second argument is a varargs pointer. Used in proxying.

    :param syscall: The number code of the syscall.
    :type syscall: int
    :param args: A pointer to the arguments for the syscall.
    :type args: void*

.. c:function:: void emscripten_set_current_thread_status(EM_THREAD_STATUS newStatus)

    Sets the profiler status of the calling thread. This is a no-op if thread profiling is not active.

    :param newStatus: The new status of the thread.
    :type newStatus: |EM_THREAD_STATUS|

    .. note::
        - This is ian internal function and generaly not intended for user code.
        - When thread profiler is not enables (not building with --threadprofiling), this is a no-op.


.. c:function:: void emscripten_conditionnal_set_current_thread_status(EM_THREAD_STATUS expectedStatus, EM_THREAD_STATUS newStatus)

    Sets the profiler status of the calling thread, but only if it was in the expected status beforehand.

    :param expectedStatus: The expected status od the thread.
    :type expectedStatus: |EM_THREAD_STATUS|
    :param newStatus: The new status of the thread.
    :type newStatus: |EM_THREAD_STATUS|

    .. note::
        - This is ian internal function and generaly not intended for user code.
        - When thread profiler is not enables (not building with --threadprofiling), this is a no-op.


.. c:function:: void emscripten_set_thread_name(pthead_t threadId, const char* name)

    Sets the name of the given thread. Pass ``pthread_self()`` as the thread ID to set the name of the calling thread.
    The name parameter is a UTF-8 encoded string wich is truncated to 32 bytes.

    :param threadId: The ID of the thread to rename.
    :type threadId: pthread_t
    :param name: The new name of the thread.
    :type name: char*

    .. note::
        - When thread profiler is not enables (not building with --threadprofiling), this is a no-op.


.. c:function:: int emscripten_pthread_attr_gettransferredcanvases(const pthread_attr_t* a, const char** str)

    Gets the stored pointer to a string representing the canvases to transfer to the created thread.

    :param a: The attributes of the thread.
    :type a: const pthread_attr_t*
    :param str: A string representating the canvases to transfer to the created thread.
    :type str: const char**

.. c:function:: int emscripten_pthread_attr_settransferredcanvases(pthread_attr_t* a, const char* str)

    Sprecifies a comma-delimited list of canvas DOM element IDs to transfer to the thread to be created.

    :param a: The attributes of the thread to pass the canvas.
    :type a: pthread_attr_t*
    :param str: A comma-list separated of canvas DOM element IDs to tranfer.
    :type str: const char*

    .. note::
        This pointer is weakly stored (not copied) to the given pthread_attr_t, so must be held alive until ``pthread_create()`` has been called. If 0 or "", no canvases are tranferred. The special value "#canvas" denotes the element stored in Module.canvas.


.. COMMENT (not rendered): Section below is automated copy and repalce text.

.. COMMENT (not rendered): The replace function return values with links (not created automatically)

.. |EM_THREAD_STATUS| replace:: :c:type:`EM_THREAD_STATUS`

