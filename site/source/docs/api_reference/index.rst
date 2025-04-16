.. _api-reference-index:

=============
API Reference
=============

This section lists Emscripten's public API, organised by header file. At a very
high level it consists of:

- :ref:`emscripten-h`:
  APIs for integrating with the browser environment.

- :ref:`html5-h`:
  Low level glue bindings for interfacing with HTML5 APIs from native code.

- :ref:`console-h`:
  Functions to writing to the console and stdout/stderr.

- :ref:`preamble-js`:
  APIs for working with compiled code from JavaScript.

- :ref:`Filesystem-API` (**library_fs.js**):
  APIs for managing file systems and synchronous file operations.

- :ref:`Fetch-API`:
  API for managing accesses to network XHR and IndexedDB.

- :ref:`wasm workers section`:
  Enables writing multithreaded programs using a web-like API.

- :ref:`wasm_audio_worklets`:
  Allows programs to implement audio processing nodes that run in a dedicated real-time audio processing thread context.

- :ref:`Module`:
  Global JavaScript object that can be used to control code execution and access
  exported methods.

- :ref:`val-h`:
  Embind API to support transliteration of JavaScript code to C++.

- :ref:`bind-h`:
  Embind API for binding C++ functions and classes so that they can be called
  from JavaScript in a natural way.

- :ref:`trace-h`:
  A tracing API for doing memory usage analysis.

- :ref:`fiber-h`:
  API for working with Fibers (co-operative threads)

- :ref:`proxying-h`:
  API for synchronously or asynchronously proxying work to a target pthread.

- :ref:`stack-h`:
  Inspecting the WebAssembly data stack.

- :ref:`api-reference-advanced-apis`:
  APIs for advanced users/core developers.


.. toctree::
   :hidden:

   emscripten.h
   html5.h
   console.h
   preamble.js
   Filesystem-API
   fetch
   module
   val.h
   bind.h
   trace.h
   fiber.h
   proxying.h
   stack.h
   wasm_workers
   wasm_audio_worklets
   advanced-apis
