/**
 * @license
 * Copyright 2010 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

import {warn, addToCompileTimeContext} from './utility.mjs';
import {ATMAINS, POINTER_SIZE, runIfMainThread} from './parseTools.mjs';

// Takes a pair of return values, stashes one in tempRet0 and returns the other.
// Should probably be renamed to `makeReturn64` but keeping this old name in
// case external JS library code uses this name.
function makeStructuralReturn(values) {
  warn('use of legacy parseTools function: makeStructuralReturn');
  assert(values.length == 2);
  return 'setTempRet0(' + values[1] + '); return ' + asmCoercion(values[0], 'i32');
}

// Replaced (at least internally) with receiveI64ParamAsI53 that does
// bounds checking.
function receiveI64ParamAsDouble(name) {
  warn('use of legacy parseTools function: receiveI64ParamAsDouble');
  if (WASM_BIGINT) {
    // Just convert the bigint into a double.
    return `${name} = Number(${name});`;
  }
  // Combine the i32 params. Use an unsigned operator on low and shift high by
  // 32 bits.
  return `var ${name} = ${name}_high * 0x100000000 + (${name}_low >>> 0);`;
}

function receiveI64ParamAsI32s(name) {
  warn('use of legacy parseTools function: receiveI64ParamAsI32s');
  if (WASM_BIGINT) {
    return `var ${name}_low = Number(${name} & 0xffffffffn) | 0, ${name}_high = Number(${name} >> 32n) | 0;`;
  }
  return '';
}

function makeMalloc(source, param) {
  warn('use of legacy parseTools function: makeMalloc');
  return `_malloc(${param})`;
}

function getNativeFieldSize(type) {
  warn('use of legacy parseTools function: getNativeFieldSize');
  return Math.max(getNativeTypeSize(type), POINTER_SIZE);
}

const Runtime = {
  getNativeFieldSize,
  POINTER_SIZE,
  QUANTUM_SIZE: POINTER_SIZE,
};

function addAtMain(code) {
  warn('use of legacy parseTools function: addAtMain');
  assert(HAS_MAIN, 'addAtMain called but program has no main function');
  ATMAINS.push(code);
}

function ensureValidFFIType(type) {
  return type === 'float' ? 'double' : type; // ffi does not tolerate float XXX
}

// FFI return values must arrive as doubles, and we can force them to floats afterwards
function asmFFICoercion(value, type) {
  value = asmCoercion(value, ensureValidFFIType(type));
  if (type === 'float') value = asmCoercion(value, 'float');
  return value;
}

// Legacy name for runIfMainThread.
const runOnMainThread = runIfMainThread;

addToCompileTimeContext({
  ATMAINS,
  Runtime,
  addAtMain,
  asmFFICoercion,
  makeMalloc,
  makeStructuralReturn,
  receiveI64ParamAsDouble,
  receiveI64ParamAsI32s,
  runOnMainThread,
});
