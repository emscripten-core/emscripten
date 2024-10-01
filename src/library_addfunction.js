/**
 * @license
 * Copyright 2020 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

addToLibrary({
  // This gives correct answers for everything less than 2^{14} = 16384
  // I hope nobody is contemplating functions with 16384 arguments...
  $uleb128Encode: (n, target) => {
#if ASSERTIONS
    assert(n < 16384);
#endif
    if (n < 128) {
      target.push(n);
    } else {
      target.push((n % 128) | 128, n >> 7);
    }
  },

  // Converts a signature like 'vii' into a description of the wasm types, like
  // { parameters: ['i32', 'i32'], results: [] }.
  $sigToWasmTypes: (sig) => {
#if ASSERTIONS && !WASM_BIGINT
    assert(!sig.includes('j'), 'i64 not permitted in function signatures when WASM_BIGINT is disabled');
#endif
    var typeNames = {
      'i': 'i32',
      'j': 'i64',
      'f': 'f32',
      'd': 'f64',
      'e': 'externref',
#if MEMORY64
      'p': 'i64',
#else
      'p': 'i32',
#endif
    };
    var type = {
      parameters: [],
      results: sig[0] == 'v' ? [] : [typeNames[sig[0]]]
    };
    for (var i = 1; i < sig.length; ++i) {
#if ASSERTIONS
      assert(sig[i] in typeNames, 'invalid signature char: ' + sig[i]);
#endif
      type.parameters.push(typeNames[sig[i]]);
    }
    return type;
  },
  $generateFuncType__deps: ['$uleb128Encode'],
  $generateFuncType: (sig, target) => {
    var sigRet = sig.slice(0, 1);
    var sigParam = sig.slice(1);
    var typeCodes = {
      'i': 0x7f, // i32
#if MEMORY64
      'p': 0x7e, // i64
#else
      'p': 0x7f, // i32
#endif
      'j': 0x7e, // i64
      'f': 0x7d, // f32
      'd': 0x7c, // f64
      'e': 0x6f, // externref
    };

    // Parameters, length + signatures
    target.push(0x60 /* form: func */);
    uleb128Encode(sigParam.length, target);
    for (var i = 0; i < sigParam.length; ++i) {
#if ASSERTIONS
      assert(sigParam[i] in typeCodes, 'invalid signature char: ' + sigParam[i]);
#endif
      target.push(typeCodes[sigParam[i]]);
    }

    // Return values, length + signatures
    // With no multi-return in MVP, either 0 (void) or 1 (anything else)
    if (sigRet == 'v') {
      target.push(0x00);
    } else {
      target.push(0x01, typeCodes[sigRet]);
    }
  },
  // Wraps a JS function as a wasm function with a given signature.
  $convertJsFunctionToWasm__deps: ['$uleb128Encode', '$sigToWasmTypes', '$generateFuncType'],
  $convertJsFunctionToWasm: (func, sig) => {
#if WASM2JS
    // return func;
#else // WASM2JS

#if ASSERTIONS && !WASM_BIGINT
    assert(!sig.includes('j'), 'i64 not permitted in function signatures when WASM_BIGINT is disabled');
#endif

    // If the type reflection proposal is available, use the new
    // "WebAssembly.Function" constructor.
    // Otherwise, construct a minimal wasm module importing the JS function and
    // re-exporting it.
    if (typeof WebAssembly.Function == "function") {
      return new WebAssembly.Function(sigToWasmTypes(sig), func);
    }

    // The module is static, with the exception of the type section, which is
    // generated based on the signature passed in.
    var typeSectionBody = [
      0x01, // count: 1
    ];
    generateFuncType(sig, typeSectionBody);

    // Rest of the module is static
    var bytes = [
      0x00, 0x61, 0x73, 0x6d, // magic ("\0asm")
      0x01, 0x00, 0x00, 0x00, // version: 1
      0x01, // Type section code
    ];
    // Write the overall length of the type section followed by the body
    uleb128Encode(typeSectionBody.length, bytes);
    bytes.push(...typeSectionBody);

    // The rest of the module is static
    bytes.push(
      0x02, 0x07, // import section
        // (import "e" "f" (func 0 (type 0)))
        0x01, 0x01, 0x65, 0x01, 0x66, 0x00, 0x00,
      0x07, 0x05, // export section
        // (export "f" (func 0 (type 0)))
        0x01, 0x01, 0x66, 0x00, 0x00,
    );

    // We can compile this wasm module synchronously because it is very small.
    // This accepts an import (at "e.f"), that it reroutes to an export (at "f")
    var module = new WebAssembly.Module(new Uint8Array(bytes));
    var instance = new WebAssembly.Instance(module, { 'e': { 'f': func } });
    var wrappedFunc = instance.exports['f'];
    return wrappedFunc;
#endif // WASM2JS
  },

  $freeTableIndexes: [],

  // Weak map of functions in the table to their indexes, created on first use.
  $functionsInTableMap: undefined,

  $getEmptyTableSlot__deps: ['$freeTableIndexes', '$wasmTable'],
  $getEmptyTableSlot: () => {
    // Reuse a free index if there is one, otherwise grow.
    if (freeTableIndexes.length) {
      return freeTableIndexes.pop();
    }
    // Grow the table
    try {
      wasmTable.grow({{{ toIndexType('1') }}});
    } catch (err) {
      if (!(err instanceof RangeError)) {
        throw err;
      }
      throw 'Unable to grow wasm table. Set ALLOW_TABLE_GROWTH.';
    }
    return {{{ from64Expr('wasmTable.length') }}} - 1;
  },

  $updateTableMap__deps: ['$getWasmTableEntry'],
  $updateTableMap: (offset, count) => {
    if (functionsInTableMap) {
      for (var i = offset; i < offset + count; i++) {
        var item = getWasmTableEntry(i);
        // Ignore null values.
        if (item) {
          functionsInTableMap.set(item, i);
        }
      }
    }
  },

  $getFunctionAddress__deps: ['$updateTableMap', '$functionsInTableMap', '$wasmTable'],
  $getFunctionAddress: (func) => {
    // First, create the map if this is the first use.
    if (!functionsInTableMap) {
      functionsInTableMap = new WeakMap();
      updateTableMap(0, {{{ from64Expr('wasmTable.length') }}});
    }
    return functionsInTableMap.get(func) || 0;
  },

  /**
   * Add a function to the table.
   * 'sig' parameter is required if the function being added is a JS function.
   */
  $addFunction__docs: '/** @param {string=} sig */',
  $addFunction__deps: ['$convertJsFunctionToWasm', '$getFunctionAddress',
                       '$functionsInTableMap', '$getEmptyTableSlot',
                       '$getWasmTableEntry', '$setWasmTableEntry',
                       '$wasmTable'],
  $addFunction: (func, sig) => {
  #if ASSERTIONS
    assert(typeof func != 'undefined');
  #endif // ASSERTIONS
    // Check if the function is already in the table, to ensure each function
    // gets a unique index.
    var rtn = getFunctionAddress(func);
    if (rtn) {
      return rtn;
    }

    // It's not in the table, add it now.

  #if ASSERTIONS >= 2
    // Make sure functionsInTableMap is actually up to date, that is, that this
    // function is not actually in the wasm Table despite not being tracked in
    // functionsInTableMap.
    for (var i = 0; i < wasmTable.length; i++) {
      assert(getWasmTableEntry(i) != func, 'function in Table but not functionsInTableMap');
    }
  #endif

    var ret = getEmptyTableSlot();

    // Set the new value.
    try {
      // Attempting to call this with JS function will cause of table.set() to fail
      setWasmTableEntry(ret, func);
    } catch (err) {
      if (!(err instanceof TypeError)) {
        throw err;
      }
  #if ASSERTIONS
      assert(typeof sig != 'undefined', 'Missing signature argument to addFunction: ' + func);
  #endif
      var wrapped = convertJsFunctionToWasm(func, sig);
      setWasmTableEntry(ret, wrapped);
    }

    functionsInTableMap.set(func, ret);

    return ret;
  },

  $removeFunction__deps: ['$functionsInTableMap', '$freeTableIndexes',
                          '$getWasmTableEntry', '$setWasmTableEntry'],
  $removeFunction: (index) => {
    functionsInTableMap.delete(getWasmTableEntry(index));
    setWasmTableEntry(index, null);
    freeTableIndexes.push(index);
  },
});

