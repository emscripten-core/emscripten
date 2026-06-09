# Cross-Origin Storage (COS) example

This example demonstrates Emscripten's experimental
`-sCROSS_ORIGIN_STORAGE=1` flag, which integrates the
[Cross-Origin Storage browser API](https://github.com/WICG/cross-origin-storage)
into the standard Wasm loading path.

## What it does

COS is a **progressive enhancement**: when the browser exposes the
`navigator.crossOriginStorage` API, loading takes one of two paths:

- **Cache miss** (first load): the `.wasm` module is fetched over the network
  and stored in the cross-origin cache, keyed by its SHA-256 hash.
- **Cache hit** (subsequent loads, same or any other origin): the module is
  retrieved from the cache without a network request for the binary.

When the browser does not expose the COS API, or when an unexpected error
occurs, the runtime falls back to the standard `fetch()` /
`WebAssembly.instantiateStreaming()` path — the page always loads.

The page reports which path was taken and, where applicable, the SHA-256 hash
of the Wasm resource and the URL it was fetched from.

## Build

```bash
emcc main.cpp -o index.js \
    -sCROSS_ORIGIN_STORAGE \
    -sENVIRONMENT=web \
    -sEXPORTED_RUNTIME_METHODS=ccall \
    -sEXPORTED_FUNCTIONS=_greet \
    -sALLOW_MEMORY_GROWTH
```

This produces `index.js` and `index.wasm`. The SHA-256 hash of `index.wasm`
is embedded in `index.js` at build time — you can verify they match:

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

Open the page in a browser with the Cross-Origin Storage API available.

The page will report:

- whether the COS API is active
- on a cache miss: the URL the Wasm was fetched from, and confirmation once it has been stored in COS with its hash
- on a cache hit: the SHA-256 hash of the Wasm resource served from COS

## See also

- [COS Emscripten docs](../../site/source/docs/compiling/CrossOriginStorage.rst)
- [WICG explainer](https://github.com/WICG/cross-origin-storage)
