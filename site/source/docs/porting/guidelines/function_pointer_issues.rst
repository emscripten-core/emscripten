=================================================
Function Pointer Issues (under-construction)
=================================================

There are three general issues with function pointers:

#. :term:`Clang` generates different code for C and C++ calls when a structure is passed **by value**: one convention is ``struct byval`` and the other is ``field a, field b``. The two formats are incompatible with each other, and you should see a warning during compilation about this. The workaround is to pass the structure by reference, or simply not mix C and C++ in that location (for example, rename the **.c** file to **.cpp**). 

	.. _function-pointer-issues-point-asmjs:
	
#. In **asm.js** mode, :ref:`function pointer casts <Asm-pointer-casts>` can cause function pointer calls to fail. Without *asm.js*, all function pointers use a single table, but in *asm.js* each function pointer type has its own table (this allows the JavaScript engine to know the exact type of each function pointer call, and optimize those calls much better than normally). As a consequence, if you have a function that is say ``int (int)`` (return int, receive int) and you cast it to ``void (int)`` (no return, receive int), then the function pointer call will fail because we are looking in the wrong table. This is undefined behavior in C in any case, so it is recommended to refactor code to avoid this type of situation. You should see compilation warnings about these things. See :ref:`Asm-pointer-casts` below for more information.

#. A related issue to do with function pointers is that in ``-O2`` and above we optimize the size of the separate function tables. That means that two functions can have the same function pointer so long as their type is different, and so potentially comparing function pointers of different types can give false positives. Also, it makes bugs with incorrect function pointers potentially more misleading, since there are fewer "holes" in function tables (holes would throw an error instead of running the wrong code). To check if this is causing issues, you can compile with `ALIASING_FUNCTION_POINTERS <https://github.com/kripken/emscripten/blob/master/src/settings.js#L201>`_ unset: ``-s ALIASING_FUNCTION_POINTERS=0``.


.. _Asm-pointer-casts:

Asm pointer casts
=================

As mentioned :ref:`above <function-pointer-issues-point-asmjs>`, in **asm.js** mode function pointers must be called using their correct type. This is because each function pointer type has its own table: casting the pointer to another type will cause code to look for the function pointer in the wrong table (and so the call will fail).

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
