.. _Interacting-with-code:

=====================
Interacting with code
=====================


Emscripten provides numerous methods to connect and interact between
JavaScript and compiled C or C++:

- Call compiled **C** functions from normal JavaScript:

  - :ref:`Using ccall or cwrap <interacting-with-code-ccall-cwrap>`.
  - :ref:`Using direct function calls <interacting-with-code-direct-function-calls>` (faster but more complicated).

- Call compiled **C++** classes from JavaScript using bindings
  created with:

  - :ref:`Embind or WebIDL-Binder<interacting-with-code-binding-cpp>`

- Call JavaScript functions from **C/C++**:

  - :ref:`Using emscripten_run_script() <interacting-with-code-call-javascript-from-native>`.
  - :ref:`Using EM_JS() <interacting-with-code-call-javascript-from-native>` (faster).
  - :ref:`Using EM_ASM() <interacting-with-code-call-javascript-from-native>` (faster).
  - :ref:`Using a C API implemented in JavaScript <implement-c-in-javascript>`.
  - :ref:`As function pointers from C <interacting-with-code-call-function-pointers-from-c>`.
  - :ref:`Using the Embind val class <embind-val-guide>`.


- :ref:`Access compiled code memory from JavaScript <interacting-with-code-access-memory>`.
- :ref:`Affect execution behaviour <interacting-with-code-execution-behaviour>`.
- :ref:`Access environment variables <interacting-with-code-environment-variables>`.

This article explains each of the methods listed above, and provides links
to more detailed information.

.. note:: For information on how compiled code interacts with the browser
   environment, see :ref:`emscripten-runtime-environment`. For file system
   related manners, see the :ref:`file-system-overview`.

.. note:: Before you can call your code, the runtime environment may need
   to load a memory initialization file, preload files, or
   do other asynchronous operations depending on optimization
   and build settings.
   See :ref:`faq-when-safe-to-call-compiled-functions` in the FAQ.


.. _interacting-with-code-ccall-cwrap:

Calling compiled C functions from JavaScript using ccall/cwrap
==============================================================

The easiest way to call compiled C functions from JavaScript is to use
:js:func:`ccall` or :js:func:`cwrap`.

:js:func:`ccall` calls a compiled C function with specified parameters
and returns the result, while :js:func:`cwrap` "wraps" a compiled C
function and returns a JavaScript function you can call normally.
:js:func:`cwrap` is therefore more useful if you plan to call a compiled
function a number of times.

Consider the **tests/hello_function.cpp** file shown below. The
``int_sqrt()`` function to be compiled is wrapped in ``extern "C"``
to prevent C++ name mangling.

.. include:: ../../../../../tests/hello_function.cpp
   :literal:

To compile this code run the following command in the Emscripten
home directory::

    emcc tests/hello_function.cpp -o function.html -s EXPORTED_FUNCTIONS='["_int_sqrt"]' -s EXTRA_EXPORTED_RUNTIME_METHODS='["ccall", "cwrap"]'

``EXPORTED_FUNCTIONS`` tells the compiler what we want to be accessible from the compiled code (everything else might be removed if it is not used), and ``EXTRA_EXPORTED_RUNTIME_METHODS`` tells the compiler that we want to use the runtime functions ``ccall`` and ``cwrap`` (otherwise, it will remove them if it does not see they are used).

.. note::

   `EXPORTED_FUNCTIONS` affects compilation to JavaScript. If you first compile to an object file,
   then compile the object to JavaScript, you need that option on the second command. If you do
   it all together as in the example here (source straight to JavaScript) then this just works,
   of course.

After compiling, you can call this function with :js:func:`cwrap` using the
following JavaScript::

    int_sqrt = Module.cwrap('int_sqrt', 'number', ['number'])
    int_sqrt(12)
    int_sqrt(28)

The first parameter is the name of the function to be wrapped, the second is
the return type of the function (or a JavaScript `null` value if there isn't one), and the third is an array of parameter
types (which may be omitted if there are no parameters). The types are
"number" (for a JavaScript number corresponding to a C integer, float, or general
pointer), "string" (for a JavaScript string that corresponds to a C ``char*`` that represents a string) or
"array" (for a JavaScript array or typed array that corresponds to a C array; for typed arrays, it must be a Uint8Array or Int8Array).

You can run this yourself by first opening the generated page
**function.html** in a web browser (nothing will happen on page
load because there is no ``main()``). Open a JavaScript environment
(**Control-Shift-K** on Firefox, **Control-Shift-J** on Chrome),
and enter the above commands as three separate commands, pressing
**Enter** after each one. You should get the results ``3`` and
``5`` — the expected output for these inputs using C++ integer
mathematics.

:js:func:`ccall` is similar, but receives another parameter with the
parameters to pass to the function:

.. code-block:: javascript

   // Call C from JavaScript
   var result = Module.ccall('int_sqrt', // name of C function
     'number', // return type
     ['number'], // argument types
     [28]); // arguments

   // result is 5

.. note::

   This example illustrates a few other points, which you should remember
   when using :js:func:`ccall` or :js:func:`cwrap`:

   - These methods can be used with compiled **C** functions — name-mangled
     C++ functions won't work.
   - We highly recommended that you *export* functions that are to be called
     from JavaScript:

     - Exporting is done at compile time. For example:
       ``-s EXPORTED_FUNCTIONS='["_main","_other_function"]'`` exports
       ``main()`` and ``other_function()``.
     - Note that you need ``_`` at the
       beginning of the function names in the ``EXPORTED_FUNCTIONS`` list.
     - Note that ``_main`` is mentioned in that list. If you don't have it there,
       the compiler will eliminate it as dead code. The list of exported
       functions is the **entire** list that will be kept alive (unless other
       code was kept alive in another manner).
     - Emscripten does :ref:`dead code elimination <faq-dead-code-elimination>`
       to minimize code size — exporting ensures the functions you need
       aren't removed.
     - At higher optimisation levels (``-O2`` and above), code is minified,
       including function names.
       Exporting functions allows you to continue to access them using the
       original name through the global ``Module`` object.
     - If you want to export a JS library function (something from a
       ``src/library*.js`` file, for example), then in addition to
       ``EXPORTED_FUNCTIONS``, you need to add it to ``DEFAULT_LIBRARY_FUNCS_TO_INCLUDE``,
       as the latter will force the method to actually be included in
       the build.

   - The compiler will remove code it does not see is used, to improve code
     size. If you use ``ccall`` in a place it sees, like code in a ``--pre-js``
     or ``--post-js``, it will just work. If you use it in a place the compiler
     didn't see, like another script tag on the HTML or in the JS console like
     we did in this tutorial, then because of optimizations
     and minification you should export ccall from the runtime, using
     ``EXTRA_EXPORTED_RUNTIME_METHODS``, for example using
     ``-s 'EXTRA_EXPORTED_RUNTIME_METHODS=["ccall", "cwrap"]'``,
     and call it on ``Module`` (which contains
     everything exported, in a safe way that is not influenced by minification
     or optimizations).


Interacting with an API written in C/C++ from NodeJS
====================================================

Say you have a C library that exposes some procedures:

.. code:: c

    //api_example.c
    #include <stdio.h>
    #include <emscripten.h>

    EMSCRIPTEN_KEEPALIVE
    void sayHi() {
      printf("Hi!\n");
    }

    EMSCRIPTEN_KEEPALIVE
    int daysInWeek() {
      return 7;
    }

Compile the library with emcc:

.. code:: bash

    emcc api_example.c -o api_example.js

Require the library and call its procedures from node:

.. code:: javascript

    var em_module = require('./api_example.js');

    em_module._sayHi(); // direct calling works
    em_module.ccall("sayHi"); // using ccall etc. also work
    console.log(em_module._daysInWeek()); // values can be returned, etc.


.. _interacting-with-code-direct-function-calls:

Call compiled C/C++ code "directly" from JavaScript
===================================================

Functions in the original source become JavaScript functions, so you can
call them directly if you do type translations yourself — this will be
faster than using :js:func:`ccall` or :js:func:`cwrap`, but a little
more complicated.

To call the method directly, you will need to use the full name as it
appears in the generated code. This will be the same as the original C
function, but with a leading ``_``.

.. note:: If you use :js:func:`ccall` or :js:func:`cwrap`, you do not need
   to prefix function calls with ``_`` -- just use the C name.

The types of the parameters you pass to functions need to make sense.
Integers and floating point values can be passed as is. Pointers are
simply integers in the generated code.

Strings in JavaScript must be converted to pointers for compiled
code -- the relevant function is :js:func:`Pointer_stringify`, which
given a pointer returns a JavaScript string. Converting a JavaScript
string ``someString`` to a pointer can be accomplished using ``ptr = ``
allocate(intArrayFromString(someString), 'i8', ALLOC_NORMAL) <allocate>``.

.. note:: The conversion to a pointer allocates memory, which needs to be
   freed up via a call to ``free(ptr)`` afterwards (``_free`` in JavaScript side)

There are other convenience functions for converting strings and encodings
in :ref:`preamble-js`.

.. _interacting-with-code-call-javascript-from-native:

Calling JavaScript from C/C++
=============================

Emscripten provides two main approaches for calling JavaScript from C/C++:
running the script using :c:func:`emscripten_run_script` or writing
"inline JavaScript".

The most direct, but slightly slower, way is to use
:c:func:`emscripten_run_script`. This effectively runs the specified
JavaScript code from C/C++ using ``eval()``. For example, to call the
browser's ``alert()`` function with the text 'hi', you would call the
following JavaScript:

.. code-block:: javascript

   emscripten_run_script("alert('hi')");

.. note:: The function ``alert`` is present in browsers, but not in *node*
   or other JavaScript shells. A more generic alternative is to call
   `console.log`.


A faster way to call JavaScript from C is to write "inline JavaScript",
using :c:func:`EM_JS` or :c:func:`EM_ASM` (and related macros).

EM_JS is used to declare JavaScript functions from inside a C file. The "alert"
example might be written using EM_JS like::

   #include <emscripten.h>

   EM_JS(void, call_alert, (), {
     alert('hello world!');
     throw 'all done';
   });

   int main() {
     call_alert();
     return 0;
   }

EM_JS's implementation is essentially a shorthand for :ref:`implementing a
JavaScript library<implement-c-in-javascript>`.

EM_ASM is used in a similar manner to inline assembly code. The "alert" example
might be written with inline JavaScript as::

   #include <emscripten.h>

   int main() {
     EM_ASM(
       alert('hello world!');
       throw 'all done';
     );
     return 0;
   }

When compiled and run, Emscripten will execute the two lines of JavaScript
as if they appeared directly in the generated code. The result would be
an alert, followed by an exception. (Note, however, that under the hood
Emscripten still does a function call even in this case, which has some
amount of overhead.)

You can also send values from C into JavaScript inside :c:macro:`EM_ASM_`
(note the extra "_" at the end), for example::

   EM_ASM_({
     console.log('I received: ' + $0);
   }, 100);

This will show ``I received: 100``.

You can also receive values back, for example the following will print out ``I received: 100``
and then ``101``::

   int x = EM_ASM_INT({
     console.log('I received: ' + $0);
     return $0 + 1;
   }, 100);
   printf("%d\n", x);

See the :c:macro:`emscripten.h docs <EM_ASM_>` for more details.

.. note::

   - You need to specify if the return value is an ``int`` or a ``double``
     using the appropriate macro :c:macro:`EM_ASM_INT` or
     :c:macro:`EM_ASM_DOUBLE`.
   - The input values appear as ``$0``, ``$1``, etc.
   - ``return`` is used to provide the value sent from JavaScript back to C.
   - See how ``{`` and ``}`` are used here to enclose the code. This is
     necessary to differentiate the code from the arguments passed later,
     which are the input values (this is how C macros work).
   - When using the :c:macro:`EM_ASM` macro, ensure that you only use
     single quotes('). Double quotes(") will cause a syntax error that
     is not detected by the compiler and is only shown when looking at
     a JavaScript console while running the offending code.


.. _implement-c-in-javascript:

Implement a C API in JavaScript
===============================

It is possible to implement a C API in JavaScript! This is the approach
used in many of Emscripten's libraries, like SDL1 and OpenGL.

You can use it to write your own APIs to call from C/C++. To do this
you define the interface, decorating with ``extern`` to mark the methods
in the API as external symbols. You then implement the symbols in
JavaScript by simply adding their definition to `library.js`_ (by
default). When compiling the C code, the compiler looks in the JavaScript
libraries for relevant external symbols.

By default, the implementation is added to **library.js** (and this is
where you'll find parts of Emscripten's *libc*). You can put
the JavaScript implementation in your own library file and add it using
the :ref:`emcc option <emcc-js-library>` ``--js-library``. See
`test_js_libraries`_ in **tests/test_other.py** for a complete working
example, including the syntax you should use inside the JavaScript library
file.

As a simple example, consider the case where you have some C code like this:

.. code-block:: c

    extern void my_js(void);

    int main() {
      my_js();
      return 1;
    }

.. note:: When using C++ you should encapsulate ``extern void my_js();``
   in an ``extern "C" {}`` block to prevent C++ name mangling:

   .. code-block:: cpp

      extern "C" {
        extern void my_js();
      }

Then you can implement ``my_js`` in JavaScript by simply adding the
implementation to **library.js** (or your own file). Like our other
examples of calling JavaScript from C, the example below just creates
a dialog box using a simple ``alert()`` function.

.. code-block:: javascript

   my_js: function() {
     alert('hi');
   },

If you add it to your own file, you should write something like

.. code-block:: javascript

   mergeInto(LibraryManager.library, {
     my_js: function() {
       alert('hi');
     },
   });

``mergeInto`` just copies the properties on the second parameter onto the
first, so this add ``my_js`` onto ``LibraryManager.library``, the global
object where all JavaScript library code should be.

JavaScript limits in library files
----------------------------------

If you're not familiar with JavaScript, say if you're a C/C++ programmer
and just using emscripten, then the following issues probably won't come up, but
if you're an experienced JavaScript programmer you need to be aware
some common JavaScript practices can not be used in certain ways in emscripten
library files.

To save space, by default, emscripten only includes library properties
referenced from C/C++. It does this by calling ``toString`` on each
used property on the JavaScript libraries that are linked in. That means
that you can't use a closure directly, for example, as ``toString``
isn't compatible with that - just like when using a string to create
a Web Worker, where you also can't pass a closure. (Note that this
limitation is just for the values for the keys of the object
passes to ``mergeInto`` in the JS library, that is, the toplevel
key-value pairs are special. Interior code inside a function can
have arbitrary JS, of course).

To avoid this limitation of JS libraries, you can put code in another file using
the ``--pre-js`` or ``--post-js`` options, which allow arbitrary normal
JS, and it is included and optimized with the rest of the output. That is
the recommended approach for most cases. Another option is another ``<script>`` tag.

Alternatively, if you prefer to use a JS library file, you can
have a function replace itself and have it called during
initialization.

.. code-block:: javascript

   mergeInto(LibraryManager.library, {

     // Solution for bind or referencing other functions directly
     good_02__postset: '_good_02();',
     good_02: function() {
       _good_02 = document.querySelector.bind(document);
     },

     // Solution for closures
     good_03__postset: '_good_03();',
     good_03: function() {
       var callCount = 0;
       _good_03 = function() {
         console.log("times called: ", ++callCount);
       };
     },

     // Solution for curry/transform
     good_05__postset: '_good_05();',
     good_05: function() {
       _good_05 = curry(scrollTo, 0);
    },

   });

A `__postset` is a string the compiler will emit directly to the
output file. For the example above this code will be emitted.

.. code-block:: javascript

     function _good_02() {
       _good_o2 = document.querySelector.bind(document);
     }

     function _good_03() {
       var callCount = 0;
       _good_03 = function() {
         console.log("times called: ", ++callCount);
       };
     }

     function _good_05() {
       _good_05 = curry(scrollTo, 0);
    };

    // Call each function once so it will replace itself
    _good_02();
    _good_03();
    _good_05();

You can also put most of your code in the ``xxx__postset`` strings.
The example below each method declares a dependency on ``$method_support``
and are otherwise dummy functions. ``$method_support`` itself has a
corresponding ``__postset`` property with all the code to set the
various methods to the functions we actually want.

.. code-block:: javascript

  mergeInto(LibraryManager.library, {
    $method_support: {},
    $method_support__postset: [
      '(function() {                                  ',
      '  var SomeLib = function() {                   ',
      '    this.callCount = 0;                        ',
      '  };                                           ',
      '                                               ',
      '  SomeLib.prototype.getCallCount = function() {',
      '    return this.callCount;                     ',
      '  };                                           ',
      '                                               ',
      '  SomeLib.prototype.process = function() {     ',
      '    ++this.callCount;                          ',
      '  };                                           ',
      '                                               ',
      '  SomeLib.prototype.reset = function() {       ',
      '    this.callCount = 0;                        ',
      '  };                                           ',
      '                                               ',
      '  var inst = new SomeLib();                    ',
      '  _method_01 = inst.getCallCount.bind(inst);   ',
      '  _method_02 = inst.process.bind(inst);        ',
      '  _method_03 = inst.reset.bind(inst);          ',
      '}());                                          ',
    ].join('\n'),
    method_01: function() {},
    method_01__deps: ['$method_support'],
    method_02: function() {},
    method_01__deps: ['$method_support'],
    method_03: function() {},
    method_01__deps: ['$method_support'],
   });

Note: If you are using node 4.1 or newer you can use multi-line strings.
They are only used at compile time not runtime so output will still run in
ES5 based environments.

Another option is to put most of your code in an object, not
a function,

.. code-block:: javascript

  mergeInto(LibraryManager.library, {
    $method_support__postset: 'method_support();',
    $method_support: {
      init: function() {
        var SomeLib = function() {
          this.callCount = 0;
        };

        SomeLib.prototype.getCallCount = function() {
          return this.callCount;
        };

        SomeLib.prototype.process = function() {
          ++this.callCount;
        };

        SomeLib.prototype.reset = function() {
          this.callCount = 0;
        };

        var inst = new SomeLib();
        _method_01 = inst.getCallCount.bind(inst);
        _method_02 = inst.process.bind(inst);
        _method_03 = inst.reset.bind(inst);
      }
    },
    method_01: function() {},
    method_01__deps: ['$method_support'],
    method_02: function() {},
    method_01__deps: ['$method_support'],
    method_03: function() {},
    method_01__deps: ['$method_support'],
   });


See the `library_*.js`_ files for other examples.

.. note::

   - JavaScript libraries can declare dependencies (``__deps``), however
     those are only for other JavaScript libraries. See examples in
     `/src <https://github.com/emscripten-core/emscripten/tree/master/src>`_
     with the name format **library_*.js**
   - You can add dependencies for all your methods using
     ``autoAddDeps(myLibrary, name)`` where myLibrary is the object with
     all your methods, and ``name`` is the thing they all depend upon.
     This is useful when all the implemented methods use a JavaScript
     singleton containing helper methods. See ``library_webgl.js`` for
     an example.
   - If a JavaScript library depends on a compiled C library (like most
     of *libc*), you must edit `src/deps_info.json`_. Search for
     "deps_info" in `tools/system_libs.py`_.
   - The keys passed into `mergeInto` generate functions that are prefixed
     by ``_``. In other words ``my_func: function() {},`` becomes
     ``function _my_func() {}``, as all C methods in emscripten have a ``_`` prefix. Keys starting with ``$`` have the ``$``
     stripped and no underscore added.


.. _interacting-with-code-call-function-pointers-from-c:

Calling JavaScript functions as function pointers from C
========================================================

You can use ``addFunction`` to return an integer value that represents a
function pointer. Passing that integer to C code then lets it call that value as
a function pointer, and the JavaScript function you sent to ``addFunction`` will
be called.

See `test_add_function in tests/test_core.py`_ for an example.

When using ``addFunction``, there is a backing array where these functions are
stored. This array must be explicitly sized, which can be done via a
compile-time setting, ``RESERVED_FUNCTION_POINTERS``. For example, to reserve
space for 20 functions to be added::

    emcc ... -s RESERVED_FUNCTION_POINTERS=20 ...

.. note:: When using ``addFunction`` on LLVM wasm backend, you need to provide
   an additional second argument, a Wasm function signature string. Each
   character within a signature string represents a type. The first character
   represents the return type of a function, and remaining characters are for
   parameter types.

   - ``'v'``: void type
   - ``'i'``: 32-bit integer type
   - ``'j'``: 64-bit integer type (currently does not exist in JavaScript)
   - ``'f'``: 32-bit float type
   - ``'d'``: 64-bit float type

   For example, if you add a function that takes an integer and does not return
   anything, you can do ``addFunction(your_function, 'vi');``. See
   `tests/interop/test_add_function_post.js <https://github.com/emscripten-core/emscripten/blob/incoming/tests/interop/test_add_function_post.js>`_ for an example.


.. _interacting-with-code-access-memory:

Access memory from JavaScript
=============================

You can access memory using :js:func:`getValue(ptr, type) <getValue>` and
:js:func:`setValue(ptr, value, type) <setValue>`. The first argument is a
pointer (a number representing a memory address). ``type`` must be an
LLVM IR type, one of ``i8``, ``i16``, ``i32``, ``i64``, ``float``,
``double`` or a pointer type like ``i8*`` (or just ``*``).

There are examples of these functions being used in the tests — see
`tests/core/test_utf.in`_ and `tests/test_core.py`_.

.. note:: This is a lower-level operation than :js:func:`ccall` and
   :js:func:`cwrap` — we *do* need to care what specific type (e.g.
   integer) is being used.

You can also access memory 'directly' by manipulating the arrays that
represent memory. This is not recommended unless you are sure you know
what you are doing, and need the additional speed over :js:func:`getValue`
and :js:func:`setValue`.

A case where this might be required is if you want to import a large amount
of data from JavaScript to be processed by compiled code. For example, the
following code allocates a buffer, copies in some data, calls a C function
to process the data, and finally frees the buffer.

.. code-block:: javascript

   var buf = Module._malloc(myTypedArray.length*myTypedArray.BYTES_PER_ELEMENT);
   Module.HEAPU8.set(myTypedArray, buf);
   Module.ccall('my_function', 'number', ['number'], [buf]);
   Module._free(buf);

Here ``my_function`` is a C function that receives a single integer parameter
(or a pointer, they are both just 32-bit integers for us) and returns an
integer. This could be something like ``int my_function(char *buf)``.


.. _interacting-with-code-execution-behaviour:

Affect execution behaviour
==========================

``Module`` is a global JavaScript object, with attributes that
Emscripten-generated code calls at various points in its execution.

Developers provide an implementation of ``Module`` to control how
notifications from Emscripten are displayed, which files that are
loaded before the main loop is run, and a number of other behaviours.
For more information see :ref:`module`.

.. _interacting-with-code-environment-variables:

Environment variables
=====================

Sometimes compiled code needs to access environment variables (for instance,
in C, by calling the ``getenv()`` function). Emscripten-generated JavaScript
cannot access the computer's environment variables directly, so a
"virtualised" environment is provided.

The JavaScript object ``ENV`` contains the virtualised environment variables,
and by modifying it you can pass variables to your compiled code. Care must
be taken to ensure that the ``ENV`` variable has been initialised by
Emscripten before it is modified — using :js:attr:`Module.preRun` is a
convenient way to do this.

For example, to set an environment variable ``MY_FILE_ROOT`` to be
``"/usr/lib/test/"`` you could add the following JavaScript to your
``Module`` :ref:`setup code <module-creating>`:

.. code:: javascript

    Module.preRun.push(function() {ENV.MY_FILE_ROOT = "/usr/lib/test"})

.. _interacting-with-code-binding-cpp:

Binding C++ and JavaScript — WebIDL Binder and Embind
======================================================

The JavaScript methods for calling compiled C functions are efficient, but
cannot be used with name-mangled C++ functions.

:ref:`WebIDL-Binder` and :ref:`embind` create bindings between C++ and
JavaScript, allowing C++ code entities to be used in a natural manner from
JavaScript. *Embind* additionally supports calling JavaScript code from C++.

*Embind* can bind almost any C++ code, including sophisticated C++ constructs
(e.g. ``shared_ptr`` and ``unique_ptr``). The *WebIDL Binder* supports C++
types that can be expressed in WebIDL. While this subset is smaller than
supported by *Embind*, it is more than sufficient for most use cases —
examples of projects that have been ported using the binder include the
`Box2D`_ and `Bullet`_ physics engines.

Both tools allow mapped items to be used from JavaScript in a similar way.
However they operate at different levels, and use very different approaches
for defining the binding:

- *Embind* declares bindings within the C/C++ file.
- *WebIDL-Binder* declares the binding in a separate file. This is run
  through the binder tool to create "glue" code that is then compiled
  with the project.

.. note:: There is no strong evidence that one tool is "better" than the
   other in terms of performance (no comparative benchmarks exist), and
   both have been used successfully in a number of projects. The selection
   of one tool over the other will usually be based on which is the most
   natural fit for the project and its build system.

.. _library.js: https://github.com/emscripten-core/emscripten/blob/master/src/library.js
.. _test_js_libraries: https://github.com/emscripten-core/emscripten/blob/1.29.12/tests/test_core.py#L5043
.. _src/deps_info.json: https://github.com/emscripten-core/emscripten/blob/master/src/deps_info.json
.. _tools/system_libs.py: https://github.com/emscripten-core/emscripten/blob/master/tools/system_libs.py
.. _library_\*.js: https://github.com/emscripten-core/emscripten/tree/master/src
.. _test_add_function in tests/test_core.py: https://github.com/emscripten-core/emscripten/blob/1.29.12/tests/test_core.py#L6237
.. _tests/core/test_utf.in: https://github.com/emscripten-core/emscripten/blob/master/tests/core/test_utf.in
.. _tests/test_core.py: https://github.com/emscripten-core/emscripten/blob/1.29.12/tests/test_core.py#L4597
.. _Box2D: https://github.com/kripken/box2d.js/#box2djs
.. _Bullet: https://github.com/kripken/ammo.js/#ammojs
