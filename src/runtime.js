/**
 * @license
 * Copyright 2010 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

// "use strict";

// code used both at compile time and runtime is defined here, then put on
// the Runtime object for compile time and support.js for the generated code

global.POINTER_SIZE = MEMORY64 ? 8 : 4;
global.STACK_ALIGN = 16;

function getNativeTypeSize(type) {
  switch (type) {
    case 'i1': case 'i8': case 'u8': return 1;
    case 'i16': case 'u16': return 2;
    case 'i32': case 'u32': return 4;
    case 'i64': case 'u64': return 8;
    case 'float': return 4;
    case 'double': return 8;
    default: {
      if (type[type.length - 1] === '*') {
        return POINTER_SIZE;
      }
      if (type[0] === 'i') {
        const bits = Number(type.substr(1));
        assert(bits % 8 === 0, 'getNativeTypeSize invalid bits ' + bits + ', type ' + type);
        return bits / 8;
      }
      return 0;
    }
  }
}

global.Runtime = {
  getNativeTypeSize: getNativeTypeSize,

  // TODO(sbc): This function is unused by emscripten but we can't be
  // sure there are not external users.
  // See: https://github.com/emscripten-core/emscripten/issues/15242
  getNativeFieldSize: function(type) {
    return Math.max(getNativeTypeSize(type), Runtime.QUANTUM_SIZE);
  },

  POINTER_SIZE: POINTER_SIZE,
  QUANTUM_SIZE: POINTER_SIZE,
};
