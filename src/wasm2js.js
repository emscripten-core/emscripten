// wasm2js.js - enough of a polyfill for the WebAssembly object so that we can load
// wasm2js code that way.

var WebAssembly = {
  Memory: function(opts) {
    // TODO: growth?
    return {
      buffer: new ArrayBuffer(opts['initial'] * {{{ WASM_PAGE_SIZE }}})
    };
  },

  Table: function(opts) {
    var ret = new Array(opts['initial']);
    // TODO: add more properties here
    return ret;
  },

  instantiate: function(binary, info) {
    // TODO: use the binary and info somehow - right now the wasm2js output is embedded in
    // the main JS
    // XXX hack to get an atob implementation
#include base64Utils.js
    var atob = decodeBase64;
    // This will be replaced by the actual wasm2js code.
    var exports = Module['__wasm2jsInstantiate__'](asmLibraryArg, wasmMemory, wasmTable);
    return {
      then: function(ok, err) {
        ok({
          'instance': {
            'exports': exports
          }
        });
      }
    };
  }
};

// We don't need to actually download a wasm binary, mark it as present.
Module['wasmBinary'] = true;

