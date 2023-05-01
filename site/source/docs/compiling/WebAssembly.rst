.. _WebAssembly:

=======================
Building to WebAssembly
=======================

WebAssembly is a binary format for executing code on the web, allowing fast start times (smaller download and much faster parsing in browsers when compared to JS or asm.js). Emscripten compiles to WebAssembly by default, but you can also compile to JS for older browsers.

For some historical background, see `these slides <https://kripken.github.io/talks/wasm.html>`_ and `this blogpost <https://hacks.mozilla.org/2015/12/compiling-to-webassembly-its-happening/>`_.

Setup
=====

WebAssembly is emitted by default, without the need for any special flags.

.. note:: If you want JS instead of WebAssembly, you can disable it with something like

    ::

      emcc [..args..] -sWASM=0

.. note:: Emscripten's JS support depends on `Binaryen <https://github.com/WebAssembly/binaryen>`_, which is provided by the emsdk (if you don't use the emsdk, you need to build it and set it up in your ``.emscripten`` file).
.. note:: Deciding to compile to wasm or JS can be done at the linking stage: it doesn't affect the object files.

Compiler output
===============

When using ``emcc`` to build to WebAssembly, you will see both a ``.wasm`` file containing that code, as well as a ``.js`` file that is the main target of compilation. Those two are built to work together: run the ``.js`` (or ``.html``, if that's what you asked for) file, and it will load and set up the WebAssembly code for you, properly setting up imports and exports for it, etc. Basically, you don't need to care about whether the compiled code is asm.js or WebAssembly, it's just a compiler flag, and otherwise everything should just work (except the WebAssembly should be faster).

- Note that the ``.wasm`` file is not standalone - it's not easy to manually run it without that ``.js`` code, as it depends on getting the proper imports that integrate with JS. For example, it receives imports for syscalls so that it can do things like print to the console. There is work in progress towards ways to create standalone ``.wasm`` files, see the `WebAssembly Standalone page <https://github.com/emscripten-core/emscripten/wiki/WebAssembly-Standalone>`_.

You may also see additional files generated, like a ``.data`` file if you are preloading files into the virtual filesystem. All that is exactly the same as when building to asm.js. One difference you may notice is the lack of a ``.mem file``, which for asm.js contains the static memory initialization data, which in WebAssembly we can pack more efficiently into the WebAssembly binary itself.

WebAssembly Features and Support
================================

WebAssembly support is enabled by default as of Firefox 52, Chrome 57, Safari 11, Edge 16, and Opera 44.

For further info on WebAssembly features supported in various browsers, see the `WebAssembly Roadmap <https://webassembly.org/roadmap/>`_.

Here are some useful WebAssembly features you may want to enable, depending on whether you are targeting VMs that support them:

 - **Bulk Memory** (``-mbulk-memory``): Reduces code size and improves
   performance for memory copies and fills. Enabled automatically by
   ``-pthread`` or ``-sUSE_PTHREADS``.

 - **Nontrapping Float-to-Int** (``-mnontrapping-fptoint``): Reduces code size
   and improves performance for float-to-int conversions.

 - **Tail Call** (``-mtail-call``): Enables tail call optimizations, potentially
   saving stack space. This is also necessary to properly support C++20
   coroutining.

 - **SIMD** (``-msimd128``): Enables SIMD instructions and autovectorization
   optimizations. See :ref:`Porting SIMD <porting-simd>` for more information.

Additionally, clang and emcc enable the Sign Extension and Mutable Globals features by default. For a complete list of available WebAssembly feature flags, see the `Clang documentation <https://clang.llvm.org/docs/ClangCommandLineReference.html#webassembly>`_.

``.wasm`` files and compilation
===============================

WebAssembly code is prepared somewhat differently than asm.js. asm.js can be bundled inside the main JS file, while as mentioned earlier WebAssembly is a binary file on the side, so you will have more than one file to distribute.

Another noticeable effect is that WebAssembly is compiled asynchronously by default, which means you must wait for compilation to complete before calling compiled code (by waiting for ``main()``, or the ``onRuntimeInitialized`` callback, etc., which you also need to do when you have anything else that makes startup async, like a ``.mem`` file for asm.js, or preloaded file data, etc.). You can turn off async compilation by setting ``WASM_ASYNC_COMPILATION=0``, but that may not work in Chrome due to current limitations there.

- Note that even with async compilation turned off, fetching the WebAssembly binary may need to be an asynchronous operation (since the Web does not allow synchronous binary downloads on the main thread). If you can fetch the binary yourself, you can set ``Module['wasmBinary']`` and it will be used from there, and then (with async compilation off) compilation should be synchronous.

Web server setup
================

To serve wasm in the most efficient way over the network, make sure your web server has the proper MIME type for ``.wasm`` files, which is application/wasm. That will allow streaming compilation, where the browser can start to compile code as it downloads.

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
