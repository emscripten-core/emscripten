
.. _emcc:

=========================================================
Emscripten Compiler Frontend (emcc) (under-construction)
=========================================================

**This document provides the command syntax for the Emscription Compiler Frontend.**

.. note:: The information in this page was output by running ``emcc --help`` on the version of *emcc* in the Emscripten 1.20.0 SDK. The most recent version is `emcc (master) <https://github.com/kripken/emscripten/blob/master/emcc>`_ 

Purpose
============================================

The Emscripten Compiler Frontend (``emcc``) is used to call the Emscripten compiler from the command line. It is effectively a drop-in replacement for a standard compiler like *gcc*.


Command line syntax
============================================

::

	emcc [options] file...

The input file(s) can be either source code files that *Clang* can handle (C or C++), LLVM bitcode in binary form, or LLVM assembly files in human-readable form.

Supported targets include: llvm bitcode, javascript, NOT elf (autoconf likes to see elf above to enable shared object support).

Arguments
---------
 
Most normal gcc/g++ options will work, for example:

  --help                   Display this information
  --version                Display compiler version information

Options that are modified or new in *emcc* are listed below:


.. list-table:: 
   :header-rows: 1
   :widths: 20 80
   :class: wrap-table-content 

   * - Command
     - Description
	 .. _emcc-compiler-optimization-options:
   * - ``-O0``
     - No optimizations (default). This is the recommended setting for starting to port a project, as it includes various assertions.
   * - ``-O1``
     - Simple optimizations, including asm.js, LLVM -O1 optimizations, relooping, and no runtime assertions or C++ exception catching, and enables ``-s ALIASING_FUNCTION_POINTERS=1``.  This is the recommended setting when you want a reasonably optimized build that is generated as quickly as possible (it builds much faster than -O2). 
	
	.. note:: 
	
		- For details on the affects of different opt levels, see apply_opt_level() in **tools/shared.py** and also **src/settings.js**.
		- (to re-enable C++ exception catching, use ``-s DISABLE_EXCEPTION_CATCHING=0`` )
		
   * - ``O2``
     - As -O1, plus various js-level optimizations and LLVM -O3 optimizations. This is the recommended setting for a release build: slower compilation time in return for the smallest and fastest output.
   * - ``-Os``
     - Like -O2 with extra optimizations for size.                   
   * - ``-Oz``
     - Like -Os but reduces code size further.
   * - ``-O3``
     - Like -O2 plus additional JS optimizations that can take a significant amount of compilation time and/or are relatively new. Note that differs from -O2 only during the bitcode to JS (final link + JS generation) stage, as it is JS-specific, so you can run -Os on your source files for example, and -O3 during JS generation if you want. For tips on optimizing your code, see https://github.com/kripken/emscripten/wiki/Optimizing-Code 
   * - ``-s OPTION=VALUE``
     - JavaScript code generation option passed into the Emscripten compiler. For the available options, see **src/settings.js**. Note that for options that are lists, you need quotation marks in most shells, for example 
	 
	::

		-s RUNTIME_LINKED_LIBS="['liblib.so']"
		
	or
	
	::

		-s "RUNTIME_LINKED_LIBS=['liblib.so']"

	without the external "s in either of those, you would get an error)

	You can also specify a file from which the value would be read, for example, 
	
	::

		-s DEAD_FUNCTIONS=@/path/to/file

	The contents of /path/to/file will be read, JSON.parsed and set into DEAD_FUNCTIONS (so the file could contain ["_func1", "func2"] ). Note that the path must be absolute, not relative.

   * - ``-g``
     - Use debug info. When compiling to bitcode, this is the same as in clang and gcc, it adds debug info to the object files. When compiling from source to JS or bitcode to JS, it is equivalent to -g3 (keep code as debuggable as possible, except for discarding LLVM debug info, so no C/C++ line numbers; use -g4 to get line number debugging info in JS).
   * - ``-g<level>``
     - When compiling from bitcode to JS, we can keep the code debuggable to different degrees. Each of these levels builds on the previous:

		- -g0  Make no effort to keep code debuggable. Will discard LLVM debug info. (default in -O1+)
		- -g1  Preserve (do not minify) whitespace
		- -g2  Preserve function names
		- -g3  Preserve variable names
		- -g4  Preserve LLVM debug info (if -g was used when compiling the C/C++ sources), show line number debug comments, and generate source maps. This is the highest level of debuggability. Note that this may make -O1 and above significantly slower because JS optimization will be limited to 1 core.  (default in -O0)	 

   * - ``-profiling``
     - Use reasonable defaults when emitting JS to make the build useful for profiling. This sets -g2 (preserve function names) and may also enable optimizations that affect performance and otherwise might not be performed in -g2.
   * - ``--emit-symbol-map``
     - Save a map file between the minified global names and the original function names. This allows you to reconstruct meaningful stack traces, for example. (This is only relevant when minifying global names, which happens in -O2 and above, and when no -g option to prevent minification was specified.).
   * - ``--typed-arrays <mode>``
     - Possible ``mode`` values are:
	 
		- ``0``: No typed arrays.
		- ``1``: Parallel typed arrays
		- ``2``: Shared (C-like) typed arrays (default)	
		
   * - ``--js-opts <level>``
     - Possible ``level`` values are:
	 
		- ``0``: Prevent JS optimizer from running
		- ``1``: Use JS optimizer (default)
		
   * - ``--llvm-opts <level>``
     - Possible ``level`` values are:
	 
		- 0: No LLVM optimizations (default in -O0)
		- 1: -O1 LLVM optimizations (default in -O1)
		- 2: -O2 LLVM optimizations
		- 3: -O3 LLVM optimizations (default in -O2+)

	You can also specify arbitrary LLVM options, e.g.::
	
		--llvm-opts "['-O3', '-somethingelse']"
							 
   * - ``--llvm-lto <level>``
     - Possible ``level`` values are: 
	 
		- 0: No LLVM LTO (default)
		- 1: LLVM LTO is performed
		- 2: We combine all the bitcode and run LLVM opt -O3 on that (which optimizes across modules, but is not the same as normal LTO), but do not do normal LTO
		- 3: We do both 2 and then 1
		
	.. note::
	
		- If LLVM optimizations are not run (see ``--llvm-opts``), setting this has no effect.
		- LLVM LTO is not perfectly stable yet, and can can cause code to behave incorrectly.					   
						   .	
   * - ``--closure <on>``
     - Possible ``on`` values are:
	 
		- 0: No closure compiler (default in -O2 and below)
		- 1: Run closure compiler. This greatly reduces code size and may in some cases increase runtime speed (although the opposite can also occur). Note that it takes time to run, and may require some changes to the code.

	In asm.js mode, closure will only be used on the 'shell' code around the compiled code (the compiled code will be processed by the custom asm.js minifier).

	.. note:: 
	
		- If closure compiler hits an out-of-memory, try adjusting JAVA_HEAP_SIZE in the environment (for example, to 4096m for 4GB).
		- Closure is only run if js opts are being done (-O2 or above, or --js-opts 1).
	 
   * - ``--pre-js <file>``
     - A file whose contents are added before the generated code. This is done *before* optimization, so it will be minified properly if closure compiler is run.
	 
   * - ``--post-js <file>``
     - A file whose contents are added after the generated code. This is done *before* optimization, so it will be minified properly if closure compiler is run.
	
	.. _emcc-embed-file:
	
   * - ``--embed-file <file>``
     - A file to embed inside the generated JavaScript. The compiled code will be able to access the file in the current directory with the same name as given here. So if you do ``--embed-file dir/file.dat``, then (1) ``dir/file.dat`` must exist relative to where you run *emcc*, and (2) your compiled code will be able to find the file by reading that same path, dir/file.dat. If a directory is passed here, its entire contents will be embedded.

	.. note:: Embedding files is much less efficient than preloading them. You should only use it for small amounts of small files. Instead, use ``--preload-file`` which emits efficient binary data.
	
	.. _emcc-preload-file:
	
   * - ``--preload-file <name>``
     - A file to preload before running the compiled code asynchronously. Otherwise similar to :ref:`--embed-file <emcc-embed-file>`, except that this option is only relevant when generating HTML (it uses asynchronous binary XHRs), or JS that will be used in a web page. If a directory is passed here, its entire contents will be preloaded. Preloaded files are stored in **filename.data**, where **filename.html** is the main file you are compiling to. To run your code, you will need both the **.html** and the **.data**.
	 
	 
	 emcc runs `tools/file_packager.py <https://github.com/kripken/emscripten/blob/master/tools/file_packager.py>`_ to do the actual packaging of embedded and preloaded files. You can run the file packager yourself if you want, see docs inside that file. You should then put the output of the file packager in an emcc ``--pre-js``, so that it executes before your main compiled code (or run it before in some other way).
	 

	 For more docs on the options ``--preload-file`` accepts, see :ref:`Filesystem-Guide`.
	
   * - ``--exclude-file <name>``
     - Files and directories to be excluded from :ref:`--embed-file <emcc-embed-file>` and :ref:`--preload-file <emcc-preload-file>`. Wildcard is supported.
	 
   * - ``--shell-file <path>``
     - The path name to a skeleton HTML file used when generating HTML output. The shell file used needs to have this token inside it: ``{{{ SCRIPT }}}``.
                           
	.. note:: 
	
		- See `src/shell.html <https://github.com/kripken/emscripten/blob/master/src/shell.html>`_ and `src/shell_minimal.html <https://github.com/kripken/emscripten/blob/master/src/shell_minimal.html>`_ for examples.                  
		- This argument is ignored if a target other than HTML is specified using the ``-o`` option.
	
   * - ``--compression <codec>``
     - Compress both the compiled code and embedded/ preloaded files. 

	``<codec>`` should be a triple: ``<native_encoder>,<js_decoder>,<js_name>``, where ``native_encoder`` is a native executable that compresses stdin to stdout (the simplest possible interface), ``js_decoder`` is a JavaScript file that implements a decoder, and ``js_name`` is the name of the function to call in the decoder file (which should receive an array/typed array and return an array/typed array. Compression only works when generating HTML. When compression is on, all filed specified to be preloaded are compressed in one big archive, which is given the same name as the output HTML but with suffix **.data.compress**.
	 
	.. warning:: **THIS OPTION IS DEPRECATED**
						   
   * - ``--minify 0``
     - Identical to -g1.
	 
   * - ``--js-transform <cmd>``
     - ``<cmd>`` will be called on the generated code before it is optimized. This lets you modify the JavaScript, for example adding some code or removing some code, in a way that those modifications will be optimized together with the generated code properly. ``<cmd>`` will be called with the filename of the generated code as a parameter; to modify the code, you can read the original data and then append to it or overwrite it with the modified data. ``<cmd>`` is interpreted as a space-separated list of arguments, for example, ``<cmd>`` of **python processor.py** will cause a Python script to be run.
	 
   * - ``--split <size>``
     - Splits the resulting javascript file into pieces to ease debugging. This option only works if Javascript is generated (target -o <name>.js). Files with function declarations must be loaded before main file upon execution.

		Without "-g" option:
			Creates files with function declarations up to the given size with the suffix "_functions.partxxx.js" and a main file with the suffix ".js".

		With "-g" option:
			Recreates the directory structure of the C source files and stores function declarations in their respective C files with the suffix ".js". If such a file exceeds the given size, files with the suffix ".partxxx.js" are created. The main file resides in the base directory and has the suffix ".js".

	.. warning:: This option is deprecated (modern JS debuggers should work acceptable even on large files).
	 
   * - ``--bind``
     - Compiles the source code using the :ref:`embind` bindings approach, which connects C/C++ and JavaScript.
	 
   * - ``--ignore-dynamic-linking``
     - Normally *emcc* will treat dynamic linking like static linking, by linking in the code from the dynamic library. This fails if the same dynamic library is linked more than once. With this option, dynamic linking is ignored, which allows the build system to proceed without errors. However, you will need to manually link to the shared libraries later on yourself.
	 
   * - ``--js-library <lib>``
     - A JavaScript library to use in addition to those in Emscripten's src/library_* .
	 
   * - ``-v``
     - Turns on verbose output. This will pass ``-v`` to Clang, and also enable ``EMCC_DEBUG`` to details emcc's operations. It will also run Emscripten's internal sanity checks, checking that things like the LLVM directory path looks correct, etc. This works with or without other arguments, so it can be useful to run ``emcc -v`` if you see odd errors, as it can help diagnose things.
	 
   * - ``--clear-cache``
     - Manually clears the cache of compiled Emscripten system libraries (libc++, libc++abi, libc). This is normally handled automatically, but if you update llvm in-place (instead of having a different directory for a new version), the caching mechanism can get confused. Clearing the cache can fix weird problems related to cache incompatibilities, like clang failing to link with library files. This also clears other cached data like the jcache and the bootstrapped relooper. After the cache  is cleared, this process will exit.
	 
   * - ``--save-bc PATH``
     - When compiling to JavaScript or HTML, this option will save a copy of the bitcode to the specified path. The bitcode will include all files being linked, including standard libraries, and after any link-time optimizations (if any).
	 
   * - ``--memory-init-file <on>``
     - Possible ``on`` values are: 
	 
		- ``0``: Do not emit a separate memory initialization file, keep the static initialization inside the generated JavaScript as text (default)
		- ``1``: Emit a separate memory initialization file in binary format. This is more efficient than storing it as text inside JavaScript, but does mean you have another file to publish. The binary file will also be loaded asynchronously, which means main() will not be called until the file is downloaded and applied; you cannot call any C functions until it arrives. (Call yourself from ``main()`` to know when all async stuff has happened and it is safe to call library functions, as main() will only be called at that time. You can also call ``addOnPreMain`` from a ``preRun``.)
	 
   * - ``-Wno-warn-absolute-paths``
     - If not specified, the compiler will warn about any uses of absolute paths in -I and -L command line directives. Pass this flag on the command line to hide these warnings and acknowledge that the explicit use of absolute paths is intentional.
	**HamishW**	 Is it really "Wno-war" - check the source.
	 
   * - ``--proxy-to-worker``
     - Runs the main application code in a worker, proxying events to it and output from it. If emitting htmlL, this emits an html and a js file, with the js to be run in a worker. If emitting js, the target filename contains the part to be run on the main thread, while a second js file with suffix ".worker.js" will contain the worker portion..
	 
   * - ``--emrun``
     - Enables the generated output to be aware of the emrun command line tool. This allows stdout, stderr and exit(returncode) capture when running the generated application through emrun.     
      
   * - ``--em-config``
     - Specifies the location of the .emscripten configuration file for the current compiler run. If not specified, the environment variable EM_CONFIG is read for this file, and if that is not set, the default location ~/.emscripten is assumed..
	 
   * - ``--default-obj-ext .ext``
     - Specifies the file suffix to generate if the location of a directory name is passed to -o directive, e.g. 
	 
	::
	 
		emcc -c a.c -o dir/

	will by default generate an output name 'dir/a.o', but this cmdline param can be passed to generate a file with a custom suffix 'dir/a.ext'.  
       
   * - ``--valid_abspath path``
     - Whitelist an absolute path to prevent warnings about absolute include paths.
	 

   * - ``-o <target>``
     - The ``target`` filename extension defines what will be generated:

		- <name> **.js** : JavaScript
		- <name> **.html** : HTML + side JavaScript file (<name>.js) (JS on the side improves page load time)
		- <name> **.bc** : LLVM bitcode (default)
		- <name> **.o** : LLVM bitcode (same as .bc).

	.. note:: If ``--memory-init-file`` is used, then in addition to a **.js** or **.html** file that is generated, a **.mem** file will also be created.

   * - ``-c``
     - Tells *gcc* not to run the linker and causes LLVM bitcode to be generated, as *emcc* only generates JavaScript in the final linking stage of building.   

Environment variables
=====================

*emcc* is affected by several environment variables, as listed below:

	- EMMAKEN_JUST_CONFIGURE
	- EMMAKEN_JUST_CONFIGURE_RECURSE
	- EMCONFIGURE_JS
	- CONFIGURE_CC
	- EMMAKEN_CXX	
	- EMMAKEN_CXX
	- EMMAKEN_COMPILER 
	- EMMAKEN_CFLAGS
	- EMCC_DEBUG
	- EMCC_FAST_COMPILER

Search for 'os.environ' in `emcc <https://github.com/kripken/emscripten/blob/master/emcc>`_ to see how these are used. The most interesting is possibly ``EMCC_DEBUG``, which forces the compiler to dump its build and temporary files to a temporary directory where they can be reviewed.


