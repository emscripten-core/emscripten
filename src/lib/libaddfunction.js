/**
 * @license
 * Copyright 2020 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

addToLibrary({
  // This gives correct answers for everything less than 2^{14} = 16384
  // I hope nobody is contemplating functions with 16384 arguments...
  $uleb128EncodeWithLen__internal: true,
  $uleb128EncodeWithLen: (arr) => {
    const n = arr.length;
#if ASSERTIONS
    assert(n < 16384);
#endif
    // Note: this LEB128 length encoding produces extra byte for n < 128,
    // but we don't care as it's only used in a temporary representation.
    return [(n % 128) | 128, n >> 7, ...arr];
  },
#if WASM_JS_TYPES
  // Converts a signature like 'vii' into a description of the wasm types, like
  // { parameters: ['i32', 'i32'], results: [] }.
  $sigToWasmTypes__internal: true,
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
#endif
  $wasmTypeCodes__internal: true,
  // Note: using template literal here instead of plain object
  // because jsify serializes objects w/o quotes and Closure will then
  // incorrectly mangle the properties.
  $wasmTypeCodes: `{
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
  }`,

  $generateTypePack__internal: true,
  $generateTypePack__deps: ['$uleb128EncodeWithLen', '$wasmTypeCodes'],
  $generateTypePack: (types) => uleb128EncodeWithLen(Array.from(types, (type) => {
    var code = wasmTypeCodes[type];
#if ASSERTIONS
    assert(code, `invalid signature char: ${type}`);
#endif
    return code;
  })),

#if !WASM2JS || WASM == 2
  // Wraps a JS function as a wasm function with a given signature.
  $convertJsFunctionToWasm__deps: [
    '$uleb128EncodeWithLen',
#if WASM_JS_TYPES
    '$sigToWasmTypes',
#endif
    '$generateTypePack'
  ],
  $convertJsFunctionToWasm: (func, sig) => {
#if ASSERTIONS && !WASM_BIGINT
    assert(!sig.includes('j'), 'i64 not permitted in function signatures when WASM_BIGINT is disabled');
#endif
#if WASM_JS_TYPES
    // If the type reflection proposal is available, use the new
    // "WebAssembly.Function" constructor.
    // Otherwise, construct a minimal wasm module importing the JS function and
    // re-exporting it.
    if (WebAssembly.Function) {
      return new WebAssembly.Function(sigToWasmTypes(sig), func);
    }
#endif

    // Rest of the module is static
    var bytes = Uint8Array.of(
      0x00, 0x61, 0x73, 0x6d, // magic ("\0asm")
      0x01, 0x00, 0x00, 0x00, // version: 1
      0x01, // Type section code
        // The module is static, with the exception of the type section, which is
        // generated based on the signature passed in.
        ...uleb128EncodeWithLen([
          0x01, // count: 1
          0x60 /* form: func */,
          // param types
          ...generateTypePack(sig.slice(1)),
          // return types (for now only supporting [] if `void` and single [T] otherwise)
          ...generateTypePack(sig[0] === 'v' ? '' : sig[0])
        ]),
      // The rest of the module is static
      0x02, 0x07, // import section
        // (import "e" "f" (func 0 (type 0)))
        0x01, 0x01, 0x65, 0x01, 0x66, 0x00, 0x00,
      0x07, 0x05, // export section
        // (export "f" (func 0 (type 0)))
        0x01, 0x01, 0x66, 0x00, 0x00,
    );

    // We can compile this wasm module synchronously because it is very small.
    // This accepts an import (at "e.f"), that it reroutes to an export (at "f")
    var module = new WebAssembly.Module(bytes);
    var instance = new WebAssembly.Instance(module, { 'e': { 'f': func } });
    var wrappedFunc = instance.exports['f'];
    return wrappedFunc;
  },
#endif // !WASM2JS && WASM != 2

  $freeTableIndexes: [],

  // Weak map of functions in the table to their indexes, created on first use.
  $functionsInTableMap: undefined,

  $getEmptyTableSlot__deps: ['$freeTableIndexes', '$wasmTable'],
  $getEmptyTableSlot: () => {
    // Reuse a free index if there is one, otherwise grow.
    if (freeTableIndexes.length) {
      return freeTableIndexes.pop();
    }
#if ASSERTIONS
    try {
  #endif
      // Grow the table
      return wasmTable['grow']({{{ toIndexType('1') }}});
#if ASSERTIONS
    } catch (err) {
      if (!(err instanceof RangeError)) {
        throw err;
      }
      abort('Unable to grow wasm table. Set ALLOW_TABLE_GROWTH.');
    }
#endif
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
  $addFunction__deps: ['$getFunctionAddress',
                       '$functionsInTableMap', '$getEmptyTableSlot',
                       '$setWasmTableEntry',
#if !WASM2JS || WASM == 2
                       '$convertJsFunctionToWasm',
#endif
#if ASSERTIONS >= 2
                       '$getWasmTableEntry', '$wasmTable',
#endif
  ],

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

#if WASM2JS && WASM != 2
    setWasmTableEntry(ret, func);
#else
    // Set the new value.
    try {
      // Attempting to call this with JS function will cause table.set() to fail
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
#endif

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
