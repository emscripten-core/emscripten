.. _WebAssembly:

=======================
Building to WebAssembly
=======================

WebAssembly is a new binary format for executing code on the web, allowing much faster start times (smaller download, much faster parsing in browsers) for Emscripten projects. Emscripten supports compiling to WebAssembly with a compiler flag, so it is easy for projects to target both WebAssembly and asm.js.

For more background, see

- `these slides <https://kripken.github.io/talks/wasm.html>`_ and
- `this blogpost <https://hacks.mozilla.org/2015/12/compiling-to-webassembly-its-happening/>`_.

Setup
=====

WebAssembly is emitted by default, without the need for any special flags.

.. note:: If you **don't** want WebAssembly, you can disable it with something like

::

  emcc [..args..] -s WASM=0

.. note:: Emscripten's WebAssembly support depends on `Binaryen <https://github.com/WebAssembly/binaryen>`_, which will be automatically fetched and built for you (you may see logging about that, the first time you compile to WebAssembly).
.. note:: The ``WASM``, ``BINARYEN*``, etc. options only matter when compiling to your final executable. In other words, the same .o files are used for both asm.js and WebAssembly. Only when linking them and compiling to asm.js or WebAssembly do you need to specify WebAssembly if you want that. That means that it is easy to build your project to both asm.js and WebAssembly.

Backends
--------

Emscripten can currently (April 2019) use 2 backends to generate WebAssembly: **fastcomp** (the asm.js backend, together with asm2wasm) and the **upstream LLVM wasm backend**.

Fastcomp is currently the default, but we hope to switch the default soon to the upstream backend.

To use fastcomp, just use the emsdk normally to get ``latest``. For the upstream backend, you can use ``latest-upstream`` for now on Linux, until we finish setting up builders, or you can set LLVM in the ``.emscripten`` file to point to a build you make of very recent LLVM (preferably from git/svn master).

Binaryen codegen options
========================

Trapping
--------

WebAssembly can trap - throw an exception - on things like division by zero, rounding a very large float to an int, and so forth. In asm.js such things were silently ignored, as in JavaScript they do not throw, so this is a difference between JavaScript and WebAssembly that you may notice, with the browser reporting an error like ``float unrepresentable in integer range``, ``integer result unrepresentable``, ``integer overflow``, or ``Out of bounds Trunc operation``.


Fastcomp/asm2wasm
~~~~~~~~~~~~~~~~~

In fastcomp/asm2wasm, emscripten will emit code that is optimized for size and speed, which means it emits code that may trap on the things mentioned before. That mode is called ``allow``. The other modes are ``clamp``, which will avoid traps by clamping values to a reasonable range, and ``js``, which ensures the exact same behavior as JavaScript does (which also does clamping, but makes sure to clamp exactly like JavaScript does, and also do other things JavaScript would).

In general, using ``clamp`` is safest, as whether such a trap occurs depends on how the LLVM optimizer optimizes code. In other words, there is no guarantee that this will not be an issue, and updating LLVM can make a problem appear or vanish (the wasm spec process has recognized this problem and intends to standardize `new operations that avoid it <https://github.com/WebAssembly/design/issues/1143>`_). Also, there is not a big downside to using ``clamp``: it is only slightly larger and slower than the default ``allow``, in most cases. To do so, build with

 ::

  -s "BINARYEN_TRAP_MODE='clamp'"


However, if the default (to allow traps) works in your codebase, then it may be worth keeping it that way, for the (small) benefits. Note that ``js``, which preserves the exact same behavior as JavaScript does, adds a large amount of overhead, so unless you really need that, use ``clamp`` (``js`` is often useful for debugging, though).

LLVM wasm backend
~~~~~~~~~~~~~~~~~

The LLVM wasm backend avoids traps by adding more code around each possible trap (basically clamping the value if it would trap). This can increase code size and decrease speed, if you don't need that extra code. The proper solution for this is to use newer wasm instructions that do not trap, by calling emcc or clang with ``-mnontrapping-fptoint``. That code may not run in older VMs, though.

Compiler output
===============

When using ``emcc`` to build to WebAssembly, you will see a ``.wasm`` file containing that code, as well as the usual ``.js`` file that is the main target of compilation. Those two are built to work together: run the ``.js`` (or ``.html``, if that's what you asked for) file, and it will load and set up the WebAssembly code for you, properly setting up imports and exports for it, etc. Basically, you don't need to care about whether the compiled code is asm.js or WebAssembly, it's just a compiler flag, and otherwise everything should just work (except the WebAssembly should be faster).

- Note that the ``.wasm`` file is not standalone - it's not easy to manually run it without that ``.js`` code, as it depends on getting the proper imports that integrate with JS. For example, it receives imports for syscalls so that it can do things like print to the console. There is work in progress towards ways to create standalone ``.wasm`` files, see the `WebAssembly Standalone page <https://github.com/emscripten-core/emscripten/wiki/WebAssembly-Standalone>`_.

You may also see additional files generated, like a ``.data`` file if you are preloading files into the virtual filesystem. All that is exactly the same as when building to asm.js. One difference you may notice is the lack of a ``.mem file``, which for asm.js contains the static memory initialization data, which in WebAssembly we can pack more efficiently into the WebAssembly binary itself.

Testing native WebAssembly in browsers
======================================

WebAssembly support is enabled by default as of Firefox 52, Chrome 57 and Opera 44. On Edge 15 you can enable it via "Experimental JavaScript Features" flag.

Debugging
=========

asm.js support is considered very stable now, and you can change between it and wasm with ``-s WASM=1``, so if you see something odd in a wasm build, comparing to a parallel asm.js build can help. In general, any difference between the two could be a compiler bug or browser bug, but there are a few legitimate causes of different behavior between the two, that you may want to rule out:

- wasm allows unaligned accesses, i.e. it will load 4 bytes from an unaligned address the same way x86 does (it doesn't care it's unaligned). asm.js works more like ARM CPUs which mostly don't accept such things (but they often trap, while asm.js just returns a wrong result). To rule this out, you can build with ``-s SAFE_HEAP=1``, that will catch all such invalid accesses.
- Timing issues - wasm might run faster or slower. To some extent you can mitigate that by building with ``-s DETERMINISTIC=1``.
- Trap mode. As mentioned above, we can generate wasm that traps or that avoids traps. Make sure the trap mode is ``"js"`` when comparing builds. The ``"js"`` trap mode is also useful in a single build, as otherwise operations like division or float-to-int may trap, and the optimizer may happen to change whether a trap occurs or not, which can be confusing (for example, enabling ``SAFE_HEAP`` may prevent some optimizations, and a trap may start to occur). Instead, in the ``"js"`` trap mode there are no traps and all operations are deterministically defined as identical to JavaScript.
- Minor libc and runtime differences exist between wasm and asm.js. We used to have a way to emit more compatable builds (``-s "BINARYEN_METHOD='asmjs,native-wasm'"`` etc.) but due to its complexity and low value it was removed.
- Floating-point behavior: WebAssembly uses 32-bit floats in a standard way, while asm.js by default implements floats using doubles. That can lead to differences in the precision of results. You can force 32-bit float behavior in asm.js with ``-s PRECISE_F32=1``, in which case it should be identical to wasm.
- Browser instability: It's worth testing multiple browsers, as one might have a wasm bug that another doesn't. You can also test the Binaryen interpreter (e.g. using the ``interpret-binary`` method, as discussed above).

If you find that an asm.js build has the same behavior as a wasm one, then it is currently easier to debug the asm.js build: you can edit the source easily (add debug printouts, etc.), there is debug info and source maps support, etc.

Debugging WebAssembly
---------------------

When you do need to debug a WebAssembly build, the following tips might help you.

WebAssembly doesn't have source maps support yet, but building with ``-g`` will emit both a text and a binary wasm, and it will include function names in both, and also include source file and line number information in the text, for example, building hello world might have this in the ``.wast``:

.. code-block:: none

    ;; tests/hello_world.c:4
    (drop
      (call $_printf
        (i32.const 1144)
        (get_local $$vararg_buffer)
      )
    )
    ;; tests/hello_world.c:5
    (return
      (i32.const 0)
    )

This indicates that the ``printf`` call comes from line 4, and the return from line 5, of ``hello_world.c``.

``.wasm`` files and compilation
===============================

WebAssembly code is prepared somewhat differently than asm.js. asm.js can be bundled inside the main JS file, while as mentioned earlier WebAssembly is a binary file on the side, so you will have more than one file to distribute.

Another noticeable effect is that WebAssembly is compiled asynchronously by default, which means you must wait for compilation to complete before calling compiled code (by waiting for ``main()``, or the ``onRuntimeInitialized`` callback, etc., which you also need to do when you have anything else that makes startup async, like a ``.mem`` file for asm.js, or preloaded file data, etc.). You can turn off async compilation by setting ``BINARYEN_ASYNC_COMPILATION=0``, but that may not work in Chrome due to current limitations there.

- Note that even with async compilation turned off, fetching the WebAssembly binary may need to be an asynchronous operation (since the Web does not allow synchronous binary downloads on the main thread). If you can fetch the binary yourself, you can set ``Module['wasmBinary']`` and it will be used from there, and then (with async compilation off) compilation should be synchronous.

Web server setup
================

To serve wasm in the most efficient way over the network, make sure your web server has the proper MIME time for ``.wasm`` files, which is application/wasm. That will allow streaming compilation, where the browser can start to compile code as it downloads.

In Apache, you can do this with

.. code-block:: none

    AddType application/wasm .wasm

Also make sure that gzip is enabled:

.. code-block:: none

    AddOutputFilterByType DEFLATE application/wasm

If you serve large ``.wasm`` files, the webserver will consume CPU compressing them on the fly at each request.
Instead you can pre-compress them to ``.wasm.gz`` and use content negotiation:

.. code-block:: none

    Options Multiviews
    RemoveType .gz
    AddEncoding x-gzip .gz
    AddType application/wasm .wasm

