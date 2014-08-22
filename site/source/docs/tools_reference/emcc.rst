.. _emccdoc:

======================================================
Emscripten Compiler Frontend (emcc) (ready-for-review)
======================================================

**This document provides the command syntax for the Emscription Compiler Frontend.**

Purpose
============================================

The Emscripten Compiler Frontend (``emcc``) is used to call the Emscripten compiler from the command line. It is effectively a drop-in replacement for a standard compiler like *gcc* or *clang*.


Command line syntax
============================================

::

	emcc [options] file...

The input file(s) can be either source code files that *Clang* can handle (C or C++), LLVM bitcode in binary form, or LLVM assembly files in human-readable form.


Arguments
---------
 
Most `clang options <http://linux.die.net/man/1/clang>`_ will work, as will `gcc options <https://gcc.gnu.org/onlinedocs/gcc/Option-Summary.html#Option-Summary>`_, for example: ::

	# Display this information
	emcc --help		
		
	Display compiler version information
	emcc --version
                  

To see the full list of *Clang* options supported on the version of *Clang* used by Emscripten, run ``clang --help``.

Options that are modified or new in *emcc* are listed below:

.. _emcc-compiler-optimization-options:

``-O0``
	No optimizations (default). This is the recommended setting for starting to port a project, as it includes various assertions.
	
.. _emcc-O1:

``-O1``
	Simple optimizations. These include using **asm.js**, LLVM ``-O1`` optimizations, relooping, removing runtime assertions and C++ exception catching, and enabling ``-s ALIASING_FUNCTION_POINTERS=1``.  This is the recommended setting when you want a reasonably optimized build that is generated as quickly as possible (it builds much faster than ``-O2``). 
	
	.. note:: 
	
		- For details on the affects of different opt levels, see ``apply_opt_level()`` in `tools/shared.py <https://github.com/kripken/emscripten/blob/master/tools/shared.py>`_ and also `src/settings.js <https://github.com/kripken/emscripten/blob/master/src/settings.js>`_.
		- To re-enable C++ exception catching, use ``-s DISABLE_EXCEPTION_CATCHING=0``.

.. _emcc-O2: 
		
``-O2``
	Like ``-O1``, but with various JavaScript-level optimizations and LLVM ``-O3`` optimizations. 
	
	.. note:: This is the recommended setting for a release build, offering slower compilation time in return for the smallest and fastest output.
	
``-Os``
	Like ``-O2``, but with extra optimizations for size.
	
``-Oz``
	Like ``-Os``, but reduces code size even further.

.. _emcc-O3:

``-O3``
	Like ``-O2``, but with additional JavaScript optimizations that can take a significant amount of compilation time and/or are relatively new. 
	
	.. note:: This differs from ``-O2`` only during the bitcode to JavaScript (final link and JavaScript generation) stage. It is JavaScript-specific, so you can run ``-Os`` on your source files for example, and ``-O3`` during JavaScript generation if you want. For more tips on optimizing your code, see :ref:`Optimizing-Code`.
	
``-s OPTION=VALUE``
	JavaScript code generation option passed into the Emscripten compiler. For the available options, see `src/settings.js <https://github.com/kripken/emscripten/blob/master/src/settings.js>`_. 
	
	.. note:: For options that are lists, you need quotation marks (") around the list in most shells (to avoid errors being raised). Two examples are shown below:
	 
		::

			-s RUNTIME_LINKED_LIBS="['liblib.so']"
			-s "RUNTIME_LINKED_LIBS=['liblib.so']"
	
	You can also specify a file from which the value would be read, for example, 
	
	::

		-s DEAD_FUNCTIONS=@/path/to/file

	The contents of **/path/to/file** will be read, JSON.parsed and set into ``DEAD_FUNCTIONS`` (so the file could contain ["_func1", "func2"] ). Note that the path must be absolute, not relative.

``-g``
	Use debug info. 
	
	- When compiling to bitcode, this is the same as in *Clang* and *gcc* (it adds debug information to the object files). 
	- When compiling from source to JavaScript or bitcode to JavaScript, it is equivalent to ``-g3`` (discards LLVM debug info including C/C++ line numbers, but otherwise keeps as much debug information as possible). Use ``-g4`` to get line number debugging information in JavaScript.
	
``-g<level>``
	Controls how much debug information is kept when compiling from bitcode to JavaScript. Each of these levels builds on the previous:

		- ``-g0``: Make no effort to keep code debuggable. Will discard LLVM debug information (default in ``O1`` and higher).
		- ``-g1``: Preserve (do not minify) whitespace.
		- ``-g2``: Preserve function names.
		- ``-g3``: Preserve variable names.
		- ``-g4``: Preserve LLVM debug information. If ``-g`` was used when compiling the C/C++ sources, show line number debug comments, and generate source maps. This is the highest level of debuggability. 
		
			.. note:: This may make compilation at optimization level ``-O1`` and above significantly slower, because JavaScript optimization will be limited to one core (default in ``-O0``). 

``-profiling``
	Use reasonable defaults when emitting JavaScript to make the build useful for profiling. This sets ``-g2`` (preserve function names) and may also enable optimizations that affect performance and otherwise might not be performed in ``-g2``.

``--emit-symbol-map``
	Save a map file between the minified global names and the original function names. This allows you, for example, to reconstruct meaningful stack traces. 
	
	.. note:: This is only relevant when :term:`minifying` global names, which happens in ``-O2`` and above, and when no ``-g`` option was specified to prevent minification.
	
``--typed-arrays <mode>``
	Set the :ref:`typed array mode <typed-arrays>`. Possible values are:
	 
		- ``0``: No typed arrays.
		- ``1``: Parallel typed arrays.
		- ``2``: Shared (C-like) typed arrays (default).
		
``--js-opts <level>``
	Possible ``level`` values are:
	 
		- ``0``: Prevent JavaScript optimizer from running.
		- ``1``: Use JavaScript optimizer (default).
		
``--llvm-opts <level>``
	Possible ``level`` values are:
	 
		- ``0``: No LLVM optimizations (default in -O0).
		- ``1``: LLVM ``-O1`` optimizations (default in -O1).
		- ``2``: LLVM ``-O2`` optimizations.
		- ``3``: LLVM ``-O3`` optimizations (default in -O2+).

	You can also specify arbitrary LLVM options, e.g.::
	
		--llvm-opts "['-O3', '-somethingelse']"
							 
``--llvm-lto <level>``
	Possible ``level`` values are: 
	 
		- ``0``: No LLVM LTO (default).
		- ``1``: LLVM LTO is performed.
		- ``2``: Combine all the bitcode and run LLVM opt ``-O3`` on it. This optimizes across modules, but is not the same as normal LTO.
		- ``3``: Does level ``2`` and then level ``1``.
		
	.. note::
	
		- If LLVM optimizations are not run (see ``--llvm-opts``), this setting has no effect.
		- LLVM LTO is not perfectly stable yet, and can can cause code to behave incorrectly.					   
						   .	
``--closure <on>``
	Runs the :term:`Closure Compiler`. Possible ``on`` values are:
	 
		- ``0``: No closure compiler (default in ``-O2`` and below).
		- ``1``: Run closure compiler. This greatly reduces code size and may in some cases increase runtime speed (although the opposite can also occur). Note that it takes time to run, and may require some changes to the code.

	In **asm.js** mode, closure will only be used on the 'shell' code around the compiled code (the compiled code will be processed by the custom **asm.js** minifier).

	.. note:: 
	
		- If closure compiler hits an out-of-memory, try adjusting ``JAVA_HEAP_SIZE`` in the environment (for example, to 4096m for 4GB).
		- Closure is only run if JavaScript opts are being done (``-O2`` or above, or ``--js-opts 1``).

.. _emcc-pre-js:
		
``--pre-js <file>``
	Specify a file whose contents are added before the generated code. This is done *before* optimization, so it will be minified properly if the *Closure Compiler* is run.
	 
``--post-js <file>``
	Specify a file whose contents are added after the generated code. This is done *before* optimization, so it will be minified properly if the *Closure Compiler* is run.
	
.. _emcc-embed-file:
	
``--embed-file <file>``
	Specify a file (with path) to embed inside the generated JavaScript. The path is relative to the current directory at compile time. If a directory is passed here, its entire contents will be embedded.
	
	For example, if the command includes ``--embed-file dir/file.dat``, then ``dir/file.dat`` must exist relative to the directory where you run *emcc*. 

	.. note:: Embedding files is much less efficient than :ref:`preloading <emcc-preload-file>` them. You should only use it for small amounts of small files. Instead, use ``--preload-file`` which emits efficient binary data.
	
.. _emcc-preload-file:
	
``--preload-file <name>``
	Specify a file to preload before running the compiled code asynchronously. The path is relative to the current directory at compile time. If a directory is passed here, its entire contents will be embedded. 
	
	Preloaded files are stored in **filename.data**, where **filename.html** is the main file you are compiling to. To run your code, you will need both the **.html** and the **.data**.
	
	.. note:: This option is similar to :ref:`--embed-file <emcc-embed-file>`, except that it is only relevant when generating HTML (it uses asynchronous binary :term:`XHRs <XHR>`), or JavaScript that will be used in a web page. 
	 
	*emcc* runs `tools/file_packager.py <https://github.com/kripken/emscripten/blob/master/tools/file_packager.py>`_ to do the actual packaging of embedded and preloaded files. You can run the file packager yourself if you want (see the documentation inside that file). You should then put the output of the file packager in an emcc ``--pre-js``, so that it executes before your main compiled code.
	 
	For more information about the ``--preload-file`` options, see :ref:`Filesystem-Guide`.
	
``--exclude-file <name>``
	Files and directories to be excluded from :ref:`--embed-file <emcc-embed-file>` and :ref:`--preload-file <emcc-preload-file>`. Wildcards (*) are supported.
	 
``--shell-file <path>``
	The path name to a skeleton HTML file used when generating HTML output. The shell file used needs to have this token inside it: ``{{{ SCRIPT }}}``.
                           
	.. note:: 
	
		- See `src/shell.html <https://github.com/kripken/emscripten/blob/master/src/shell.html>`_ and `src/shell_minimal.html <https://github.com/kripken/emscripten/blob/master/src/shell_minimal.html>`_ for examples.                  
		- This argument is ignored if a target other than HTML is specified using the ``-o`` option.
	
``--compression <codec>``
	Compress both the compiled code and embedded/ preloaded files. 
	
	.. warning:: This option is deprecated.

	``<codec>`` should be a triple: ``<native_encoder>,<js_decoder>,<js_name>``, where:

		- ``native_encoder`` is a native executable that compresses ``stdin`` to ``stdout`` (the simplest possible interface).
		- ``js_decoder`` is a JavaScript file that implements a decoder.
		- ``js_name`` is the name of the function to call in the decoder file (which should receive an array/typed array and return an array/typed array. 
		
	Compression only works when generating HTML. When compression is on, all files specified to be preloaded are compressed in one big archive, which is given the same name as the output HTML but with suffix **.data.compress**.

	
						   
``--minify 0``
	Identical to ``-g1``.
	 
``--js-transform <cmd>``
	Specifies a ``<cmd>`` to be called on the generated code before it is optimized. This lets you modify the JavaScript, for example adding or removing some code, in a way that those modifications will be optimized together with the generated code. 
	
	``<cmd>`` will be called with the file name of the generated code as a parameter. To modify the code, you can read the original data and then append to it or overwrite it with the modified data. 
	
	``<cmd>`` is interpreted as a space-separated list of arguments, for example, ``<cmd>`` of **python processor.py** will cause a Python script to be run.
	 
``--split <size>``
	Splits the resulting JavaScript file into pieces to ease debugging. 
	
	.. warning:: This option is deprecated (modern JavaScript debuggers should work even on large files).
	
	This option only works if JavaScript is generated (``target -o <name>.js``). Files with function declarations must be loaded before main file upon execution.

		- Without ``-g`` option this creates files with function declarations up to the given size with the suffix **_functions.partxxx.js** and a main file with the suffix ".js".
		- With the ``-g`` option this recreates the directory structure of the C source files and stores function declarations in their respective C files with the suffix ".js". If such a file exceeds the given size, files with the suffix ".partxxx.js" are created. The main file resides in the base directory and has the suffix ".js".
	 
``--bind``
	Compiles the source code using the :ref:`embind` bindings approach, which connects C/C++ and JavaScript.
	 
``--ignore-dynamic-linking``
	Tells the compiler to ignore dynamic linking (the user will need to manually link to the shared libraries later on).
	
	Normally *emcc* will simply link in code from the dynamic library as though it were statically linked, which will fail if the same dynamic library is linked more than once. With this option, dynamic linking is ignored, which allows the build system to proceed without errors. 
	 
``--js-library <lib>``
	A JavaScript library to use in addition to those in Emscripten's core libraries (src/library_*).
	 
``-v``
	Turns on verbose output. 
	
	This will pass ``-v`` to *Clang*, and also enable ``EMCC_DEBUG`` (gets intermediate files for the compiler’s various stages). It will also run Emscripten's internal sanity checks on the toolchain, etc. 
	
	.. tip:: ``emcc -v`` is a useful tool for diagnosing errors. It works with or without other arguments. 
	
.. _emcc-clear-cache:
	 
``--clear-cache``
	Manually clears the cache of compiled Emscripten system libraries (libc++, libc++abi, libc). 
	
	This is normally handled automatically, but if you update LLVM in-place (instead of having a different directory for a new version), the caching mechanism can get confused. Clearing the cache can fix weird problems related to cache incompatibilities, like *Clang* failing to link with library files. This also clears other cached data like the jcache and the bootstrapped relooper. After the cache is cleared, this process will exit.
	 
``--save-bc PATH``
	When compiling to JavaScript or HTML, this option will save a copy of the bitcode to the specified path. The bitcode will include all files being linked after link-time optimizations have been performed (if any), including standard libraries.
	 
``--memory-init-file <on>``
	Specifies whether to emit a separate memory initialization file. Possible ``on`` values are: 
	 
		- ``0``: Do not emit a separate memory initialization file (default). Instead keep the static initialization inside the generated JavaScript as text.
		- ``1``: Emit a separate memory initialization file in binary format. This is more efficient than storing it as text inside JavaScript, but does mean you have another file to publish. The binary file will also be loaded asynchronously, which means ``main()`` will not be called until the file is downloaded and applied; you cannot call any C functions until it arrives. 
		
			.. note:: The :ref:`safest way <faq-when-safe-to-call-compiled-functions>` to ensure that it is safe to call C functions (the initialisation file has loaded) is to call a notifier function from ``main()``. 
	
``-Wno-warn-absolute-paths``
	Suppress warnings about the use of absolute paths in ``-I`` and ``-L`` command line directives. This is used to hide the warnings and acknowledge that the explicit use of absolute paths is intentional.
	 
``--proxy-to-worker``
	Runs the main application code in a worker, proxying events to it and output from it. If emitting HTML, this emits a **.html** and a **.js** file, with the JavaScript to be run in a worker. If emitting JavaScript, the target file name contains the part to be run on the main thread, while a second **.js** file with suffix ".worker.js" will contain the worker portion.
	 
``--emrun``
	Enables the generated output to be aware of the :ref:`emrun <Running-html-files-with-emrun>` command line tool. This allows ``stdout``, ``stderr`` and ``exit(returncode)`` capture when running the generated application through *emrun*.     
      
``--em-config``
	Specifies the location of the **.emscripten** configuration file for the current compiler run. If not specified, the environment variable ``EM_CONFIG`` is first read for this location. If neither are specified, the default location **~/.emscripten** is used.
	 
``--default-obj-ext .ext``
	Specifies the file suffix to generate if the location of a directory name is passed to the ``-o`` directive. 
	
	For example, consider the following command which will by default generate an output name **dir/a.o**. With ``--default-obj-ext .ext`` the generated file has the custom suffix *dir/a.ext*. 
	 
	::
	 
		emcc -c a.c -o dir/
 
       
``--valid_abspath path``
	Whitelist an absolute path to prevent warnings about absolute include paths.
	 
.. _emcc-o-target:

``-o <target>``
	The ``target`` file name extension defines what type of output be generated:

		- <name> **.js** : JavaScript.
		- <name> **.html** : HTML + separate JavaScript file (**<name>.js**). Having the separate JavaScript file improves page load time.
		- <name> **.bc** : LLVM bitcode (default).
		- <name> **.o** : LLVM bitcode (same as .bc).

	.. note:: If ``--memory-init-file`` is used, then in addition to the **.js** or **.html** file which is generated, a **.mem** file will also be created.

``-c``
	Tells *emcc* to generate LLVM bitcode (which can then be linked with other bitcode files), instead of compiling all the way to JavaScript.

	

Environment variables
=====================

*emcc* is affected by several environment variables, as listed below:

	- ``EMMAKEN_JUST_CONFIGURE``
	- ``EMMAKEN_JUST_CONFIGURE_RECURSE``
	- ``EMCONFIGURE_JS``
	- ``CONFIGURE_CC``
	- ``EMMAKEN_CXX``
	- ``EMMAKEN_CXX``
	- ``EMMAKEN_COMPILER``
	- ``EMMAKEN_CFLAGS``
	- ``EMCC_DEBUG``
	- ``EMCC_FAST_COMPILER``

Search for 'os.environ' in `emcc <https://github.com/kripken/emscripten/blob/master/emcc>`_ to see how these are used. The most interesting is possibly ``EMCC_DEBUG``, which forces the compiler to dump its build and temporary files to a temporary directory where they can be reviewed.


