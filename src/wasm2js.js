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
  // Note that we do not use closure quoting (this['buffer'], etc.) on these
  // functions, as they are just meant for internal use. In other words, this is
  // not a fully general polyfill.
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

  // Table is not a normal constructor and instead returns the array object.
  // That lets us use the length property automatically, which is simpler and
  // smaller (but instanceof will not report that an instance of Table is an
  // instance of this function).
  Table: /** @constructor */ function(opts) {
    var ret = new Array(opts['initial']);
#if ALLOW_TABLE_GROWTH
    ret.grow = function(by) {
      ret.push(null);
    };
#else
#if ASSERTIONS // without assertions we'll throw on calling the missing function
    ret.grow = function(by) {
      abort('Unable to grow wasm table. Build with ALLOW_TABLE_GROWTH.')
    };
#endif // ASSERTIONS
#endif // ALLOW_TABLE_GROWTH
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
    this.exports = Module['__wasm2jsInstantiate__'](asmLibraryArg, wasmMemory, wasmTable);
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
