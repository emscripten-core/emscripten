.. _portability-function-pointer-issues:

=======================
Function Pointer Issues
=======================

There are three general issues with function pointers:


#. 
  :ref:`Function pointer casts <Asm-pointer-casts>` can cause function pointer calls to fail.

  Function pointers are stored in a specific table based on their signature when they are *declared*. When a function is called the code searches for it in the table associated with its *current* function pointer signature. If the function pointer used to call the function does not have the same signature as the original type, the calling code will look for it in the wrong table. 
  
  For example, consider a function declared as ``int (int)`` (return ``int``, receive ``int``) and hence added to the table ``FUNCTION_TABLE_ii``. If you cast a function pointer to it to ``void (int)`` (no return, receive ``int``), then the code will look for the function in ``FUNCTION_TABLE_vi``.
  
  You may see compilation warnings for this error: 

  :: 
  
    warning: implicit declaration of function

  The recommended workaround is to refactor code to avoid this type of situation, as described in :ref:`Asm-pointer-casts` below. 


#. 

  When using optimisation :ref:`-O2 <emcc-O2>` and above, comparing function pointers of different types can give false positives, and bugs with incorrect function pointers are potentially more misleading. To check if this is the cause of problems with your code, you can compile with `ALIASING_FUNCTION_POINTERS <https://github.com/kripken/emscripten/blob/1.29.12/src/settings.js#L213>`_ unset (``-s ALIASING_FUNCTION_POINTERS=0``).

  .. note:: In **asm.js**, function pointers are stored within a function-type specific table (as in the ``FUNCTION_TABLE_ii`` example from before).
  
    At lower levels of optimisation each function pointer has a unique index value across all the function-type tables (a function pointer will exist at a specific index in one table only, and there will be an empty slot at that index in all the other tables). As a result, comparing function pointers (indexes) gives an accurate result, and attempting to call a function pointer in the wrong table will throw an error as that index will be empty.
    
    At optimisation ``-O2`` and above, the tables are optimised so that all the function pointers are in sequential indexes. This is a useful optimisation because the tables are much more compact without all the empty slots, but it does mean that the  function index is no longer "globally" unique. Each function is now uniquely indexed using both its table and its index within that table. 
    
    As a result, at higher optimisations:
    
      - Comparisons of the function pointers can give a false positive, because functions of different types can have the same index (albeit in different tables). 
      - Mistakes in function pointer code can be more difficult to debug, because they result in the wrong code being called rather than an explicit error (as is raised in the case of a "hole" in the table). 


#. 
  Older versions of :term:`clang` can generate different code for C and C++ calls when a structure is passed **by value** (for completeness, one convention is ``struct byval`` and the other is ``field a, field b``). The two formats are incompatible with each other, and you may get a warning.
  
  The workaround is to pass the structure by reference, or simply not mix C and C++ in that location (for example, rename the **.c** file to **.cpp**). 

  .. _function-pointer-issues-point-asmjs:

.. _Asm-pointer-casts:

Asm pointer casts
=================

As mentioned :ref:`above <function-pointer-issues-point-asmjs>`, in **asm.js** mode function pointers must be called using their correct type or the call will fail. This is because each function pointer is stored in a specific table based on its signature when it is declared: casting the pointer to another type will cause calling code to look for the function pointer in the wrong place.

.. note:: Having a separate table for each type of function pointer allows the JavaScript engine to know the exact type of each function pointer call, and optimize those calls much better than would otherwise be possible.

There are three solutions to this problem (the second is preferred):

  - Cast the function pointer back to the correct type before it is called. This is problematic because it requires that the caller knows the original type.
  - Make an adapter function that does not need to be cast, and will hence be found in the correct function-pointer table. From the adapter function call the original function.
  - Use ``EMULATE_FUNCTION_POINTER_CASTS``. When you build with ``-s EMULATE_FUNCTION_POINTER_CASTS=1``, Emscripten emits code to emulate function pointer casts at runtime, adding extra arguments/dropping them/changing their type/adding or dropping a return type/etc. This can add significant runtime overhead, so it is not recommended, but might be worth trying.

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

The code runs (and works) when compiled to machine code. You can try it by saving the code as **main.c** and executing **cc main.c** and then **./a.out**. You'll see this output:

::

  voidReturn: hello world
  intReturn: hello world
  voidReturnNoParam:

However, the code fails with a runtime exception in Emscripten, and displays the console output:

::

  voidReturn: hello world

.. note:: You can try this yourself. Save the code as **main.c**, compile using ``emcc -O0 main.c -o main.html``, and then load **main.html** into a browser.

To see why this breaks, look at the html source. You should be able to find the section below in the code, showing the methods are stored in function tables based on their *original* function signatures.

.. code:: javascript

  var FUNCTION_TABLE_vi = [b0,b0,b0,b0,b0,b0,_voidReturn,b0];
  var FUNCTION_TABLE_ii = [b1,b1,_intReturn,b1,b1,b1,b1,b1];
  var FUNCTION_TABLE_iii = [b2,b2,b2,b2,b2,b2,b2,b2];
  var FUNCTION_TABLE_v = [b3,b3,b3,b3,_voidReturnNoParam,b3,b3,b3];

When Emscripten-generated code executes a function, it looks up the function in the table based on its signature. In our ``callFunctions()`` method we are passed a list of functions with the signature ``vi``, so Emscripten looks for all the functions in the table ``FUNCTION_TABLE_vi``. Only the function ``voidReturn`` is found in ``FUNCTION_TABLE_vi``. The other two functions are not found, causing the exception.

As stated above, there are two solutions to this problem.

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
