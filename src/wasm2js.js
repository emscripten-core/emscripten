// wasm2js.js - enough of a polyfill for the WebAssembly object so that we can load
// wasm2js code that way.

var WebAssembly = {
  // This will be replaced by the actual wasm2js code.
  temp: Module['__wasm2jsInstantiate__'](asmLibraryArg, wasmMemory, wasmTable);
};

