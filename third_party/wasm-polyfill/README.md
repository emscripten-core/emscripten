# WebAssembly Polyfill (prototype)

**This repo contains a prototype built to demonstrate the
viability of a WebAssembly polyfill. Work is ongoing in other repos to design
the actual standard binary format. This prototype also makes no attempt to call
native browser decoding and so it is not, technically, a polyfill but rather a
pure JS library.**

## Demos

See [FAQ entry](https://github.com/WebAssembly/design/blob/master/FAQ.md#can-the-polyfill-really-be-efficient).

## How it (currently) works

A real (non-prototype) polyfill would hook into an [ES6 Module loader
polyfill](https://github.com/ModuleLoader/es6-module-loader) so that loading a
WebAssembly module will Just Work. The scheme described here is just a proof of
concept for how the polyfill can avoid massive copying, string manipulation and
garbage.

1. On the main thread, the client kicks off a load of a URL by calling
   [`loadWebAssembly`](https://github.com/WebAssembly/polyfill-prototype-1/blob/master/jslib/load-wasm.js#L6)
   and receives a `Promise<Function>`.
2. The polyfill library starts up a worker containing asm.js code compiled from
   [`unpack.cpp`](https://github.com/WebAssembly/polyfill-prototype-1/blob/master/src/unpack.cpp)
   concatenated with the glue code in
   [`load-wasm-worker.js`](https://github.com/WebAssembly/polyfill-prototype-1/blob/master/src/load-wasm-worker.js).
2. The worker glue code fetches the binary via XHR then copies the result into the asm.js heap.
3. The asm.js code decodes the binary into asm.js in the form of UTF8 bytes in a separate region of the asm.js heap.
4. The worker glue code creates a `Blob` (a read-only copy) from a view of just the asm.js UTF8 bytes.
5. The `Blob` is `postMessage()`ed back to the main thread (a non-copying
   operation) where it is loaded as a script element with `script.src = URL.getObjectURL(blob)`.
6. When the asm.js script is executed, it passes the asm.js module function
   object to a callback which resolves the promise in step 1.

## Build instructions

The library in `jslib/` should be ready to use, just copy both files and call `loadWebAssembly()`.

Running `make` compiles the C++ implementation into the JS files in `jslib/` and
into native executables in `tools/`. (The Makefile is currently super-unportable.
Sorry! Patches welcome.)

## Packing asm.js into the binary format

The polyfill also comes with a tool (`tools/pack-asmjs`) that compiles a single JS
file (containing only asm.js) into the (experimental) WebAssembly format decoded by
the library. Given an existing asm.js application, one can experiment with this 
polyfill (perform direct size/load-time comparisons) by following the steps:
 1. Separate the asm.js module out into a separate file.
 2. Run `tools/pack-asmjs` to produce a `.wasm` file.
 3. Refactor the code that called the asm.js module to instead call `loadWebAssembly()`
    (which returns a promise that resolves to the unlinked asm.js module function).

## Future work

 * Decode while downloading (using HTTP `Range` requests or splitting into separate files)
 * Perform generic compression on top of the `.wasm` file (e.g., 
   [lzham](https://github.com/richgel999/lzham_codec) gives a further 24% boost over `gzip`).
