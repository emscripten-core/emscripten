.. _Modularized-Output:

==================
Modularized Output
==================

By default, emscripten outputs regular JS scripts which can be loaded on the Web
via a ``script`` tag, or on the command line using Node or other JS runtime.

In this default configuration the generated :ref:`module` variable (and indeed
all of the emscripten internal symbols) are added to the global scope.  This
means that only one emscripten-built program can be loaded in a given scope
(since multiple modules would clobber each other's state).

The :ref:`modularize` setting (along with other related settings) can be used to
isolate the generated module within its own private scope and/or allow multiple
instances of the same module to be created.

This section covers the various modularization options available and how to use
them.


Using -sMODULARIZE
==================

The :ref:`modularize` setting has two primary functions.  Firstly, it
puts all of the generated code inside its own scope so that the global namespace
is not polluted when the JS is loaded (for example, via ``script`` tag).
Secondly, it allows multiple instances of the module to be created.

The generated factory function is async and it returns a promise of an instance
of the program.  It can be called any number of times to create multiple
separate and isolated instances of the same program.

By default, the factory function is called ``Module``, but it can be given a
unique name via the ``-sEXPORT_NAME`` setting.

For example, a program is built using ``-sMODULARIZE -sEXPORT_NAME=Foo`` can
be instantiated using:

::

  var myFoo = await Foo({ /* optional params */ });
  myFoo._nativeMethod();


or:

::

  Foo({ /* optional params */ }).then((myFoo) => {
    myFoo._nativeMethod();
  }

If node is included in the ``-sENVIRONMENT`` setting then the generated module
also acts as a ``CommonJS`` module under node.

Because there is no global ``Module`` object in this case any parameters to the
module creation are passed via the optional parameter to the factory function.
For example, if you wanted to use a custom ``print`` method you could write:

::

  var myFoo = await Foo({ print: myPrint });


Generating ES6 Modules
======================

The :ref:`EXPORT_ES6` setting can be used to instead output JavaScript in the
form of an ES module.  (The ES module format did not exist at time when the
`MODULARIZE` setting was first created otherwise this would likely be the
default).

Using an output filename that ends in `.mjs` will automatically enable this
setting.

In this mode the module has a single default export which is the factory
function for creating new instances of the module.  For example:

::

  import Foo from './emcc-output.js';
  var myFoo = await Foo({ print: myPrint });

The name ``Foo`` here can be whatever you choose and will always be assigned to
the default export of the module.


Using -sMODULARIZE=instance (experimental)
==========================================

Emscripten has experimental support for performing only the encapsulation part of
modularization, and not the ability to create multiple instances.  In this
mode a module is created that exports a single instance of the program rather
than a factory function.

This setting implicitly enables :ref:`export_es6` and will not work when
:ref:`export_es6` is explictly disabled.

In this mode the default export of the module is an initializer function which
allows input parameters to be passed to the instance.  Other elements normally
found on the module object are instead exported directly.  For example:

::

  // Import the default init function and a named native method
  import init, { _nativeMethod }  from './emcc-output.js';
  await init({ print: myPrint });
  _nativeMethod();

Limitations
-----------

Some major features still do not work in this mode.  Many of these we hope to
fix in future releses.  Current limitations include:

* Internal usage (e.g. usage within EM_JS / JS libary code) of the ``Module``
  global does not work.  This is because symbols are exported directly using
  ES6 module syntax rathar than using the ``Module`` global.

* The ``wasmExports`` internal global does not exist.

* `ccall`/`cwrap` are not supported (depends on the ``Module`` global).

* :ref:`minimal_runtime` is not supported.

* The output of file_packager is not compatible so :ref:`emcc-preload-file` and
  :ref:`emcc-embed-file` do not work.


Source Phase Imports (experimental)
===================================

`Source phase imports`_ is a JavaScript proposal that adds support for importing
Wasm modules via ES import statements.  This allows emscripten to elide some of
the auto-generated code for finding and fetching the Wasm binary.

See :ref:`source_phase_imports`.

This setting implicitly enables :ref:`export_es6` and will not work when
:ref:`export_es6` is explictly disabled.


ES Module Integration (experimental)
====================================

`Wasm ESM integration`_ is a WebAssembly proposal that allows Wasm instances to
be imported directly as ES modules.  This allows emscripten to elide a lot of
boilerplate code for linking up Wasm and JavaScript.

See :ref:`wasm_esm_integration`.

Limitations
-----------

This setting implicitly enables :ref:`export_es6` and sets :ref:`MODULARIZE` to
``instance``.  Because of this all the same limitations mentioned above for
``-sMODULARIZE=intance`` apply.

Some additional limitations are:

* :ref:`wasm_workers` is not yet supported.

* :ref:`abort_on_wasm_exceptions` is not supported (requires wrapping wasm
  exports).

* :ref:`asyncify` is not supported (depends on ``wasmExports`` global)

* Setting :ref:`wasm` to ``0`` is not supported.

* Setting :ref:`wasm_async_compilation` to ``0`` is not supported.


.. _Source phase imports: https://github.com/tc39/proposal-source-phase-imports
.. _Wasm ESM integration: https://github.com/WebAssembly/esm-integration
