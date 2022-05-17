/**
 * @license
 * Copyright 2010 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

// Takes a pair of return values, stashes one in tempRet0 and returns the other.
// Should probably be renamed to `makeReturn64` but keeping this old name in
// case external JS library code uses this name.
function makeStructuralReturn(values) {
  assert(values.length == 2);
  return 'setTempRet0(' + values[1] + '); return ' + asmCoercion(values[0], 'i32');
}

// Replaced (at least internally) with receiveI64ParamAsI53 that does
// bounds checking.
function receiveI64ParamAsDouble(name) {
  if (WASM_BIGINT) {
    // Just convert the bigint into a double.
    return `${name} = Number(${name});`;
  }
  // Combine the i32 params. Use an unsigned operator on low and shift high by
  // 32 bits.
  return `var ${name} = ${name}_high * 0x100000000 + (${name}_low >>> 0);`;
}
