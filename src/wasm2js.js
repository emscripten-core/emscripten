/**
 * @license
 * Copyright 2019 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

// wasm2js.js - enough of a polyfill for the WebAssembly object so that we can load
// wasm2js code that way.

// Emit "var WebAssembly" if definitely using wasm2js. Otherwise, in MAYBE_WASM2JS
// mode, we can't use a "var" since it would prevent normal wasm from working.
/** @suppress{duplicate, const} */
#if WASM2JS || WASM == 2
var
#endif
WebAssembly = {
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

#if RELOCATABLE
  // Only needed in RELOCATABLE builds since normal builds export the table
  // from the wasm module.
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
#endif

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
