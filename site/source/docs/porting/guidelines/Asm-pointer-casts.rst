=======================================
Function Pointer Issues (wiki-import)
=======================================

There are three general issues with function pointers:

#. clang generates different code for C and C++ calls when a structure is passed **by value**. The two formats are incompatible in how we generate code in Emscripten, but each is compatible with itself. You should see a warning during compilation about this, and the workaround is to either not mix C and C++ in that location (just renaming a **.c** file to **.cpp** for example) or to pass the structure by reference. (More specifically, one convention is ``struct byval`` and the other is ``field a, field b``.)

#. In **asm.js** mode, there is an additional concern, with function tables. Without **asm.js**, all function pointers use a single table, but in **asm.js** each function pointer type has its own table (this allows the JavaScript engine to know the exact type of each function pointer call, and optimize those calls much better than normally). As a consequence though, if you have a function that is say ``int (int)`` (return int, receive int) and you cast it to ``void (int)`` (no return, receive int), then the function pointer call will fail because we are looking in the wrong table. This is undefined behavior in C in any case, so it is recommended to refactor code to avoid this type of situation. You should see compilation warnings about these things. See :ref:`Asm-pointer-casts` for more information.

#. A related issue to do with function pointers is that in ``-O2`` and above we optimize the size of the separate function tables. That means that two functions can have the same function pointer so long as their type is different, and so potentially comparing function pointers of different types can give false positives. Also, it makes bugs with incorrect function pointers potentially more misleading, since there are fewer "holes" in function tables (holes would throw an error instead of running the wrong code). To check if this is causing issues, you can compile with ``-s ALIASING_FUNCTION_POINTERS=0``.


.. _Asm-pointer-casts:

===============================
Asm pointer casts (wiki-import)
===============================
.. note:: This article was migrated from the wiki (Fri, 25 Jul 2014 04:21) and is now the "master copy" (the version in the wiki will be deleted). It may not be a perfect rendering of the original but we hope to fix that soon!

As mentioned in :ref:`CodeGuidelinesAndLimitations`, function pointers in **asm.js** must be used in a coherent way, with each function called using its correct type. Let's look at an example:

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
		functionList[1] = (voidReturnType)intReturn;         // This will break
		functionList[2] = (voidReturnType)voidReturnNoParam; // This will break
		
		callFunctions(functionList, 3);
	}

This will compile just fine when compiled to machine code. You can try it by saving the code as **main.c** and executing: **cc main.c** and then **./a.out**. You'll see this output:

::

	voidReturn: hello world
	intReturn: hello world
	voidReturnNoParam:

However, this will break in Emscripten. You can compile this on your own by saving this code as ``main.c`` and executing: ``emcc -O2 -g -s ASM_JS=1  main.c -o main.html``. Then load it into your browser. It will break. To see why, let's look at the html source. You should be able to find this section:

.. code:: javascript

	var FUNCTION_TABLE_vi = [b0,b0,b0,b0,b0,b0,_voidReturn,b0];
	var FUNCTION_TABLE_ii = [b1,b1,_intReturn,b1,b1,b1,b1,b1];
	var FUNCTION_TABLE_iii = [b2,b2,b2,b2,b2,b2,b2,b2];
	var FUNCTION_TABLE_v = [b3,b3,b3,b3,_voidReturnNoParam,b3,b3,b3];

.. note:: Because we are compiling using ``-g`` the function names aren't mangled and we can see there are different function tables categorized by the signature of the function.

When Emscripten-generated code executes a function, it will look up the function in the table based on its signature and execute it. In our ``callFunctions()`` method we are passed a list of functions of the signature ``vi`` and for that reason, ``FUNCTION_TABLE_vi`` is the table used to find them. It doesn't matter that in ``main()`` we've added compatible functions to ``functionList``. They will not be found because their signature is different.

There are two ways to fix this.

The first is to cast the function pointer back to its original signature just before calling it.

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

This requires the receiver of the table to have special knowledge about what is in the list (you can see this in the special case for index ``1`` in the while loop). Additionally, ``emcc`` will continue to complain about the original cast taking place in ``main()`` when adding the function to ``functionList[1]``.

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
