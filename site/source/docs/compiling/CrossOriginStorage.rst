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

When to use this flag
---------------------

COS only delivers a benefit when the ``.wasm`` binary is **byte-identical
across many different origins** — that is, a popular library whose compiled
binary is loaded by many independent sites. If every visitor to every site
downloads the exact same bytes, COS means they only download it once, ever.

Good candidates are libraries or toolkits that are:

- popular enough that many independent sites load the same binary,
- distributed as a stable, version-pinned ``.wasm`` file, and
- a **single primary** ``.wasm`` file (COS only covers the binary that
  Emscripten compiles; any additional Wasm files loaded at runtime are not
  covered).

**Do not** enable this flag for application-specific Wasm code built for
your own site. That binary is unique to you; no other origin will ever have
the same hash, so it will never get a COS cache hit. The normal HTTP cache
already handles per-origin caching efficiently.

Usage
=====

Pass ``-sCROSS_ORIGIN_STORAGE=1`` at link time::

  emcc hello.cpp -o hello.js -sCROSS_ORIGIN_STORAGE=1

The flag is a **link-time** setting and has no effect during compilation of
individual object files.

Controlling which origins can read the cached file
--------------------------------------------------

The ``-sCROSS_ORIGIN_STORAGE_ORIGINS`` setting controls the ``origins`` field
passed to ``requestFileHandles()`` on the write (cache-miss) path.  It has no
effect on the read (cache-hit) path.  Three modes are available:

**Globally available** (default, no explicit setting needed) — any origin
can retrieve the file.  This is applied automatically when
``-sCROSS_ORIGIN_STORAGE=1`` is used without specifying
``-sCROSS_ORIGIN_STORAGE_ORIGINS``:

.. code-block:: bash

   emcc hello.cpp -o hello.js -sCROSS_ORIGIN_STORAGE=1

Use this for popular binaries loaded by many independent origins.  This is
the recommended mode for resources where global COS cache hits are expected.

**Restricted to a specific set of origins** — only the listed origins can
retrieve the file:

.. code-block:: bash

   emcc hello.cpp -o hello.js \
       -sCROSS_ORIGIN_STORAGE=1 \
       '-sCROSS_ORIGIN_STORAGE_ORIGINS=["https://app.example.com","https://api.example.com"]'

Use this for proprietary resources shared across a controlled set of related
sites.  Each entry must be a valid serialised HTTPS origin (scheme + host +
optional port, no path).  Mixing ``'*'`` with explicit origins is a
**link-time error**.

**Same-site only** — pass an explicit empty list to omit the ``origins``
field, making the file available only to same-site origins:

.. code-block:: bash

   emcc hello.cpp -o hello.js \
       -sCROSS_ORIGIN_STORAGE=1 \
       -sCROSS_ORIGIN_STORAGE_ORIGINS=[]

Use this for resources that should be shared across subdomains of a single
site but not beyond.

.. note::
   The COS spec defines a **visibility upgrade** rule: a resource's
   availability can be widened but never narrowed.  If a resource is already
   stored as globally available (``'*'``), any subsequent attempt to store it
   with a more restrictive ``origins`` list is ignored by the browser.

Requirements and restrictions
------------------------------

- The flag emits a **warning** when the target environment does not include
  the web (``-sENVIRONMENT=node``, ``-sENVIRONMENT=shell``):
  ``navigator.crossOriginStorage`` is a browser API and is never available
  in those environments.
- It produces a **hard link-time error** in **SINGLE_FILE** mode
  (``-sSINGLE_FILE``): the Wasm binary is embedded directly into the JS
  output and has no standalone ``.wasm`` file or fetchable URL to key the
  hash on.
- It emits a **warning** with ``-sWASM_ASYNC_COMPILATION=0``: the
  synchronous instantiation path bypasses ``instantiateAsync()`` entirely,
  so the COS code is never reached.
- It covers **only the primary ``.wasm`` file**. Secondary files produced by
  ``-sSPLIT_MODULE`` (``.deferred.wasm``) and side modules loaded at runtime
  via ``dlopen`` in ``-sMAIN_MODULE`` builds are fetched through the normal
  network path and are not stored in or retrieved from COS. A warning is
  emitted for both of these combinations.
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
   Then invoke ``Module['onCOSCacheHit'](hash)`` if defined.

3. **Cache miss** — if a ``NotFoundError`` is thrown, fetch the ``.wasm``
   over the network as usual, invoke ``Module['onCOSCacheMiss'](url)`` if
   defined, call ``WebAssembly.instantiate()`` immediately so the page loads
   without delay, and then write the bytes into COS in the background
   (fire-and-forget) using the ``origins`` value controlled by
   ``-sCROSS_ORIGIN_STORAGE_ORIGINS`` (``'*'`` by default).
   Once the write completes, invoke ``Module['onCOSStore'](hash)`` if defined.

4. **Fallback** — any unexpected error (``NotAllowedError`` from the browser,
   network failure during the miss path, etc.) is logged with ``err()`` and
   the runtime falls through to the standard streaming-instantiation path
   below. The page always loads.

Instrumentation callbacks
-------------------------

Three optional ``Module`` properties let you observe COS events at runtime:

.. code-block:: javascript

   var Module = {
     // Called when the Wasm binary was served from the cross-origin cache.
     onCOSCacheHit: (hash) => {
       console.log('Cache hit, SHA-256:', hash);
     },

     // Called when the Wasm binary was not in COS and was fetched over the
     // network.  |url| is the resolved URL of the .wasm file.
     onCOSCacheMiss: (url) => {
       console.log('Cache miss, fetched from:', url);
     },

     // Called after the Wasm binary has been successfully written to COS.
     onCOSStore: (hash) => {
       console.log('Stored in COS, SHA-256:', hash);
     },
   };

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
- `COS browser extension (Chrome Web Store) <https://chromewebstore.google.com/detail/cross-origin-storage/denpnpcgjgikjpoglpjefakmdcbmlgih>`_
- `COS browser extension (source code) <https://github.com/web-ai-community/cross-origin-storage-extension>`_
- :ref:`settings-reference` — ``CROSS_ORIGIN_STORAGE`` entry
- :ref:`WebAssembly` — general guide to building Wasm with Emscripten
