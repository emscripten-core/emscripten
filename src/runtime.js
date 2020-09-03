/**
 * @license
 * Copyright 2010 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

//"use strict";

var Compiletime = {
  isPointerType: isPointerType,
  isStructType: isStructType,

  isNumberType: function(type) {
    return type in Compiletime.INT_TYPES || type in Compiletime.FLOAT_TYPES;
  },

  INT_TYPES: set('i1', 'i8', 'i16', 'i32', 'i64'),
  FLOAT_TYPES: set('float', 'double'),
};

// code used both at compile time and runtime is defined here, then put on
// the Runtime object for compile time and support.js for the generated code

function getNativeTypeSize(type) {
  switch (type) {
    case 'i1': case 'i8': return 1;
    case 'i16': return 2;
    case 'i32': return 4;
    case 'i64': return 8;
    case 'float': return 4;
    case 'double': return 8;
    default: {
      if (type[type.length-1] === '*') {
        return 4; // A pointer
      } else if (type[0] === 'i') {
        var bits = Number(type.substr(1));
        assert(bits % 8 === 0, 'getNativeTypeSize invalid bits ' + bits + ', type ' + type);
        return bits / 8;
      } else {
        return 0;
      }
    }
  }
}

function alignMemory(size, factor) {
  if (!factor) factor = STACK_ALIGN; // stack alignment (16-byte) by default
  return Math.ceil(size / factor) * factor;
}

var Runtime = {
  getNativeTypeSize: getNativeTypeSize,

  //! Returns the size of a structure field, as C/C++ would have it (in 32-bit,
  //! for now).
  //! @param type The type, by name.
  getNativeFieldSize: function(type) {
    return Math.max(getNativeTypeSize(type), Runtime.QUANTUM_SIZE);
  },

  POINTER_SIZE: 4,
  QUANTUM_SIZE: 4,
};

// Additional runtime elements, that need preprocessing

// "Process info" for syscalls is static and cannot change, so define it using
// some fixed values
var PROCINFO = {
  ppid: 1,
  pid: 42,
  sid: 42,
  pgid: 42
};
