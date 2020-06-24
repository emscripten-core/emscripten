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
  // The Memory, Table, Module, and Instance functions are constructors, which
  // set properties on the 'this' object, normally. Note that they do not use
  // closure quoting (this['buffer']), which means they are unsafe for use
  // outside of the compiled code. As this polyfill is only used internally,
  // this is good enough, and more compact.
  Memory: function(opts) {
    this.buffer = new ArrayBuffer(opts['initial'] * {{{ WASM_PAGE_SIZE }}});
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
    var array = new Array(opts['initial']);
    this.grow = function(by) {
#if !ALLOW_TABLE_GROWTH
      if (array.length >= {{{ getQuoted('WASM_TABLE_SIZE') }}} + {{{ RESERVED_FUNCTION_POINTERS }}}) {
        abort('Unable to grow wasm table. Use a higher value for RESERVED_FUNCTION_POINTERS or set ALLOW_TABLE_GROWTH.')
      }
#endif
      array.push(null);
    };
    this.set = function(i, func) {
      array[i] = func;
    };
    this.get = function(i) {
      return array[i];
    };
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
        ok({
          'instance': new WebAssembly.Instance(new WebAssembly.Module(binary))
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
