.. _portability-function-pointer-issues:

=======================
Function Pointer Issues
=======================

There are two main issues with function pointers:


#.
  Function pointer casts can cause function pointer calls to fail.

  Function pointers must be called with the correct type: it is undefined behavior in C and C++ to cast a function pointer to another type and call it that way. This does work in most native platforms, however, despite it being UB, but in asm.js and in wasm it can fail. In that case, you may see an ``abort(10)`` or some other number, and if assertions are on you may see a message with details that start with

  ::

    Invalid function pointer called

  Rarely, you may see a compiler warning like this:

  ::

    warning: implicit declaration of function

  This may be related to a function pointer cast problem as implicit declarations may have a different type than how you call them. However, in general the compiler cannot warn about this, and you will only see a problem at runtime.

#.
  Older versions of :term:`clang` can generate different code for C and C++ calls when a structure is passed **by value** (for completeness, one convention is ``struct byval`` and the other is ``field a, field b``). The two formats are incompatible with each other, and you may get a warning.

  The workaround is to pass the structure by reference, or simply not mix C and C++ in that location (for example, rename the **.c** file to **.cpp**).

  .. _function-pointer-issues-point-asmjs:

.. _Asm-pointer-casts:

Debugging function pointer issues
=================================

The ``SAFE_HEAP`` and ``ASSERTION`` options can catch some of these errors at runtime and provide useful information. You can also see if ``EMULATE_FUNCTION_POINTER_CASTS`` fixes things for you, but see later down about the overhead.

Working around function pointer issues
======================================

There are three solutions to this problem (the second is preferred):

  - Cast the function pointer back to the correct type before it is called. This is problematic because it requires that the caller knows the original type.
  - Manually write an adapter function that does not need to be cast, and calls the original function. For example, it might ignore a parameter, and in that way bridge between the different function pointer types.
  - Use ``EMULATE_FUNCTION_POINTER_CASTS``. When you build with ``-s EMULATE_FUNCTION_POINTER_CASTS=1``, Emscripten emits code to emulate function pointer casts at runtime, adding extra arguments/dropping them/changing their type/adding or dropping a return type/etc. This can add significant runtime overhead, so it is not recommended, but is be worth trying.

For a real-world example, consider the code below:

.. code:: cpp

  #include <stdio.h>

  typedef void(*voidReturnType)(const char *);

  void voidReturn(const char *message) {
    printf( "voidReturn: %s\n", message );
  }


  int intReturn(const char *message) {
    printf( "intReturn: %s\n", message );
    return 1;
  }

  void voidReturnNoParam() {
    printf( "voidReturnNoParam:\n" );
  }

  void callFunctions(const voidReturnType * funcs, size_t size) {
    size_t current = 0;
    while (current < size) {
      funcs[current]("hello world");
      current++;
    }
  }

  int main() {
    voidReturnType functionList[3];

    functionList[0] = voidReturn;
    functionList[1] = (voidReturnType)intReturn;         // Breaks in Emscripten.
    functionList[2] = (voidReturnType)voidReturnNoParam; // Breaks in Emscripten.

    callFunctions(functionList, 3);
  }

The code defines three functions with different signatures: ``voidReturn`` of type ``vi`` (``void (int)``), ``intReturn`` of type ``ii``, and ``voidReturnNoParam`` of type ``v``. These function pointers are cast to type ``vi`` and added to a list. The functions are then called using the function pointers in the list.

The code runs (and works) when compiled to native machine code (on all major platforms). You can try it by saving the code as **main.c** and executing **cc main.c** and then **./a.out**. You'll see this output:

::

  voidReturn: hello world
  intReturn: hello world
  voidReturnNoParam:

However, the code fails with a runtime exception in Emscripten, and displays the console output:

::

  voidReturn: hello world
  Invalid function pointer called with signature 'vi'. Perhaps this is an invalid value (e.g. caused by calling a virtual method on a NULL pointer)? Or calling a function with an incorrect type, which will fail? (it is worth building your source files with -Werror (warnings are errors), as warnings can indicate undefined behavior which can cause this)
  Build with ASSERTIONS=2 for more info.

.. note:: You can try this yourself. Save the code as **main.c**, compile using ``emcc -O0 main.c -o main.html``, and then load **main.html** into a browser.

The code fragment below shows how we can cast the function pointer back to its original signature just before calling it, so that it is found in the correct table. This requires the receiver of the table to have special knowledge about what is in the list (you can see this in the special case for index ``1`` in the while loop). Additionally, :ref:`emcc <emccdoc>` will continue to complain about the original cast taking place in ``main()`` when adding the function to ``functionList[1]``.


.. code:: cpp

    void callFunctions(const voidReturnType * funcs, size_t size) {
      size_t current = 0;
      while (current < size) {
        if ( current == 1 ) {
          ((intReturnType)funcs[current])("hello world"); // Special-case cast
        } else {
          funcs[current]("hello world");
        }
        current++;
      }
    }

The code fragment below shows how to make and use an adapter function that calls the original function. The adapter is defined with the same signature as it will have when called, and is hence available in the expected function-pointer table.

.. code:: cpp

  void voidReturnNoParamAdapter(const char *message) {
    voidReturnNoParam();
  }

  int main() {
    voidReturnType functionList[3];

    functionList[0] = voidReturn;
    functionList[1] = (voidReturnType)intReturn; // Fixed in callFunctions
    functionList[2] = voidReturnNoParamAdapter; // Fixed by Adapter

    callFunctions(functionList, 3);
  }
