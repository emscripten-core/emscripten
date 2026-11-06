/**
 * @license
 * Copyright 2010 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

import {cDefs} from './modules.mjs';
import {warn, addToCompileTimeContext, onSettingsLoaded} from './utility.mjs';
import {POINTER_SIZE, POINTER_TYPE, runIfMainThread} from './parseTools.mjs';

// Given an array of elements [elem1,elem2,elem3], returns a string "['elem1','elem2','elem3']"
function buildStringArray(array) {
  warn('use of legacy parseTools function: buildStringArray');
  if (array.length > 0) {
    return "['" + array.join("','") + "']";
  } else {
    return '[]';
  }
}

// Legacy function that existed solely to give error message.  These are now
// provided by the cDefs proxy object.
function cDefine(key) {
  warn('use of legacy parseTools function: cDefine');
  return cDefs[key];
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

// Legacy name for runIfMainThread.
const runOnMainThread = runIfMainThread;

onSettingsLoaded(() => {
  if (STRICT) {
    return;
  }
  const Runtime = {
    POINTER_SIZE,
    QUANTUM_SIZE: POINTER_SIZE,
  };

  const STACK_ALIGN = 16;
  const POINTER_BITS = POINTER_SIZE * 8;
  const POINTER_HEAP = MEMORY64 ? 'HEAP64' : 'HEAP32';
  const POINTER_JS_TYPE = MEMORY64 ? "'bigint'" : "'number'";
  const POINTER_MAX = MEMORY64 ? 'Number.MAX_SAFE_INTEGER' : '0xFFFFFFFF';
  const POINTER_SHIFT = MEMORY64 ? '3' : '2';
  // Similar to POINTER_TYPE, but this is the actual wasm type that is
  // used in practice, while POINTER_TYPE is the more refined internal
  // type (that is unsigned, where as core wasm does not have unsigned
  // types).
  const POINTER_WASM_TYPE = `i${POINTER_BITS}`;

  addToCompileTimeContext({
    POINTER_BITS,
    POINTER_HEAP,
    POINTER_JS_TYPE,
    POINTER_MAX,
    POINTER_SHIFT,
    POINTER_TYPE,
    POINTER_WASM_TYPE,
    Runtime,
    STACK_ALIGN,
    buildStringArray,
    cDefine,
    makeMalloc,
    receiveI64ParamAsDouble,
    receiveI64ParamAsI32s,
    runOnMainThread,
  });
});
