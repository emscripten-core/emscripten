.. _Optimizing-Code:

===============
Optimizing Code
===============

Generally you should first compile and run your code without optimizations (the default). Once you are sure that the code runs correctly, you can use the techniques in this article to make it load and run faster.

How to optimize code
====================

Code is optimized by specifying :ref:`optimization flags <emcc-compiler-optimization-options>` when running :ref:`emcc <emccdoc>`. The levels include: :ref:`-O0 <emcc-O0>` (no optimization), :ref:`-O1 <emcc-O1>`, :ref:`-O2 <emcc-O2>`, :ref:`-Os <emcc-Os>`, :ref:`-Oz <emcc-Oz>`, and :ref:`-O3 <emcc-O3>`.

For example, to compile with optimization level ``-O2``:

.. code-block:: bash

  emcc -O2 file.cpp

The higher optimization levels introduce progressively more aggressive optimization, resulting in improved performance and code size at the cost of increased compilation time. The levels can also highlight different issues related to undefined behavior in code.

The optimization level you should use depends mostly on the current stage of development:

- When first porting code, run *emcc* on your code using the default settings (without optimization). Check that your code works and :ref:`debug <Debugging>` and fix any issues before continuing.
- Build with lower optimization levels during development for a shorter compile/test iteration cycle (``-O0`` or ``-O1``).
- Build with ``-O2`` to get a well-optimized build.
- Building with ``-O3`` or ``-Os`` can produce an ever better build than ``-O2``, and are worth considering for release builds. ``-O3`` builds are even more optimized than ``-O2``, but at the cost of significantly longer compilation time and potentially larger code size. ``-Os`` is similar in increasing compile times, but focuses on reducing code size while doing additional optimization. It's worth trying these different optimization options to see what works best for your application.
- Other optimizations are discussed in the following sections.

In addition to the ``-Ox`` options, there are separate compiler options that can be used to control the JavaScript optimizer (:ref:`js-opts <emcc-js-opts>`), LLVM optimizations (:ref:`llvm-opts <emcc-llvm-opts>`) and LLVM link-time optimizations (:ref:`llvm-lto <emcc-llvm-lto>`).

.. note::

  -  The meanings of the *emcc* optimization flags (``-O1, -O2`` etc.) are similar to *gcc*, *clang*, and other compilers, but also different because optimizing asm.js and WebAssembly includes some additional types of optimizations. The mapping of the *emcc* levels to the LLVM bitcode optimization levels is documented in the reference.


Advanced compiler settings
==========================

There are several flags you can :ref:`pass to the compiler <emcc-s-option-value>` to affect code generation, which will also affect performance — for example :ref:`DISABLE_EXCEPTION_CATCHING <optimizing-code-exception-catching>`. These are documented in `src/settings.js <https://github.com/emscripten-core/emscripten/blob/master/src/settings.js>`_. Some of these will be directly affected by the optimization settings (you can find out which ones by searching for ``apply_opt_level`` in `tools/shared.py <https://github.com/emscripten-core/emscripten/blob/1.29.12/tools/shared.py#L958>`_).

WebAssembly
===========

Emscripten will emit WebAssembly by default. You can switch that off with ``-s WASM=0`` (and then emscripten emits asm.js), which is necessary if you want the output to run in places where wasm support is not present yet, but the downside is larger and slower code.

.. _optimizing-code-size:

Code size
=========

This section describes optimisations and issues that are relevant to code size. They are useful both for small projects or libraries where you want the smallest footprint you can achieve, and in large projects where the sheer size may cause issues (like slow startup speed) that you want to avoid.

.. _optimizing-code-oz-os:

Trading off code size and performance
-------------------------------------

You may wish to build the less performance-sensitive source files in your project using :ref:`-Os <emcc-Os>` or :ref:`-Oz <emcc-Oz>` and the remainder using :ref:`-O2 <emcc-O2>` (:ref:`-Os <emcc-Os>` and :ref:`-Oz <emcc-Oz>` are similar to :ref:`-O2 <emcc-O2>`, but reduce code size at the expense of performance. :ref:`-Oz <emcc-Oz>` reduces code size more than :ref:`-Os <emcc-Os>`.)

Separately, you can do the final link/build command with ``-Os`` or ``-Oz`` to make the compiler focus more on code size when generating WebAssembly/asm.js.

Miscellaneous code size tips
----------------------------

In addition to the above, the following tips can help to reduce code size:

- Use :ref:`the closure compiler <emcc-closure>` on the non-compiled code: ``--closure 1``. This can hugely reduce the size of the support JavaScript code, and is highly recommended. However, if you add your own additional JavaScript code (in a ``--pre-js``, for example) then you need to make sure it uses `closure annotations properly <https://developers.google.com/closure/compiler/docs/api-tutorial3>`_.
- `Floh's blogpost on this topic <http://floooh.github.io/2016/08/27/asmjs-diet.html>`_ is very helpful.
- Use :ref:`llvm-lto <emcc-llvm-lto>` when compiling from bitcode to JavaScript: ``--llvm-lto 1``. This can break some code as the LTO code path is less tested.
- Make sure to use gzip compression on your webserver, which all browsers now support.
- You can move some of your code into the `Emterpreter <https://github.com/emscripten-core/emscripten/wiki/Emterpreter>`_, which will then run much slower (as it is interpreted), but it will transfer all that code into a smaller amount of data.

The following compiler settings can help (see ``src/settings.js`` for more details):

- Disable inlining when possible, using ``-s INLINING_LIMIT=1``. Compiling with -Os or -Oz generally avoids inlining too. (Inlining can make code faster, though, so use this carefully.)
- You can use the ``-s FILESYSTEM=0`` option to disable bundling of filesystem support code (the compiler should optimize it out if not used, but may not always succeed). This can be useful if you are building a pure computational library, for example.
- The ``ENVIRONMENT`` flag lets you specify that the output will only run on the web, or only run in node.js, etc. This prevents the compiler from emitting code to support all possible runtime environments, saving ~2KB.
- You can use ``ELIMINATE_DUPLICATE_FUNCTIONS`` to remove duplicate functions, which C++ templates often create. (This is already done by default for wasm, in ``-O1`` and above.)

Very large codebases
====================

The previous section on reducing code size can be helpful on very large codebases. In addition, here are some other topics that might be useful.

.. _optimizing-code-separating_asm:

Avoid memory spikes by separating out asm.js
--------------------------------------------

When emitting asm.js, by default Emscripten emits one JS file, containing the entire codebase: Both the asm.js code that was compiled, and the general code that sets up the environment, connects to browser APIs, etc. in a very large codebase, this can be inefficient in terms of memory usage, as having all of that in one script means the JS engine might use some memory to parse and compile the asm.js, and might not free it before starting to run the codebase. And in a large game, starting to run the code might allocate a large typed array for memory, so you might see a "spike" of memory, after which temporary compilation memory will be freed. And if big enough, that spike can cause the browser to run out of memory and fail to load the application. This is a known problem on `Chrome <https://code.google.com/p/v8/issues/detail?id=4392>`_ (other browsers do not seem to have this issue).

A workaround is to separate out the asm.js into another file, and to make sure that the browser has a turn of the event loop between compiling the asm.js module and starting to run the application. This can be achieved by running **emcc** with ``--separate-asm``.

You can also do this manually, as follows:

 * Run ``tools/separate_asm.py``. This receives as inputs the filename of the full project, and two filenames to emit: the asm.js file and a file for everything else.
 * Load the asm.js script first, then after a turn of the event loop, the other one, for example using code like this in your HTML file: ::

    var script = document.createElement('script');
    script.src = "the_asm.js";
    script.onload = function() {
      setTimeout(function() {
        var script = document.createElement('script');
        script.src = "the_rest.js";
        document.body.appendChild(script);
      }, 1); // delaying even 1ms is enough
    };
    document.body.appendChild(script);

.. _optimizing-code-outlining:

Running by itself
-----------------

If you hit memory limits in browsers, it can help to run your project by itself, as opposed to inside a web page containing other content. If you open a new web page (as a new tab, or a new window) that contains just your project, then you have the best chance at avoiding memory fragmentation issues.


Outlining
---------

JavaScript engines will often compile very large functions slowly (relative to their size), and fail to optimize them effectively (or at all). One approach to this problem is to use "outlining": breaking them into smaller functions that can be compiled and optimized more effectively.

Outlining increases overall code size, and can itself make some code less optimised. Despite this, outlining can sometimes improve both startup and runtime speed. For more information read `Outlining: a workaround for JITs and big functions <http://mozakai.blogspot.com/2013/08/outlining-workaround-for-jits-and-big.html>`_.

The ``OUTLINING_LIMIT`` setting defines the function size at which Emscripten will try to break large functions into smaller ones. Search for this setting in `settings.js <https://github.com/emscripten-core/emscripten/blob/master/src/settings.js>`_ for information on how to determine what functions may need to be outlined and how to choose an appropriate function size.

.. note:: This setting is useful for asm.js, but generally not helpful for WebAssembly.

.. _optimizing-code-aggressive-variable-elimination:

Aggressive variable elimination
-------------------------------

Aggressive variable elimination attempts to remove variables whenever possible, even at the cost of increasing code size by duplicating expressions. This can improve speed in cases where you have extremely large functions. For example it can make sqlite (which has a huge interpreter loop with thousands of lines in it) 7% faster.

You can enable aggressive variable elimination with ``-s AGGRESSIVE_VARIABLE_ELIMINATION=1``.

.. note:: This setting is useful for asm.js, but generally not helpful for WebAssembly.

.. note:: This setting can be harmful in some cases. Test before using it.


Other optimization issues
=========================

.. _optimizing-code-exception-catching:

C++ exceptions
--------------

Catching C++ exceptions (specifically, emitting catch blocks) is turned off by default in ``-O1`` (and above). Due to how asm.js/wasm currently implement exceptions, this makes the code much smaller and faster (eventually, wasm should gain native support for exceptions, and not have this issue).

To re-enable exceptions in optimized code, run *emcc* with ``-s DISABLE_EXCEPTION_CATCHING=0`` (see `src/settings.js <https://github.com/emscripten-core/emscripten/blob/master/src/settings.js>`_).

.. note:: When exception catching is disabled, a thrown exception terminates the application. In other words, an exception is still thrown, but it isn't caught.

.. note:: Even with catch blocks not being emitted, there is some code size overhead unless you build your source files with ``-fno-exceptions``, which will omit all exceptions support code (for example, it will avoid creating proper C++ exception objects in errors in std::vector, and just abort the application if they occur)

C++ RTTI
--------

C++ run-time type info support (dynamic casts, etc.) adds overhead that is sometimes not needed. For example, in Box2D neither rtti nor exceptions are needed, and if you build the source files with ``-fno-rtti -fno-exceptions`` then it shrinks the output by 15% (!).

Memory Growth
-------------

Building with ``-s ALLOW_MEMORY_GROWTH=1`` allows the total amount of memory used to change depending on the demands of the application. This is useful for apps that don't know ahead of time how much they will need, but it disables asm.js optimizations. In WebAssembly, however, there should be little or no overhead.

Viewing code optimization passes
--------------------------------

Enable :ref:`debugging-EMCC_DEBUG` to output files for each compilation phase, including the main optimization operations.

.. _optimizing-code-unsafe-optimisations:

Unsafe optimizations
====================

A few **UNSAFE** optimizations you might want to try are:

- ``--closure 1``: This can help with reducing the size of the non-generated (support/glue) JS code, and with startup. However it can break if you do not do proper :term:`Closure Compiler` annotations and exports. But it's worth it!
- ``--llvm-lto 1``: This enables LLVM's link-time optimizations, which can help in some cases. However there are known issues with these optimizations, so code must be extensively tested. See :ref:`llvm-lto <emcc-llvm-lto>` for information about the other modes.

.. _optimizing-code-profiling:

Profiling
=========

Modern browsers have JavaScript profilers that can help find the slower parts in your code. As each browser's profiler has limitations, profiling in multiple browsers is highly recommended.

To ensure that compiled code contains enough information for profiling, build your project with :ref:`profiling <emcc-profiling>` as well as optimization and other flags:

.. code-block:: bash

  emcc -O2 --profiling file.cpp


Troubleshooting poor performance
================================

Emscripten-compiled code can currently achieve approximately half the speed of a native build. If the performance is significantly poorer than expected, you can also run through the additional troubleshooting steps below:

-  :ref:`Building-Projects` is a two-stage process: compiling source code files to LLVM **and** generating JavaScript from LLVM. Did you build using the same optimization values in **both** steps (``-O2`` or ``-O3``)?
-  Test on multiple browsers. If performance is acceptable on one browser and significantly poorer on another, then :ref:`file a bug report <bug-reports>`, noting the problem browser and other relevant information.
- Does the code *validate* in Firefox (look for "Successfully compiled asm.js code" in the web console). If you see a validation error when using an up-to-date version of Firefox and Emscripten then please :ref:`file a bug report <bug-reports>`.

