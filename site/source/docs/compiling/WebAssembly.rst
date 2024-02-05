.. _WebAssembly:

=======================
Building to WebAssembly
=======================

WebAssembly is a binary format for executing code on the web, allowing fast start times (smaller download and much faster parsing in browsers when compared to JS or asm.js). Emscripten compiles to WebAssembly by default, but you can also compile to JS for older browsers.

For some historical background, see `these slides <https://kripken.github.io/talks/wasm.html>`_ and `this blogpost <https://hacks.mozilla.org/2015/12/compiling-to-webassembly-its-happening/>`_.

Setup
=====

WebAssembly is emitted by default, without the need for any special flags.

.. note:: If you **don't** want WebAssembly, you can disable it with something like

    ::

      emcc [..args..] -sWASM=0

.. note:: Deciding to compile to Wasm or JS can be done at the linking stage: it doesn't affect the object files.

Backends
--------

Emscripten emits WebAssembly using the **upstream LLVM Wasm backend**, since
version ``1.39.0`` (October 2019). Previously emscripten also supported the
old **fastcomp** backend which was removed in ``2.0.0`` (August 2020).

There are some differences you may notice between the two backends, if you
upgrade from fastcomp to upstream:

* The Wasm backend is strict about linking files with different features sets -
  for example, if one file was built with atomics but another was not, it will
  error at link time. This prevents possible bugs, but may mean you need to make
  some build system fixes.

* ``WASM=0`` behaves differently in the two backends. In fastcomp we emit
  asm.js, while in upstream we emit JS (since not all Wasm constructs can be
  expressed in asm.js). Also, the JS support implements the same external
  ``WebAssembly.*`` API, so in particular startup will be async just like Wasm
  by default, and you can control that with ``WASM_ASYNC_COMPILATION`` (even
  though ``WASM=0``).

* The Wasm backend uses Wasm object files by default. That means that it does
  codegen at the compile step, which makes the link step much faster - like a
  normal native compiler. For comparison, in fastcomp the compile step emits
  LLVM IR in object files.

  * You normally wouldn't notice this, but some compiler flags affect codegen,
    like ``DISABLE_EXCEPTION_CATCHING``. Such flags must be passed during
    codegen. The simple and safe thing is to pass all ``-s`` flags at both
    compile and link time.

  * You can enable Link Time Optimization (LTO) with the usual llvm flags
    (``-flto``, ``-flto=full``, ``-flto=thin``, at both compile and link times;
    note, however, that thin LTO is not heavily tested atm and so regular LTO
    is recommended).

  * With fastcomp, LTO optimization passes were not be run by default; for that
    it was necessary to pass ``--llvm-lto 1``.  With the llvm backend LTO passes
    will be run on any object files that are in bitcode format.

  * Another thing you might notice is that fastcomp's link stage is able to
    perform some minor types of link time optimization even without LTO being
    set. The LLVM backend requires actually setting LTO for those things.

* `wasm-ld`, the linker used by the Wasm backend, requires libraries (`.a`
  archives) to contain symbol indexes.  This matches the behaviour the native
  GNU linker.  While `emar` will create such indexes by default, native tools
  such as GNU `ar` and GNU `strip` are not aware of the WebAssembly object
  format and cannot create archive indexes.  In particular, if you run GNU
  `strip` on an archive file that contains WebAssembly object files it will
  remove the index which makes the archive unusable at link time.

* Also see the `blocker bugs on the Wasm backend <https://github.com/emscripten-core/emscripten/projects/1>`_, and the `Wasm backend tagged issues <https://github.com/emscripten-core/emscripten/issues?utf8=✓&q=is%3Aissue+is%3Aopen+label%3A"LLVM+wasm+backend">`_.

Trapping
========

WebAssembly can trap - throw an exception - on things like division by zero, rounding a very large float to an int, and so forth. In asm.js such things were silently ignored, as in JavaScript they do not throw, so this is a difference between JavaScript and WebAssembly that you may notice, with the browser reporting an error like ``float unrepresentable in integer range``, ``integer result unrepresentable``, ``integer overflow``, or ``Out of bounds Trunc operation``.

The LLVM Wasm backend avoids traps by adding more code around each possible trap (basically clamping the value if it would trap). This can increase code size and decrease speed, if you don't need that extra code. The proper solution for this is to use newer Wasm instructions that do not trap, by calling emcc or clang with ``-mnontrapping-fptoint``. That code may not run in older VMs, though.

Compiler output
===============

When using ``emcc`` to build to WebAssembly, you will see a ``.wasm`` file containing that code, as well as the usual ``.js`` file that is the main target of compilation. Those two are built to work together: run the ``.js`` (or ``.html``, if that's what you asked for) file, and it will load and set up the WebAssembly code for you, properly setting up imports and exports for it, etc. Basically, you don't need to care about whether the compiled code is asm.js or WebAssembly, it's just a compiler flag, and otherwise everything should just work (except the WebAssembly should be faster).

- Note that the ``.wasm`` file is not standalone - it's not easy to manually run it without that ``.js`` code, as it depends on getting the proper imports that integrate with JS. For example, it receives imports for syscalls so that it can do things like print to the console. There is work in progress towards ways to create standalone ``.wasm`` files, see the `WebAssembly Standalone page <https://github.com/emscripten-core/emscripten/wiki/WebAssembly-Standalone>`_.

You may also see additional files generated, like a ``.data`` file if you are preloading files into the virtual filesystem. All that is exactly the same as when building to asm.js. One difference you may notice is the lack of a ``.mem file``, which for asm.js contains the static memory initialization data, which in WebAssembly we can pack more efficiently into the WebAssembly binary itself.

WebAssembly support in browsers
===============================

WebAssembly is supported by all major browsers going back to Firefox 52, Chrome
57, Safari 11 and Opera 44.

For further info on WebAssembly features supported in various browsers, see the
`WebAssembly Roadmap <https://webassembly.org/roadmap/>`_

``.wasm`` files and compilation
===============================

WebAssembly code is prepared somewhat differently than asm.js. asm.js can be bundled inside the main JS file, while as mentioned earlier WebAssembly is a binary file on the side, so you will have more than one file to distribute.

Another noticeable effect is that WebAssembly is compiled asynchronously by default, which means you must wait for compilation to complete before calling compiled code (by waiting for ``main()``, or the ``onRuntimeInitialized`` callback, etc., which you also need to do when you have anything else that makes startup async, like a ``.mem`` file for asm.js, or preloaded file data, etc.). You can turn off async compilation by setting ``WASM_ASYNC_COMPILATION=0``, but that may not work in Chrome due to current limitations there.

- Note that even with async compilation turned off, fetching the WebAssembly binary may need to be an asynchronous operation (since the Web does not allow synchronous binary downloads on the main thread). If you can fetch the binary yourself, you can set ``Module['wasmBinary']`` and it will be used from there, and then (with async compilation off) compilation should be synchronous.

Web server setup
================

To serve Wasm in the most efficient way over the network, make sure your web server has the proper MIME type for ``.wasm`` files, which is application/wasm. That will allow streaming compilation, where the browser can start to compile code as it downloads.

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

