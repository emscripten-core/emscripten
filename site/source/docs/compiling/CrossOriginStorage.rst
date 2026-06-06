.. _CrossOriginStorage:

==========================
Cross-Origin Storage (COS)
==========================

.. note::
   This feature is **experimental**. The underlying `Cross-Origin Storage
   browser API <https://github.com/WICG/cross-origin-storage>`_ is a WICG
   proposal that has not yet shipped in any browser. Emscripten's support is
   provided as a progressive enhancement — the runtime falls back to the
   standard ``fetch()`` path automatically when the browser does not expose
   the API.

Overview
========

The **Cross-Origin Storage (COS)** API is a proposed browser standard that
allows web applications on different origins to share large cached files,
identified by their cryptographic hashes. A file stored in COS by one site
can be retrieved by any other site using the same hash, eliminating redundant
downloads.

Emscripten's ``-sCROSS_ORIGIN_STORAGE=1`` flag integrates this into the
standard Wasm loading path. At build time, Emscripten computes the SHA-256
hash of the final ``.wasm`` binary. At runtime, the generated JavaScript
tries to retrieve the compiled Wasm module from COS before falling back to
a normal network fetch. If the module is not yet in COS it is stored there
after download, making it available to other origins immediately.

This is particularly beneficial for popular Wasm modules that many sites
ship independently — game engines, scientific computing runtimes, and
frameworks such as Flutter or Pyodide — where users would otherwise download
the same bytes many times.

Usage
=====

Pass ``-sCROSS_ORIGIN_STORAGE=1`` at link time::

  emcc hello.cpp -o hello.js -sCROSS_ORIGIN_STORAGE=1

The flag is a **link-time** setting and has no effect during compilation of
individual object files.

Requirements and restrictions
------------------------------

- The flag only has an effect when the output targets the **web** environment.
  It is silently ignored for Node.js-only or shell targets (``-sENVIRONMENT=node``).
- It has no effect in **SINGLE_FILE** mode (``-sSINGLE_FILE``), because the
  Wasm binary is embedded inline as base64 and there is no standalone
  ``.wasm`` file to hash.
- The COS API is a progressive enhancement. Browsers without the API
  continue to load the Wasm module via the normal ``fetch()`` and
  ``WebAssembly.instantiateStreaming()`` path without any error.

How it works
============

Build time
----------

After all optimizations — including any ``wasm-opt`` passes run by Binaryen
— Emscripten reads the final ``.wasm`` binary and computes its SHA-256
digest. That digest is embedded in the generated JavaScript glue as a
build-time constant::

  var wasmHashValue = 'a3f2...c9d1';  // 64 hex characters

No extra files are produced; the hash is part of the regular ``.js`` output.

Runtime (web only)
------------------

When the page loads, the generated JavaScript follows this logic:

1. **Feature detection** — check ``'crossOriginStorage' in navigator``.
   If the API is absent, skip to the normal fetch path immediately.

2. **Cache hit** — call
   ``navigator.crossOriginStorage.requestFileHandles([{algorithm: 'SHA-256', value: wasmHashValue}])``.
   If the handle is returned (the module is already in COS), read it with
   ``handle.getFile()`` → ``.arrayBuffer()`` and pass the bytes to
   ``WebAssembly.instantiate()``.

3. **Cache miss** — if a ``NotFoundError`` is thrown, fetch the ``.wasm``
   over the network as usual, call ``WebAssembly.instantiate()`` immediately
   so the page loads without delay, and then write the bytes into COS in the
   background (fire-and-forget) with ``origins: '*'`` so any other origin
   can benefit::

     navigator.crossOriginStorage.requestFileHandles([hash], { create: true, origins: '*' })

4. **Fallback** — any unexpected error (``NotAllowedError`` from the browser,
   network failure during the miss path, etc.) is logged with ``err()`` and
   the runtime falls through to the standard streaming-instantiation path
   below. The page always loads.

Testing with the extension polyfill
====================================

Because no browser ships the COS API natively yet, you can experiment using
the `Cross-Origin Storage extension
<https://chromewebstore.google.com/detail/cross-origin-storage/denpnpcgjgikjpoglpjefakmdcbmlgih>`_,
which injects a ``navigator.crossOriginStorage`` polyfill on every page.

1. Install the extension in Chrome.
2. Build your project with ``-sCROSS_ORIGIN_STORAGE=1 -sENVIRONMENT=web``.
3. Serve the output over HTTP (e.g. with ``emrun`` or ``python3 -m http.server``).
4. Open the page — on the first load the Wasm binary is fetched and stored in
   COS. Open the same page in a second tab or from a different origin: the
   module is loaded from COS without a network request.

Verifying the embedded hash
============================

You can confirm that the hash embedded in the ``.js`` output matches the
actual ``.wasm`` file using standard tools:

.. code-block:: bash

   # SHA-256 of the wasm file
   sha256sum hello.wasm

   # Extract the hash embedded in the JS
   grep -oP "value: '\K[0-9a-f]{64}" hello.js

Both values must be identical. The Emscripten test suite checks this
automatically via ``test_cross_origin_storage_js_output`` in
``test/test_other.py``.

Relationship to other caching mechanisms
==========================================

COS is a complement to, not a replacement for, existing browser caches:

- **HTTP cache / Service Worker cache** — still used for per-origin caching.
  COS adds cross-origin sharing on top.
- **``NODE_CODE_CACHING``** — a Node.js-specific V8 bytecode cache; unrelated
  to COS.
- **IndexedDB / OPFS** — per-origin storage; COS shares across origins.

See also
========

- `WICG Cross-Origin Storage explainer <https://github.com/WICG/cross-origin-storage>`_
- `COS browser extension (polyfill) <https://github.com/web-ai-community/cross-origin-storage-extension>`_
- :ref:`settings-reference` — ``CROSS_ORIGIN_STORAGE`` entry
- :ref:`WebAssembly` — general guide to building Wasm with Emscripten
