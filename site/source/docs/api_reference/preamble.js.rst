.. _preamble-js:

===========
preamble.js
===========

The JavaScript APIs in `preamble.js <https://github.com/kripken/emscripten/blob/master/src/preamble.js>`_ provide programmatic access for interacting with the compiled C code, including: calling compiled C functions, accessing memory, converting pointers to JavaScript ``Strings`` and ``Strings`` to pointers (with different encodings/formats), and other convenience functions.

.. note:: All functions should be called though the :ref:`Module <module>` object (for example: ``Module.functionName``). At optimisation ``-O2`` (and higher) function names are minified by the closure compiler, and calling them directly will fail.


.. contents:: Table of Contents
    :local:
    :depth: 1



Calling compiled C functions from JavaScript
============================================

.. js:function:: ccall(ident, returnType, argTypes, args)

	Call a compiled C function from JavaScript.

	The function executes a compiled C function from JavaScript and returns the result. C++ name mangling means that "normal" C++ functions cannot be called; the function must either be defined in a **.c** file or be a C++ function defined with ``extern "C"``.
	
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
		- If you need a string to live forever, you can create it, for example, using ``_malloc`` and :js:func:`writeStringToMemory`. However, you must later delete it manually!	
		- LLVM optimizations can inline and remove functions, after which you will not be able to call them. Similarly, function names minified by the *Closure Compiler* are inaccessible. In either case, the solution is to add the functions to the ``EXPORTED_FUNCTIONS`` list when you invoke *emcc* :  
		
			::

				-s EXPORTED_FUNCTIONS="['_main', '_myfunc']"
			
			Exported functions can be called as normal: ::
			
				a_result = Module.ccall('myfunc', 'number', ['number'], 10)
		

	:param ident: The name of the C function to be called.	
	:param returnType: The return type of the function. This can be ``"number"``, ``"string"`` or ``"array"``, which correspond to the appropriate JavaScript types (use ``"number"`` for any C pointer, and ``"array"`` for JavaScript arrays and typed arrays; note that arrays are 8-bit), or for a void function it can be ``null`` (note: the JavaScript ``null`` value, not a string containing the word "null").
	.. note:: 64-bit integers become two 32-bit parameters, for the low and high bits (since 64-bit integers cannot be represented in JavaScript numbers).
	:param argTypes: An array of the types of arguments for the function (if there are no arguments, this can be omitted). Types are as in ``returnType``, except that ``array`` is not supported as there is no way for us to know the length of the array).
	:param args: An array of the arguments to the function, as native JavaScript values (as in ``returnType``). Note that string arguments will be stored on the stack (the JavaScript string will become a C string on the stack).
	:returns: The result of the function call as a native JavaScript value (as in ``returnType``). 
	
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
		- ``cwrap`` uses the C stack for temporary values. If you pass a string then it is only "alive" until the call is complete. If the code being called saves the pointer to be used later, it may point to invalid data. 
		- If you need a string to live forever, you can create it, for example, using ``_malloc`` and :js:func:`writeStringToMemory`. However, you must later delete it manually!
		- LLVM optimizations can inline and remove functions, after which you will not be able to "wrap" them. Similarly, function names minified by the *Closure Compiler* are inaccessible. In either case, the solution is to add the functions to the ``EXPORTED_FUNCTIONS`` list when you invoke *emcc* :  
		
			::

				-s EXPORTED_FUNCTIONS="['_main', '_myfunc']"
			
			Exported functions can be called as normal: ::
			
				my_func = Module.cwrap('myfunc', 'number', ['number'])
				my_func(12)

	:param ident: The name of the C function to be called.	
	:param returnType: The return type of the function. This will be one of the JavaScript types ``number``, ``string`` or ``array`` (use ``number`` for any C pointer, and ``array`` for JavaScript arrays and typed arrays; note that arrays are 8-bit).
	:param argTypes: An array of the types of arguments for the function (if there are no arguments, this can be omitted). Types are as in ``returnType``, except that ``array`` is not supported as there is no way for us to know the length of the array).
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
	:param noSafe: Developers should ignore this variable. It is on used in ``SAFE_HEAP`` compilation mode, where it can be avoid infinite recursion in some specialist use cases. 
	:type noSafe: bool
	:returns: The value stored at the specified memory address.




Conversion functions — strings, pointers and arrays
===================================================

.. js:function:: Pointer_stringify(ptr[, length])
	
	Returns a JavaScript String from a pointer, for use in compiled code.
	
	:param ptr: The pointer to be converted to a ``String``.
	:param length: The length of the data in the pointer (optional).
	:returns: A JavaScript ``String`` containing the data from ``ptr``.
	:rtype: String



.. js:function:: UTF16ToString(ptr)

	Given a pointer ``ptr`` to a null-terminated UTF16LE-encoded string in the Emscripten HEAP, returns a copy of that string as a Javascript ``String`` object.

	:param ptr: A pointer to a null-terminated UTF16LE-encoded string in the Emscripten HEAP.
	:returns: A Javascript ``String`` object	
	


.. js:function:: stringToUTF16(str, outPtr)

	Copies the given JavaScript ``String`` object ``str`` to the Emscripten HEAP at address ``outPtr``, null-terminated and encoded in UTF16LE form. 
	
	The copy will require at most ``(str.length*2+1)*2`` bytes of space in the HEAP.

	:param str: A JavaScript ``String`` object.
	:type str: String
	:param outPtr: Pointer to data copied from ``str``, encoded in UTF16LE format and null-terminated.



.. js:function:: UTF32ToString(ptr)

	Given a pointer ``ptr`` to a null-terminated UTF32LE-encoded string in the Emscripten HEAP, returns a copy of that string as a JavaScript ``String`` object.

	:param ptr: A pointer to a null-terminated UTF32LE-encoded string in the Emscripten HEAP.
	:returns: A Javascript ``String`` object.
	

.. js:function:: stringToUTF32(str, outPtr)

	Copies the given JavaScript ``String`` object ``str`` to the Emscripten HEAP at address ``outPtr``, null-terminated and encoded in UTF32LE form. 
	
	The copy will require at most ``(str.length+1)*4`` bytes of space in the HEAP, but can use less, since ``str.length`` does not return the number of characters in the string, but the number of UTF-16 code units in the string.
	
	:param str: A JavaScript ``String`` object.
	:type str: String
	:param outPtr: Pointer to data copied from ``str``, encoded in encoded in UTF32LE format and null-terminated.



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


	
.. js:function:: writeStringToMemory(string, buffer, dontAddNull)

	Writes a JavaScript string to a specified address in the heap. 
	
	.. code-block:: javascript
	
		// Allocate space for string and extra '0' at the end
		var buffer = Module._malloc(myString.length+1);
		
		// Write the string to memory
		Module.writeStringToMemory(myString, buffer);
		
		// We can now send buffer into a C function, it is just a normal char* pointer

	:param string: The string to write into memory.
	:type string: String
	:param buffer: The address (number) where ``string`` is to be written.
	:type buffer: Number
	:param dontAddNull: If ``true``, the new array is not zero-terminated.
	:type dontAddNull: bool	
	


.. js:function:: writeArrayToMemory(array, buffer)

	Writes an array to a specified address in the heap. Note that memory should to be allocated for the array before it is written.

	:param array: The array to write to memory.
	:param buffer: The address (number) where ``array`` is to be written.
	:type buffer: Number

	

.. js:function:: writeAsciiToMemory(str, buffer, dontAddNull)

	Writes an ASCII string to a specified address in the heap. Note that memory should to be allocated for the string before it is written.
	
	The string is assumed to only have characters in the ASCII character set. If ASSERTIONS are enabled and this is not the case, it will fail.
	
	.. code-block:: javascript
	
		// Allocate space for string
		var buffer = Module._malloc(myString.length);
		
		// Write the string to memory
		Module.writeStringToMemory(myString, buffer);

	:param string: The string to write into memory.
	:param buffer: The address where ``string`` is to be written.
	:param dontAddNull: If ``true``, the new string is not zero-terminated.
	:type dontAddNull: bool	



Run dependencies 
=====================================

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
=====================

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

	View for 32-bit unsigned memory. 

	
.. js:data:: HEAPU8

	View for 32-bit unsigned memory.

	
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
		associated contants ALLOC_NORMAL, ALLOC_STACK, ALLOC_STATIC, ALLOC_DYNAMIC, ALLOC_NONE

	function addOnPreRun
	function addOnInit
	function addOnPreMain
	function addOnExit
	function addOnPostRun
	Module['ALLOC_NORMAL'] = ALLOC_NORMAL;
	Module['ALLOC_STACK'] = ALLOC_STACK;
	Module['ALLOC_STATIC'] = ALLOC_STATIC;
	Module['ALLOC_DYNAMIC'] = ALLOC_DYNAMIC;
	Module['ALLOC_NONE'] = ALLOC_NONE;
	Module['HEAP'] = HEAP;
	Module['IHEAP'] = IHEAP;
	Module['FHEAP'] = FHEAP;
	function alignMemoryPage(x)
	function enlargeMemory()
	function demangle(func)
	function demangleAll(text)
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
	function CHECK_ALIGN_8(addr)
	function CHECK_ALIGN_4(addr)
	function CHECK_ALIGN_2(addr)
	function CHECK_OVERFLOW(value, bits, ignore, sig)
	Module['PGOMonitor'] = PGOMonitor; — a bit confusing
	Module["preloadedImages"]
	Module["preloadedAudios"]


.. PRIVATE NOTES (not rendered) :
	- In theory JavaScript exceptions could be thrown for very bad input (e.g., provide an integer instead of a string, and the function does .length on it). These are implied everywhere and not documented.
	- noSafe parameter: It solves technical problem of infinite recursion in some cases where SAFE_HEAP is being used. Not really interesting for users. 


	



