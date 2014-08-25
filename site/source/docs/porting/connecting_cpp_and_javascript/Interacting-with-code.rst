.. _Interacting-with-code:

===================================
Interacting with code (wiki-import)
===================================

.. note:: This article was migrated from the wiki (Fri, 25 Jul 2014 04:21) and is now the "master copy" (the version in the wiki will be deleted). It may not be a perfect rendering of the original but we hope to fix that soon!

(If you are looking for how compiled code interacts with the browser environment, see :ref:`Emscripten-Browser-Environment`.)

There are various ways to connect and interact between JS and compiled C++ in JS. An overview appears in the second half of these slides:
http://kripken.github.io/mloc\_emscripten\_talk/qcon.html

Calling Compiled Functions From Normal JavaScript
=================================================

It's easy to call compiled code from normal JavaScript. For example, run this command in the Emscripten home directory:

::

    ./emcc tests/hello_function.cpp -o function.html -s EXPORTED_FUNCTIONS="['_int_sqrt']"

Open the page in a web browser. Nothing will happen because you compiled a function, and there is no ``main`` so nothing will be run by default. But, open a JavaScript environment (Control-Shift-K on Firefox, Control-Shift-J on Chrome), then type (as three separate commands, pressing Enter after each one):

::

    int_sqrt = Module.cwrap('int_sqrt', 'number', ['number'])
    int_sqrt(12)
    int_sqrt(28)

You should get the results ``3, 5``, which are the correct output: The compiled function does a square root operation, but acts on integers. Open ``tests/hello_function.cpp`` to see the source code that you just called here.

:js:func:`cwrap` which was used in this example will wrap a compiled C function, returning a JavaScript function you can call normally. :js:func:`cwrap` gets as its first parameter the name of the function to be wrapped, then the return type of the function, then an array of parameter types (the array can be omitted if there are no parameters). The types are native JavaScript types, "number" (for a C integer, float, or general pointer) or "string" (for a C ``char*`` that represents a string).

There is also :js:func:`ccall`, which is like :js:func:`cwrap` but receives another parameter with the parameters to pass to the function, and calls the function. :js:func:`cwrap` is useful to wrap a function once and call it several times, while :js:func:`ccall` is useful for a single call to a function.

Some things to keep in mind with :js:func:`ccall` and :js:func:`cwrap`:

-  In the example above, we compiled a C function. That's easier than C++ because C++ functions are name-mangled.
-  By default Emscripten does dead code elimination to minimize code size. See the :ref:`FAQ` entry on "Functions in my C/C++ source code vanish when I compile to JavaScript..?"
-  In ``-O2`` and above, closure compiler is run, which minifies function names - which means you won't be able to find your compiled
   functions. To prevent that, run emcc with ``-s EXPORTED_FUNCTIONS='["_main","_other_function"]'``. Exported functions will retain their names even through closure compiler.
   **Note:** you need ``_`` at the beginning of the functions when exporting (but not with ccall), and note also that you need to use ``Module.ccall`` and not :js:func:`ccall` by itself, since closure will minify the function name, leaving only the Module object where we export names.

Accessing Memory
================

You can access memory using :js:func:`getValue(ptr, type) <getValue>` and :js:func:`setValue(ptr, value, type) <setValue>`. The first argument is a pointer, a number representing a memory address. ``type`` must be an LLVM IR type, one of ``i8,i16,i32,i64,float,double`` or a pointer type like ``i8*`` (or just ``*``). Note that the types here are not as in :js:func:`ccall` and :js:func:`cwrap` - this is a lower-level operation, and we do need to care what specific integer etc. type is being used.

You can also access memory 'directly' by manipulating the arrays that represent memory. This is not recommended unless you are sure you know what you are doing, and need the additional speed over :js:func:`getValue`/:js:func:`setValue`. A case where you might need this is if you want to import a large amount of data from JS to be processed by compiled code, then you might do something like this:

::

    var buf = Module._malloc(myTypedArray.length*myTypedArray.BYTES_PER_ELEMENT);
    Module.HEAPU8.set(myTypedArray, buf);
    Module.ccall('my_function', 'number', ['number'], [buf]);
    Module._free(buf);

That allocates a buffer, copies in some data, then calls a C function to process the data, and finally frees the buffer. Here ``my_function`` is a C function that receives a single integer parameter (could be a pointer as well, as they are just 32-bit integers for us), and returns an integer as well, something like ``int my_function(char *buf)``.

Calling JavaScript From C/C++
=============================

The most direct way is to just use :c:func:`emscripten_run_script`, which basically runs some JS code from C/C++ using eval. So ``emscripten_run_script("alert('hi')");`` will show an alert with 'hi' (note: this calls ``alert`` which is present in browsers, but not in node or other JS shells. You can call ``Module.print`` to print to stdout). This is not very fast though. A faster alternative is to write "inline JavaScript", basically the same as inline assembly would be used, for example

::

    #include <emscripten.h>
    int main() {
      EM_ASM(
        alert('hello world!');
        throw 'all done';
      );
      return 0;
    }

If you compile that C file, Emscripten will execute those two lines of JavaScript as if they appeared directly there in the generated code, so that when the compiled program is run you will see an alert and then an exception thrown.

You can also send values from C into JS inside :c:macro:`EM_ASM`, as well as receive values back. See ``emscripten.h`` for details. One example is

::

      int x = EM_ASM_INT({
        Module.print('I received: ' + $0);
        return $0 + 1;
      }, 100);
      printf("%d\n", x);

This will print out ``I received: 100`` and then ``101``. Note how you need to specify if the return value is an int or a double (with ``_INT`` here), also how the input values appear as ``$0, $1, etc.``, how ``return`` is used to provide the value sent from JS back to C, and finally how ``{, }`` are used here to enclose the code (this is necessary because of how C macros work, to differentiate the code from the arguments passed later which are the input values).

.. note:: When using the :c:macro:`EM_ASM` macro, ensure that you only use single quotes('). Double quotes(") will cause a syntax error that is not detected by the compiler and is only shown when looking at a JavaScript console while running the offending code.

More generally, our entire libc, SDL etc. implementations are exactly JS code that is called from C/C++ using a C API. You can add your own libraries as well. For example, if you have some C code like this

::

    extern void my_js();

    int main() {
      my_js();
      return 1;
    }

.. note:: When using C++ you should encapsulate ``extern void my_js();`` in ``extern "C" {}`` block:

::

    extern "C" {
      extern void my_js();
    }

then you can implement ``my_js`` in JS by simply adding

::

       my_js: function() {
         alert('hi');
       },

to **library.js**. How this works is that when there is an external symbol, the compiler looks in the JS libraries and pulls in the relevant symbols. See the library\*.js files for more details and examples.

You can use the emcc option ``--js-library`` to add a file with such code, instead of placing it inside **library.js**. This lets you be more modular. See ``test_js_libraries`` in ``tests/test_other.py`` for a complete working example, including the syntax you should use inside the JS library file.

-  JS libraries can declare dependencies (``__deps``, see examples in ``library*.js``), however those are only for other JS libraries. If a JS library depends on a compiled C library (like most of libc), you must edit ``src/deps_info.json``, see ``tools/system_libs.py`` (search for deps\_info).

Calling JS functions as function pointers from C
--------------------------------------------------

You can use ``Runtime.addFunction`` to return an integer value that represents a function pointer. Passing that integer to C code then lets it call that value as a function pointer, and the JS function you sent to ``Runtime.addFunction`` will be called. See ``test_add_function`` in ``tests/test_core.py`` for an example.

WebIDL Binder
=============

The :ref:`WebIDL-Binder` is a tool to make C++ classes usable from JS as JS classes. It is used to port Bullet Physics to the web in the **ammo.js** project, and is a fairly simple lightweight approach to binding between the two languages.

Embind
======

Embind is a method to communicate from JS to C++ and C++ to JS, in a C++-like manner (whereas JS libraries are using C APIs, and just one direction). The only downside is that it is not as lightweight as JS libraries or the WebIDL binder. Docs: :ref:`embind`.

Other methods
=============

You can directly interact in various other ways with the compiled code:

-  Functions in the original source become JS functions, so you can call them directly if you do type translations yourself - this will be faster than using :js:func:`ccall` or :js:func:`cwrap`, but a little more complex. To call the method directly, you will need to use the full name as it appears in the generated code, and note that a leading ``_`` is added to all C methods. (However if you use :js:func:`ccall` or :js:func:`cwrap`, you do not need to prefix function calls with ``_`` - :js:func:`ccall` and :js:func:`cwrap` use the C name, which has no extra leading ``_``).
-  The types of the parameters you pass to functions need to make sense. Integers and floating point values can be passed as is. Aside from those, there are pointers, which are simply integers in the generated code.
-  Strings in JavaScript must be converted to pointers for compiled code, the relevant functions are :js:func:`Pointer_stringify` which given a pointer returns a JavaScript string, and the other direction can be accomplished by ``allocate(intArrayFromString(someString), 'i8', ALLOC_STACK)`` which will convert a JavaScript string ``someString`` to a pointer. Note that conversion to a pointer allocates memory (that's the call to ``allocate`` there), and in this case we allocate it on the stack (if you are called from a compiled function, it will rewind the stack for you; otherwise, you should do ``Runtime.stackSave()`` before and ``Runtime.stackRestore(..that value..)`` afterwards).
-  There are various other convenience functions, see **preamble.js** (that file will be included with the generated code).
-  For filesystem-related manners, see the :ref:`Filesystem-Guide`.

Affecting execution
===================

``Module`` is a global JavaScript object, with attributes that Emscripten-generated code calls at various points in its execution. 

Developers provide an implementation of ``Module`` to control, for example, how notifications from Emscripten are displayed, which files that are loaded before the main loop is run, etc. For more information see :ref:`module`.


Environment variables
==============================

Sometimes compiled code needs to access environment variables (for instance, in C, by calling the ``getenv()`` function). Just as with the file system, Emscripten generated JavaScript cannot access the computer's environment variables directly, so a virtualised environment is provided. 

The JavaScript object ``ENV`` contains these virtualised environment variables, and by modifying it you can pass variables to your compiled code. Care must be taken to ensure that the ``ENV`` variable has been initialised by Emscripten before it is modified - using :js:attr:`Module.preRun` is a convenient way to do this. 

For example to set an environment variable ``MY_FILE_ROOT`` to be ``"/usr/lib/test/"`` you could add the following JavaScript to your :ref:`setup code <module-creating>`:

.. code:: javascript

    Module.preRun.push(function() {ENV.MY_FILE_ROOT = "/usr/lib/test"})


