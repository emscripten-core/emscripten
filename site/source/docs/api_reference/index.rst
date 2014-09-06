==================================
API Reference (under-construction) 
==================================

This section lists Emscripten's public API, organised by header file. At very high level it consists of:

- **emscripten.h**: *APIs for integrating with the browser environment.*

	This includes APIs for: controlling application execution, calling JavaScript from C code, loading files, logging etc.
	
- **html5.h**: *Low level glue bindings for interfacing with HTML5 APIs from native code.*

	This includes APIs for registering event callbacks for all types of HTML5 events, ranging from key, mouse and touch events, through to screen and device orientation events, through to WebGL context events etc.
	
- **preamble.js**: *APIs for working with compiled code.*

	This includes APIs for calling compiled C functions, accessing memory, converting pointers to JavaScript ``Strings`` and ``Strings`` to pointers (with different encodings/formats), and other convenience functions.
	
- **Filesystem-API (library_fs.js)**: *APIs for (primarily) synchronous File I/O.*

- **val.h**: *Embind APIs for* **HamishW** - add description

- **bind.h**: *Embind APIs for* **HamishW**  - add description

- **Advanced APIs**: *APIs for advanced users/core developers*

	In addition to the public APIs listed above, there are additional APIs which are intended only when developing Emscripten itself, or which are complicated to use and hence considered suitable for "advanced users". At time of writing are not documented individually, but are mentioned at high level in :doc:`advanced-apis`.


.. toctree:: 
   :maxdepth: 1
   
   emscripten.h
   html5.h
   preamble.js
   Filesystem-API
   val.h
   bind.h
   advanced-apis


