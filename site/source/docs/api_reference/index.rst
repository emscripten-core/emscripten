==================================
API Reference (under-construction) 
==================================

This section lists Emscripten's public API, organised by header file. At very high level it consists of:

- :ref:`emscripten-h`: *APIs for integrating with the browser environment.*

	This includes APIs for: controlling application execution, calling JavaScript from C code, loading files, logging etc.
	
- :ref:`html5-h`: *Low level glue bindings for interfacing with HTML5 APIs from native code.*

	This includes APIs for registering event callbacks for all types of HTML5 events, ranging from key, mouse and touch events, through to screen and device orientation events, through to WebGL context events etc.
	
- :ref:`preamble-js`: *APIs for working with compiled code.*

	This includes APIs for calling compiled C functions, accessing memory, converting pointers to JavaScript ``Strings`` and ``Strings`` to pointers (with different encodings/formats), and other convenience functions.
	
- :ref:`Filesystem-API` (library_fs.js): *APIs for (primarily) synchronous File I/O.*

- :ref:`Module`: *User-populated global JavaScript object, with attributes that Emscripten-generated code calls at various points in its execution.*

- :ref:`val-h`: *Embind API to support transliteration of JavaScript code to C++.*

- :ref:`bind-h`: *Embind APIs for* **HamishW**  - add description

- :ref:`api-reference-advanced-apis`: *APIs for advanced users/core developers*


.. toctree:: 
   :hidden:
   
   emscripten.h
   html5.h
   preamble.js
   Filesystem-API
   module
   val.h
   bind.h
   advanced-apis


