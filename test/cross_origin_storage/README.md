# Cross-Origin Storage (COS) example

This example demonstrates Emscripten's experimental
`-sCROSS_ORIGIN_STORAGE=1` flag, which integrates the
[Cross-Origin Storage browser API](https://github.com/WICG/cross-origin-storage)
into the standard Wasm loading path.

## What it does

On the **first load** the `.wasm` module is fetched over the network and
stored in the cross-origin cache, keyed by its SHA-256 hash.

On **subsequent loads** — from the same origin or any other — the module is
retrieved from the cache without a network request for the binary.

## Prerequisites

The COS API is not yet natively supported by any browser. Install the
polyfill extension to try it today:

- **Chrome Web Store**:
  [Cross-Origin Storage](https://chromewebstore.google.com/detail/cross-origin-storage/denpnpcgjgikjpoglpjefakmdcbmlgih)

## Build

```bash
emcc main.cpp -o index.js \
    -sCROSS_ORIGIN_STORAGE=1 \
    -sENVIRONMENT=web \
    -sEXPORTED_RUNTIME_METHODS=ccall \
    -sEXPORTED_FUNCTIONS=_greet \
    -sALLOW_MEMORY_GROWTH
```

This produces `index.js` and `index.wasm`. The SHA-256 hash of `index.wasm`
is embedded in `index.js` at build time — you can verify them match:

```bash
sha256sum index.wasm
grep -oP "value: '\K[0-9a-f]{64}" index.js
```

## Run

Serve the directory over HTTP (the `file://` protocol does not support
`fetch()`):

```bash
emrun .
# or
python3 -m http.server
```

Open `http://localhost:8080` (or whichever port your server uses) in a
browser with the COS extension installed.

Open DevTools → Console. On the first load you should see a cache-miss log
message and a network request for `index.wasm` in the Network tab. Reload
the page — the network request disappears and the console shows that the
module was served from the cross-origin cache.

## See also

- [COS Emscripten docs](../../site/source/docs/compiling/CrossOriginStorage.rst)
- [WICG explainer](https://github.com/WICG/cross-origin-storage)
- [COS extension source](https://github.com/web-ai-community/cross-origin-storage-extension)
