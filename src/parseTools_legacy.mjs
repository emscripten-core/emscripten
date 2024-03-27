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

function stripCorrections(param) {
  warn('use of legacy parseTools function: stripCorrections');
  let m;
  while (true) {
    if ((m = /^\((.*)\)$/.exec(param))) {
      param = m[1];
      continue;
    }
    if ((m = /^\(([$_\w]+)\)&\d+$/.exec(param))) {
      param = m[1];
      continue;
    }
    if ((m = /^\(([$_\w()]+)\)\|0$/.exec(param))) {
      param = m[1];
      continue;
    }
    if ((m = /^\(([$_\w()]+)\)\>>>0$/.exec(param))) {
      param = m[1];
      continue;
    }
    if ((m = /CHECK_OVERFLOW\(([^,)]*),.*/.exec(param))) {
      param = m[1];
      continue;
    }
    break;
  }
  return param;
}

const UNROLL_LOOP_MAX = 8;

function makeCopyValues(dest, src, num, type, modifier, align, sep = ';') {
  warn('use of legacy parseTools function: makeCopyValues');
  assert(typeof align === 'undefined');
  function unroll(type, num, jump = 1) {
    const setValues = range(num).map((i) =>
      makeSetValue(dest, i * jump, makeGetValue(src, i * jump, type), type),
    );
    return setValues.join(sep);
  }
  // If we don't know how to handle this at compile-time, or handling it is best
  // done in a large amount of code, call memcpy
  if (!isNumber(num)) num = stripCorrections(num);
  if (!isNumber(align)) align = stripCorrections(align);
  if (!isNumber(num) || parseInt(num) / align >= UNROLL_LOOP_MAX) {
    return '(_memcpy(' + dest + ', ' + src + ', ' + num + ')|0)';
  }
  num = parseInt(num);
  // remove corrections, since we will be correcting after we add anyhow,
  dest = stripCorrections(dest);
  src = stripCorrections(src);
  // and in the heap assignment expression
  const ret = [];
  [4, 2, 1].forEach((possibleAlign) => {
    if (num == 0) return;
    if (align >= possibleAlign) {
      ret.push(unroll('i' + possibleAlign * 8, Math.floor(num / possibleAlign), possibleAlign));
      src = getFastValue(src, '+', Math.floor(num / possibleAlign) * possibleAlign);
      dest = getFastValue(dest, '+', Math.floor(num / possibleAlign) * possibleAlign);
      num %= possibleAlign;
    }
  });
  return ret.join(sep);
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
  makeCopyValues,
  makeMalloc,
  makeStructuralReturn,
  receiveI64ParamAsDouble,
  receiveI64ParamAsI32s,
  runOnMainThread,
});
