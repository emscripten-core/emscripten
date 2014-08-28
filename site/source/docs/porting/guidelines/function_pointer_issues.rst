=================================================
Function Pointer Issues (under-construction)
=================================================

There are three general issues with function pointers:

#. 
	:term:`Clang` generates different code for C and C++ calls when a structure is passed **by value** (for completeness, one convention is ``struct byval`` and the other is ``field a, field b``). The two formats are incompatible with each other, and you should get a warning during compilation:

	:: 
	
		Warning: Casting potentially incompatible function pointer
	
	The workaround is to pass the structure by reference, or simply not mix C and C++ in that location (for example, rename the **.c** file to **.cpp**). 

	.. _function-pointer-issues-point-asmjs:
	
#. 
	:ref:`Function pointer casts <Asm-pointer-casts>` can cause function pointer calls to fail.

	Each function type in **asm.js** has its own table — if you change the type of the pointer the calling code will look for the function pointer in the wrong table. For example, consider a function that is say ``int (int)`` (return ``int``, receive ``int``) and that will be indexed in the table ``FUNCTION_TABLE_ii``. If you cast a function pointer to it to ``void (int)`` (no return, receive ``int``), then the code will look for the function in ``FUNCTION_TABLE_vi``.
	
	You should see compilation warnings for this error: 

	:: 
	
		warning: implicit declaration of function

	The recommended workaround is to refactor code to avoid this type of situation, as described in :ref:`Asm-pointer-casts` below. 


#. 

	When using optimisation :ref:`-O2 <emcc-O2>` and above, comparing function pointers of different types can give false positives, and bugs with incorrect function pointers potentially more misleading. To check if this is causing issues, you can compile with `ALIASING_FUNCTION_POINTERS <https://github.com/kripken/emscripten/blob/master/src/settings.js#L201>`_ unset (``-s ALIASING_FUNCTION_POINTERS=0``).

	.. note:: In **asm.js** function pointers are stored within a function-type specific table.
	
		At lower levels of optimisation each function pointer has a unique index value across all the function-type tables (a function pointer will exist at a specific index in one table only, and there will be an empty slot at that index in all the other tables). As a result, comparing function pointers (indexes) gives an accurate result, and attempting to call a function pointer in the wrong table will throw an error as that index will be empty.
		
		A optimisation ``O2`` and above, the tables are optimised so that all the function pointers are in sequential indexes. This is a useful optimisation because the tables are much more compact without all the empty slots, but it does mean that the  function index is no longer "globally" unique. Function is now uniquely indexed using both its table and its index within that table. As a result:
		
			- Comparisons of the function pointers can give a false positive, because functions of different types can share the same index. 
			- Mistakes in function pointer code can be more difficult to debug, because they result in the wrong code being called rather than an explicit error (as in the case of a "hole" in the table). 


.. _Asm-pointer-casts:

Asm pointer casts
=================

As mentioned :ref:`above <function-pointer-issues-point-asmjs>`, in **asm.js** mode function pointers must be called using their correct type. This is because each function pointer type has its own table: casting the pointer to another type will cause code to look for the function pointer in the wrong table (and so the call will fail).

.. note:: Having a separate table for each type of function pointer allows the JavaScript engine to know the exact type of each function pointer call, and optimize those calls much better than normally.

Let's look at an example:

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

This code runs (and works) when compiled to machine code. You can try it by saving the code as **main.c** and executing: **cc main.c** and then **./a.out**. You'll see this output:

::

	voidReturn: hello world
	intReturn: hello world
	voidReturnNoParam:

However, this code will break in Emscripten. You can compile this on your own by saving this code as ``main.c`` and executing: ``emcc -O2 -g -s ASM_JS=1  main.c -o main.html``. Then load it into your browser. 

To see why this breaks, let's look at the html source. You should be able to find this section:

.. code:: javascript

	var FUNCTION_TABLE_vi = [b0,b0,b0,b0,b0,b0,_voidReturn,b0];
	var FUNCTION_TABLE_ii = [b1,b1,_intReturn,b1,b1,b1,b1,b1];
	var FUNCTION_TABLE_iii = [b2,b2,b2,b2,b2,b2,b2,b2];
	var FUNCTION_TABLE_v = [b3,b3,b3,b3,_voidReturnNoParam,b3,b3,b3];

.. note:: Because we are compiling using :ref:`-g <emcc-g>` the function names aren't mangled and we can see there are different function tables categorized by the signature of the function.

When Emscripten-generated code executes a function, it will look up the function in the table based on its signature and execute it. In our ``callFunctions()`` method we are passed a list of functions of the signature ``vi`` and for that reason, ``FUNCTION_TABLE_vi`` is the table used to find them. It doesn't matter that in ``main()`` we've added compatible functions to ``functionList``. They will not be found because their signature is different.

There are two ways to fix this. The first is to cast the function pointer back to its original signature just before calling it:

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

This requires the receiver of the table to have special knowledge about what is in the list (you can see this in the special case for index ``1`` in the while loop). Additionally, :ref:`emcc <emccdoc>` will continue to complain about the original cast taking place in ``main()`` when adding the function to ``functionList[1]``.

A second (and better) solution is to make an adapter function which calls the original function and to place the *adapter* in the list.

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

Here, we've made a function ``voidReturnNoParamAdapter()`` and added that to ``functionList[2]``.
