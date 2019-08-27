// wasm2js.js - enough of a polyfill for the WebAssembly object so that we can load
// wasm2js code that way.

// Emit "var WebAssembly" if definitely using wasm2js. Otherwise, in MAYBE_WASM2JS
// mode, we can't use a "var" since it would prevent normal wasm from working.
#if WASM2JS
var
#endif
WebAssembly = {
  Memory: function(opts) {
    return {
      buffer: new ArrayBuffer(opts['initial'] * {{{ WASM_PAGE_SIZE }}}),
      grow: function(amount) {
#if ASSERTIONS
        var oldBuffer = this.buffer;
#endif
        var ret = __growWasmMemory(amount);
#if ASSERTIONS
        assert(this.buffer !== oldBuffer); // the call should have updated us
#endif
        return ret;
      }
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
    return {};
  },

  Instance: function(module, info) {
    // TODO: use the module and info somehow - right now the wasm2js output is embedded in
    // the main JS
    // XXX hack to get an atob implementation
#include base64Utils.js
    var atob = decodeBase64;
    // This will be replaced by the actual wasm2js code.
    var exports = Module['__wasm2jsInstantiate__'](asmLibraryArg, wasmMemory, wasmTable);
    return {
      'exports': exports
    };
  },

  instantiate: function(binary, info) {
    return {
      then: function(ok, err) {
        ok({
          'instance': new WebAssembly.Instance(new WebAssembly.Module(binary, info))
        });
      }
    };
  }
};

// We don't need to actually download a wasm binary, mark it as present but empty.
wasmBinary = [];

