/**
 * @license
 * Copyright 2019 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

// wasm2js.js - enough of a polyfill for the WebAssembly object so that we can load
// wasm2js code that way.

// Emit "var WebAssembly" if definitely using wasm2js. Otherwise, in MAYBE_WASM2JS
// mode, we can't use a "var" since it would prevent normal wasm from working.
/** @suppress{const} */
#if WASM2JS || WASM == 2
var
#endif
WebAssembly = {
  Memory: function(opts) {
#if USE_PTHREADS
    this.buffer = new SharedArrayBuffer(opts['initial'] * {{{ WASM_PAGE_SIZE }}});
#else
    this.buffer = new ArrayBuffer(opts['initial'] * {{{ WASM_PAGE_SIZE }}});
#endif
    this.grow = function(amount) {
#if ASSERTIONS
      var oldBuffer = this.buffer;
#endif
      var ret = __growWasmMemory(amount);
#if ASSERTIONS
      assert(this.buffer !== oldBuffer); // the call should have updated us
#endif
      return ret;
    };
  },

  Table: function(opts) {
    var ret = new Array(opts['initial']);
    ret.grow = function(by) {
#if !ALLOW_TABLE_GROWTH
      if (ret.length >= {{{ getQuoted('WASM_TABLE_SIZE') }}} + {{{ RESERVED_FUNCTION_POINTERS }}}) {
        abort('Unable to grow wasm table. Use a higher value for RESERVED_FUNCTION_POINTERS or set ALLOW_TABLE_GROWTH.')
      }
#endif
      ret.push(null);
    };
    ret.set = function(i, func) {
      ret[i] = func;
    };
    ret.get = function(i) {
      return ret[i];
    };
    return ret;
  },

  Module: function(binary) {
    // TODO: use the binary and info somehow - right now the wasm2js output is embedded in
    // the main JS
  },

  Instance: function(module, info) {
    // TODO: use the module and info somehow - right now the wasm2js output is embedded in
    // the main JS
    // This will be replaced by the actual wasm2js code.
    var exports = Module['__wasm2jsInstantiate__'](asmLibraryArg, wasmMemory, wasmTable);
    return {
      'exports': exports
    };
  },

  instantiate: /** @suppress{checkTypes} */ function(binary, info) {
    return {
      then: function(ok) {
        var module = new WebAssembly.Module(binary);
        ok({
#if USE_PTHREADS
          'module': module,
#endif
          'instance': new WebAssembly.Instance(module)
        });
#if ASSERTIONS
        // Emulate a simple WebAssembly.instantiate(..).then(()=>{}).catch(()=>{}) syntax.
        return { catch: function() {} };
#endif
      }
    };
  },

  RuntimeError: Error
};

#if !MINIMAL_RUNTIME
// We don't need to actually download a wasm binary, mark it as present but empty.
wasmBinary = [];
#endif
