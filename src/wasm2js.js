/**
 * @license
 * Copyright 2019 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

// wasm2js.js - enough of a polyfill for the WebAssembly object so that we can load
// wasm2js code that way.

/** @suppress{duplicate, const, checkTypes} */
var WebAssembly = {
  // Note that we do not use closure quoting (this['buffer'], etc.) on these
  // functions, as they are just meant for internal use. In other words, this is
  // not a fully general polyfill.
  /** @constructor */
  Memory: function(opts) {
#if SHARED_MEMORY
    this.buffer = new SharedArrayBuffer(opts['initial'] * {{{ WASM_PAGE_SIZE }}});
#else
    this.buffer = new ArrayBuffer(opts['initial'] * {{{ WASM_PAGE_SIZE }}});
#endif
  },

  Module: function(binary) {
    // TODO: use the binary and info somehow - right now the wasm2js output is embedded in
    // the main JS
  },

  /** @constructor */
  Instance: function(module, info) {
    // TODO: use the module somehow - right now the wasm2js output is embedded in
    // the main JS
    // This will be replaced by the actual wasm2js code.
    this.exports = Module['__wasm2jsInstantiate__'](info);
  },

  instantiate: /** @suppress{checkTypes} */ function(binary, info) {
    return {
      then: function(ok) {
        var module = new WebAssembly.Module(binary);
        ok({
#if SHARED_MEMORY
          'module': module,
#endif
          'instance': new WebAssembly.Instance(module, info)
        });
#if ASSERTIONS || WASM == 2 // see postamble_minimal.js which uses .catch
        // Emulate a simple WebAssembly.instantiate(..).then(()=>{}).catch(()=>{}) syntax.
        return { catch: function() {} };
#endif
      }
    };
  },

  RuntimeError: Error,

#if !MINIMAL_RUNTIME
  isWasm2js: true,
#endif
};
