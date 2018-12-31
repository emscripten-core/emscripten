.. _Debugging:

=========
Debugging
=========

One of the main advantages of debugging cross-platform Emscripten code is that the same cross-platform source code can be debugged on either the native platform or using the web browser's increasingly powerful toolset — including debugger, profiler, and other tools. 

Emscripten provides a lot of functionality and tools to aid debugging:

- :ref:`Compiler debug information flags <debugging-debug-information-g>` that allow you to preserve debug information in compiled code and even create source maps so that you can step through the native C++ source when debugging in the browser.
- :ref:`Debug mode <debugging-EMCC_DEBUG>`, which emits debug logs and stores intermediate build files for analysis.
- :ref:`Compiler settings <debugging-compilation-settings>` to enable runtime checking of memory accesses and common allocation errors.
- :ref:`debugging-manual-debugging` of Emscripten-generated code is also supported, which is in some ways even better than on native platforms.
- :ref:`debugging-autodebugger`, which automatically instruments LLVM bitcode to write out each store to memory.

This article describes the main tools and settings provided by Emscripten for debugging, along with a section explaining how to debug a number of :ref:`debugging-emscripten-specific-issues`.

	
.. _debugging-debug-information-g:

Debug information
=================

:ref:`Emcc <emccdoc>` strips out most of the debug information from :ref:`optimized builds <Optimizing-Code>` by default. Optimisation levels :ref:`-01 <emcc-O1>` and above remove LLVM debug information, and also disable runtime :ref:`ASSERTIONS <debugging-ASSERTIONS>` checks. From optimization level :ref:`-02 <emcc-O2>` the code is minified by the :term:`Closure Compiler` and becomes virtually unreadable.

The *emcc* :ref:`-g flag <emcc-g>` can be used to preserve debug information in the compiled output. By default, this option preserves white-space, function names and variable names. 

The flag can also be specified with one of five levels: :ref:`-g0 <emcc-g0>`, :ref:`-g1 <emcc-g1>`, :ref:`-g2 <emcc-g2>`, :ref:`-g3 <emcc-g3>`, and :ref:`-g4 <emcc-g4>`. Each level builds on the last to provide progressively more debug information in the compiled output. The :ref:`-g3 flag <emcc-g3>` provides the same level of debug information as the :ref:`-g flag <emcc-g>`. 

The :ref:`-g4 <emcc-g4>` option provides the most debug information — it generates source maps that allow you to view and debug the *C/C++ source code* in your browser's debugger on Firefox, Chrome or Safari! 
 
.. note:: Some optimizations may be disabled when used in conjunction with the debug flags. For example, if you compile with ``-O3 -g4`` some of the normal ``-O3`` optimizations will be disabled in order to provide the requested debugging information. 

.. _debugging-EMCC_DEBUG:

Debug mode (EMCC_DEBUG)
=======================

The ``EMCC_DEBUG`` environment variable can be set to enable Emscripten's debug mode:

.. code-block:: bash

	# Linux or Mac OS X
	EMCC_DEBUG=1 ./emcc tests/hello_world.cpp -o hello.html

	# Windows
	set EMCC_DEBUG=1 
	emcc tests/hello_world.cpp -o hello.html
	set EMCC_DEBUG=0 

With ``EMCC_DEBUG=1`` set, :ref:`emcc <emccdoc>` emits debug output and generates intermediate files for the compiler's various stages. ``EMCC_DEBUG=2`` additionally generates intermediate files for each JavaScript optimizer pass. 

The debug logs and intermediate files are output to **TEMP_DIR/emscripten_temp**, where ``TEMP_DIR`` is by default **/tmp** (it is defined in the :ref:`.emscripten configuration file <compiler-configuration-file>`). 
	
The debug logs can be analysed to profile and review the changes that were made in each step.

.. note:: The debug mode can also be enabled by specifying the :ref:`verbose output <debugging-emcc-v>` compiler flag (``emcc -v``). 


.. _debugging-compilation-settings:

Compiler settings
==================

Emscripten has a number of compiler settings that can be useful for debugging. These are set using the :ref:`emcc -s <emcc-s-option-value>` option, and will override any optimization flags. For example:

.. code-block:: bash

	./emcc -01 -s ASSERTIONS=1 tests/hello_world

The most important settings are:

	- 
		.. _debugging-ASSERTIONS:
	
		``ASSERTIONS=1`` is used to enable runtime checks for common memory allocation errors (e.g. writing more memory than was allocated). It also defines how Emscripten should handle errors in program flow. The value can be set to ``ASSERTIONS=2`` in order to run additional tests.
		
		``ASSERTIONS=1`` is enabled by default. Assertions are turned off for optimized code (:ref:`-01 <emcc-O1>` and above). 
		
	- 
		.. _debugging-SAFE-HEAP:

		``SAFE_HEAP=1`` adds additional memory access checks, and will give clear errors for problems like dereferencing 0 and memory alignment issues.

		You can also set ``SAFE_HEAP_LOG`` to log ``SAFE_HEAP`` operations.

	-
		.. _debugging-STACK_OVERFLOW_CHECK:

		Passing the ``STACK_OVERFLOW_CHECK=1`` linker flag adds a runtime magic token value at the end of the stack, which is checked in certain locations to verify that the user code does not accidentally write past the end of the stack. While overrunning the Emscripten stack is not a security issue (JavaScript is sandboxed already), writing past the stack causes memory corruption in global data and dynamically allocated memory sections in the Emscripten HEAP, which makes the application fail in unexpected ways. The value ``STACK_OVERFLOW_CHECK=2`` enables slightly more detailed stack guard checks, which can give a more precise callstack at the expense of some performance. Default value is 2 if ``ASSERTIONS=1`` is set, and disabled otherwise.

A number of other useful debug settings are defined in `src/settings.js <https://github.com/kripken/emscripten/blob/master/src/settings.js>`_. For more information, search that file for the keywords "check" and "debug".


.. _debugging-emcc-v:

emcc verbose output
===================

Compiling with the :ref:`emcc -v <emcc-verbose>` option passes ``-v`` to LLVM and runs Emscripten's internal sanity checks on the toolchain. 

The verbose mode also enables Emscripten's :ref:`debugging-EMCC_DEBUG` to generate intermediate files for the compiler’s various stages. 


.. _debugging-manual-debugging:

Manual print debugging
======================

You can also manually instrument the source code with ``printf()`` statements, then compile and run the code to investigate issues.

If you have a good idea of the problem line you can add ``print(new Error().stack)`` to the JavaScript to get a stack trace at that point. Also available is :js:func:`stackTrace`, which emits a stack trace and tries to demangle C++ function names (if you don't want or need C++ demangling, you can call :js:func:`jsStackTrace`).

Debug printouts can even execute arbitrary JavaScript. For example:

.. code-block:: cpp

	function _addAndPrint($left, $right) {
		$left = $left | 0;
		$right = $right | 0;
		//---
		if ($left < $right) console.log('l<r at ' + stackTrace());
		//---
		_printAnInteger($left + $right | 0);
	}


Disabling optimizations
=======================

It can sometimes be useful to compile with either LLVM optimizations (:ref:`llvm-opts <emcc-llvm-opts>`) or JavaScript optimizations (:ref:`js-opts <emcc-js-opts>`) disabled. 

For example, the following command enables :ref:`debugging-debug-information-g` and :ref:`-02 <emcc-O2>` optimization (for both LLVM and JavaScript), but then explicitly turns off the JavaScript optimizer.

.. code-block:: bash

	./emcc -O2 --js-opts 0 -g4 tests/hello_world_loop.cpp

The result is code that can be more useful for debugging issues related to LLVM-optimized code:

.. code-block:: javascript

	function _main() {
		var label = 0;
		var $puts=_puts(((8)|0)); //@line 4 "tests/hello_world.c"
		return 1; //@line 5 "tests/hello_world.c"
	}	


	
.. _debugging-emscripten-specific-issues:

Emscripten-specific issues
==========================

Memory Alignment Issues
-----------------------

The :ref:`Emscripten memory representation <emscripten-memory-model>` assumes loads and stores are aligned. Performing a normal load or store on an unaligned address can fail. 

.. tip:: :ref:`SAFE_HEAP <debugging-SAFE-HEAP>` can be used to reveal memory alignment issues.

Generally it is best to avoid unaligned reads and writes — often they occur as the result of undefined behavior. In some cases, however, they are unavoidable — for example if the code to be ported reads an ``int`` from a packed structure in some pre-existing data format.

Emscripten supports unaligned reads and writes, but they will be much slower, and should be used only when absolutely necessary.  To force an unaligned read or write you can:

- Manually read individual bytes and reconstruct the full value
- Use the :c:type:`emscripten_align* <emscripten_align1_short>` typedefs, which define unaligned versions of the basic types (``short``, ``int``, ``float``, ``double``). All operations on those types are not fully aligned (use the ``1`` variants in most cases, which mean no alignment whatsoever). 


Function Pointer Issues
-----------------------

If you get an ``abort()`` from a function pointer call to ``nullFunc`` or ``b0`` or ``b1`` (possibly with an error message saying "incorrect function pointer"), the problem is that the function pointer was not found in the expected function pointer table when called. 

.. note:: ``nullFunc`` is the function used to populate empty index entries in the function pointer tables (``b0`` and ``b1`` are shorter names used for ``nullFunc`` in more optimized builds).  A function pointer to an invalid index will call this function, which simply calls ``abort()``. 

There are several possible causes:

- Your code is calling a function pointer that has been cast from another type (this is undefined behavior but it does happen in real-world code). In optimized Emscripten output, each function pointer type is stored in a separate table based on its original signature, so you *must* call a function pointer with that same signature to get the right behavior (see :ref:`portability-function-pointer-issues` in the code portability section for more information).
- Your code is calling a method on a ``NULL`` pointer or dereferencing 0. This sort of bug can be caused by any sort of coding error, but manifests as a function pointer error because the function can't be found in the expected table at runtime.

In order to debug these sorts of issues:

- Compile with ``-Werror``. This turns warnings into errors, which can be useful as some cases of undefined behavior would otherwise show warnings. 
- Use ``-s ASSERTIONS=2`` to get some useful information about the function pointer being called, and its type. 
- Look at the browser stack trace to see where the error occurs and which function should have been called. 
- Build with :ref:`SAFE_HEAP=1 <debugging-SAFE-HEAP>` and function pointer aliasing disabled (``ALIASING_FUNCTION_POINTERS=0``). This should make it impossible for a function pointer to be called with the wrong type without raising an error: ``-s SAFE_HEAP=1 -s ALIASING_FUNCTION_POINTERS=0``


Another function pointer issue is when the wrong function is called. :ref:`SAFE_HEAP=1 <debugging-SAFE-HEAP>` can help with this as it detects some possible errors with function table accesses. 

``ALIASING_FUNCTION_POINTERS=0`` is also useful because it ensures that calls to function pointer addresses in the wrong table result in clear errors. Without this setting such calls just execute whatever function is at the address, which can be much harder to debug.



Infinite loops
--------------

Infinite loops cause your page to hang. After a period the browser will notify the user that the page is stuck and offer to halt or close it.

If your code hits an infinite loop, one easy way to find the problem code is to use a *JavaScript profiler*. In the Firefox profiler, if the code enters an infinite loop you will see a block of code doing the same thing repeatedly near the end of the profile.

.. note:: The :ref:`emscripten-runtime-environment-main-loop` may need to be re-coded if your application uses an infinite main loop.



.. _debugging-autodebugger:

AutoDebugger
============

The *AutoDebugger* is the 'nuclear option' for debugging Emscripten code.

.. warning:: This option is primarily intended for Emscripten core developers.

The *AutoDebugger* will rewrite the LLVM bitcode so it prints out each store to memory. This is useful because you can compare the output for different compiler settings in order to detect regressions, or compare the output of JavaScript and LLVM bitcode compiled using :term:`LLVM Nativizer` or :term:`LLVM interpreter`.

The *AutoDebugger* can potentially find **any** problem in the generated code, so it is strictly more powerful than the ``CHECK_*`` settings and ``SAFE_HEAP``. One use of the *AutoDebugger* is to quickly emit lots of logging output, which can then be reviewed for odd behavior. The *AutoDebugger* is also particularly useful for :ref:`debugging regressions <debugging-autodebugger-regressions>`.

The *AutoDebugger* has some limitations:

-  It generates a lot of output. Using *diff* can be very helpful for identifying changes.
-  It prints out simple numerical values rather than pointer addresses (because pointer addresses change between runs, and hence can't be compared). This is a limitation because sometimes inspection of addresses can show errors where the pointer address is 0 or impossibly large. It is possible to modify the tool to print out addresses as integers in ``tools/autodebugger.py``.

To run the *AutoDebugger*, compile with the environment variable ``EMCC_AUTODEBUG=1`` set. For example:

.. code-block:: bash

	# Linux or Mac OS X
	EMCC_AUTODEBUG=1 ./emcc tests/hello_world.cpp -o hello.html

	# Windows
	set EMCC_AUTODEBUG=1 
	emcc tests/hello_world.cpp -o hello.html
	set EMCC_AUTODEBUG=0 


.. _debugging-autodebugger-regressions:

AutoDebugger Regression Workflow
---------------------------------

Use the following workflow to find regressions with the *AutoDebugger*:

- Compile the working code with ``EMCC_AUTODEBUG=1`` set in the environment.
- Compile the code using ``EMCC_AUTODEBUG=1`` in the environment again, but this time with the settings that cause the regression. Following this step we have one build before the regression and one after.
- Run both versions of the compiled code and save their output. 
- Compare the output using a *diff* tool.

Any difference between the outputs is likely to be caused by the bug. 

.. note:: False positives can be caused by calls to ``clock()``, which will differ slightly between runs.

You can also make native builds using the :term:`LLVM Nativizer` tool. This can be run on the autodebugged **.ll** file, which will be emitted in ``/tmp/emscripten_temp`` when ``EMCC_DEBUG=1`` is set. 

.. note:: 

	- The native build created using the :term:`LLVM Nativizer` will use native system libraries. Direct comparisons of output with Emscripten-compiled code can therefore be misleading.
	- Attempting to interpret code compiled with ``-g`` using the *LLVM Nativizer* or :term:`lli` may crash, so you may need to build once without ``-g`` for these tools, then build again with ``-g``. Another option is to use `tools/exec_llvm.py <https://github.com/kripken/emscripten/blob/master/tools/exec_llvm.py>`_ in Emscripten, which will run *lli* after cleaning out debug info.


Useful Links
============

- `Blogpost about reading compiler output <http://mozakai.blogspot.com/2014/06/looking-through-emscripten-output.html>`_.
- `GDC 2014: Getting started with asm.js and Emscripten <http://people.mozilla.org/~lwagner/gdc-pres/gdc-2014.html#/20>`_ (Debugging slides).

Need help?
==========

The :ref:`Emscripten Test Suite <emscripten-test-suite>` contains good examples of almost all functionality offered by Emscripten. If you have a problem, it is a good idea to search the suite to determine whether test code with similar behavior is able to run.

If you've tried the ideas here and you need more help, please :ref:`contact`.

