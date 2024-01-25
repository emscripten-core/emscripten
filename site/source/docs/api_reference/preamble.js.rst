.. _preamble-js:

===========
preamble.js
===========

The JavaScript APIs in `preamble.js <https://github.com/emscripten-core/emscripten/blob/main/src/preamble.js>`_ provide programmatic access for interacting with the compiled C code, including: calling compiled C functions, accessing memory, converting pointers to JavaScript ``Strings`` and ``Strings`` to pointers (with different encodings/formats), and other convenience functions.

We call this "``preamble.js``" because Emscripten's output JS, at a high level, contains the preamble (from ``src/preamble.js``), then the compiled code, then the postamble. (In slightly more detail, the preamble contains utility functions and setup, while the postamble connects things and handles running the application.)

The preamble code is included in the output JS, which is then optimized all together by the compiler, together with any ``--pre-js`` and ``--post-js`` files you added and code from any JavaScript libraries (``--js-library``). That means that you can call methods from the preamble directly, and the compiler will see that you need them, and not remove them as being unused.

If you want to call preamble methods from somewhere the compiler can't see, like another script tag on the HTML, you need to **export** them. To do so, add them to ``EXPORTED_RUNTIME_METHODS`` (for example, ``-sEXPORTED_RUNTIME_METHODS=ccall,cwrap`` will export ``ccall`` and ``cwrap``). Once exported, you can access them on the ``Module`` object (as ``Module.ccall``, for example).

.. note:: If you try to use ``Module.ccall`` or another runtime method without exporting it, you will get an error. In a build with ``-sASSERTIONS``, the compiler emits code to show you a useful error message, which will explain that you need to export it. In general, if you see something odd, it's useful to build with assertions.


.. contents:: Table of Contents
    :local:
    :depth: 1



Calling compiled C functions from JavaScript
============================================

.. js:function:: ccall(ident, returnType, argTypes, args, opts)

  Call a compiled C function from JavaScript.

  The function executes a compiled C function from JavaScript and returns the result. C++ name mangling means that "normal" C++ functions cannot be called; the function must either be defined in a **.c** file or be a C++ function defined with ``extern "C"``.

  ``returnType`` and ``argTypes`` let you specify the types of parameters and the return value. The possible types are ``"number"``, ``"string"``, ``"array"``, or ``"boolean"``, which correspond to the appropriate JavaScript types. Use ``"number"`` for any numeric type or C pointer, ``string`` for C ``char*`` that represent strings, ``"boolean"`` for a boolean type, ``"array"`` for JavaScript arrays and typed arrays, containing 8-bit integer data - that is, the data is written into a C array of 8-bit integers; and in particular if you provide a typed array here, it must be a Uint8Array or Int8Array. If you want to receive an array of another type of data, you can manually allocate memory and write to it, then provide a pointer here (as a ``"number"``, as pointers are just numbers).

  .. code-block:: javascript

    // Call C from JavaScript
    var result = Module.ccall('c_add', // name of C function
      'number', // return type
      ['number', 'number'], // argument types
      [10, 20]); // arguments

    // result is 30

  .. COMMENT (not rendered): There is more complete documentation in the guide: **HamishW** — add link to guide when it exists (currently in wiki at "Interacting with code").

  .. note::
    - ``ccall`` uses the C stack for temporary values. If you pass a string then it is only "alive" until the call is complete. If the code being called saves the pointer to be used later, it may point to invalid data.
    - If you need a string to live forever, you can create it, for example, using ``_malloc`` and :js:func:`stringToUTF8`. However, you must later delete it manually!
    - LLVM optimizations can inline and remove functions, after which you will not be able to call them. Similarly, function names minified by the *Closure Compiler* are inaccessible. In either case, the solution is to add the functions to the ``EXPORTED_FUNCTIONS`` list when you invoke *emcc*:

      .. code-block:: none

        -sEXPORTED_FUNCTIONS=_main,_myfunc"

      (Note that we also export ``main`` - if we didn't, the compiler would assume we don't need it.) Exported functions can then be called as normal:

      .. code-block:: javascript

        a_result = Module.ccall('myfunc', 'number', ['number'], [10])


  :param ident: The name of the C function to be called.
  :param returnType: The return type of the function. Note that ``array`` is not supported as there is no way for us to know the length of the array. For a void function this can be ``null`` (note: the JavaScript ``null`` value, not a string containing the word "null").

  .. note:: 64-bit integers become two 32-bit parameters, for the low and high bits (since 64-bit integers cannot be represented in JavaScript numbers).

  :param argTypes: An array of the types of arguments for the function (if there are no arguments, this can be omitted).
  :param args: An array of the arguments to the function, as native JavaScript values (as in ``returnType``). Note that string arguments will be stored on the stack (the JavaScript string will become a C string on the stack).
  :returns: The result of the function call as a native JavaScript value (as in ``returnType``) or, if the ``async`` option is set, a JavaScript Promise of the result.
  :opts: An optional options object. It can contain the following properties:

      - ``async``: If ``true``, implies that the ccall will perform an async operation. This assumes you are build with asyncify support.

  .. note:: Async calls currently don't support promise error handling.

  .. COMMENT (not rendered): The ccall/cwrap functions only work for C++ functions that use "extern C". In theory ordinary C++ names can be unmangled, but it would require tool to ship a fairly large amount of code just for this purpose.


.. js:function:: cwrap(ident, returnType, argTypes)

  Returns a native JavaScript wrapper for a C function.

  This is similar to :js:func:`ccall`, but returns a JavaScript function that can be reused as many time as needed. The C function can be defined in a C file, or be a C-compatible C++ function defined using ``extern "C"`` (to prevent name mangling).


  .. code-block:: javascript

    // Call C from JavaScript
    var c_javascript_add = Module.cwrap('c_add', // name of C function
      'number', // return type
      ['number', 'number']); // argument types

    // Call c_javascript_add normally
    console.log(c_javascript_add(10, 20)); // 30
    console.log(c_javascript_add(20, 30)); // 50

  .. COMMENT (not rendered): There is more complete documentation in the guide: **HamishW** — add link to guide when it exists (currently in wiki at "Interacting with code").

  .. note::
    - ``cwrap`` uses the C stack for temporary values. If you pass a string then it is only "alive" until the call is complete. If the code being called saves the pointer to be used later, it may point to invalid data. If you need a string to live forever, you can create it, for example, using ``_malloc`` and :js:func:`stringToUTF8`. However, you must later delete it manually!
    - To wrap a function it must be exported by adding it to the ``EXPORTED_FUNCTIONS`` list when you invoke *emcc*. If a function is not exported, optimizations may remove it, and ``cwrap`` will not be able to find it at runtime. (In builds with ``ASSERTIONS`` enabled, ``cwrap`` will show an error in such a situation; in release builds without assertions, trying to wrap a non-existent function will error, either by returning `undefined` or by returning a function that will error when actually called, depending on how ``cwrap`` optimizes.)
    - ``cwrap`` does not actually call compiled code (only calling the wrapper it returns does that). That means that it is safe to call ``cwrap`` early, before the runtime is fully initialized (but calling the returned wrapped function must wait for the runtime, of course, like calling compiled code in general).

      .. code-block:: none

        -sEXPORTED_FUNCTIONS=_main,_myfunc

      Exported functions can be called as normal:

      .. code-block:: javascript

        my_func = Module.cwrap('myfunc', 'number', ['number'])
        my_func(12)

  :param ident: The name of the C function to be called.
  :param returnType: The return type of the function. This can be ``"number"``, ``"string"`` or ``"array"``, which correspond to the appropriate JavaScript types (use ``"number"`` for any C pointer, and ``"array"`` for JavaScript arrays and typed arrays; note that arrays are 8-bit), or for a void function it can be ``null`` (note: the JavaScript ``null`` value, not a string containing the word "null").
  :param argTypes: An array of the types of arguments for the function (if there are no arguments, this can be omitted). Types are as in ``returnType``, except that ``array`` is not supported as there is no way for us to know the length of the array).
  :param opts: An optional options object, see :js:func:`ccall`.
  :returns: A JavaScript function that can be used for running the C function.


Accessing memory
================


.. js:function:: setValue(ptr, value, type[, noSafe])

  Sets a value at a specific memory address at run-time.

  .. note::
    - :js:func:`setValue` and :js:func:`getValue` only do *aligned* writes and reads.
    - The ``type`` is an LLVM IR type (one of ``i8``, ``i16``, ``i32``, ``i64``, ``float``, ``double``, or a pointer type like ``i8*`` or just ``*``), not JavaScript types as used in :js:func:`ccall` or :js:func:`cwrap`. This is a lower-level operation, and we do need to care what specific type is being used.

  :param ptr: A pointer (number) representing the memory address.
  :param value: The value to be stored
  :param type: An LLVM IR type as a string (see "note" above).
  :param noSafe: Developers should ignore this variable. It is only used in ``SAFE_HEAP`` compilation mode, where it can help avoid infinite recursion in some specialist use cases.
  :type noSafe: bool


.. js:function:: getValue(ptr, type[, noSafe])

  Gets a value at a specific memory address at run-time.

  .. note::
    - :js:func:`setValue` and :js:func:`getValue` only do *aligned* writes and reads!
    - The ``type`` is an LLVM IR type (one of ``i8``, ``i16``, ``i32``, ``i64``, ``float``, ``double``, or a pointer type like ``i8*`` or just ``*``), not JavaScript types as used in :js:func:`ccall` or :js:func:`cwrap`. This is a lower-level operation, and we do need to care what specific type is being used.

  :param ptr: A pointer (number) representing the memory address.
  :param type: An LLVM IR type as a string (see "note" above).
  :param noSafe: Developers should ignore this variable. It is only used in ``SAFE_HEAP`` compilation mode, where it can help avoid infinite recursion in some specialist use cases.
  :type noSafe: bool
  :returns: The value stored at the specified memory address.




Conversion functions — strings, pointers and arrays
===================================================

.. js:function:: UTF8ToString(ptr[, maxBytesToRead])

  Given a pointer ``ptr`` to a null-terminated UTF8-encoded string in the Emscripten HEAP, returns a copy of that string as a JavaScript ``String`` object.

  :param ptr: A pointer to a null-terminated UTF8-encoded string in the Emscripten HEAP.
  :param maxBytesToRead: An optional length that specifies the maximum number of bytes to read. You can omit this parameter to scan the string until the first \0 byte. If maxBytesToRead is passed, and the string at ``[ptr, ptr+maxBytesToReadr)`` contains a null byte in the middle, then the string will cut short at that byte index (i.e. maxBytesToRead will not produce a string of exact length ``[ptr, ptr+maxBytesToRead)``) N.B. mixing frequent uses of ``UTF8ToString()`` with and without maxBytesToRead may throw JS JIT optimizations off, so it is worth to consider consistently using one style or the other.
  :returns: A JavaScript ``String`` object


.. js:function:: stringToUTF8(str, outPtr, maxBytesToWrite)

  Copies the given JavaScript ``String`` object ``str`` to the Emscripten HEAP at address ``outPtr``, null-terminated and encoded in UTF8 form.

  The copy will require at most ``str.length*4+1`` bytes of space in the HEAP. You can use the function ``lengthBytesUTF8()`` to compute the exact amount of bytes (excluding the null terminator) needed to encode the string.

  :param str: A JavaScript ``String`` object.
  :type str: String
  :param outPtr: Pointer to data copied from ``str``, encoded in UTF8 format and null-terminated.
  :param maxBytesToWrite: A limit on the number of bytes that this function can at most write out. If the string is longer than this, the output is truncated. The outputted string will always be null terminated, even if truncation occurred, as long as ``maxBytesToWrite > 0``.


.. js:function:: UTF16ToString(ptr)

  Given a pointer ``ptr`` to a null-terminated UTF16LE-encoded string in the Emscripten HEAP, returns a copy of that string as a JavaScript ``String`` object.

  :param ptr: A pointer to a null-terminated UTF16LE-encoded string in the Emscripten HEAP.
  :returns: A JavaScript ``String`` object



.. js:function:: stringToUTF16(str, outPtr, maxBytesToWrite)

  Copies the given JavaScript ``String`` object ``str`` to the Emscripten HEAP at address ``outPtr``, null-terminated and encoded in UTF16LE form.

  The copy will require exactly ``(str.length+1)*2`` bytes of space in the HEAP.

  :param str: A JavaScript ``String`` object.
  :type str: String
  :param outPtr: Pointer to data copied from ``str``, encoded in UTF16LE format and null-terminated.
  :param maxBytesToWrite: A limit on the number of bytes that this function can at most write out. If the string is longer than this, the output is truncated. The outputted string will always be null terminated, even if truncation occurred, as long as ``maxBytesToWrite >= 2`` so that there is space for the null terminator.



.. js:function:: UTF32ToString(ptr)

  Given a pointer ``ptr`` to a null-terminated UTF32LE-encoded string in the Emscripten HEAP, returns a copy of that string as a JavaScript ``String`` object.

  :param ptr: A pointer to a null-terminated UTF32LE-encoded string in the Emscripten HEAP.
  :returns: A JavaScript ``String`` object.


.. js:function:: stringToUTF32(str, outPtr, maxBytesToWrite)

  Copies the given JavaScript ``String`` object ``str`` to the Emscripten HEAP at address ``outPtr``, null-terminated and encoded in UTF32LE form.

  The copy will require at most ``(str.length+1)*4`` bytes of space in the HEAP, but can use less, since ``str.length`` does not return the number of characters in the string, but the number of UTF-16 code units in the string. You can use the function ``lengthBytesUTF32()`` to compute the exact amount of bytes (excluding the null terminator) needed to encode the string.

  :param str: A JavaScript ``String`` object.
  :type str: String
  :param outPtr: Pointer to data copied from ``str``, encoded in encoded in UTF32LE format and null-terminated.
  :param maxBytesToWrite: A limit on the number of bytes that this function can at most write out. If the string is longer than this, the output is truncated. The outputted string will always be null terminated, even if truncation occurred, as long as ``maxBytesToWrite >= 4`` so that there is space for the null terminator.



.. js:function:: AsciiToString(ptr)

  Converts an ASCII or Latin-1 encoded string to a JavaScript String object.

  :param ptr: The pointer to be converted to a ``String``.
  :returns: A JavaScript ``String`` containing the data from ``ptr``.
  :rtype: String


.. js:function:: intArrayFromString(stringy, dontAddNull[, length])

  This converts a JavaScript string into a C-line array of numbers, 0-terminated.

  :param stringy: The string to be converted.
  :type stringy: String
  :param dontAddNull: If ``true``, the new array is not zero-terminated.
  :type dontAddNull: bool
  :param length: The length of the array (optional).
  :returns: The array created from ``stringy``.


.. js:function:: intArrayToString(array)

  This creates a JavaScript string from a zero-terminated C-line array of numbers.

  :param array: The array to convert.
  :returns: A ``String``, containing the content of ``array``.


.. js:function:: writeArrayToMemory(array, buffer)

  Writes an array to a specified address in the heap. Note that memory should to be allocated for the array before it is written.

  :param array: The array to write to memory.
  :param buffer: The address (number) where ``array`` is to be written.
  :type buffer: Number



Run dependencies
================

Note that generally run dependencies are managed by the file packager and other parts of the system. It is rare for developers to use this API directly.


.. js:function:: addRunDependency(id)

  Adds an ``id`` to the list of run dependencies.

  This adds a run dependency and increments the run dependency counter.

  .. COMMENT (not rendered): **HamishW** Remember to link to Execution lifecycle in Browser environment or otherwise link to information on using this. Possibly its own topic.

  :param id: An arbitrary id representing the operation.
  :type id: String



.. js:function:: removeRunDependency(id)

  Removes a specified ``id`` from the list of run dependencies.

  :param id: The identifier for the specific dependency to be removed (added with :js:func:`addRunDependency`)
  :type id: String



Stack trace
===========

.. js:function:: stackTrace()

  Returns the current stack track.

    .. note:: The stack trace is not available at least on IE10 and Safari 6.

  :returns: The current stack trace, if available.




Type accessors for the memory model
===================================

The :ref:`emscripten-memory-model` uses a typed array buffer (``ArrayBuffer``) to represent memory, with different views into it giving access to the different types. The views for accessing different types of memory are listed below.


.. js:data:: HEAP8

  View for 8-bit signed memory.


.. js:data:: HEAP16

  View for 16-bit signed memory.


.. js:data:: HEAP32

  View for 32-bit signed memory.


.. js:data:: HEAPU8

  View for 8-bit unsigned memory.


.. js:data:: HEAPU16

  View for 16-bit unsigned memory.


.. js:data:: HEAPU32

  View for 32-bit unsigned memory.


.. js:data:: HEAPF32

  View for 32-bit float memory.


.. js:data:: HEAPF64

  View for 64-bit float memory.






.. COMMENT (not rendered) : The following methods are explicitly not part of the public API and not documented. Note that in some case referred to by function name, other cases by Module assignment.

  function allocate(slab, types, allocator, ptr) — Internal and use is discouraged. Documentation can remain in source code but not here.
    associated constants ALLOC_NORMAL, ALLOC_STACK

  function addOnPreRun
  function addOnInit
  function addOnPreMain
  function addOnExit
  function addOnPostRun
  Module['ALLOC_NORMAL'] = ALLOC_NORMAL;
  Module['ALLOC_STACK'] = ALLOC_STACK;
  Module['HEAP'] = HEAP;
  Module['IHEAP'] = IHEAP;
  function demangle(func)
  function parseJSFunc(jsfunc)
  function callRuntimeCallbacks(callbacks)
  function preRun()
  function ensureInitRuntime()
  function preMain()
  function exitRuntime()
  function postRun()
  function getCFunc(ident)
  function jsCall()
  function SAFE_HEAP_CLEAR(dest)
  function SAFE_HEAP_ACCESS(dest, type, store, ignore, storeValue)
  function SAFE_HEAP_STORE(dest, value, type, ignore)
  function SAFE_HEAP_LOAD(dest, type, unsigned, ignore)
  function SAFE_HEAP_COPY_HISTORY(dest, src)
  function SAFE_HEAP_FILL_HISTORY(from, to, type)
  function getSafeHeapType(bytes, isFloat)
  function SAFE_HEAP_STORE(dest, value, bytes, isFloat)
  function SAFE_HEAP_LOAD(dest, bytes, isFloat, unsigned)
  function SAFE_FT_MASK(value, mask)
  function CHECK_OVERFLOW(value, bits, ignore, sig)
  Module["preloadedImages"]
  Module["preloadedAudios"]


.. PRIVATE NOTES (not rendered) :
  - In theory JavaScript exceptions could be thrown for very bad input (e.g., provide an integer instead of a string, and the function does .length on it). These are implied everywhere and not documented.
  - noSafe parameter: It solves technical problem of infinite recursion in some cases where SAFE_HEAP is being used. Not really interesting for users.
