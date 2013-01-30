// Relooper, (C) 2012 Alon Zakai, MIT license, https://github.com/kripken/Relooper
var Relooper = (function() {
// Note: For maximum-speed code, see "Optimizing Code" on the Emscripten wiki, https://github.com/kripken/emscripten/wiki/Optimizing-Code
// Note: Some Emscripten settings may limit the speed of the generated code.
// TODO: " u s e   s t r i c t ";

try {
  this['Module'] = Module;
} catch(e) {
  this['Module'] = Module = {};
}

// The environment setup code below is customized to use Module.
// *** Environment setup code ***
var ENVIRONMENT_IS_NODE = typeof process === 'object' && typeof require === 'function';
var ENVIRONMENT_IS_WEB = typeof window === 'object';
var ENVIRONMENT_IS_WORKER = typeof importScripts === 'function';
var ENVIRONMENT_IS_SHELL = !ENVIRONMENT_IS_WEB && !ENVIRONMENT_IS_NODE && !ENVIRONMENT_IS_WORKER;

if (ENVIRONMENT_IS_NODE) {
  // Expose functionality in the same simple way that the shells work
  // Note that we pollute the global namespace here, otherwise we break in node
  Module['print'] = function(x) {
    process['stdout'].write(x + '\n');
  };
  Module['printErr'] = function(x) {
    process['stderr'].write(x + '\n');
  };

  var nodeFS = require('fs');
  var nodePath = require('path');

  Module['read'] = function(filename) {
    filename = nodePath['normalize'](filename);
    var ret = nodeFS['readFileSync'](filename).toString();
    // The path is absolute if the normalized version is the same as the resolved.
    if (!ret && filename != nodePath['resolve'](filename)) {
      filename = path.join(__dirname, '..', 'src', filename);
      ret = nodeFS['readFileSync'](filename).toString();
    }
    return ret;
  };

  Module['load'] = function(f) {
    globalEval(read(f));
  };

  if (!Module['arguments']) {
    Module['arguments'] = process['argv'].slice(2);
  }
}

if (ENVIRONMENT_IS_SHELL) {
  Module['print'] = print;
  if (typeof printErr != 'undefined') Module['printErr'] = printErr; // not present in v8 or older sm

  // Polyfill over SpiderMonkey/V8 differences
  if (typeof read != 'undefined') {
    Module['read'] = read;
  } else {
    Module['read'] = function(f) { snarf(f) };
  }

  if (!Module['arguments']) {
    if (typeof scriptArgs != 'undefined') {
      Module['arguments'] = scriptArgs;
    } else if (typeof arguments != 'undefined') {
      Module['arguments'] = arguments;
    }
  }
}

if (ENVIRONMENT_IS_WEB && !ENVIRONMENT_IS_WORKER) {
  if (!Module['print']) {
    Module['print'] = function(x) {
      console.log(x);
    };
  }

  if (!Module['printErr']) {
    Module['printErr'] = function(x) {
      console.log(x);
    };
  }
}

if (ENVIRONMENT_IS_WEB || ENVIRONMENT_IS_WORKER) {
  Module['read'] = function(url) {
    var xhr = new XMLHttpRequest();
    xhr.open('GET', url, false);
    xhr.send(null);
    return xhr.responseText;
  };

  if (!Module['arguments']) {
    if (typeof arguments != 'undefined') {
      Module['arguments'] = arguments;
    }
  }
}

if (ENVIRONMENT_IS_WORKER) {
  // We can do very little here...
  var TRY_USE_DUMP = false;
  if (!Module['print']) {
    Module['print'] = (TRY_USE_DUMP && (typeof(dump) !== "undefined") ? (function(x) {
      dump(x);
    }) : (function(x) {
      // self.postMessage(x); // enable this if you want stdout to be sent as messages
    }));
  }

  Module['load'] = importScripts;
}

if (!ENVIRONMENT_IS_WORKER && !ENVIRONMENT_IS_WEB && !ENVIRONMENT_IS_NODE && !ENVIRONMENT_IS_SHELL) {
  // Unreachable because SHELL is dependant on the others
  throw 'Unknown runtime environment. Where are we?';
}

function globalEval(x) {
  eval.call(null, x);
}
if (!Module['load'] == 'undefined' && Module['read']) {
  Module['load'] = function(f) {
    globalEval(Module['read'](f));
  };
}
if (!Module['print']) {
  Module['print'] = function(){};
}
if (!Module['printErr']) {
  Module['printErr'] = Module['print'];
}
if (!Module['arguments']) {
  Module['arguments'] = [];
}
// *** Environment setup code ***

// Closure helpers
Module.print = Module['print'];
Module.printErr = Module['printErr'];

// Callbacks
if (!Module['preRun']) Module['preRun'] = [];
if (!Module['postRun']) Module['postRun'] = [];

  
// === Auto-generated preamble library stuff ===

//========================================
// Runtime code shared with compiler
//========================================

var Runtime = {
  stackSave: function () {
    return STACKTOP;
  },
  stackRestore: function (stackTop) {
    STACKTOP = stackTop;
  },
  forceAlign: function (target, quantum) {
    quantum = quantum || 4;
    if (quantum == 1) return target;
    if (isNumber(target) && isNumber(quantum)) {
      return Math.ceil(target/quantum)*quantum;
    } else if (isNumber(quantum) && isPowerOfTwo(quantum)) {
      var logg = log2(quantum);
      return '((((' +target + ')+' + (quantum-1) + ')>>' + logg + ')<<' + logg + ')';
    }
    return 'Math.ceil((' + target + ')/' + quantum + ')*' + quantum;
  },
  isNumberType: function (type) {
    return type in Runtime.INT_TYPES || type in Runtime.FLOAT_TYPES;
  },
  isPointerType: function isPointerType(type) {
  return type[type.length-1] == '*';
},
  isStructType: function isStructType(type) {
  if (isPointerType(type)) return false;
  if (/^\[\d+\ x\ (.*)\]/.test(type)) return true; // [15 x ?] blocks. Like structs
  if (/<?{ ?[^}]* ?}>?/.test(type)) return true; // { i32, i8 } etc. - anonymous struct types
  // See comment in isStructPointerType()
  return type[0] == '%';
},
  INT_TYPES: {"i1":0,"i8":0,"i16":0,"i32":0,"i64":0},
  FLOAT_TYPES: {"float":0,"double":0},
  BITSHIFT64_SHL: 0,
  BITSHIFT64_ASHR: 1,
  BITSHIFT64_LSHR: 2,
  bitshift64: function (low, high, op, bits) {
    var ret;
    var ander = Math.pow(2, bits)-1;
    if (bits < 32) {
      switch (op) {
        case Runtime.BITSHIFT64_SHL:
          ret = [low << bits, (high << bits) | ((low&(ander << (32 - bits))) >>> (32 - bits))];
          break;
        case Runtime.BITSHIFT64_ASHR:
          ret = [(((low >>> bits ) | ((high&ander) << (32 - bits))) >> 0) >>> 0, (high >> bits) >>> 0];
          break;
        case Runtime.BITSHIFT64_LSHR:
          ret = [((low >>> bits) | ((high&ander) << (32 - bits))) >>> 0, high >>> bits];
          break;
      }
    } else if (bits == 32) {
      switch (op) {
        case Runtime.BITSHIFT64_SHL:
          ret = [0, low];
          break;
        case Runtime.BITSHIFT64_ASHR:
          ret = [high, (high|0) < 0 ? ander : 0];
          break;
        case Runtime.BITSHIFT64_LSHR:
          ret = [high, 0];
          break;
      }
    } else { // bits > 32
      switch (op) {
        case Runtime.BITSHIFT64_SHL:
          ret = [0, low << (bits - 32)];
          break;
        case Runtime.BITSHIFT64_ASHR:
          ret = [(high >> (bits - 32)) >>> 0, (high|0) < 0 ? ander : 0];
          break;
        case Runtime.BITSHIFT64_LSHR:
          ret = [high >>>  (bits - 32) , 0];
          break;
      }
    }
    HEAP32[tempDoublePtr>>2] = ret[0]; // cannot use utility functions since we are in runtime itself
    HEAP32[tempDoublePtr+4>>2] = ret[1];
  },
  or64: function (x, y) {
    var l = (x | 0) | (y | 0);
    var h = (Math.round(x / 4294967296) | Math.round(y / 4294967296)) * 4294967296;
    return l + h;
  },
  and64: function (x, y) {
    var l = (x | 0) & (y | 0);
    var h = (Math.round(x / 4294967296) & Math.round(y / 4294967296)) * 4294967296;
    return l + h;
  },
  xor64: function (x, y) {
    var l = (x | 0) ^ (y | 0);
    var h = (Math.round(x / 4294967296) ^ Math.round(y / 4294967296)) * 4294967296;
    return l + h;
  },
  getNativeTypeSize: function (type, quantumSize) {
    if (Runtime.QUANTUM_SIZE == 1) return 1;
    var size = {
      '%i1': 1,
      '%i8': 1,
      '%i16': 2,
      '%i32': 4,
      '%i64': 8,
      "%float": 4,
      "%double": 8
    }['%'+type]; // add '%' since float and double confuse Closure compiler as keys, and also spidermonkey as a compiler will remove 's from '_i8' etc
    if (!size) {
      if (type.charAt(type.length-1) == '*') {
        size = Runtime.QUANTUM_SIZE; // A pointer
      } else if (type[0] == 'i') {
        var bits = parseInt(type.substr(1));
        assert(bits % 8 == 0);
        size = bits/8;
      }
    }
    return size;
  },
  getNativeFieldSize: function (type) {
    return Math.max(Runtime.getNativeTypeSize(type), Runtime.QUANTUM_SIZE);
  },
  dedup: function dedup(items, ident) {
  var seen = {};
  if (ident) {
    return items.filter(function(item) {
      if (seen[item[ident]]) return false;
      seen[item[ident]] = true;
      return true;
    });
  } else {
    return items.filter(function(item) {
      if (seen[item]) return false;
      seen[item] = true;
      return true;
    });
  }
},
  set: function set() {
  var args = typeof arguments[0] === 'object' ? arguments[0] : arguments;
  var ret = {};
  for (var i = 0; i < args.length; i++) {
    ret[args[i]] = 0;
  }
  return ret;
},
  calculateStructAlignment: function calculateStructAlignment(type) {
    type.flatSize = 0;
    type.alignSize = 0;
    var diffs = [];
    var prev = -1;
    type.flatIndexes = type.fields.map(function(field) {
      var size, alignSize;
      if (Runtime.isNumberType(field) || Runtime.isPointerType(field)) {
        size = Runtime.getNativeTypeSize(field); // pack char; char; in structs, also char[X]s.
        alignSize = size;
      } else if (Runtime.isStructType(field)) {
        size = Types.types[field].flatSize;
        alignSize = Types.types[field].alignSize;
      } else {
        throw 'Unclear type in struct: ' + field + ', in ' + type.name_ + ' :: ' + dump(Types.types[type.name_]);
      }
      alignSize = type.packed ? 1 : Math.min(alignSize, Runtime.QUANTUM_SIZE);
      type.alignSize = Math.max(type.alignSize, alignSize);
      var curr = Runtime.alignMemory(type.flatSize, alignSize); // if necessary, place this on aligned memory
      type.flatSize = curr + size;
      if (prev >= 0) {
        diffs.push(curr-prev);
      }
      prev = curr;
      return curr;
    });
    type.flatSize = Runtime.alignMemory(type.flatSize, type.alignSize);
    if (diffs.length == 0) {
      type.flatFactor = type.flatSize;
    } else if (Runtime.dedup(diffs).length == 1) {
      type.flatFactor = diffs[0];
    }
    type.needsFlattening = (type.flatFactor != 1);
    return type.flatIndexes;
  },
  generateStructInfo: function (struct, typeName, offset) {
    var type, alignment;
    if (typeName) {
      offset = offset || 0;
      type = (typeof Types === 'undefined' ? Runtime.typeInfo : Types.types)[typeName];
      if (!type) return null;
      if (type.fields.length != struct.length) {
        printErr('Number of named fields must match the type for ' + typeName + ': possibly duplicate struct names. Cannot return structInfo');
        return null;
      }
      alignment = type.flatIndexes;
    } else {
      var type = { fields: struct.map(function(item) { return item[0] }) };
      alignment = Runtime.calculateStructAlignment(type);
    }
    var ret = {
      __size__: type.flatSize
    };
    if (typeName) {
      struct.forEach(function(item, i) {
        if (typeof item === 'string') {
          ret[item] = alignment[i] + offset;
        } else {
          // embedded struct
          var key;
          for (var k in item) key = k;
          ret[key] = Runtime.generateStructInfo(item[key], type.fields[i], alignment[i]);
        }
      });
    } else {
      struct.forEach(function(item, i) {
        ret[item[1]] = alignment[i];
      });
    }
    return ret;
  },
  dynCall: function (sig, ptr, args) {
    if (args && args.length) {
      return FUNCTION_TABLE[ptr].apply(null, args);
    } else {
      return FUNCTION_TABLE[ptr]();
    }
  },
  addFunction: function (func, sig) {
    assert(sig);
    var table = FUNCTION_TABLE; // TODO: support asm
    var ret = table.length;
    table.push(func);
    table.push(0);
    return ret;
  },
  warnOnce: function (text) {
    if (!Runtime.warnOnce.shown) Runtime.warnOnce.shown = {};
    if (!Runtime.warnOnce.shown[text]) {
      Runtime.warnOnce.shown[text] = 1;
      Module.printErr(text);
    }
  },
  funcWrappers: {},
  getFuncWrapper: function (func, sig) {
    assert(sig);
    if (!Runtime.funcWrappers[func]) {
      Runtime.funcWrappers[func] = function() {
        Runtime.dynCall(sig, func, arguments);
      };
    }
    return Runtime.funcWrappers[func];
  },
  UTF8Processor: function () {
    var buffer = [];
    var needed = 0;
    this.processCChar = function (code) {
      code = code & 0xff;
      if (needed) {
        buffer.push(code);
        needed--;
      }
      if (buffer.length == 0) {
        if (code < 128) return String.fromCharCode(code);
        buffer.push(code);
        if (code > 191 && code < 224) {
          needed = 1;
        } else {
          needed = 2;
        }
        return '';
      }
      if (needed > 0) return '';
      var c1 = buffer[0];
      var c2 = buffer[1];
      var c3 = buffer[2];
      var ret;
      if (c1 > 191 && c1 < 224) {
        ret = String.fromCharCode(((c1 & 31) << 6) | (c2 & 63));
      } else {
        ret = String.fromCharCode(((c1 & 15) << 12) | ((c2 & 63) << 6) | (c3 & 63));
      }
      buffer.length = 0;
      return ret;
    }
    this.processJSString = function(string) {
      string = unescape(encodeURIComponent(string));
      var ret = [];
      for (var i = 0; i < string.length; i++) {
        ret.push(string.charCodeAt(i));
      }
      return ret;
    }
  },
  stackAlloc: function stackAlloc(size) { var ret = STACKTOP;STACKTOP = (STACKTOP + size)|0;STACKTOP = ((((STACKTOP)+3)>>2)<<2); return ret; },
  staticAlloc: function staticAlloc(size) { var ret = STATICTOP;STATICTOP = (STATICTOP + size)|0;STATICTOP = ((((STATICTOP)+3)>>2)<<2); if (STATICTOP >= TOTAL_MEMORY) enlargeMemory();; return ret; },
  alignMemory: function alignMemory(size,quantum) { var ret = size = Math.ceil((size)/(quantum ? quantum : 4))*(quantum ? quantum : 4); return ret; },
  makeBigInt: function makeBigInt(low,high,unsigned) { var ret = (unsigned ? (((low)>>>0)+(((high)>>>0)*4294967296)) : (((low)>>>0)+(((high)|0)*4294967296))); return ret; },
  QUANTUM_SIZE: 4,
  __dummy__: 0
}



var CorrectionsMonitor = {
  MAX_ALLOWED: 0, // XXX
  corrections: 0,
  sigs: {},

  note: function(type, succeed, sig) {
    if (!succeed) {
      this.corrections++;
      if (this.corrections >= this.MAX_ALLOWED) abort('\n\nToo many corrections!');
    }
  },

  print: function() {
  }
};





//========================================
// Runtime essentials
//========================================

var __THREW__ = 0; // Used in checking for thrown exceptions.
var setjmpId = 1; // Used in setjmp/longjmp
var setjmpLabels = {};

var ABORT = false;

var undef = 0;
// tempInt is used for 32-bit signed values or smaller. tempBigInt is used
// for 32-bit unsigned values or more than 32 bits. TODO: audit all uses of tempInt
var tempValue, tempInt, tempBigInt, tempInt2, tempBigInt2, tempPair, tempBigIntI, tempBigIntR, tempBigIntS, tempBigIntP, tempBigIntD;
var tempI64, tempI64b;
var tempRet0, tempRet1, tempRet2, tempRet3, tempRet4, tempRet5, tempRet6, tempRet7, tempRet8, tempRet9;

function abort(text) {
  Module.print(text + ':\n' + (new Error).stack);
  ABORT = true;
  throw "Assertion: " + text;
}

function assert(condition, text) {
  if (!condition) {
    abort('Assertion failed: ' + text);
  }
}

var globalScope = this;

// C calling interface. A convenient way to call C functions (in C files, or
// defined with extern "C").
//
// Note: LLVM optimizations can inline and remove functions, after which you will not be
//       able to call them. Closure can also do so. To avoid that, add your function to
//       the exports using something like
//
//         -s EXPORTED_FUNCTIONS='["_main", "_myfunc"]'
//
// @param ident      The name of the C function (note that C++ functions will be name-mangled - use extern "C")
// @param returnType The return type of the function, one of the JS types 'number', 'string' or 'array' (use 'number' for any C pointer, and
//                   'array' for JavaScript arrays and typed arrays).
// @param argTypes   An array of the types of arguments for the function (if there are no arguments, this can be ommitted). Types are as in returnType,
//                   except that 'array' is not possible (there is no way for us to know the length of the array)
// @param args       An array of the arguments to the function, as native JS values (as in returnType)
//                   Note that string arguments will be stored on the stack (the JS string will become a C string on the stack).
// @return           The return value, as a native JS value (as in returnType)
function ccall(ident, returnType, argTypes, args) {
  return ccallFunc(getCFunc(ident), returnType, argTypes, args);
}
Module["ccall"] = ccall;

// Returns the C function with a specified identifier (for C++, you need to do manual name mangling)
function getCFunc(ident) {
  try {
    var func = eval('_' + ident);
  } catch(e) {
    try {
      func = globalScope['Module']['_' + ident]; // closure exported function
    } catch(e) {}
  }
  assert(func, 'Cannot call unknown function ' + ident + ' (perhaps LLVM optimizations or closure removed it?)');
  return func;
}

// Internal function that does a C call using a function, not an identifier
function ccallFunc(func, returnType, argTypes, args) {
  var stack = 0;
  function toC(value, type) {
    if (type == 'string') {
      if (value === null || value === undefined || value === 0) return 0; // null string
      if (!stack) stack = Runtime.stackSave();
      var ret = Runtime.stackAlloc(value.length+1);
      writeStringToMemory(value, ret);
      return ret;
    } else if (type == 'array') {
      if (!stack) stack = Runtime.stackSave();
      var ret = Runtime.stackAlloc(value.length);
      writeArrayToMemory(value, ret);
      return ret;
    }
    return value;
  }
  function fromC(value, type) {
    if (type == 'string') {
      return Pointer_stringify(value);
    }
    assert(type != 'array');
    return value;
  }
  var i = 0;
  var cArgs = args ? args.map(function(arg) {
    return toC(arg, argTypes[i++]);
  }) : [];
  var ret = fromC(func.apply(null, cArgs), returnType);
  if (stack) Runtime.stackRestore(stack);
  return ret;
}

// Returns a native JS wrapper for a C function. This is similar to ccall, but
// returns a function you can call repeatedly in a normal way. For example:
//
//   var my_function = cwrap('my_c_function', 'number', ['number', 'number']);
//   alert(my_function(5, 22));
//   alert(my_function(99, 12));
//
function cwrap(ident, returnType, argTypes) {
  var func = getCFunc(ident);
  return function() {
    return ccallFunc(func, returnType, argTypes, Array.prototype.slice.call(arguments));
  }
}
Module["cwrap"] = cwrap;

// Sets a value in memory in a dynamic way at run-time. Uses the
// type data. This is the same as makeSetValue, except that
// makeSetValue is done at compile-time and generates the needed
// code then, whereas this function picks the right code at
// run-time.
// Note that setValue and getValue only do *aligned* writes and reads!
// Note that ccall uses JS types as for defining types, while setValue and
// getValue need LLVM types ('i8', 'i32') - this is a lower-level operation
function setValue(ptr, value, type, noSafe) {
  type = type || 'i8';
  if (type.charAt(type.length-1) === '*') type = 'i32'; // pointers are 32-bit
    switch(type) {
      case 'i1': HEAP8[(ptr)]=value; break;
      case 'i8': HEAP8[(ptr)]=value; break;
      case 'i16': HEAP16[((ptr)>>1)]=value; break;
      case 'i32': HEAP32[((ptr)>>2)]=value; break;
      case 'i64': (tempI64 = [value>>>0,Math.min(Math.floor((value)/4294967296), 4294967295)],HEAP32[((ptr)>>2)]=tempI64[0],HEAP32[(((ptr)+(4))>>2)]=tempI64[1]); break;
      case 'float': HEAPF32[((ptr)>>2)]=value; break;
      case 'double': (HEAPF64[(tempDoublePtr)>>3]=value,HEAP32[((ptr)>>2)]=HEAP32[((tempDoublePtr)>>2)],HEAP32[(((ptr)+(4))>>2)]=HEAP32[(((tempDoublePtr)+(4))>>2)]); break;
      default: abort('invalid type for setValue: ' + type);
    }
}
Module['setValue'] = setValue;

// Parallel to setValue.
function getValue(ptr, type, noSafe) {
  type = type || 'i8';
  if (type.charAt(type.length-1) === '*') type = 'i32'; // pointers are 32-bit
    switch(type) {
      case 'i1': return HEAP8[(ptr)];
      case 'i8': return HEAP8[(ptr)];
      case 'i16': return HEAP16[((ptr)>>1)];
      case 'i32': return HEAP32[((ptr)>>2)];
      case 'i64': return HEAP32[((ptr)>>2)];
      case 'float': return HEAPF32[((ptr)>>2)];
      case 'double': return (HEAP32[((tempDoublePtr)>>2)]=HEAP32[((ptr)>>2)],HEAP32[(((tempDoublePtr)+(4))>>2)]=HEAP32[(((ptr)+(4))>>2)],HEAPF64[(tempDoublePtr)>>3]);
      default: abort('invalid type for setValue: ' + type);
    }
  return null;
}
Module['getValue'] = getValue;

var ALLOC_NORMAL = 0; // Tries to use _malloc()
var ALLOC_STACK = 1; // Lives for the duration of the current function call
var ALLOC_STATIC = 2; // Cannot be freed
var ALLOC_NONE = 3; // Do not allocate
Module['ALLOC_NORMAL'] = ALLOC_NORMAL;
Module['ALLOC_STACK'] = ALLOC_STACK;
Module['ALLOC_STATIC'] = ALLOC_STATIC;
Module['ALLOC_NONE'] = ALLOC_NONE;

// allocate(): This is for internal use. You can use it yourself as well, but the interface
//             is a little tricky (see docs right below). The reason is that it is optimized
//             for multiple syntaxes to save space in generated code. So you should
//             normally not use allocate(), and instead allocate memory using _malloc(),
//             initialize it with setValue(), and so forth.
// @slab: An array of data, or a number. If a number, then the size of the block to allocate,
//        in *bytes* (note that this is sometimes confusing: the next parameter does not
//        affect this!)
// @types: Either an array of types, one for each byte (or 0 if no type at that position),
//         or a single type which is used for the entire block. This only matters if there
//         is initial data - if @slab is a number, then this does not matter at all and is
//         ignored.
// @allocator: How to allocate memory, see ALLOC_*
function allocate(slab, types, allocator, ptr) {
  var zeroinit, size;
  if (typeof slab === 'number') {
    zeroinit = true;
    size = slab;
  } else {
    zeroinit = false;
    size = slab.length;
  }

  var singleType = typeof types === 'string' ? types : null;

  var ret;
  if (allocator == ALLOC_NONE) {
    ret = ptr;
  } else {
    ret = [_malloc, Runtime.stackAlloc, Runtime.staticAlloc][allocator === undefined ? ALLOC_STATIC : allocator](Math.max(size, singleType ? 1 : types.length));
  }

  if (zeroinit) {
      _memset(ret, 0, size);
      return ret;
  }
  
  var i = 0, type;
  while (i < size) {
    var curr = slab[i];

    if (typeof curr === 'function') {
      curr = Runtime.getFunctionIndex(curr);
    }

    type = singleType || types[i];
    if (type === 0) {
      i++;
      continue;
    }

    if (type == 'i64') type = 'i32'; // special case: we have one i32 here, and one i32 later

    setValue(ret+i, curr, type);
    i += Runtime.getNativeTypeSize(type);
  }

  return ret;
}
Module['allocate'] = allocate;

function Pointer_stringify(ptr, /* optional */ length) {
  var utf8 = new Runtime.UTF8Processor();
  var nullTerminated = typeof(length) == "undefined";
  var ret = "";
  var i = 0;
  var t;
  while (1) {
    t = HEAPU8[((ptr)+(i))];
    if (nullTerminated && t == 0) break;
    ret += utf8.processCChar(t);
    i += 1;
    if (!nullTerminated && i == length) break;
  }
  return ret;
}
Module['Pointer_stringify'] = Pointer_stringify;

function Array_stringify(array) {
  var ret = "";
  for (var i = 0; i < array.length; i++) {
    ret += String.fromCharCode(array[i]);
  }
  return ret;
}
Module['Array_stringify'] = Array_stringify;

// Memory management

var PAGE_SIZE = 4096;
function alignMemoryPage(x) {
  return ((x+4095)>>12)<<12;
}

var HEAP;
var HEAP8, HEAPU8, HEAP16, HEAPU16, HEAP32, HEAPU32, HEAPF32, HEAPF64;

var STACK_ROOT, STACKTOP, STACK_MAX;
var STATICTOP;
function enlargeMemory() {
  abort('Cannot enlarge memory arrays. Either (1) compile with -s TOTAL_MEMORY=X with X higher than the current value, (2) compile with ALLOW_MEMORY_GROWTH which adjusts the size at runtime but prevents some optimizations, or (3) set Module.TOTAL_MEMORY before the program runs.');
}

var TOTAL_STACK = Module['TOTAL_STACK'] || 5242880;
var TOTAL_MEMORY = Module['TOTAL_MEMORY'] || 52428800;
var FAST_MEMORY = Module['FAST_MEMORY'] || 2097152;

// Initialize the runtime's memory
// check for full engine support (use string 'subarray' to avoid closure compiler confusion)
  assert(!!Int32Array && !!Float64Array && !!(new Int32Array(1)['subarray']) && !!(new Int32Array(1)['set']),
         'Cannot fallback to non-typed array case: Code is too specialized');

  var buffer = new ArrayBuffer(TOTAL_MEMORY);
  HEAP8 = new Int8Array(buffer);
  HEAP16 = new Int16Array(buffer);
  HEAP32 = new Int32Array(buffer);
  HEAPU8 = new Uint8Array(buffer);
  HEAPU16 = new Uint16Array(buffer);
  HEAPU32 = new Uint32Array(buffer);
  HEAPF32 = new Float32Array(buffer);
  HEAPF64 = new Float64Array(buffer);

  // Endianness check (note: assumes compiler arch was little-endian)
  HEAP32[0] = 255;
  assert(HEAPU8[0] === 255 && HEAPU8[3] === 0, 'Typed arrays 2 must be run on a little-endian system');

Module['HEAP'] = HEAP;
Module['HEAP8'] = HEAP8;
Module['HEAP16'] = HEAP16;
Module['HEAP32'] = HEAP32;
Module['HEAPU8'] = HEAPU8;
Module['HEAPU16'] = HEAPU16;
Module['HEAPU32'] = HEAPU32;
Module['HEAPF32'] = HEAPF32;
Module['HEAPF64'] = HEAPF64;

STACK_ROOT = STACKTOP = Runtime.alignMemory(1);
STACK_MAX = TOTAL_STACK; // we lose a little stack here, but TOTAL_STACK is nice and round so use that as the max

var tempDoublePtr = Runtime.alignMemory(allocate(12, 'i8', ALLOC_STACK), 8);
assert(tempDoublePtr % 8 == 0);
function copyTempFloat(ptr) { // functions, because inlining this code is increases code size too much
  HEAP8[tempDoublePtr] = HEAP8[ptr];
  HEAP8[tempDoublePtr+1] = HEAP8[ptr+1];
  HEAP8[tempDoublePtr+2] = HEAP8[ptr+2];
  HEAP8[tempDoublePtr+3] = HEAP8[ptr+3];
}
function copyTempDouble(ptr) {
  HEAP8[tempDoublePtr] = HEAP8[ptr];
  HEAP8[tempDoublePtr+1] = HEAP8[ptr+1];
  HEAP8[tempDoublePtr+2] = HEAP8[ptr+2];
  HEAP8[tempDoublePtr+3] = HEAP8[ptr+3];
  HEAP8[tempDoublePtr+4] = HEAP8[ptr+4];
  HEAP8[tempDoublePtr+5] = HEAP8[ptr+5];
  HEAP8[tempDoublePtr+6] = HEAP8[ptr+6];
  HEAP8[tempDoublePtr+7] = HEAP8[ptr+7];
}

STATICTOP = STACK_MAX;
assert(STATICTOP < TOTAL_MEMORY); // Stack must fit in TOTAL_MEMORY; allocations from here on may enlarge TOTAL_MEMORY

var nullString = allocate(intArrayFromString('(null)'), 'i8', ALLOC_STACK);

function callRuntimeCallbacks(callbacks) {
  while(callbacks.length > 0) {
    var callback = callbacks.shift();
    var func = callback.func;
    if (typeof func === 'number') {
      if (callback.arg === undefined) {
        Runtime.dynCall('v', func);
      } else {
        Runtime.dynCall('vi', func, [callback.arg]);
      }
    } else {
      func(callback.arg === undefined ? null : callback.arg);
    }
  }
}

var __ATINIT__ = []; // functions called during startup
var __ATMAIN__ = []; // functions called when main() is to be run
var __ATEXIT__ = []; // functions called during shutdown

function initRuntime() {
  callRuntimeCallbacks(__ATINIT__);
}
function preMain() {
  callRuntimeCallbacks(__ATMAIN__);
}
function exitRuntime() {
  callRuntimeCallbacks(__ATEXIT__);

  // Print summary of correction activity
  CorrectionsMonitor.print();
}

function String_len(ptr) {
  var i = ptr;
  while (HEAP8[(i++)]) { // Note: should be |!= 0|, technically. But this helps catch bugs with undefineds
  }
  return i - ptr - 1;
}
Module['String_len'] = String_len;

// Tools

// This processes a JS string into a C-line array of numbers, 0-terminated.
// For LLVM-originating strings, see parser.js:parseLLVMString function
function intArrayFromString(stringy, dontAddNull, length /* optional */) {
  var ret = (new Runtime.UTF8Processor()).processJSString(stringy);
  if (length) {
    ret.length = length;
  }
  if (!dontAddNull) {
    ret.push(0);
  }
  return ret;
}
Module['intArrayFromString'] = intArrayFromString;

function intArrayToString(array) {
  var ret = [];
  for (var i = 0; i < array.length; i++) {
    var chr = array[i];
    if (chr > 0xFF) {
      chr &= 0xFF;
    }
    ret.push(String.fromCharCode(chr));
  }
  return ret.join('');
}
Module['intArrayToString'] = intArrayToString;

// Write a Javascript array to somewhere in the heap
function writeStringToMemory(string, buffer, dontAddNull) {
  var array = intArrayFromString(string, dontAddNull);
  var i = 0;
  while (i < array.length) {
    var chr = array[i];
    HEAP8[((buffer)+(i))]=chr
    i = i + 1;
  }
}
Module['writeStringToMemory'] = writeStringToMemory;

function writeArrayToMemory(array, buffer) {
  for (var i = 0; i < array.length; i++) {
    HEAP8[((buffer)+(i))]=array[i];
  }
}
Module['writeArrayToMemory'] = writeArrayToMemory;

function unSign(value, bits, ignore, sig) {
  if (value >= 0) {
    return value;
  }
  return bits <= 32 ? 2*Math.abs(1 << (bits-1)) + value // Need some trickery, since if bits == 32, we are right at the limit of the bits JS uses in bitshifts
                    : Math.pow(2, bits)         + value;
  // TODO: clean up previous line
}
function reSign(value, bits, ignore, sig) {
  if (value <= 0) {
    return value;
  }
  var half = bits <= 32 ? Math.abs(1 << (bits-1)) // abs is needed if bits == 32
                        : Math.pow(2, bits-1);
  if (value >= half && (bits <= 32 || value > half)) { // for huge values, we can hit the precision limit and always get true here. so don't do that
                                                       // but, in general there is no perfect solution here. With 64-bit ints, we get rounding and errors
                                                       // TODO: In i64 mode 1, resign the two parts separately and safely
    value = -2*half + value; // Cannot bitshift half, as it may be at the limit of the bits JS uses in bitshifts
  }
  return value;
}

// A counter of dependencies for calling run(). If we need to
// do asynchronous work before running, increment this and
// decrement it. Incrementing must happen in a place like
// PRE_RUN_ADDITIONS (used by emcc to add file preloading).
// Note that you can add dependencies in preRun, even though
// it happens right before run - run will be postponed until
// the dependencies are met.
var runDependencies = 0;
var runDependencyTracking = {};
var calledRun = false;
var runDependencyWatcher = null;
function addRunDependency(id) {
  runDependencies++;
  if (Module['monitorRunDependencies']) {
    Module['monitorRunDependencies'](runDependencies);
  }
  if (id) {
    assert(!runDependencyTracking[id]);
    runDependencyTracking[id] = 1;
    if (runDependencyWatcher === null && typeof setInterval !== 'undefined') {
      // Check for missing dependencies every few seconds
      runDependencyWatcher = setInterval(function() {
        var shown = false;
        for (var dep in runDependencyTracking) {
          if (!shown) {
            shown = true;
            Module.printErr('still waiting on run dependencies:');
          }
          Module.printErr('dependency: ' + dep);
        }
        if (shown) {
          Module.printErr('(end of list)');
        }
      }, 6000);
    }
  } else {
    Module.printErr('warning: run dependency added without ID');
  }
}
Module['addRunDependency'] = addRunDependency;
function removeRunDependency(id) {
  runDependencies--;
  if (Module['monitorRunDependencies']) {
    Module['monitorRunDependencies'](runDependencies);
  }
  if (id) {
    assert(runDependencyTracking[id]);
    delete runDependencyTracking[id];
  } else {
    Module.printErr('warning: run dependency removed without ID');
  }
  if (runDependencies == 0) {
    if (runDependencyWatcher !== null) {
      clearInterval(runDependencyWatcher);
      runDependencyWatcher = null;
    } 
    // If run has never been called, and we should call run (INVOKE_RUN is true, and Module.noInitialRun is not false)
    if (!calledRun && shouldRunNow) run();
  }
}
Module['removeRunDependency'] = removeRunDependency;

Module["preloadedImages"] = {}; // maps url to image data
Module["preloadedAudios"] = {}; // maps url to audio data

// === Body ===



assert(STATICTOP == STACK_MAX); assert(STACK_MAX == TOTAL_STACK);

STATICTOP += 1916;

assert(STATICTOP < TOTAL_MEMORY);



__ATINIT__ = __ATINIT__.concat([
  { func: function() { __GLOBAL__I_a() } }
]);







































var ___dso_handle;








var __ZTVN10__cxxabiv120__si_class_type_infoE;
var __ZTVN10__cxxabiv117__class_type_infoE;














var __ZTISt9type_info;
var __ZTISt9exception;

















allocate(24, "i8", ALLOC_NONE, 5242880);
allocate([33,68,101,102,97,117,108,116,84,97,114,103,101,116,0] /* !DefaultTarget\00 */, "i8", ALLOC_NONE, 5242904);
allocate([108,97,98,101,108,32,61,32,48,59,10,0] /* label = 0;\0A\00 */, "i8", ALLOC_NONE, 5242920);
allocate([66,114,97,110,99,104,101,115,79,117,116,46,102,105,110,100,40,84,97,114,103,101,116,41,32,61,61,32,66,114,97,110,99,104,101,115,79,117,116,46,101,110,100,40,41,0] /* BranchesOut.find(Tar */, "i8", ALLOC_NONE, 5242932);
allocate([114,101,108,111,111,112,101,114,47,82,101,108,111,111,112,101,114,46,99,112,112,0] /* relooper/Relooper.cp */, "i8", ALLOC_NONE, 5242980);
allocate([37,115,59,10,0] /* %s;\0A\00 */, "i8", ALLOC_NONE, 5243004);
allocate([99,111,110,116,105,110,117,101,0] /* continue\00 */, "i8", ALLOC_NONE, 5243012);
allocate([119,114,105,116,116,101,110,32,60,32,108,101,102,116,0] /* written _ left\00 */, "i8", ALLOC_NONE, 5243024);
allocate([110,101,101,100,101,100,32,60,32,108,101,102,116,0] /* needed _ left\00 */, "i8", ALLOC_NONE, 5243040);
allocate([115,116,100,58,58,98,97,100,95,97,108,108,111,99,0] /* std::bad_alloc\00 */, "i8", ALLOC_NONE, 5243056);
allocate([79,117,116,112,117,116,66,117,102,102,101,114,32,43,32,73,110,100,101,110,116,101,114,58,58,67,117,114,114,73,110,100,101,110,116,42,50,32,45,32,79,117,116,112,117,116,66,117,102,102,101,114,82,111,111,116,32,60,32,79,117,116,112,117,116,66,117,102,102,101,114,83,105,122,101,0] /* OutputBuffer + Inden */, "i8", ALLOC_NONE, 5243072);
allocate([79,117,116,112,117,116,66,117,102,102,101,114,0] /* OutputBuffer\00 */, "i8", ALLOC_NONE, 5243148);
allocate([73,110,110,101,114,66,108,111,99,107,115,46,115,105,122,101,40,41,32,62,32,48,0] /* InnerBlocks.size() _ */, "i8", ALLOC_NONE, 5243164);
allocate([76,111,111,112,83,116,97,99,107,46,115,105,122,101,40,41,32,62,32,48,0] /* LoopStack.size() _ 0 */, "i8", ALLOC_NONE, 5243188);
allocate([98,114,101,97,107,0] /* break\00 */, "i8", ALLOC_NONE, 5243212);
allocate([119,104,105,108,101,40,49,41,32,123,10,0] /* while(1) {\0A\00 */, "i8", ALLOC_NONE, 5243220);
allocate([76,37,100,58,32,119,104,105,108,101,40,49,41,32,123,10,0] /* L%d: while(1) {\0A\0 */, "i8", ALLOC_NONE, 5243232);
allocate([37,115,105,102,32,40,108,97,98,101,108,32,61,61,32,37,100,41,32,123,10,0] /* %sif (label == %d) { */, "i8", ALLOC_NONE, 5243252);
allocate([101,108,115,101,32,0] /* else \00 */, "i8", ALLOC_NONE, 5243276);
allocate([37,115,105,102,32,40,40,108,97,98,101,108,124,48,41,32,61,61,32,37,100,41,32,123,10,0] /* %sif ((label|0) == % */, "i8", ALLOC_NONE, 5243284);
allocate([125,32,119,104,105,108,101,40,48,41,59,10,0] /* } while(0);\0A\00 */, "i8", ALLOC_NONE, 5243312);
allocate([100,111,32,123,10,0] /* do {\0A\00 */, "i8", ALLOC_NONE, 5243328);
allocate([76,37,100,58,32,100,111,32,123,10,0] /* L%d: do {\0A\00 */, "i8", ALLOC_NONE, 5243336);
allocate([125,10,0] /* }\0A\00 */, "i8", ALLOC_NONE, 5243348);
allocate([125,32,101,108,115,101,32,123,10,0] /* } else {\0A\00 */, "i8", ALLOC_NONE, 5243352);
allocate([37,115,32,76,37,100,59,10,0] /* %s L%d;\0A\00 */, "i8", ALLOC_NONE, 5243364);
allocate([125,32,101,108,115,101,32,105,102,32,40,37,115,41,32,123,10,0] /* } else if (%s) {\0A\ */, "i8", ALLOC_NONE, 5243376);
allocate([105,102,32,40,37,115,41,32,123,10,0] /* if (%s) {\0A\00 */, "i8", ALLOC_NONE, 5243396);
allocate([41,0] /* )\00 */, "i8", ALLOC_NONE, 5243408);
allocate([33,40,0] /* !(\00 */, "i8", ALLOC_NONE, 5243412);
allocate([32,38,38,32,0] /*  && \00 */, "i8", ALLOC_NONE, 5243416);
allocate([125,32,101,108,115,101,32,0] /* } else \00 */, "i8", ALLOC_NONE, 5243424);
allocate(1, "i8", ALLOC_NONE, 5243432);
allocate([37,115,105,102,32,40,37,115,41,32,123,10,0] /* %sif (%s) {\0A\00 */, "i8", ALLOC_NONE, 5243436);
allocate([68,101,116,97,105,108,115,45,62,67,111,110,100,105,116,105,111,110,0] /* Details-_Condition\0 */, "i8", ALLOC_NONE, 5243452);
allocate([68,101,102,97,117,108,116,84,97,114,103,101,116,0] /* DefaultTarget\00 */, "i8", ALLOC_NONE, 5243472);
allocate([108,97,98,101,108,32,61,32,37,100,59,10,0] /* label = %d;\0A\00 */, "i8", ALLOC_NONE, 5243488);
allocate([37,115,10,0] /* %s\0A\00 */, "i8", ALLOC_NONE, 5243504);
allocate(468, "i8", ALLOC_NONE, 5243508);
allocate(12, "i8", ALLOC_NONE, 5243976);
allocate([83,104,97,112,101,32,42,82,101,108,111,111,112,101,114,58,58,67,97,108,99,117,108,97,116,101,40,66,108,111,99,107,32,42,41,58,58,65,110,97,108,121,122,101,114,58,58,77,97,107,101,76,111,111,112,40,66,108,111,99,107,83,101,116,32,38,44,32,66,108,111,99,107,83,101,116,32,38,44,32,66,108,111,99,107,83,101,116,32,38,41,0] /* Shape _Relooper::Cal */, "i8", ALLOC_NONE, 5243988);
allocate([118,111,105,100,32,82,101,108,111,111,112,101,114,58,58,67,97,108,99,117,108,97,116,101,40,66,108,111,99,107,32,42,41,58,58,80,111,115,116,79,112,116,105,109,105,122,101,114,58,58,70,105,110,100,76,97,98,101,108,101,100,76,111,111,112,115,40,83,104,97,112,101,32,42,41,0] /* void Relooper::Calcu */, "i8", ALLOC_NONE, 5244080);
allocate([118,111,105,100,32,66,108,111,99,107,58,58,82,101,110,100,101,114,40,98,111,111,108,41,0] /* void Block::Render(b */, "i8", ALLOC_NONE, 5244156);
allocate([118,111,105,100,32,66,108,111,99,107,58,58,65,100,100,66,114,97,110,99,104,84,111,40,66,108,111,99,107,32,42,44,32,99,111,110,115,116,32,99,104,97,114,32,42,44,32,99,111,110,115,116,32,99,104,97,114,32,42,41,0] /* void Block::AddBranc */, "i8", ALLOC_NONE, 5244184);
allocate([118,111,105,100,32,80,114,105,110,116,73,110,100,101,110,116,101,100,40,99,111,110,115,116,32,99,104,97,114,32,42,44,32,46,46,46,41,0] /* void PrintIndented(c */, "i8", ALLOC_NONE, 5244248);
allocate([118,111,105,100,32,80,117,116,73,110,100,101,110,116,101,100,40,99,111,110,115,116,32,99,104,97,114,32,42,41,0] /* void PutIndented(con */, "i8", ALLOC_NONE, 5244288);
allocate([0, 0, 0, 0, 5244660, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0], ["*",0,0,0,"*",0,0,0,"*",0,0,0,"*",0,0,0,"*",0,0,0], ALLOC_NONE, 5244320);
allocate(1, "i8", ALLOC_NONE, 5244340);
__ZTVN10__cxxabiv120__si_class_type_infoE=allocate([0, 0, 0, 0, 5244672, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0], ["*",0,0,0,"*",0,0,0,"*",0,0,0,"*",0,0,0,"*",0,0,0,"*",0,0,0,"*",0,0,0,"*",0,0,0,"*",0,0,0,"*",0,0,0], ALLOC_STATIC);
allocate(1, "i8", ALLOC_STATIC);
__ZTVN10__cxxabiv117__class_type_infoE=allocate([0, 0, 0, 0, 5244684, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0], ["*",0,0,0,"*",0,0,0,"*",0,0,0,"*",0,0,0,"*",0,0,0,"*",0,0,0,"*",0,0,0,"*",0,0,0,"*",0,0,0,"*",0,0,0], ALLOC_STATIC);
allocate(1, "i8", ALLOC_STATIC);
allocate([0, 0, 0, 0, 5244708, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0], ["*",0,0,0,"*",0,0,0,"*",0,0,0,"*",0,0,0,"*",0,0,0], ALLOC_NONE, 5244344);
allocate(1, "i8", ALLOC_NONE, 5244364);
allocate([0, 0, 0, 0, 5244720, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0], ["*",0,0,0,"*",0,0,0,"*",0,0,0,"*",0,0,0,"*",0,0,0], ALLOC_NONE, 5244368);
allocate(1, "i8", ALLOC_NONE, 5244388);
allocate([0, 0, 0, 0, 5244728, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0], ["*",0,0,0,"*",0,0,0,"*",0,0,0,"*",0,0,0,"*",0,0,0], ALLOC_NONE, 5244392);
allocate(1, "i8", ALLOC_NONE, 5244412);
allocate([0, 0, 0, 0, 5244740, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0], ["*",0,0,0,"*",0,0,0,"*",0,0,0,"*",0,0,0,"*",0,0,0], ALLOC_NONE, 5244416);
allocate(1, "i8", ALLOC_NONE, 5244436);
allocate([0, 0, 0, 0, 5244752, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0], ["*",0,0,0,"*",0,0,0,"*",0,0,0,"*",0,0,0,"*",0,0,0], ALLOC_NONE, 5244440);
allocate(1, "i8", ALLOC_NONE, 5244460);
allocate([83,116,57,98,97,100,95,97,108,108,111,99,0] /* St9bad_alloc\00 */, "i8", ALLOC_NONE, 5244464);
allocate([78,49,48,95,95,99,120,120,97,98,105,118,49,50,48,95,95,115,105,95,99,108,97,115,115,95,116,121,112,101,95,105,110,102,111,69,0] /* N10__cxxabiv120__si_ */, "i8", ALLOC_NONE, 5244480);
allocate([78,49,48,95,95,99,120,120,97,98,105,118,49,49,55,95,95,99,108,97,115,115,95,116,121,112,101,95,105,110,102,111,69,0] /* N10__cxxabiv117__cla */, "i8", ALLOC_NONE, 5244520);
allocate([78,49,48,95,95,99,120,120,97,98,105,118,49,49,54,95,95,115,104,105,109,95,116,121,112,101,95,105,110,102,111,69,0] /* N10__cxxabiv116__shi */, "i8", ALLOC_NONE, 5244556);
allocate([57,76,111,111,112,83,104,97,112,101,0] /* 9LoopShape\00 */, "i8", ALLOC_NONE, 5244592);
allocate([53,83,104,97,112,101,0] /* 5Shape\00 */, "i8", ALLOC_NONE, 5244604);
allocate([49,51,77,117,108,116,105,112,108,101,83,104,97,112,101,0] /* 13MultipleShape\00 */, "i8", ALLOC_NONE, 5244612);
allocate([49,50,76,97,98,101,108,101,100,83,104,97,112,101,0] /* 12LabeledShape\00 */, "i8", ALLOC_NONE, 5244628);
allocate([49,49,83,105,109,112,108,101,83,104,97,112,101,0] /* 11SimpleShape\00 */, "i8", ALLOC_NONE, 5244644);
allocate(12, "i8", ALLOC_NONE, 5244660);
allocate([0, 0, 0, 0, 0, 0, 0, 0, 5244684, 0, 0, 0], ["*",0,0,0,"*",0,0,0,"*",0,0,0], ALLOC_NONE, 5244672);
allocate([0, 0, 0, 0, 0, 0, 0, 0, 5244696, 0, 0, 0], ["*",0,0,0,"*",0,0,0,"*",0,0,0], ALLOC_NONE, 5244684);
allocate(12, "i8", ALLOC_NONE, 5244696);
allocate([0, 0, 0, 0, 0, 0, 0, 0, 5244740, 0, 0, 0], ["*",0,0,0,"*",0,0,0,"*",0,0,0], ALLOC_NONE, 5244708);
allocate(8, "i8", ALLOC_NONE, 5244720);
allocate([0, 0, 0, 0, 0, 0, 0, 0, 5244740, 0, 0, 0], ["*",0,0,0,"*",0,0,0,"*",0,0,0], ALLOC_NONE, 5244728);
allocate([0, 0, 0, 0, 0, 0, 0, 0, 5244720, 0, 0, 0], ["*",0,0,0,"*",0,0,0,"*",0,0,0], ALLOC_NONE, 5244740);
allocate([0, 0, 0, 0, 0, 0, 0, 0, 5244720, 0, 0, 0], ["*",0,0,0,"*",0,0,0,"*",0,0,0], ALLOC_NONE, 5244752);
allocate([1], ["i32",0,0,0], ALLOC_NONE, 5244764);
allocate(4, "i8", ALLOC_NONE, 5244768);
allocate([1], ["i32",0,0,0], ALLOC_NONE, 5244772);
allocate(4, "i8", ALLOC_NONE, 5244776);
allocate(4, "i8", ALLOC_NONE, 5244780);
allocate(4, "i8", ALLOC_NONE, 5244784);
allocate(4, "i8", ALLOC_NONE, 5244788);
allocate(4, "i8", ALLOC_NONE, 5244792);
HEAP32[((5244328)>>2)]=(42);
HEAP32[((5244332)>>2)]=(8);
HEAP32[((5244336)>>2)]=(16);
HEAP32[(((__ZTVN10__cxxabiv120__si_class_type_infoE)+(8))>>2)]=(44);
HEAP32[(((__ZTVN10__cxxabiv120__si_class_type_infoE)+(12))>>2)]=(50);
HEAP32[(((__ZTVN10__cxxabiv120__si_class_type_infoE)+(16))>>2)]=(38);
HEAP32[(((__ZTVN10__cxxabiv120__si_class_type_infoE)+(20))>>2)]=(52);
HEAP32[(((__ZTVN10__cxxabiv120__si_class_type_infoE)+(24))>>2)]=(28);
HEAP32[(((__ZTVN10__cxxabiv120__si_class_type_infoE)+(28))>>2)]=(30);
HEAP32[(((__ZTVN10__cxxabiv120__si_class_type_infoE)+(32))>>2)]=(18);
HEAP32[(((__ZTVN10__cxxabiv120__si_class_type_infoE)+(36))>>2)]=(56);
HEAP32[(((__ZTVN10__cxxabiv117__class_type_infoE)+(8))>>2)]=(10);
HEAP32[(((__ZTVN10__cxxabiv117__class_type_infoE)+(12))>>2)]=(14);
HEAP32[(((__ZTVN10__cxxabiv117__class_type_infoE)+(16))>>2)]=(38);
HEAP32[(((__ZTVN10__cxxabiv117__class_type_infoE)+(20))>>2)]=(52);
HEAP32[(((__ZTVN10__cxxabiv117__class_type_infoE)+(24))>>2)]=(28);
HEAP32[(((__ZTVN10__cxxabiv117__class_type_infoE)+(28))>>2)]=(4);
HEAP32[(((__ZTVN10__cxxabiv117__class_type_infoE)+(32))>>2)]=(32);
HEAP32[(((__ZTVN10__cxxabiv117__class_type_infoE)+(36))>>2)]=(46);
HEAP32[((5244352)>>2)]=(2);
HEAP32[((5244356)>>2)]=(26);
HEAP32[((5244360)>>2)]=(22);
HEAP32[((5244376)>>2)]=(58);
HEAP32[((5244380)>>2)]=(6);
HEAP32[((5244384)>>2)]=(60);
HEAP32[((5244400)>>2)]=(40);
HEAP32[((5244404)>>2)]=(62);
HEAP32[((5244408)>>2)]=(20);
HEAP32[((5244424)>>2)]=(48);
HEAP32[((5244428)>>2)]=(12);
HEAP32[((5244432)>>2)]=(60);
HEAP32[((5244448)>>2)]=(34);
HEAP32[((5244452)>>2)]=(36);
HEAP32[((5244456)>>2)]=(24);
HEAP32[((5244660)>>2)]=(((__ZTVN10__cxxabiv120__si_class_type_infoE+8)|0));
HEAP32[((5244664)>>2)]=((5244464)|0);
HEAP32[((5244668)>>2)]=__ZTISt9exception;
HEAP32[((5244672)>>2)]=(((__ZTVN10__cxxabiv120__si_class_type_infoE+8)|0));
HEAP32[((5244676)>>2)]=((5244480)|0);
HEAP32[((5244684)>>2)]=(((__ZTVN10__cxxabiv120__si_class_type_infoE+8)|0));
HEAP32[((5244688)>>2)]=((5244520)|0);
HEAP32[((5244696)>>2)]=(((__ZTVN10__cxxabiv120__si_class_type_infoE+8)|0));
HEAP32[((5244700)>>2)]=((5244556)|0);
HEAP32[((5244704)>>2)]=__ZTISt9type_info;
HEAP32[((5244708)>>2)]=(((__ZTVN10__cxxabiv120__si_class_type_infoE+8)|0));
HEAP32[((5244712)>>2)]=((5244592)|0);
HEAP32[((5244720)>>2)]=(((__ZTVN10__cxxabiv117__class_type_infoE+8)|0));
HEAP32[((5244724)>>2)]=((5244604)|0);
HEAP32[((5244728)>>2)]=(((__ZTVN10__cxxabiv120__si_class_type_infoE+8)|0));
HEAP32[((5244732)>>2)]=((5244612)|0);
HEAP32[((5244740)>>2)]=(((__ZTVN10__cxxabiv120__si_class_type_infoE+8)|0));
HEAP32[((5244744)>>2)]=((5244628)|0);
HEAP32[((5244752)>>2)]=(((__ZTVN10__cxxabiv120__si_class_type_infoE+8)|0));
HEAP32[((5244756)>>2)]=((5244644)|0);

  function _strdup(ptr) {
      var len = String_len(ptr);
      var newStr = _malloc(len + 1);
      _memcpy(newStr, ptr, len);
      HEAP8[((newStr)+(len))]=0;
      return newStr;
    }

  function ___gxx_personality_v0() {
    }
var __ZSt9terminatev; // stub for __ZSt9terminatev

  function ___assert_func(filename, line, func, condition) {
      throw 'Assertion failed: ' + (condition ? Pointer_stringify(condition) : 'unknown condition') + ', at: ' + [filename ? Pointer_stringify(filename) : 'unknown filename', line, func ? Pointer_stringify(func) : 'unknown function'] + ' at ' + new Error().stack;
    }

  function _strchr(ptr, chr) {
      ptr--;
      do {
        ptr++;
        var val = HEAP8[(ptr)];
        if (val == chr) return ptr;
      } while (val);
      return 0;
    }

  
  function _atexit(func, arg) {
      __ATEXIT__.unshift({ func: func, arg: arg });
    }var ___cxa_atexit;

  var _llvm_memcpy_p0i8_p0i8_i32;

  function ___cxa_call_unexpected(exception) {
      ABORT = true;
      throw exception;
    }

  
  function __ZSt18uncaught_exceptionv() { // std::uncaught_exception()
      return !!__ZSt18uncaught_exceptionv.uncaught_exception;
    }function ___cxa_begin_catch(ptr) {
      __ZSt18uncaught_exceptionv.uncaught_exception--;
      return ptr;
    }

  
  function _llvm_eh_exception() {
      return HEAP32[((_llvm_eh_exception.buf)>>2)];
    }
  
  
  function ___cxa_free_exception(ptr) {
      return _free(ptr);
    }function ___cxa_end_catch() {
      if (___cxa_end_catch.rethrown) {
        ___cxa_end_catch.rethrown = false;
        return;
      }
      // Clear state flag.
      __THREW__ = 0;
      // Clear type.
      HEAP32[(((_llvm_eh_exception.buf)+(4))>>2)]=0
      // Call destructor if one is registered then clear it.
      var ptr = HEAP32[((_llvm_eh_exception.buf)>>2)];
      var destructor = HEAP32[(((_llvm_eh_exception.buf)+(8))>>2)];
      if (destructor) {
        Runtime.dynCall('vi', destructor, [ptr]);
        HEAP32[(((_llvm_eh_exception.buf)+(8))>>2)]=0
      }
      // Free ptr if it isn't null.
      if (ptr) {
        ___cxa_free_exception(ptr);
        HEAP32[((_llvm_eh_exception.buf)>>2)]=0
      }
    }function ___cxa_rethrow() {
      ___cxa_end_catch.rethrown = true;
      throw HEAP32[((_llvm_eh_exception.buf)>>2)] + " - Exception catching is disabled, this exception cannot be caught. Compile with -s DISABLE_EXCEPTION_CATCHING=0 to catch.";;
    }


  
  function _memmove(dest, src, num, align) {
      if (src < dest && dest < src + num) {
        // Copy backwards in a safe manner
        src += num;
        dest += num;
        while (num--) {
          dest--;
          src--;
          HEAP8[(dest)]=HEAP8[(src)];
        }
      } else {
        _memcpy(dest, src, num);
      }
    }var _llvm_memmove_p0i8_p0i8_i32;

  function ___cxa_pure_virtual() {
      ABORT = true;
      throw 'Pure virtual function called!';
    }

  function _strlen(ptr) {
      return String_len(ptr);
    }

  function _strcpy(pdest, psrc) {
      var i = 0;
      do {
        HEAP8[(pdest+i)]=HEAP8[(psrc+i)];
        i ++;
      } while (HEAP8[((psrc)+(i-1))] != 0);
      return pdest;
    }
var _llvm_va_start; // stub for _llvm_va_start

  
  
  function __formatString(format, varargs) {
      var textIndex = format;
      var argIndex = 0;
      function getNextArg(type) {
        // NOTE: Explicitly ignoring type safety. Otherwise this fails:
        //       int x = 4; printf("%c\n", (char)x);
        var ret;
        if (type === 'double') {
          ret = (HEAP32[((tempDoublePtr)>>2)]=HEAP32[(((varargs)+(argIndex))>>2)],HEAP32[(((tempDoublePtr)+(4))>>2)]=HEAP32[(((varargs)+((argIndex)+(4)))>>2)],HEAPF64[(tempDoublePtr)>>3]);
        } else if (type == 'i64') {
          ret = [HEAP32[(((varargs)+(argIndex))>>2)],
                 HEAP32[(((varargs)+(argIndex+4))>>2)]];
        } else {
          type = 'i32'; // varargs are always i32, i64, or double
          ret = HEAP32[(((varargs)+(argIndex))>>2)];
        }
        argIndex += Runtime.getNativeFieldSize(type);
        return ret;
      }
  
      var ret = [];
      var curr, next, currArg;
      while(1) {
        var startTextIndex = textIndex;
        curr = HEAP8[(textIndex)];
        if (curr === 0) break;
        next = HEAP8[(textIndex+1)];
        if (curr == '%'.charCodeAt(0)) {
          // Handle flags.
          var flagAlwaysSigned = false;
          var flagLeftAlign = false;
          var flagAlternative = false;
          var flagZeroPad = false;
          flagsLoop: while (1) {
            switch (next) {
              case '+'.charCodeAt(0):
                flagAlwaysSigned = true;
                break;
              case '-'.charCodeAt(0):
                flagLeftAlign = true;
                break;
              case '#'.charCodeAt(0):
                flagAlternative = true;
                break;
              case '0'.charCodeAt(0):
                if (flagZeroPad) {
                  break flagsLoop;
                } else {
                  flagZeroPad = true;
                  break;
                }
              default:
                break flagsLoop;
            }
            textIndex++;
            next = HEAP8[(textIndex+1)];
          }
  
          // Handle width.
          var width = 0;
          if (next == '*'.charCodeAt(0)) {
            width = getNextArg('i32');
            textIndex++;
            next = HEAP8[(textIndex+1)];
          } else {
            while (next >= '0'.charCodeAt(0) && next <= '9'.charCodeAt(0)) {
              width = width * 10 + (next - '0'.charCodeAt(0));
              textIndex++;
              next = HEAP8[(textIndex+1)];
            }
          }
  
          // Handle precision.
          var precisionSet = false;
          if (next == '.'.charCodeAt(0)) {
            var precision = 0;
            precisionSet = true;
            textIndex++;
            next = HEAP8[(textIndex+1)];
            if (next == '*'.charCodeAt(0)) {
              precision = getNextArg('i32');
              textIndex++;
            } else {
              while(1) {
                var precisionChr = HEAP8[(textIndex+1)];
                if (precisionChr < '0'.charCodeAt(0) ||
                    precisionChr > '9'.charCodeAt(0)) break;
                precision = precision * 10 + (precisionChr - '0'.charCodeAt(0));
                textIndex++;
              }
            }
            next = HEAP8[(textIndex+1)];
          } else {
            var precision = 6; // Standard default.
          }
  
          // Handle integer sizes. WARNING: These assume a 32-bit architecture!
          var argSize;
          switch (String.fromCharCode(next)) {
            case 'h':
              var nextNext = HEAP8[(textIndex+2)];
              if (nextNext == 'h'.charCodeAt(0)) {
                textIndex++;
                argSize = 1; // char (actually i32 in varargs)
              } else {
                argSize = 2; // short (actually i32 in varargs)
              }
              break;
            case 'l':
              var nextNext = HEAP8[(textIndex+2)];
              if (nextNext == 'l'.charCodeAt(0)) {
                textIndex++;
                argSize = 8; // long long
              } else {
                argSize = 4; // long
              }
              break;
            case 'L': // long long
            case 'q': // int64_t
            case 'j': // intmax_t
              argSize = 8;
              break;
            case 'z': // size_t
            case 't': // ptrdiff_t
            case 'I': // signed ptrdiff_t or unsigned size_t
              argSize = 4;
              break;
            default:
              argSize = null;
          }
          if (argSize) textIndex++;
          next = HEAP8[(textIndex+1)];
  
          // Handle type specifier.
          if (['d', 'i', 'u', 'o', 'x', 'X', 'p'].indexOf(String.fromCharCode(next)) != -1) {
            // Integer.
            var signed = next == 'd'.charCodeAt(0) || next == 'i'.charCodeAt(0);
            argSize = argSize || 4;
            var currArg = getNextArg('i' + (argSize * 8));
            var origArg = currArg;
            var argText;
            // Flatten i64-1 [low, high] into a (slightly rounded) double
            if (argSize == 8) {
              currArg = Runtime.makeBigInt(currArg[0], currArg[1], next == 'u'.charCodeAt(0));
            }
            // Truncate to requested size.
            if (argSize <= 4) {
              var limit = Math.pow(256, argSize) - 1;
              currArg = (signed ? reSign : unSign)(currArg & limit, argSize * 8);
            }
            // Format the number.
            var currAbsArg = Math.abs(currArg);
            var prefix = '';
            if (next == 'd'.charCodeAt(0) || next == 'i'.charCodeAt(0)) {
              if (argSize == 8 && i64Math) argText = i64Math.stringify(origArg[0], origArg[1], null); else
              argText = reSign(currArg, 8 * argSize, 1).toString(10);
            } else if (next == 'u'.charCodeAt(0)) {
              if (argSize == 8 && i64Math) argText = i64Math.stringify(origArg[0], origArg[1], true); else
              argText = unSign(currArg, 8 * argSize, 1).toString(10);
              currArg = Math.abs(currArg);
            } else if (next == 'o'.charCodeAt(0)) {
              argText = (flagAlternative ? '0' : '') + currAbsArg.toString(8);
            } else if (next == 'x'.charCodeAt(0) || next == 'X'.charCodeAt(0)) {
              prefix = flagAlternative ? '0x' : '';
              if (argSize == 8 && i64Math) argText = (origArg[1]>>>0).toString(16) + (origArg[0]>>>0).toString(16); else
              if (currArg < 0) {
                // Represent negative numbers in hex as 2's complement.
                currArg = -currArg;
                argText = (currAbsArg - 1).toString(16);
                var buffer = [];
                for (var i = 0; i < argText.length; i++) {
                  buffer.push((0xF - parseInt(argText[i], 16)).toString(16));
                }
                argText = buffer.join('');
                while (argText.length < argSize * 2) argText = 'f' + argText;
              } else {
                argText = currAbsArg.toString(16);
              }
              if (next == 'X'.charCodeAt(0)) {
                prefix = prefix.toUpperCase();
                argText = argText.toUpperCase();
              }
            } else if (next == 'p'.charCodeAt(0)) {
              if (currAbsArg === 0) {
                argText = '(nil)';
              } else {
                prefix = '0x';
                argText = currAbsArg.toString(16);
              }
            }
            if (precisionSet) {
              while (argText.length < precision) {
                argText = '0' + argText;
              }
            }
  
            // Add sign if needed
            if (flagAlwaysSigned) {
              if (currArg < 0) {
                prefix = '-' + prefix;
              } else {
                prefix = '+' + prefix;
              }
            }
  
            // Add padding.
            while (prefix.length + argText.length < width) {
              if (flagLeftAlign) {
                argText += ' ';
              } else {
                if (flagZeroPad) {
                  argText = '0' + argText;
                } else {
                  prefix = ' ' + prefix;
                }
              }
            }
  
            // Insert the result into the buffer.
            argText = prefix + argText;
            argText.split('').forEach(function(chr) {
              ret.push(chr.charCodeAt(0));
            });
          } else if (['f', 'F', 'e', 'E', 'g', 'G'].indexOf(String.fromCharCode(next)) != -1) {
            // Float.
            var currArg = getNextArg('double');
            var argText;
  
            if (isNaN(currArg)) {
              argText = 'nan';
              flagZeroPad = false;
            } else if (!isFinite(currArg)) {
              argText = (currArg < 0 ? '-' : '') + 'inf';
              flagZeroPad = false;
            } else {
              var isGeneral = false;
              var effectivePrecision = Math.min(precision, 20);
  
              // Convert g/G to f/F or e/E, as per:
              // http://pubs.opengroup.org/onlinepubs/9699919799/functions/printf.html
              if (next == 'g'.charCodeAt(0) || next == 'G'.charCodeAt(0)) {
                isGeneral = true;
                precision = precision || 1;
                var exponent = parseInt(currArg.toExponential(effectivePrecision).split('e')[1], 10);
                if (precision > exponent && exponent >= -4) {
                  next = ((next == 'g'.charCodeAt(0)) ? 'f' : 'F').charCodeAt(0);
                  precision -= exponent + 1;
                } else {
                  next = ((next == 'g'.charCodeAt(0)) ? 'e' : 'E').charCodeAt(0);
                  precision--;
                }
                effectivePrecision = Math.min(precision, 20);
              }
  
              if (next == 'e'.charCodeAt(0) || next == 'E'.charCodeAt(0)) {
                argText = currArg.toExponential(effectivePrecision);
                // Make sure the exponent has at least 2 digits.
                if (/[eE][-+]\d$/.test(argText)) {
                  argText = argText.slice(0, -1) + '0' + argText.slice(-1);
                }
              } else if (next == 'f'.charCodeAt(0) || next == 'F'.charCodeAt(0)) {
                argText = currArg.toFixed(effectivePrecision);
              }
  
              var parts = argText.split('e');
              if (isGeneral && !flagAlternative) {
                // Discard trailing zeros and periods.
                while (parts[0].length > 1 && parts[0].indexOf('.') != -1 &&
                       (parts[0].slice(-1) == '0' || parts[0].slice(-1) == '.')) {
                  parts[0] = parts[0].slice(0, -1);
                }
              } else {
                // Make sure we have a period in alternative mode.
                if (flagAlternative && argText.indexOf('.') == -1) parts[0] += '.';
                // Zero pad until required precision.
                while (precision > effectivePrecision++) parts[0] += '0';
              }
              argText = parts[0] + (parts.length > 1 ? 'e' + parts[1] : '');
  
              // Capitalize 'E' if needed.
              if (next == 'E'.charCodeAt(0)) argText = argText.toUpperCase();
  
              // Add sign.
              if (flagAlwaysSigned && currArg >= 0) {
                argText = '+' + argText;
              }
            }
  
            // Add padding.
            while (argText.length < width) {
              if (flagLeftAlign) {
                argText += ' ';
              } else {
                if (flagZeroPad && (argText[0] == '-' || argText[0] == '+')) {
                  argText = argText[0] + '0' + argText.slice(1);
                } else {
                  argText = (flagZeroPad ? '0' : ' ') + argText;
                }
              }
            }
  
            // Adjust case.
            if (next < 'a'.charCodeAt(0)) argText = argText.toUpperCase();
  
            // Insert the result into the buffer.
            argText.split('').forEach(function(chr) {
              ret.push(chr.charCodeAt(0));
            });
          } else if (next == 's'.charCodeAt(0)) {
            // String.
            var arg = getNextArg('i8*') || nullString;
            var argLength = String_len(arg);
            if (precisionSet) argLength = Math.min(argLength, precision);
            if (!flagLeftAlign) {
              while (argLength < width--) {
                ret.push(' '.charCodeAt(0));
              }
            }
            for (var i = 0; i < argLength; i++) {
              ret.push(HEAPU8[(arg++)]);
            }
            if (flagLeftAlign) {
              while (argLength < width--) {
                ret.push(' '.charCodeAt(0));
              }
            }
          } else if (next == 'c'.charCodeAt(0)) {
            // Character.
            if (flagLeftAlign) ret.push(getNextArg('i8'));
            while (--width > 0) {
              ret.push(' '.charCodeAt(0));
            }
            if (!flagLeftAlign) ret.push(getNextArg('i8'));
          } else if (next == 'n'.charCodeAt(0)) {
            // Write the length written so far to the next parameter.
            var ptr = getNextArg('i32*');
            HEAP32[((ptr)>>2)]=ret.length
          } else if (next == '%'.charCodeAt(0)) {
            // Literal percent sign.
            ret.push(curr);
          } else {
            // Unknown specifiers remain untouched.
            for (var i = startTextIndex; i < textIndex + 2; i++) {
              ret.push(HEAP8[(i)]);
            }
          }
          textIndex += 2;
          // TODO: Support a/A (hex float) and m (last error) specifiers.
          // TODO: Support %1${specifier} for arg selection.
        } else {
          ret.push(curr);
          textIndex += 1;
        }
      }
      return ret;
    }function _snprintf(s, n, format, varargs) {
      // int snprintf(char *restrict s, size_t n, const char *restrict format, ...);
      // http://pubs.opengroup.org/onlinepubs/000095399/functions/printf.html
      var result = __formatString(format, varargs);
      var limit = (n === undefined) ? result.length
                                    : Math.min(result.length, Math.max(n - 1, 0));
      for (var i = 0; i < limit; i++) {
        HEAP8[((s)+(i))]=result[i];
      }
      if (limit < n || (n === undefined)) HEAP8[((s)+(i))]=0;
      return result.length;
    }var _vsnprintf;

  function _llvm_va_end() {}

  var _llvm_memcpy_p0i8_p0i8_i64;
var __ZNSt9type_infoD2Ev; // stub for __ZNSt9type_infoD2Ev

  
  function _memset(ptr, value, num, align) {
      // TODO: make these settings, and in memcpy, {{'s
      if (num >= 20) {
        // This is unaligned, but quite large, so work hard to get to aligned settings
        var stop = ptr + num;
        while (ptr % 4) { // no need to check for stop, since we have large num
          HEAP8[ptr++] = value;
        }
        if (value < 0) value += 256; // make it unsigned
        var ptr4 = ptr >> 2, stop4 = stop >> 2, value4 = value | (value << 8) | (value << 16) | (value << 24);
        while (ptr4 < stop4) {
          HEAP32[ptr4++] = value4;
        }
        ptr = ptr4 << 2;
        while (ptr < stop) {
          HEAP8[ptr++] = value;
        }
      } else {
        while (num--) {
          HEAP8[ptr++] = value;
        }
      }
    }var _llvm_memset_p0i8_i64;

  function _abort() {
      ABORT = true;
      throw 'abort() at ' + (new Error().stack);
    }

  
  function ___setErrNo(value) {
      // For convenient setting and returning of errno.
      if (!___setErrNo.ret) ___setErrNo.ret = allocate([0], 'i32', ALLOC_STATIC);
      HEAP32[((___setErrNo.ret)>>2)]=value
      return value;
    }
  
  var ERRNO_CODES={E2BIG:7,EACCES:13,EADDRINUSE:98,EADDRNOTAVAIL:99,EAFNOSUPPORT:97,EAGAIN:11,EALREADY:114,EBADF:9,EBADMSG:74,EBUSY:16,ECANCELED:125,ECHILD:10,ECONNABORTED:103,ECONNREFUSED:111,ECONNRESET:104,EDEADLK:35,EDESTADDRREQ:89,EDOM:33,EDQUOT:122,EEXIST:17,EFAULT:14,EFBIG:27,EHOSTUNREACH:113,EIDRM:43,EILSEQ:84,EINPROGRESS:115,EINTR:4,EINVAL:22,EIO:5,EISCONN:106,EISDIR:21,ELOOP:40,EMFILE:24,EMLINK:31,EMSGSIZE:90,EMULTIHOP:72,ENAMETOOLONG:36,ENETDOWN:100,ENETRESET:102,ENETUNREACH:101,ENFILE:23,ENOBUFS:105,ENODATA:61,ENODEV:19,ENOENT:2,ENOEXEC:8,ENOLCK:37,ENOLINK:67,ENOMEM:12,ENOMSG:42,ENOPROTOOPT:92,ENOSPC:28,ENOSR:63,ENOSTR:60,ENOSYS:38,ENOTCONN:107,ENOTDIR:20,ENOTEMPTY:39,ENOTRECOVERABLE:131,ENOTSOCK:88,ENOTSUP:95,ENOTTY:25,ENXIO:6,EOVERFLOW:75,EOWNERDEAD:130,EPERM:1,EPIPE:32,EPROTO:71,EPROTONOSUPPORT:93,EPROTOTYPE:91,ERANGE:34,EROFS:30,ESPIPE:29,ESRCH:3,ESTALE:116,ETIME:62,ETIMEDOUT:110,ETXTBSY:26,EWOULDBLOCK:11,EXDEV:18};function _sysconf(name) {
      // long sysconf(int name);
      // http://pubs.opengroup.org/onlinepubs/009695399/functions/sysconf.html
      switch(name) {
        case 8: return PAGE_SIZE;
        case 54:
        case 56:
        case 21:
        case 61:
        case 63:
        case 22:
        case 67:
        case 23:
        case 24:
        case 25:
        case 26:
        case 27:
        case 69:
        case 28:
        case 101:
        case 70:
        case 71:
        case 29:
        case 30:
        case 199:
        case 75:
        case 76:
        case 32:
        case 43:
        case 44:
        case 80:
        case 46:
        case 47:
        case 45:
        case 48:
        case 49:
        case 42:
        case 82:
        case 33:
        case 7:
        case 108:
        case 109:
        case 107:
        case 112:
        case 119:
        case 121:
          return 200809;
        case 13:
        case 104:
        case 94:
        case 95:
        case 34:
        case 35:
        case 77:
        case 81:
        case 83:
        case 84:
        case 85:
        case 86:
        case 87:
        case 88:
        case 89:
        case 90:
        case 91:
        case 94:
        case 95:
        case 110:
        case 111:
        case 113:
        case 114:
        case 115:
        case 116:
        case 117:
        case 118:
        case 120:
        case 40:
        case 16:
        case 79:
        case 19:
          return -1;
        case 92:
        case 93:
        case 5:
        case 72:
        case 6:
        case 74:
        case 92:
        case 93:
        case 96:
        case 97:
        case 98:
        case 99:
        case 102:
        case 103:
        case 105:
          return 1;
        case 38:
        case 66:
        case 50:
        case 51:
        case 4:
          return 1024;
        case 15:
        case 64:
        case 41:
          return 32;
        case 55:
        case 37:
        case 17:
          return 2147483647;
        case 18:
        case 1:
          return 47839;
        case 59:
        case 57:
          return 99;
        case 68:
        case 58:
          return 2048;
        case 0: return 2097152;
        case 3: return 65536;
        case 14: return 32768;
        case 73: return 32767;
        case 39: return 16384;
        case 60: return 1000;
        case 106: return 700;
        case 52: return 256;
        case 62: return 255;
        case 2: return 100;
        case 65: return 64;
        case 36: return 20;
        case 100: return 16;
        case 20: return 6;
        case 53: return 4;
      }
      ___setErrNo(ERRNO_CODES.EINVAL);
      return -1;
    }

  function _time(ptr) {
      var ret = Math.floor(Date.now()/1000);
      if (ptr) {
        HEAP32[((ptr)>>2)]=ret
      }
      return ret;
    }

  
  function ___errno_location() {
      return ___setErrNo.ret;
    }var ___errno;

  function _sbrk(bytes) {
      // Implement a Linux-like 'memory area' for our 'process'.
      // Changes the size of the memory area by |bytes|; returns the
      // address of the previous top ('break') of the memory area
  
      // We need to make sure no one else allocates unfreeable memory!
      // We must control this entirely. So we don't even need to do
      // unfreeable allocations - the HEAP is ours, from STATICTOP up.
      // TODO: We could in theory slice off the top of the HEAP when
      //       sbrk gets a negative increment in |bytes|...
      var self = _sbrk;
      if (!self.called) {
        STATICTOP = alignMemoryPage(STATICTOP); // make sure we start out aligned
        self.called = true;
        _sbrk.DYNAMIC_START = STATICTOP;
      }
      var ret = STATICTOP;
      if (bytes != 0) Runtime.staticAlloc(bytes);
      return ret;  // Previous break location.
    }

  function ___cxa_allocate_exception(size) {
      return _malloc(size);
    }

  
  
  
  function ___cxa_is_number_type(type) {
      var isNumber = false;
      try { if (type == __ZTIi) isNumber = true } catch(e){}
      try { if (type == __ZTIj) isNumber = true } catch(e){}
      try { if (type == __ZTIl) isNumber = true } catch(e){}
      try { if (type == __ZTIm) isNumber = true } catch(e){}
      try { if (type == __ZTIx) isNumber = true } catch(e){}
      try { if (type == __ZTIy) isNumber = true } catch(e){}
      try { if (type == __ZTIf) isNumber = true } catch(e){}
      try { if (type == __ZTId) isNumber = true } catch(e){}
      try { if (type == __ZTIe) isNumber = true } catch(e){}
      try { if (type == __ZTIc) isNumber = true } catch(e){}
      try { if (type == __ZTIa) isNumber = true } catch(e){}
      try { if (type == __ZTIh) isNumber = true } catch(e){}
      try { if (type == __ZTIs) isNumber = true } catch(e){}
      try { if (type == __ZTIt) isNumber = true } catch(e){}
      return isNumber;
    }function ___cxa_does_inherit(definiteType, possibilityType, possibility) {
      if (possibility == 0) return false;
      if (possibilityType == 0 || possibilityType == definiteType)
        return true;
      var possibility_type_info;
      if (___cxa_is_number_type(possibilityType)) {
        possibility_type_info = possibilityType;
      } else {
        var possibility_type_infoAddr = HEAP32[((possibilityType)>>2)] - 8;
        possibility_type_info = HEAP32[((possibility_type_infoAddr)>>2)];
      }
      switch (possibility_type_info) {
      case 0: // possibility is a pointer
        // See if definite type is a pointer
        var definite_type_infoAddr = HEAP32[((definiteType)>>2)] - 8;
        var definite_type_info = HEAP32[((definite_type_infoAddr)>>2)];
        if (definite_type_info == 0) {
          // Also a pointer; compare base types of pointers
          var defPointerBaseAddr = definiteType+8;
          var defPointerBaseType = HEAP32[((defPointerBaseAddr)>>2)];
          var possPointerBaseAddr = possibilityType+8;
          var possPointerBaseType = HEAP32[((possPointerBaseAddr)>>2)];
          return ___cxa_does_inherit(defPointerBaseType, possPointerBaseType, possibility);
        } else
          return false; // one pointer and one non-pointer
      case 1: // class with no base class
        return false;
      case 2: // class with base class
        var parentTypeAddr = possibilityType + 8;
        var parentType = HEAP32[((parentTypeAddr)>>2)];
        return ___cxa_does_inherit(definiteType, parentType, possibility);
      default:
        return false; // some unencountered type
      }
    }function ___cxa_find_matching_catch(thrown, throwntype, typeArray) {
      // If throwntype is a pointer, this means a pointer has been
      // thrown. When a pointer is thrown, actually what's thrown
      // is a pointer to the pointer. We'll dereference it.
      if (throwntype != 0 && !___cxa_is_number_type(throwntype)) {
        var throwntypeInfoAddr= HEAP32[((throwntype)>>2)] - 8;
        var throwntypeInfo= HEAP32[((throwntypeInfoAddr)>>2)];
        if (throwntypeInfo == 0)
          thrown = HEAP32[((thrown)>>2)];
      }
      // The different catch blocks are denoted by different types.
      // Due to inheritance, those types may not precisely match the
      // type of the thrown object. Find one which matches, and
      // return the type of the catch block which should be called.
      for (var i = 0; i < typeArray.length; i++) {
        if (___cxa_does_inherit(typeArray[i], throwntype, thrown))
          return (tempRet0 = typeArray[i],thrown);
      }
      // Shouldn't happen unless we have bogus data in typeArray
      // or encounter a type for which emscripten doesn't have suitable
      // typeinfo defined. Best-efforts match just in case.
      return (tempRet0 = throwntype,thrown);
    }function ___cxa_throw(ptr, type, destructor) {
      if (!___cxa_throw.initialized) {
        try {
          HEAP32[((__ZTVN10__cxxabiv119__pointer_type_infoE)>>2)]=0; // Workaround for libcxxabi integration bug
        } catch(e){}
        try {
          HEAP32[((__ZTVN10__cxxabiv117__class_type_infoE)>>2)]=1; // Workaround for libcxxabi integration bug
        } catch(e){}
        try {
          HEAP32[((__ZTVN10__cxxabiv120__si_class_type_infoE)>>2)]=2; // Workaround for libcxxabi integration bug
        } catch(e){}
        ___cxa_throw.initialized = true;
      }
      HEAP32[((_llvm_eh_exception.buf)>>2)]=ptr
      HEAP32[(((_llvm_eh_exception.buf)+(4))>>2)]=type
      HEAP32[(((_llvm_eh_exception.buf)+(8))>>2)]=destructor
      if (!("uncaught_exception" in __ZSt18uncaught_exceptionv)) {
        __ZSt18uncaught_exceptionv.uncaught_exception = 1;
      } else {
        __ZSt18uncaught_exceptionv.uncaught_exception++;
      }
      throw ptr + " - Exception catching is disabled, this exception cannot be caught. Compile with -s DISABLE_EXCEPTION_CATCHING=0 to catch.";;
    }

  function __ZNSt9exceptionD2Ev(){}





  
  
  
  
  var _stdin=allocate(1, "i32*", ALLOC_STACK);
  
  var _stdout=allocate(1, "i32*", ALLOC_STACK);
  
  var _stderr=allocate(1, "i32*", ALLOC_STACK);
  
  var __impure_ptr=allocate(1, "i32*", ALLOC_STACK);var FS={currentPath:"/",nextInode:2,streams:[null],ignorePermissions:true,joinPath:function (parts, forceRelative) {
        var ret = parts[0];
        for (var i = 1; i < parts.length; i++) {
          if (ret[ret.length-1] != '/') ret += '/';
          ret += parts[i];
        }
        if (forceRelative && ret[0] == '/') ret = ret.substr(1);
        return ret;
      },absolutePath:function (relative, base) {
        if (typeof relative !== 'string') return null;
        if (base === undefined) base = FS.currentPath;
        if (relative && relative[0] == '/') base = '';
        var full = base + '/' + relative;
        var parts = full.split('/').reverse();
        var absolute = [''];
        while (parts.length) {
          var part = parts.pop();
          if (part == '' || part == '.') {
            // Nothing.
          } else if (part == '..') {
            if (absolute.length > 1) absolute.pop();
          } else {
            absolute.push(part);
          }
        }
        return absolute.length == 1 ? '/' : absolute.join('/');
      },analyzePath:function (path, dontResolveLastLink, linksVisited) {
        var ret = {
          isRoot: false,
          exists: false,
          error: 0,
          name: null,
          path: null,
          object: null,
          parentExists: false,
          parentPath: null,
          parentObject: null
        };
        path = FS.absolutePath(path);
        if (path == '/') {
          ret.isRoot = true;
          ret.exists = ret.parentExists = true;
          ret.name = '/';
          ret.path = ret.parentPath = '/';
          ret.object = ret.parentObject = FS.root;
        } else if (path !== null) {
          linksVisited = linksVisited || 0;
          path = path.slice(1).split('/');
          var current = FS.root;
          var traversed = [''];
          while (path.length) {
            if (path.length == 1 && current.isFolder) {
              ret.parentExists = true;
              ret.parentPath = traversed.length == 1 ? '/' : traversed.join('/');
              ret.parentObject = current;
              ret.name = path[0];
            }
            var target = path.shift();
            if (!current.isFolder) {
              ret.error = ERRNO_CODES.ENOTDIR;
              break;
            } else if (!current.read) {
              ret.error = ERRNO_CODES.EACCES;
              break;
            } else if (!current.contents.hasOwnProperty(target)) {
              ret.error = ERRNO_CODES.ENOENT;
              break;
            }
            current = current.contents[target];
            if (current.link && !(dontResolveLastLink && path.length == 0)) {
              if (linksVisited > 40) { // Usual Linux SYMLOOP_MAX.
                ret.error = ERRNO_CODES.ELOOP;
                break;
              }
              var link = FS.absolutePath(current.link, traversed.join('/'));
              ret = FS.analyzePath([link].concat(path).join('/'),
                                   dontResolveLastLink, linksVisited + 1);
              return ret;
            }
            traversed.push(target);
            if (path.length == 0) {
              ret.exists = true;
              ret.path = traversed.join('/');
              ret.object = current;
            }
          }
        }
        return ret;
      },findObject:function (path, dontResolveLastLink) {
        FS.ensureRoot();
        var ret = FS.analyzePath(path, dontResolveLastLink);
        if (ret.exists) {
          return ret.object;
        } else {
          ___setErrNo(ret.error);
          return null;
        }
      },createObject:function (parent, name, properties, canRead, canWrite) {
        if (!parent) parent = '/';
        if (typeof parent === 'string') parent = FS.findObject(parent);
  
        if (!parent) {
          ___setErrNo(ERRNO_CODES.EACCES);
          throw new Error('Parent path must exist.');
        }
        if (!parent.isFolder) {
          ___setErrNo(ERRNO_CODES.ENOTDIR);
          throw new Error('Parent must be a folder.');
        }
        if (!parent.write && !FS.ignorePermissions) {
          ___setErrNo(ERRNO_CODES.EACCES);
          throw new Error('Parent folder must be writeable.');
        }
        if (!name || name == '.' || name == '..') {
          ___setErrNo(ERRNO_CODES.ENOENT);
          throw new Error('Name must not be empty.');
        }
        if (parent.contents.hasOwnProperty(name)) {
          ___setErrNo(ERRNO_CODES.EEXIST);
          throw new Error("Can't overwrite object.");
        }
  
        parent.contents[name] = {
          read: canRead === undefined ? true : canRead,
          write: canWrite === undefined ? false : canWrite,
          timestamp: Date.now(),
          inodeNumber: FS.nextInode++
        };
        for (var key in properties) {
          if (properties.hasOwnProperty(key)) {
            parent.contents[name][key] = properties[key];
          }
        }
  
        return parent.contents[name];
      },createFolder:function (parent, name, canRead, canWrite) {
        var properties = {isFolder: true, isDevice: false, contents: {}};
        return FS.createObject(parent, name, properties, canRead, canWrite);
      },createPath:function (parent, path, canRead, canWrite) {
        var current = FS.findObject(parent);
        if (current === null) throw new Error('Invalid parent.');
        path = path.split('/').reverse();
        while (path.length) {
          var part = path.pop();
          if (!part) continue;
          if (!current.contents.hasOwnProperty(part)) {
            FS.createFolder(current, part, canRead, canWrite);
          }
          current = current.contents[part];
        }
        return current;
      },createFile:function (parent, name, properties, canRead, canWrite) {
        properties.isFolder = false;
        return FS.createObject(parent, name, properties, canRead, canWrite);
      },createDataFile:function (parent, name, data, canRead, canWrite) {
        if (typeof data === 'string') {
          var dataArray = new Array(data.length);
          for (var i = 0, len = data.length; i < len; ++i) dataArray[i] = data.charCodeAt(i);
          data = dataArray;
        }
        var properties = {
          isDevice: false,
          contents: data.subarray ? data.subarray(0) : data // as an optimization, create a new array wrapper (not buffer) here, to help JS engines understand this object
        };
        return FS.createFile(parent, name, properties, canRead, canWrite);
      },createLazyFile:function (parent, name, url, canRead, canWrite) {
  
        if (typeof XMLHttpRequest !== 'undefined') {
          if (!ENVIRONMENT_IS_WORKER) throw 'Cannot do synchronous binary XHRs outside webworkers in modern browsers. Use --embed-file or --preload-file in emcc';
          // Lazy chunked Uint8Array (implements get and length from Uint8Array). Actual getting is abstracted away for eventual reuse.
          var LazyUint8Array = function(chunkSize, length) {
            this.length = length;
            this.chunkSize = chunkSize;
            this.chunks = []; // Loaded chunks. Index is the chunk number
          }
          LazyUint8Array.prototype.get = function(idx) {
            if (idx > this.length-1 || idx < 0) {
              return undefined;
            }
            var chunkOffset = idx % chunkSize;
            var chunkNum = Math.floor(idx / chunkSize);
            return this.getter(chunkNum)[chunkOffset];
          }
          LazyUint8Array.prototype.setDataGetter = function(getter) {
            this.getter = getter;
          }
    
          // Find length
          var xhr = new XMLHttpRequest();
          xhr.open('HEAD', url, false);
          xhr.send(null);
          if (!(xhr.status >= 200 && xhr.status < 300 || xhr.status === 304)) throw new Error("Couldn't load " + url + ". Status: " + xhr.status);
          var datalength = Number(xhr.getResponseHeader("Content-length"));
          var header;
          var hasByteServing = (header = xhr.getResponseHeader("Accept-Ranges")) && header === "bytes";
          var chunkSize = 1024*1024; // Chunk size in bytes
          if (!hasByteServing) chunkSize = datalength;
    
          // Function to get a range from the remote URL.
          var doXHR = (function(from, to) {
            if (from > to) throw new Error("invalid range (" + from + ", " + to + ") or no bytes requested!");
            if (to > datalength-1) throw new Error("only " + datalength + " bytes available! programmer error!");
    
            // TODO: Use mozResponseArrayBuffer, responseStream, etc. if available.
            var xhr = new XMLHttpRequest();
            xhr.open('GET', url, false);
            if (datalength !== chunkSize) xhr.setRequestHeader("Range", "bytes=" + from + "-" + to);
    
            // Some hints to the browser that we want binary data.
            if (typeof Uint8Array != 'undefined') xhr.responseType = 'arraybuffer';
            if (xhr.overrideMimeType) {
              xhr.overrideMimeType('text/plain; charset=x-user-defined');
            }
    
            xhr.send(null);
            if (!(xhr.status >= 200 && xhr.status < 300 || xhr.status === 304)) throw new Error("Couldn't load " + url + ". Status: " + xhr.status);
            if (xhr.response !== undefined) {
              return new Uint8Array(xhr.response || []);
            } else {
              return intArrayFromString(xhr.responseText || '', true);
            }
          });
    
          var lazyArray = new LazyUint8Array(chunkSize, datalength);
          lazyArray.setDataGetter(function(chunkNum) {
            var start = chunkNum * lazyArray.chunkSize;
            var end = (chunkNum+1) * lazyArray.chunkSize - 1; // including this byte
            end = Math.min(end, datalength-1); // if datalength-1 is selected, this is the last block
            if (typeof(lazyArray.chunks[chunkNum]) === "undefined") {
              lazyArray.chunks[chunkNum] = doXHR(start, end);
            }
            if (typeof(lazyArray.chunks[chunkNum]) === "undefined") throw new Error("doXHR failed!");
            return lazyArray.chunks[chunkNum];
          });
          var properties = { isDevice: false, contents: lazyArray };
        } else {
          var properties = { isDevice: false, url: url };
        }
  
        return FS.createFile(parent, name, properties, canRead, canWrite);
      },createPreloadedFile:function (parent, name, url, canRead, canWrite, onload, onerror, dontCreateFile) {
        Browser.ensureObjects();
        var fullname = FS.joinPath([parent, name], true);
        function processData(byteArray) {
          function finish(byteArray) {
            if (!dontCreateFile) {
              FS.createDataFile(parent, name, byteArray, canRead, canWrite);
            }
            if (onload) onload();
            removeRunDependency('cp ' + fullname);
          }
          var handled = false;
          Module['preloadPlugins'].forEach(function(plugin) {
            if (handled) return;
            if (plugin['canHandle'](fullname)) {
              plugin['handle'](byteArray, fullname, finish, function() {
                if (onerror) onerror();
                removeRunDependency('cp ' + fullname);
              });
              handled = true;
            }
          });
          if (!handled) finish(byteArray);
        }
        addRunDependency('cp ' + fullname);
        if (typeof url == 'string') {
          Browser.asyncLoad(url, function(byteArray) {
            processData(byteArray);
          }, onerror);
        } else {
          processData(url);
        }
      },createLink:function (parent, name, target, canRead, canWrite) {
        var properties = {isDevice: false, link: target};
        return FS.createFile(parent, name, properties, canRead, canWrite);
      },createDevice:function (parent, name, input, output) {
        if (!(input || output)) {
          throw new Error('A device must have at least one callback defined.');
        }
        var ops = {isDevice: true, input: input, output: output};
        return FS.createFile(parent, name, ops, Boolean(input), Boolean(output));
      },forceLoadFile:function (obj) {
        if (obj.isDevice || obj.isFolder || obj.link || obj.contents) return true;
        var success = true;
        if (typeof XMLHttpRequest !== 'undefined') {
          throw new Error("Lazy loading should have been performed (contents set) in createLazyFile, but it was not. Lazy loading only works in web workers. Use --embed-file or --preload-file in emcc on the main thread.");
        } else if (Module['read']) {
          // Command-line.
          try {
            // WARNING: Can't read binary files in V8's d8 or tracemonkey's js, as
            //          read() will try to parse UTF8.
            obj.contents = intArrayFromString(Module['read'](obj.url), true);
          } catch (e) {
            success = false;
          }
        } else {
          throw new Error('Cannot load without read() or XMLHttpRequest.');
        }
        if (!success) ___setErrNo(ERRNO_CODES.EIO);
        return success;
      },ensureRoot:function () {
        if (FS.root) return;
        // The main file system tree. All the contents are inside this.
        FS.root = {
          read: true,
          write: true,
          isFolder: true,
          isDevice: false,
          timestamp: Date.now(),
          inodeNumber: 1,
          contents: {}
        };
      },init:function (input, output, error) {
        // Make sure we initialize only once.
        assert(!FS.init.initialized, 'FS.init was previously called. If you want to initialize later with custom parameters, remove any earlier calls (note that one is automatically added to the generated code)');
        FS.init.initialized = true;
  
        FS.ensureRoot();
  
        // Allow Module.stdin etc. to provide defaults, if none explicitly passed to us here
        input = input || Module['stdin'];
        output = output || Module['stdout'];
        error = error || Module['stderr'];
  
        // Default handlers.
        var stdinOverridden = true, stdoutOverridden = true, stderrOverridden = true;
        if (!input) {
          stdinOverridden = false;
          input = function() {
            if (!input.cache || !input.cache.length) {
              var result;
              if (typeof window != 'undefined' &&
                  typeof window.prompt == 'function') {
                // Browser.
                result = window.prompt('Input: ');
                if (result === null) result = String.fromCharCode(0); // cancel ==> EOF
              } else if (typeof readline == 'function') {
                // Command line.
                result = readline();
              }
              if (!result) result = '';
              input.cache = intArrayFromString(result + '\n', true);
            }
            return input.cache.shift();
          };
        }
        var utf8 = new Runtime.UTF8Processor();
        function simpleOutput(val) {
          if (val === null || val === '\n'.charCodeAt(0)) {
            output.printer(output.buffer.join(''));
            output.buffer = [];
          } else {
            output.buffer.push(utf8.processCChar(val));
          }
        }
        if (!output) {
          stdoutOverridden = false;
          output = simpleOutput;
        }
        if (!output.printer) output.printer = Module['print'];
        if (!output.buffer) output.buffer = [];
        if (!error) {
          stderrOverridden = false;
          error = simpleOutput;
        }
        if (!error.printer) error.printer = Module['print'];
        if (!error.buffer) error.buffer = [];
  
        // Create the temporary folder, if not already created
        try {
          FS.createFolder('/', 'tmp', true, true);
        } catch(e) {}
  
        // Create the I/O devices.
        var devFolder = FS.createFolder('/', 'dev', true, true);
        var stdin = FS.createDevice(devFolder, 'stdin', input);
        var stdout = FS.createDevice(devFolder, 'stdout', null, output);
        var stderr = FS.createDevice(devFolder, 'stderr', null, error);
        FS.createDevice(devFolder, 'tty', input, output);
  
        // Create default streams.
        FS.streams[1] = {
          path: '/dev/stdin',
          object: stdin,
          position: 0,
          isRead: true,
          isWrite: false,
          isAppend: false,
          isTerminal: !stdinOverridden,
          error: false,
          eof: false,
          ungotten: []
        };
        FS.streams[2] = {
          path: '/dev/stdout',
          object: stdout,
          position: 0,
          isRead: false,
          isWrite: true,
          isAppend: false,
          isTerminal: !stdoutOverridden,
          error: false,
          eof: false,
          ungotten: []
        };
        FS.streams[3] = {
          path: '/dev/stderr',
          object: stderr,
          position: 0,
          isRead: false,
          isWrite: true,
          isAppend: false,
          isTerminal: !stderrOverridden,
          error: false,
          eof: false,
          ungotten: []
        };
        assert(Math.max(_stdin, _stdout, _stderr) < 128); // make sure these are low, we flatten arrays with these
        HEAP32[((_stdin)>>2)]=1;
        HEAP32[((_stdout)>>2)]=2;
        HEAP32[((_stderr)>>2)]=3;
  
        // Other system paths
        FS.createPath('/', 'dev/shm/tmp', true, true); // temp files
  
        // Newlib initialization
        for (var i = FS.streams.length; i < Math.max(_stdin, _stdout, _stderr) + 4; i++) {
          FS.streams[i] = null; // Make sure to keep FS.streams dense
        }
        FS.streams[_stdin] = FS.streams[1];
        FS.streams[_stdout] = FS.streams[2];
        FS.streams[_stderr] = FS.streams[3];
        allocate([ allocate(
          [0, 0, 0, 0, _stdin, 0, 0, 0, _stdout, 0, 0, 0, _stderr, 0, 0, 0],
          'void*', ALLOC_STATIC) ], 'void*', ALLOC_NONE, __impure_ptr);
      },quit:function () {
        if (!FS.init.initialized) return;
        // Flush any partially-printed lines in stdout and stderr. Careful, they may have been closed
        if (FS.streams[2] && FS.streams[2].object.output.buffer.length > 0) FS.streams[2].object.output('\n'.charCodeAt(0));
        if (FS.streams[3] && FS.streams[3].object.output.buffer.length > 0) FS.streams[3].object.output('\n'.charCodeAt(0));
      },standardizePath:function (path) {
        if (path.substr(0, 2) == './') path = path.substr(2);
        return path;
      },deleteFile:function (path) {
        path = FS.analyzePath(path);
        if (!path.parentExists || !path.exists) {
          throw 'Invalid path ' + path;
        }
        delete path.parentObject.contents[path.name];
      }};
  
  function _pwrite(fildes, buf, nbyte, offset) {
      // ssize_t pwrite(int fildes, const void *buf, size_t nbyte, off_t offset);
      // http://pubs.opengroup.org/onlinepubs/000095399/functions/write.html
      var stream = FS.streams[fildes];
      if (!stream || stream.object.isDevice) {
        ___setErrNo(ERRNO_CODES.EBADF);
        return -1;
      } else if (!stream.isWrite) {
        ___setErrNo(ERRNO_CODES.EACCES);
        return -1;
      } else if (stream.object.isFolder) {
        ___setErrNo(ERRNO_CODES.EISDIR);
        return -1;
      } else if (nbyte < 0 || offset < 0) {
        ___setErrNo(ERRNO_CODES.EINVAL);
        return -1;
      } else {
        var contents = stream.object.contents;
        while (contents.length < offset) contents.push(0);
        for (var i = 0; i < nbyte; i++) {
          contents[offset + i] = HEAPU8[((buf)+(i))];
        }
        stream.object.timestamp = Date.now();
        return i;
      }
    }function _write(fildes, buf, nbyte) {
      // ssize_t write(int fildes, const void *buf, size_t nbyte);
      // http://pubs.opengroup.org/onlinepubs/000095399/functions/write.html
      var stream = FS.streams[fildes];
      if (!stream) {
        ___setErrNo(ERRNO_CODES.EBADF);
        return -1;
      } else if (!stream.isWrite) {
        ___setErrNo(ERRNO_CODES.EACCES);
        return -1;
      } else if (nbyte < 0) {
        ___setErrNo(ERRNO_CODES.EINVAL);
        return -1;
      } else {
        if (stream.object.isDevice) {
          if (stream.object.output) {
            for (var i = 0; i < nbyte; i++) {
              try {
                stream.object.output(HEAP8[((buf)+(i))]);
              } catch (e) {
                ___setErrNo(ERRNO_CODES.EIO);
                return -1;
              }
            }
            stream.object.timestamp = Date.now();
            return i;
          } else {
            ___setErrNo(ERRNO_CODES.ENXIO);
            return -1;
          }
        } else {
          var bytesWritten = _pwrite(fildes, buf, nbyte, stream.position);
          if (bytesWritten != -1) stream.position += bytesWritten;
          return bytesWritten;
        }
      }
    }function _fputs(s, stream) {
      // int fputs(const char *restrict s, FILE *restrict stream);
      // http://pubs.opengroup.org/onlinepubs/000095399/functions/fputs.html
      return _write(stream, s, _strlen(s));
    }
  
  function _fputc(c, stream) {
      // int fputc(int c, FILE *stream);
      // http://pubs.opengroup.org/onlinepubs/000095399/functions/fputc.html
      var chr = unSign(c & 0xFF);
      HEAP8[(_fputc.ret)]=chr
      var ret = _write(stream, _fputc.ret, 1);
      if (ret == -1) {
        if (FS.streams[stream]) FS.streams[stream].error = true;
        return -1;
      } else {
        return chr;
      }
    }function _puts(s) {
      // int puts(const char *s);
      // http://pubs.opengroup.org/onlinepubs/000095399/functions/puts.html
      // NOTE: puts() always writes an extra newline.
      var stdout = HEAP32[((_stdout)>>2)];
      var ret = _fputs(s, stdout);
      if (ret < 0) {
        return ret;
      } else {
        var newlineRet = _fputc('\n'.charCodeAt(0), stdout);
        return (newlineRet < 0) ? -1 : ret + 1;
      }
    }
_llvm_eh_exception.buf = allocate(12, "void*", ALLOC_STATIC);
___setErrNo(0);
__ATINIT__.unshift({ func: function() { if (!Module["noFSInit"] && !FS.init.initialized) FS.init() } });__ATMAIN__.push({ func: function() { FS.ignorePermissions = false } });__ATEXIT__.push({ func: function() { FS.quit() } });Module["FS_createFolder"] = FS.createFolder;Module["FS_createPath"] = FS.createPath;Module["FS_createDataFile"] = FS.createDataFile;Module["FS_createPreloadedFile"] = FS.createPreloadedFile;Module["FS_createLazyFile"] = FS.createLazyFile;Module["FS_createLink"] = FS.createLink;Module["FS_createDevice"] = FS.createDevice;
_fputc.ret = allocate([0], "i8", ALLOC_STATIC);

var FUNCTION_TABLE = [0,0,__ZN9LoopShapeD1Ev,0,__ZNK10__cxxabiv117__class_type_info16search_above_dstEPNS_19__dynamic_cast_infoEPKvS4_ib,0,__ZN5ShapeD0Ev,0,__ZNSt9bad_allocD0Ev,0,__ZN10__cxxabiv117__class_type_infoD1Ev,0,__ZN12LabeledShapeD0Ev,0,__ZN10__cxxabiv117__class_type_infoD0Ev,0,__ZNKSt9bad_alloc4whatEv,0,__ZNK10__cxxabiv120__si_class_type_info16search_below_dstEPNS_19__dynamic_cast_infoEPKvib,0,__ZN13MultipleShape6RenderEb,0,__ZN9LoopShape6RenderEb,0,__ZN11SimpleShape6RenderEb,0,__ZN9LoopShapeD0Ev,0,__ZNK10__cxxabiv117__class_type_info9can_catchEPKNS_16__shim_type_infoERPv,0,__ZNK10__cxxabiv120__si_class_type_info16search_above_dstEPNS_19__dynamic_cast_infoEPKvS4_ib,0,__ZNK10__cxxabiv117__class_type_info16search_below_dstEPNS_19__dynamic_cast_infoEPKvib,0,__ZN11SimpleShapeD1Ev,0,__ZN11SimpleShapeD0Ev,0,__ZNK10__cxxabiv116__shim_type_info5noop1Ev,0,__ZN13MultipleShapeD1Ev,0,__ZNSt9bad_allocD1Ev,0,__ZN10__cxxabiv120__si_class_type_infoD1Ev,0,__ZNK10__cxxabiv117__class_type_info27has_unambiguous_public_baseEPNS_19__dynamic_cast_infoEPvi,0,__ZN12LabeledShapeD1Ev,0,__ZN10__cxxabiv120__si_class_type_infoD0Ev,0,__ZNK10__cxxabiv116__shim_type_info5noop2Ev,0,__ZNSt3__13mapIPviNS_4lessIS1_EENS_9allocatorINS_4pairIKS1_iEEEEED1Ev,0,__ZNK10__cxxabiv120__si_class_type_info27has_unambiguous_public_baseEPNS_19__dynamic_cast_infoEPvi,0,__ZN5ShapeD1Ev,0,___cxa_pure_virtual,0,__ZN13MultipleShapeD0Ev,0];

function __ZN10ministring4sizeEv($this_0_0_val) {
  return $this_0_0_val;
}
function __ZN10ministring5c_strEv($this_0_1_val) {
  return $this_0_1_val;
}
function __ZN8Indenter6IndentEv() {
  HEAP32[1311191] = HEAP32[1311191] + 1 | 0;
  return;
}
function __ZN8Indenter8UnindentEv() {
  HEAP32[1311191] = HEAP32[1311191] - 1 | 0;
  return;
}
function __ZN5Shape10IsMultipleEPS_($It) {
  do {
    if (($It | 0) == 0) {
      var $cond = 0;
    } else {
      if ((HEAP32[$It + 12 >> 2] | 0) != 1) {
        var $cond = 0;
        break;
      }
      var $cond = $It;
    }
  } while (0);
  var $cond;
  return $cond;
}
function __ZN6BranchC1EPKcS1_($this, $ConditionInit, $CodeInit) {
  __ZN6BranchC2EPKcS1_($this, $ConditionInit, $CodeInit);
  return;
}
function __ZN6BranchC2EPKcS1_($this, $ConditionInit, $CodeInit) {
  HEAP32[$this >> 2] = 0;
  HEAP8[$this + 8 | 0] = 0;
  if (($ConditionInit | 0) == 0) {
    var $cond = 0;
  } else {
    var $cond = _strdup($ConditionInit);
  }
  var $cond;
  HEAP32[$this + 12 >> 2] = $cond;
  if (($CodeInit | 0) == 0) {
    var $cond7 = 0;
    var $cond7;
    var $Code = $this + 16 | 0;
    HEAP32[$Code >> 2] = $cond7;
    return;
  }
  var $cond7 = _strdup($CodeInit);
  var $cond7;
  var $Code = $this + 16 | 0;
  HEAP32[$Code >> 2] = $cond7;
  return;
}
function __ZN6BranchD1Ev($this) {
  __ZN6BranchD2Ev($this);
  return;
}
function __ZN6BranchD2Ev($this) {
  var $0 = HEAP32[$this + 12 >> 2];
  if (($0 | 0) != 0) {
    _free($0);
  }
  var $1 = HEAP32[$this + 16 >> 2];
  if (($1 | 0) == 0) {
    return;
  }
  _free($1);
  return;
}
function __ZN6Branch6RenderEP5Blockb($this, $Target, $SetLabel) {
  var __stackBase__ = STACKTOP;
  var $0 = HEAP32[$this + 16 >> 2];
  if (($0 | 0) != 0) {
    __ZL13PrintIndentedPKcz(5243504, (tempInt = STACKTOP, STACKTOP = STACKTOP + 4 | 0, HEAP32[tempInt >> 2] = $0, tempInt));
  }
  if ($SetLabel) {
    __ZL13PrintIndentedPKcz(5243488, (tempInt = STACKTOP, STACKTOP = STACKTOP + 4 | 0, HEAP32[tempInt >> 2] = HEAP32[$Target + 52 >> 2], tempInt));
  }
  var $2 = HEAP32[$this >> 2];
  if (($2 | 0) == 0) {
    STACKTOP = __stackBase__;
    return;
  }
  var $3 = HEAP32[$this + 4 >> 2];
  if (($3 | 0) == 0) {
    STACKTOP = __stackBase__;
    return;
  }
  var $cond = ($3 | 0) == 1 ? 5243212 : 5243012;
  if ((HEAP8[$this + 8 | 0] & 1) << 24 >> 24 == 0) {
    __ZL13PrintIndentedPKcz(5243004, (tempInt = STACKTOP, STACKTOP = STACKTOP + 4 | 0, HEAP32[tempInt >> 2] = $cond, tempInt));
    STACKTOP = __stackBase__;
    return;
  } else {
    var $6 = HEAP32[$2 + 4 >> 2];
    __ZL13PrintIndentedPKcz(5243364, (tempInt = STACKTOP, STACKTOP = STACKTOP + 8 | 0, HEAP32[tempInt >> 2] = $cond, HEAP32[tempInt + 4 >> 2] = $6, tempInt));
    STACKTOP = __stackBase__;
    return;
  }
}
function __ZL13PrintIndentedPKcz($Format, varrp) {
  var __stackBase__ = STACKTOP;
  STACKTOP = STACKTOP + 4 | 0;
  var $Args = __stackBase__;
  var $0 = HEAP32[1311198];
  if (($0 | 0) == 0) {
    ___assert_func(5242980, 36, 5244248, 5243148);
    var $1 = HEAP32[1311198];
  } else {
    var $1 = $0;
  }
  var $1;
  var $2 = HEAP32[1311191];
  if ((($2 << 1) + $1 - HEAP32[1311196] | 0) < (HEAP32[1311195] | 0)) {
    var $5 = $2;
  } else {
    ___assert_func(5242980, 37, 5244248, 5243072);
    var $5 = HEAP32[1311191];
  }
  var $5;
  var $mul45 = $5 << 1;
  var $cmp56 = ($mul45 | 0) > 0;
  var $_pre9 = HEAP32[1311198];
  L52 : do {
    if ($cmp56) {
      var $i_07 = 0;
      var $6 = $_pre9;
      while (1) {
        var $6;
        var $i_07;
        HEAP8[$6] = 32;
        var $inc = $i_07 + 1 | 0;
        var $incdec_ptr = $6 + 1 | 0;
        HEAP32[1311198] = $incdec_ptr;
        if (($inc | 0) < ($mul45 | 0)) {
          var $i_07 = $inc;
          var $6 = $incdec_ptr;
        } else {
          var $7 = $incdec_ptr;
          break L52;
        }
      }
    } else {
      var $7 = $_pre9;
    }
  } while (0);
  var $7;
  HEAP32[$Args >> 2] = varrp;
  var $sub = HEAP32[1311196] - $7 + HEAP32[1311195] | 0;
  var $call = _snprintf($7, $sub, $Format, HEAP32[$Args >> 2]);
  if (($call | 0) < ($sub | 0)) {
    var $11 = HEAP32[1311198];
    var $add_ptr14 = $11 + $call | 0;
    HEAP32[1311198] = $add_ptr14;
    STACKTOP = __stackBase__;
    return;
  }
  ___assert_func(5242980, 43, 5244248, 5243024);
  var $11 = HEAP32[1311198];
  var $add_ptr14 = $11 + $call | 0;
  HEAP32[1311198] = $add_ptr14;
  STACKTOP = __stackBase__;
  return;
}
__ZL13PrintIndentedPKcz["X"] = 1;
function __ZN5BlockC1EPKc($this, $CodeInit) {
  __ZN5BlockC2EPKc($this, $CodeInit);
  return;
}
function __ZN5BlockC2EPKc($this, $CodeInit) {
  __ZNSt3__16__treeINS_4pairIP5BlockP6BranchEENS_19__map_value_compareIS3_S5_NS_4lessIS3_EELb1EEENS_9allocatorIS6_EEEC1ERKSA_($this | 0);
  __ZNSt3__16__treeINS_4pairIP5BlockP6BranchEENS_19__map_value_compareIS3_S5_NS_4lessIS3_EELb1EEENS_9allocatorIS6_EEEC1ERKSA_($this + 12 | 0);
  __ZNSt3__16__treeINS_4pairIP5BlockP6BranchEENS_19__map_value_compareIS3_S5_NS_4lessIS3_EELb1EEENS_9allocatorIS6_EEEC1ERKSA_($this + 24 | 0);
  __ZNSt3__16__treeINS_4pairIP5BlockP6BranchEENS_19__map_value_compareIS3_S5_NS_4lessIS3_EELb1EEENS_9allocatorIS6_EEEC1ERKSA_($this + 36 | 0);
  HEAP32[$this + 48 >> 2] = 0;
  var $0 = HEAP32[1311193];
  HEAP32[1311193] = $0 + 1 | 0;
  HEAP32[$this + 52 >> 2] = $0;
  HEAP32[$this + 60 >> 2] = 0;
  HEAP8[$this + 64 | 0] = 0;
  HEAP32[$this + 56 >> 2] = _strdup($CodeInit);
  return;
}
function __ZNSt3__13mapIP5BlockP6BranchNS_4lessIS2_EENS_9allocatorINS_4pairIKS2_S4_EEEEED1Ev($this) {
  __ZNSt3__13mapIP5BlockP6BranchNS_4lessIS2_EENS_9allocatorINS_4pairIKS2_S4_EEEEED2Ev($this);
  return;
}
function __ZN5BlockD1Ev($this) {
  __ZN5BlockD2Ev($this);
  return;
}
function __ZN5BlockD2Ev($this) {
  var $0 = HEAP32[$this + 56 >> 2];
  if (($0 | 0) != 0) {
    _free($0);
  }
  var $ProcessedBranchesIn = $this + 36 | 0;
  var $1 = HEAP32[$ProcessedBranchesIn >> 2];
  var $__first__i_i_i_i_i = $this + 40 | 0;
  var $lnot_i_i13 = ($1 | 0) == ($__first__i_i_i_i_i | 0);
  L68 : do {
    if (!$lnot_i_i13) {
      var $3 = $__first__i_i_i_i_i;
      var $iter_sroa_0_014 = $1;
      while (1) {
        var $iter_sroa_0_014;
        var $5 = HEAP32[$iter_sroa_0_014 + 20 >> 2];
        if (($5 | 0) != 0) {
          __ZN6BranchD1Ev($5);
          __ZdlPv($5);
        }
        var $call_i_i_i = __ZNSt3__111__tree_nextIPNS_16__tree_node_baseIPvEEEET_S5_($iter_sroa_0_014 | 0);
        if (($call_i_i_i | 0) == ($3 | 0)) {
          break L68;
        } else {
          var $iter_sroa_0_014 = $call_i_i_i;
        }
      }
    }
  } while (0);
  var $ProcessedBranchesOut = $this + 24 | 0;
  var $9 = HEAP32[$ProcessedBranchesOut >> 2];
  var $__first__i_i_i_i_i135 = $this + 28 | 0;
  var $lnot_i_i15711 = ($9 | 0) == ($__first__i_i_i_i_i135 | 0);
  L76 : do {
    if (!$lnot_i_i15711) {
      var $11 = $__first__i_i_i_i_i135;
      var $iter10_sroa_0_012 = $9;
      while (1) {
        var $iter10_sroa_0_012;
        var $13 = HEAP32[$iter10_sroa_0_012 + 20 >> 2];
        if (($13 | 0) != 0) {
          __ZN6BranchD1Ev($13);
          __ZdlPv($13);
        }
        var $call_i_i_i174 = __ZNSt3__111__tree_nextIPNS_16__tree_node_baseIPvEEEET_S5_($iter10_sroa_0_012 | 0);
        if (($call_i_i_i174 | 0) == ($11 | 0)) {
          break L76;
        } else {
          var $iter10_sroa_0_012 = $call_i_i_i174;
        }
      }
    }
  } while (0);
  __ZNSt3__13mapIP5BlockP6BranchNS_4lessIS2_EENS_9allocatorINS_4pairIKS2_S4_EEEEED1Ev($ProcessedBranchesIn);
  __ZNSt3__13mapIP5BlockP6BranchNS_4lessIS2_EENS_9allocatorINS_4pairIKS2_S4_EEEEED1Ev($ProcessedBranchesOut);
  __ZNSt3__13mapIP5BlockP6BranchNS_4lessIS2_EENS_9allocatorINS_4pairIKS2_S4_EEEEED1Ev($this + 12 | 0);
  __ZNSt3__13mapIP5BlockP6BranchNS_4lessIS2_EENS_9allocatorINS_4pairIKS2_S4_EEEEED1Ev($this | 0);
  return;
}
__ZN5BlockD2Ev["X"] = 1;
function __ZN5Block11AddBranchToEPS_PKcS2_($this, $Target, $Condition, $Code) {
  var __stackBase__ = STACKTOP;
  STACKTOP = STACKTOP + 8 | 0;
  var $agg_tmp_i = __stackBase__;
  var $Target_addr = __stackBase__ + 4;
  HEAP32[$Target_addr >> 2] = $Target;
  __ZNSt3__16__treeINS_4pairIP5BlockP6BranchEENS_19__map_value_compareIS3_S5_NS_4lessIS3_EELb1EEENS_9allocatorIS6_EEE4findIS3_EENS_15__tree_iteratorIS6_PNS_11__tree_nodeIS6_PvEEiEERKT_($agg_tmp_i, $this | 0, $Target_addr);
  if ((HEAP32[$agg_tmp_i >> 2] | 0) != ($this + 4 | 0)) {
    ___assert_func(5242980, 117, 5244184, 5242932);
  }
  var $1 = __Znwj(20);
  __ZN6BranchC1EPKcS1_($1, $Condition, $Code);
  HEAP32[__ZNSt3__13mapIP5BlockP6BranchNS_4lessIS2_EENS_9allocatorINS_4pairIKS2_S4_EEEEEixERS9_($this | 0, $Target_addr) >> 2] = $1;
  STACKTOP = __stackBase__;
  return;
}
function __ZNSt3__13mapIP5BlockP6BranchNS_4lessIS2_EENS_9allocatorINS_4pairIKS2_S4_EEEEEixERS9_($this, $__k) {
  var __stackBase__ = STACKTOP;
  STACKTOP = STACKTOP + 16 | 0;
  var $__parent = __stackBase__;
  var $temp_lvalue = __stackBase__ + 4;
  var $call = __ZNSt3__13mapIP5BlockP6BranchNS_4lessIS2_EENS_9allocatorINS_4pairIKS2_S4_EEEEE16__find_equal_keyERPNS_16__tree_node_baseIPvEERS9_($this, $__parent, $__k);
  var $0 = HEAP32[$call >> 2];
  if (($0 | 0) != 0) {
    var $__r_0 = $0;
    var $__r_0;
    var $second = $__r_0 + 20 | 0;
    STACKTOP = __stackBase__;
    return $second;
  }
  __ZNSt3__13mapIP5BlockP6BranchNS_4lessIS2_EENS_9allocatorINS_4pairIKS2_S4_EEEEE16__construct_nodeERS9_($temp_lvalue, $this, $__k);
  var $__first__i_i_i28_i_i = $temp_lvalue | 0;
  var $2 = HEAP32[$__first__i_i_i28_i_i >> 2];
  HEAP32[$__first__i_i_i28_i_i >> 2] = 0;
  __ZNSt3__16__treeINS_4pairIP5BlockP6BranchEENS_19__map_value_compareIS3_S5_NS_4lessIS3_EELb1EEENS_9allocatorIS6_EEE16__insert_node_atEPNS_16__tree_node_baseIPvEERSH_SH_($this | 0, HEAP32[$__parent >> 2], $call, $2 | 0);
  var $__r_0 = $2;
  var $__r_0;
  var $second = $__r_0 + 20 | 0;
  STACKTOP = __stackBase__;
  return $second;
}
function __ZN5Block6RenderEb($this, $InLoop) {
  var $Details_0$s2;
  var $DefaultTarget57_pre$s2;
  var $Target$s2;
  var label = 0;
  var __stackBase__ = STACKTOP;
  STACKTOP = STACKTOP + 24 | 0;
  var $agg_tmp_i501 = __stackBase__;
  var $agg_tmp_i332 = __stackBase__ + 4;
  var $RemainingConditions = __stackBase__ + 8;
  var $Target = __stackBase__ + 20, $Target$s2 = $Target >> 2;
  if (!((HEAP8[$this + 64 | 0] & 1) << 24 >> 24 == 0 | $InLoop ^ 1)) {
    __ZL13PrintIndentedPKcz(5242920, (tempInt = STACKTOP, STACKTOP = STACKTOP + 1 | 0, STACKTOP = STACKTOP + 3 >> 2 << 2, HEAP32[tempInt >> 2] = 0, tempInt));
  }
  var $2 = HEAP32[$this + 56 >> 2];
  var $tobool3 = ($2 | 0) == 0;
  L97 : do {
    if (!$tobool3) {
      if (HEAP8[$2] << 24 >> 24 == 0) {
        break;
      } else {
        var $Start_058 = $2;
      }
      while (1) {
        var $Start_058;
        var $call = _strchr($Start_058, 10);
        if (($call | 0) == 0) {
          break;
        }
        HEAP8[$call] = 0;
        __ZL11PutIndentedPKc($Start_058);
        HEAP8[$call] = 10;
        var $add_ptr = $call + 1 | 0;
        if (HEAP8[$add_ptr] << 24 >> 24 == 0) {
          break L97;
        } else {
          var $Start_058 = $add_ptr;
        }
      }
      __ZL11PutIndentedPKc($Start_058);
    }
  } while (0);
  var $ProcessedBranchesOut = $this + 24 | 0;
  var $__first__i_i_i_i = $this + 32 | 0;
  var $5 = HEAP32[$__first__i_i_i_i >> 2];
  if (($5 | 0) == 1) {
    var $SetLabel_0 = (HEAP32[HEAP32[HEAP32[$ProcessedBranchesOut >> 2] + 20 >> 2] + 4 >> 2] | 0) != 0 & 1;
  } else if (($5 | 0) == 0) {
    STACKTOP = __stackBase__;
    return;
  } else {
    var $SetLabel_0 = 1;
  }
  var $SetLabel_0;
  var $Next = HEAP32[$this + 48 >> 2] + 8 | 0;
  var $11 = HEAP32[$Next >> 2];
  var $call27 = __ZN5Shape10IsMultipleEPS_($11);
  var $tobool28 = ($call27 | 0) != 0;
  do {
    if ($tobool28) {
      HEAP32[$Next >> 2] = HEAP32[$11 + 8 >> 2];
      __ZN13MultipleShape16RenderLoopPrefixEv($call27);
      if ($SetLabel_0 << 24 >> 24 == 0) {
        var $SetLabel_1 = 0;
        break;
      }
      var $SetLabel_1 = (HEAP32[$call27 + 28 >> 2] | 0) == (HEAP32[$__first__i_i_i_i >> 2] | 0) ? 0 : $SetLabel_0;
    } else {
      var $SetLabel_1 = $SetLabel_0;
    }
  } while (0);
  var $SetLabel_1;
  var $__begin_node__i_i_i309 = $ProcessedBranchesOut | 0;
  var $15 = HEAP32[$__begin_node__i_i_i309 >> 2];
  var $__first__i_i_i_i_i365 = $this + 28 | 0;
  var $16 = $__first__i_i_i_i_i365;
  var $lnot_i_i54 = ($15 | 0) == ($16 | 0);
  var $DefaultTarget57_pre = $this + 60 | 0, $DefaultTarget57_pre$s2 = $DefaultTarget57_pre >> 2;
  L112 : do {
    if (!$lnot_i_i54) {
      var $17 = $__first__i_i_i_i_i365;
      var $iter_sroa_0_055 = $15;
      while (1) {
        var $iter_sroa_0_055;
        if ((HEAP32[HEAP32[$iter_sroa_0_055 + 20 >> 2] + 12 >> 2] | 0) == 0) {
          if ((HEAP32[$DefaultTarget57_pre$s2] | 0) != 0) {
            ___assert_func(5242980, 185, 5244156, 5242904);
          }
          HEAP32[$DefaultTarget57_pre$s2] = HEAP32[$iter_sroa_0_055 + 16 >> 2];
        }
        var $call_i_i_i406 = __ZNSt3__111__tree_nextIPNS_16__tree_node_baseIPvEEEET_S5_($iter_sroa_0_055 | 0);
        if (($call_i_i_i406 | 0) == ($17 | 0)) {
          break L112;
        } else {
          var $iter_sroa_0_055 = $call_i_i_i406;
        }
      }
    }
  } while (0);
  if ((HEAP32[$DefaultTarget57_pre$s2] | 0) == 0) {
    ___assert_func(5242980, 189, 5244156, 5243472);
  }
  __ZN10ministringC1Ev($RemainingConditions);
  var $tobool98 = ($SetLabel_1 & 1) << 24 >> 24 == 0;
  var $__tree__i503 = $call27 + 20 | 0;
  var $agg_tmp2_i498_sroa_0_0_tmp3_i505_idx = $agg_tmp_i501 | 0;
  var $28 = $call27 + 24 | 0;
  var $agg_tmp2_i330_sroa_0_0_tmp3_i336_idx = $agg_tmp_i332 | 0;
  var $RemainingConditions_idx = $RemainingConditions | 0;
  var $RemainingConditions_idx2 = $RemainingConditions + 4 | 0;
  var $First_0 = 1;
  var $iter65_sroa_0_0 = HEAP32[$__begin_node__i_i_i309 >> 2];
  while (1) {
    var $iter65_sroa_0_0;
    var $First_0;
    var $cmp_i_i_i479 = ($iter65_sroa_0_0 | 0) == ($16 | 0);
    do {
      if ($cmp_i_i_i479) {
        var $34 = HEAP32[$DefaultTarget57_pre$s2];
        HEAP32[$Target$s2] = $34;
        var $Details_0 = HEAP32[__ZNSt3__13mapIP5BlockP6BranchNS_4lessIS2_EENS_9allocatorINS_4pairIKS2_S4_EEEEEixERS9_($ProcessedBranchesOut, $DefaultTarget57_pre) >> 2], $Details_0$s2 = $Details_0 >> 2;
        var $36 = $34;
        label = 103;
        break;
      } else {
        var $29 = HEAP32[$iter65_sroa_0_0 + 16 >> 2];
        HEAP32[$Target$s2] = $29;
        if (($29 | 0) == (HEAP32[$DefaultTarget57_pre$s2] | 0)) {
          var $First_2 = $First_0;
          break;
        }
        var $32 = HEAP32[$iter65_sroa_0_0 + 20 >> 2];
        if ((HEAP32[$32 + 12 >> 2] | 0) != 0) {
          var $Details_0 = $32, $Details_0$s2 = $Details_0 >> 2;
          var $36 = $29;
          label = 103;
          break;
        }
        ___assert_func(5242980, 200, 5244156, 5243452);
        var $Details_0 = $32, $Details_0$s2 = $Details_0 >> 2;
        var $36 = $29;
        label = 103;
        break;
      }
    } while (0);
    if (label == 103) {
      label = 0;
      var $36;
      var $Details_0;
      if ($tobool98) {
        var $39 = 0;
      } else {
        var $39 = (HEAP8[$36 + 64 | 0] & 1) << 24 >> 24 != 0;
      }
      var $39;
      if ($tobool28) {
        __ZNSt3__16__treeINS_4pairIP5BlockP5ShapeEENS_19__map_value_compareIS3_S5_NS_4lessIS3_EELb1EEENS_9allocatorIS6_EEE4findIS3_EENS_15__tree_iteratorIS6_PNS_11__tree_nodeIS6_PvEEiEERKT_($agg_tmp_i501, $__tree__i503, $Target);
        var $40 = (HEAP32[$agg_tmp2_i498_sroa_0_0_tmp3_i505_idx >> 2] | 0) != ($28 | 0);
      } else {
        var $40 = 0;
      }
      var $40;
      do {
        if ($39) {
          var $43 = 1;
        } else {
          if ((HEAP32[$Details_0$s2 + 1] | 0) != 0 | $40) {
            var $43 = 1;
            break;
          }
          var $43 = (HEAP32[$Details_0$s2 + 4] | 0) != 0;
        }
      } while (0);
      var $43;
      do {
        if ($cmp_i_i_i479) {
          if (!$43) {
            label = 123;
            break;
          }
          var $tobool156 = ($First_0 & 1) << 24 >> 24 != 0;
          if ((__ZN10ministring4sizeEv(HEAP32[$RemainingConditions_idx >> 2]) | 0) <= 0) {
            if ($tobool156) {
              label = 123;
              break;
            }
            __ZL13PrintIndentedPKcz(5243352, (tempInt = STACKTOP, STACKTOP = STACKTOP + 1 | 0, STACKTOP = STACKTOP + 3 >> 2 << 2, HEAP32[tempInt >> 2] = 0, tempInt));
            label = 123;
            break;
          }
          var $call159 = __ZN10ministring5c_strEv(HEAP32[$RemainingConditions_idx2 >> 2]);
          if ($tobool156) {
            __ZL13PrintIndentedPKcz(5243396, (tempInt = STACKTOP, STACKTOP = STACKTOP + 4 | 0, HEAP32[tempInt >> 2] = $call159, tempInt));
            var $First_150 = 0;
            label = 124;
            break;
          } else {
            __ZL13PrintIndentedPKcz(5243376, (tempInt = STACKTOP, STACKTOP = STACKTOP + 4 | 0, HEAP32[tempInt >> 2] = $call159, tempInt));
            label = 123;
            break;
          }
        } else {
          if ($43) {
            var $45 = HEAP32[$Details_0$s2 + 3];
            __ZL13PrintIndentedPKcz(5243436, (tempInt = STACKTOP, STACKTOP = STACKTOP + 8 | 0, HEAP32[tempInt >> 2] = ($First_0 & 1) << 24 >> 24 != 0 ? 5243432 : 5243424, HEAP32[tempInt + 4 >> 2] = $45, tempInt));
            var $First_150 = 0;
            label = 124;
            break;
          }
          if ((__ZN10ministring4sizeEv(HEAP32[$RemainingConditions_idx >> 2]) | 0) > 0) {
            __ZN10ministringpLEPKc($RemainingConditions, 5243416);
          }
          __ZN10ministringpLEPKc($RemainingConditions, 5243412);
          __ZN10ministringpLEPKc($RemainingConditions, HEAP32[$Details_0$s2 + 3]);
          __ZN10ministringpLEPKc($RemainingConditions, 5243408);
          label = 123;
          break;
        }
      } while (0);
      do {
        if (label == 123) {
          label = 0;
          if (($First_0 & 1) << 24 >> 24 == 0) {
            var $First_150 = $First_0;
            label = 124;
            break;
          } else {
            var $First_151 = $First_0;
            var $tobool17453 = 1;
            break;
          }
        }
      } while (0);
      if (label == 124) {
        label = 0;
        var $First_150;
        __ZN8Indenter6IndentEv();
        var $First_151 = $First_150;
        var $tobool17453 = 0;
      }
      var $tobool17453;
      var $First_151;
      __ZN6Branch6RenderEP5Blockb($Details_0, HEAP32[$Target$s2], $39);
      if ($40) {
        __ZNSt3__16__treeINS_4pairIP5BlockP5ShapeEENS_19__map_value_compareIS3_S5_NS_4lessIS3_EELb1EEENS_9allocatorIS6_EEE4findIS3_EENS_15__tree_iteratorIS6_PNS_11__tree_nodeIS6_PvEEiEERKT_($agg_tmp_i332, $__tree__i503, $Target);
        var $51 = HEAP32[HEAP32[$agg_tmp2_i330_sroa_0_0_tmp3_i336_idx >> 2] + 20 >> 2];
        FUNCTION_TABLE[HEAP32[HEAP32[$51 >> 2] + 8 >> 2]]($51, $InLoop);
      }
      if (!$tobool17453) {
        __ZN8Indenter8UnindentEv();
      }
      if ($cmp_i_i_i479) {
        break;
      } else {
        var $First_2 = $First_151;
      }
    }
    var $First_2;
    var $First_0 = $First_2;
    var $iter65_sroa_0_0 = __ZNSt3__111__tree_nextIPNS_16__tree_node_baseIPvEEEET_S5_($iter65_sroa_0_0 | 0);
  }
  if (!$tobool17453) {
    __ZL13PrintIndentedPKcz(5243348, (tempInt = STACKTOP, STACKTOP = STACKTOP + 1 | 0, STACKTOP = STACKTOP + 3 >> 2 << 2, HEAP32[tempInt >> 2] = 0, tempInt));
  }
  if ($tobool28) {
    __ZN13MultipleShape17RenderLoopPostfixEv(HEAP32[$call27 + 32 >> 2]);
  }
  __ZN10ministringD1Ev(HEAP32[$RemainingConditions_idx2 >> 2]);
  STACKTOP = __stackBase__;
  return;
}
__ZN5Block6RenderEb["X"] = 1;
function __ZL11PutIndentedPKc($String) {
  var $0 = HEAP32[1311198];
  if (($0 | 0) == 0) {
    ___assert_func(5242980, 49, 5244288, 5243148);
    var $1 = HEAP32[1311198];
  } else {
    var $1 = $0;
  }
  var $1;
  var $2 = HEAP32[1311191];
  if ((($2 << 1) + $1 - HEAP32[1311196] | 0) < (HEAP32[1311195] | 0)) {
    var $5 = $2;
    var $_pre10 = $1;
  } else {
    ___assert_func(5242980, 50, 5244288, 5243072);
    var $5 = HEAP32[1311191];
    var $_pre10 = HEAP32[1311198];
  }
  var $_pre10;
  var $5;
  var $mul45 = $5 << 1;
  var $cmp56 = ($mul45 | 0) > 0;
  L191 : do {
    if ($cmp56) {
      var $i_07 = 0;
      var $6 = $_pre10;
      while (1) {
        var $6;
        var $i_07;
        HEAP8[$6] = 32;
        var $inc = $i_07 + 1 | 0;
        var $incdec_ptr = $6 + 1 | 0;
        HEAP32[1311198] = $incdec_ptr;
        if (($inc | 0) < ($mul45 | 0)) {
          var $i_07 = $inc;
          var $6 = $incdec_ptr;
        } else {
          var $7 = $incdec_ptr;
          break L191;
        }
      }
    } else {
      var $7 = $_pre10;
    }
  } while (0);
  var $7;
  var $sub = HEAP32[1311195] - $7 + HEAP32[1311196] | 0;
  if ((_strlen($String) + 1 | 0) < ($sub | 0)) {
    var $10 = $7;
  } else {
    ___assert_func(5242980, 54, 5244288, 5243040);
    var $10 = HEAP32[1311198];
  }
  var $10;
  _strcpy($10, $String);
  var $call14 = _strlen($String);
  var $11 = HEAP32[1311198];
  var $incdec_ptr16 = $call14 + ($11 + 1) | 0;
  HEAP32[1311198] = $incdec_ptr16;
  HEAP8[$11 + $call14 | 0] = 10;
  HEAP8[$incdec_ptr16] = 0;
  return;
}
__ZL11PutIndentedPKc["X"] = 1;
function __ZN13MultipleShape16RenderLoopPrefixEv($this) {
  var __stackBase__ = STACKTOP;
  if ((HEAP32[$this + 32 >> 2] | 0) == 0) {
    STACKTOP = __stackBase__;
    return;
  }
  if ((HEAP8[$this + 16 | 0] & 1) << 24 >> 24 == 0) {
    __ZL13PrintIndentedPKcz(5243328, (tempInt = STACKTOP, STACKTOP = STACKTOP + 1 | 0, STACKTOP = STACKTOP + 3 >> 2 << 2, HEAP32[tempInt >> 2] = 0, tempInt));
  } else {
    __ZL13PrintIndentedPKcz(5243336, (tempInt = STACKTOP, STACKTOP = STACKTOP + 4 | 0, HEAP32[tempInt >> 2] = HEAP32[$this + 4 >> 2], tempInt));
  }
  __ZN8Indenter6IndentEv();
  STACKTOP = __stackBase__;
  return;
}
function __ZN10ministringC1Ev($this) {
  __ZN10ministringC2Ev($this);
  return;
}
function __ZN10ministringpLEPKc($this, $s) {
  var $used$s2;
  var $call = _strlen($s);
  var $used$s2 = ($this | 0) >> 2;
  var $0 = HEAP32[$used$s2];
  var $bufferSize = $this + 8 | 0;
  var $1 = HEAP32[$bufferSize >> 2];
  if (($0 + ($call + 2) | 0) <= ($1 | 0)) {
    var $4 = $0;
    var $3 = HEAP32[$this + 4 >> 2];
    var $3;
    var $4;
    var $add_ptr = $3 + $4 | 0;
    var $call17 = _strcpy($add_ptr, $s);
    var $5 = HEAP32[$used$s2];
    var $add19 = $5 + $call | 0;
    HEAP32[$used$s2] = $add19;
    return;
  }
  var $mul = $1 + $call << 1;
  var $add8 = 1024 - ($mul | 0) % 1024 + $mul | 0;
  HEAP32[$bufferSize >> 2] = $add8;
  var $buffer = $this + 4 | 0;
  var $2 = HEAP32[$buffer >> 2];
  if (($2 | 0) == 0) {
    var $cond = _malloc($add8);
  } else {
    var $cond = _realloc($2, $add8);
  }
  var $cond;
  HEAP32[$buffer >> 2] = $cond;
  var $4 = HEAP32[$used$s2];
  var $3 = $cond;
  var $3;
  var $4;
  var $add_ptr = $3 + $4 | 0;
  var $call17 = _strcpy($add_ptr, $s);
  var $5 = HEAP32[$used$s2];
  var $add19 = $5 + $call | 0;
  HEAP32[$used$s2] = $add19;
  return;
}
function __ZN13MultipleShape17RenderLoopPostfixEv($this_0_2_val) {
  var __stackBase__ = STACKTOP;
  if (($this_0_2_val | 0) == 0) {
    STACKTOP = __stackBase__;
    return;
  }
  __ZN8Indenter8UnindentEv();
  __ZL13PrintIndentedPKcz(5243312, (tempInt = STACKTOP, STACKTOP = STACKTOP + 1 | 0, STACKTOP = STACKTOP + 3 >> 2 << 2, HEAP32[tempInt >> 2] = 0, tempInt));
  STACKTOP = __stackBase__;
  return;
}
function __ZN10ministringD1Ev($this_0_1_val) {
  __ZN10ministringD2Ev($this_0_1_val);
  return;
}
function __ZN13MultipleShape6RenderEb($this, $InLoop) {
  var __stackBase__ = STACKTOP;
  __ZN13MultipleShape16RenderLoopPrefixEv($this);
  var $0 = HEAP32[$this + 20 >> 2];
  var $__first__i_i_i_i_i = $this + 24 | 0;
  var $lnot_i_i9 = ($0 | 0) == ($__first__i_i_i_i_i | 0);
  L225 : do {
    if (!$lnot_i_i9) {
      var $2 = $__first__i_i_i_i_i;
      var $iter_sroa_0_010 = $0;
      var $First_011 = 5243432;
      while (1) {
        var $First_011;
        var $iter_sroa_0_010;
        var $5 = HEAP32[HEAP32[$iter_sroa_0_010 + 16 >> 2] + 52 >> 2];
        if ((HEAP32[1311194] | 0) == 0) {
          __ZL13PrintIndentedPKcz(5243252, (tempInt = STACKTOP, STACKTOP = STACKTOP + 8 | 0, HEAP32[tempInt >> 2] = $First_011, HEAP32[tempInt + 4 >> 2] = $5, tempInt));
        } else {
          __ZL13PrintIndentedPKcz(5243284, (tempInt = STACKTOP, STACKTOP = STACKTOP + 8 | 0, HEAP32[tempInt >> 2] = $First_011, HEAP32[tempInt + 4 >> 2] = $5, tempInt));
        }
        __ZN8Indenter6IndentEv();
        var $7 = HEAP32[$iter_sroa_0_010 + 20 >> 2];
        FUNCTION_TABLE[HEAP32[HEAP32[$7 >> 2] + 8 >> 2]]($7, $InLoop);
        __ZN8Indenter8UnindentEv();
        __ZL13PrintIndentedPKcz(5243348, (tempInt = STACKTOP, STACKTOP = STACKTOP + 1 | 0, STACKTOP = STACKTOP + 3 >> 2 << 2, HEAP32[tempInt >> 2] = 0, tempInt));
        var $call_i_i_i = __ZNSt3__111__tree_nextIPNS_16__tree_node_baseIPvEEEET_S5_($iter_sroa_0_010 | 0);
        if (($call_i_i_i | 0) == ($2 | 0)) {
          break L225;
        } else {
          var $iter_sroa_0_010 = $call_i_i_i;
          var $First_011 = 5243276;
        }
      }
    }
  } while (0);
  __ZN13MultipleShape17RenderLoopPostfixEv(HEAP32[$this + 32 >> 2]);
  var $12 = HEAP32[$this + 8 >> 2];
  if (($12 | 0) == 0) {
    STACKTOP = __stackBase__;
    return;
  }
  FUNCTION_TABLE[HEAP32[HEAP32[$12 >> 2] + 8 >> 2]]($12, $InLoop);
  STACKTOP = __stackBase__;
  return;
}
__ZN13MultipleShape6RenderEb["X"] = 1;
function __ZN9LoopShape6RenderEb($this, $InLoop) {
  var __stackBase__ = STACKTOP;
  if ((HEAP8[$this + 16 | 0] & 1) << 24 >> 24 == 0) {
    __ZL13PrintIndentedPKcz(5243220, (tempInt = STACKTOP, STACKTOP = STACKTOP + 1 | 0, STACKTOP = STACKTOP + 3 >> 2 << 2, HEAP32[tempInt >> 2] = 0, tempInt));
  } else {
    __ZL13PrintIndentedPKcz(5243232, (tempInt = STACKTOP, STACKTOP = STACKTOP + 4 | 0, HEAP32[tempInt >> 2] = HEAP32[$this + 4 >> 2], tempInt));
  }
  __ZN8Indenter6IndentEv();
  var $3 = HEAP32[$this + 20 >> 2];
  FUNCTION_TABLE[HEAP32[HEAP32[$3 >> 2] + 8 >> 2]]($3, 1);
  __ZN8Indenter8UnindentEv();
  __ZL13PrintIndentedPKcz(5243348, (tempInt = STACKTOP, STACKTOP = STACKTOP + 1 | 0, STACKTOP = STACKTOP + 3 >> 2 << 2, HEAP32[tempInt >> 2] = 0, tempInt));
  var $6 = HEAP32[$this + 8 >> 2];
  if (($6 | 0) == 0) {
    STACKTOP = __stackBase__;
    return;
  }
  FUNCTION_TABLE[HEAP32[HEAP32[$6 >> 2] + 8 >> 2]]($6, $InLoop);
  STACKTOP = __stackBase__;
  return;
}
function __ZN8RelooperC1Ev($this) {
  __ZN8RelooperC2Ev($this);
  return;
}
function __ZN8RelooperC2Ev($this) {
  var $0 = $this;
  for (var $$dest = $0 >> 2, $$stop = $$dest + 13; $$dest < $$stop; $$dest++) {
    HEAP32[$$dest] = 0;
  }
  return;
}
function __ZNSt3__15dequeIP5BlockNS_9allocatorIS2_EEED1Ev($this) {
  __ZNSt3__15dequeIP5BlockNS_9allocatorIS2_EEED2Ev($this);
  return;
}
function __ZN8RelooperD1Ev($this) {
  __ZN8RelooperD2Ev($this);
  return;
}
function __ZN8RelooperD2Ev($this) {
  var $Blocks = $this | 0;
  var $__first__i_i_i_i = $this + 20 | 0;
  var $0 = HEAP32[$__first__i_i_i_i >> 2];
  var $cmp12 = ($0 | 0) == 0;
  L252 : do {
    if (!$cmp12) {
      var $__start__i = $this + 16 | 0;
      var $__begin__i_i = $this + 4 | 0;
      var $i_013 = 0;
      var $2 = $0;
      while (1) {
        var $2;
        var $i_013;
        var $add_i = HEAP32[$__start__i >> 2] + $i_013 | 0;
        var $6 = HEAP32[HEAP32[HEAP32[$__begin__i_i >> 2] + ($add_i >>> 10 << 2) >> 2] + (($add_i & 1023) << 2) >> 2];
        if (($6 | 0) == 0) {
          var $8 = $2;
        } else {
          __ZN5BlockD1Ev($6);
          __ZdlPv($6);
          var $8 = HEAP32[$__first__i_i_i_i >> 2];
        }
        var $8;
        var $inc = $i_013 + 1 | 0;
        if ($inc >>> 0 < $8 >>> 0) {
          var $i_013 = $inc;
          var $2 = $8;
        } else {
          break L252;
        }
      }
    }
  } while (0);
  var $__first__i_i_i_i44 = $this + 44 | 0;
  var $1 = HEAP32[$__first__i_i_i_i44 >> 2];
  if (($1 | 0) == 0) {
    var $Shapes = $this + 24 | 0;
    __ZNSt3__15dequeIP5ShapeNS_9allocatorIS2_EEED1Ev($Shapes);
    __ZNSt3__15dequeIP5BlockNS_9allocatorIS2_EEED1Ev($Blocks);
    return;
  }
  var $__start__i50 = $this + 40 | 0;
  var $__begin__i_i54 = $this + 28 | 0;
  var $i6_08 = 0;
  var $9 = $1;
  while (1) {
    var $9;
    var $i6_08;
    var $add_i51 = HEAP32[$__start__i50 >> 2] + $i6_08 | 0;
    var $13 = HEAP32[HEAP32[HEAP32[$__begin__i_i54 >> 2] + ($add_i51 >>> 10 << 2) >> 2] + (($add_i51 & 1023) << 2) >> 2];
    if (($13 | 0) == 0) {
      var $16 = $9;
    } else {
      FUNCTION_TABLE[HEAP32[HEAP32[$13 >> 2] + 4 >> 2]]($13);
      var $16 = HEAP32[$__first__i_i_i_i44 >> 2];
    }
    var $16;
    var $inc19 = $i6_08 + 1 | 0;
    if ($inc19 >>> 0 < $16 >>> 0) {
      var $i6_08 = $inc19;
      var $9 = $16;
    } else {
      break;
    }
  }
  var $Shapes = $this + 24 | 0;
  __ZNSt3__15dequeIP5ShapeNS_9allocatorIS2_EEED1Ev($Shapes);
  __ZNSt3__15dequeIP5BlockNS_9allocatorIS2_EEED1Ev($Blocks);
  return;
}
__ZN8RelooperD2Ev["X"] = 1;
function __ZNSt3__15dequeIP5ShapeNS_9allocatorIS2_EEED1Ev($this) {
  __ZNSt3__15dequeIP5ShapeNS_9allocatorIS2_EEED2Ev($this);
  return;
}
function __ZN8Relooper8AddBlockEP5Block($this, $New) {
  var __stackBase__ = STACKTOP;
  STACKTOP = STACKTOP + 4 | 0;
  var $New_addr = __stackBase__;
  HEAP32[$New_addr >> 2] = $New;
  __ZNSt3__15dequeIP5BlockNS_9allocatorIS2_EEE9push_backERKS2_($this | 0, $New_addr);
  STACKTOP = __stackBase__;
  return;
}
function __ZNSt3__15dequeIP5BlockNS_9allocatorIS2_EEE9push_backERKS2_($this, $__v) {
  var $__first__i_i_i_i$s2;
  var __stackBase__ = STACKTOP;
  STACKTOP = STACKTOP + 8 | 0;
  var $tmp = __stackBase__;
  var $1 = HEAP32[$this + 8 >> 2];
  var $2 = HEAP32[$this + 4 >> 2];
  if (($1 | 0) == ($2 | 0)) {
    var $cond_i_i = 0;
  } else {
    var $cond_i_i = ($1 - $2 << 8) - 1 | 0;
  }
  var $cond_i_i;
  var $__first__i_i_i_i$s2 = ($this + 20 | 0) >> 2;
  if (($cond_i_i | 0) == (HEAP32[$__first__i_i_i_i$s2] + HEAP32[$this + 16 >> 2] | 0)) {
    __ZNSt3__15dequeIP5BlockNS_9allocatorIS2_EEE19__add_back_capacityEv($this);
  }
  __ZNSt3__112__deque_baseIP5BlockNS_9allocatorIS2_EEE3endEv($tmp, $this | 0);
  var $5 = HEAP32[$tmp + 4 >> 2];
  if (($5 | 0) == 0) {
    var $7 = HEAP32[$__first__i_i_i_i$s2];
    var $inc = $7 + 1 | 0;
    HEAP32[$__first__i_i_i_i$s2] = $inc;
    STACKTOP = __stackBase__;
    return;
  }
  HEAP32[$5 >> 2] = HEAP32[$__v >> 2];
  var $7 = HEAP32[$__first__i_i_i_i$s2];
  var $inc = $7 + 1 | 0;
  HEAP32[$__first__i_i_i_i$s2] = $inc;
  STACKTOP = __stackBase__;
  return;
}
function __ZN8Relooper9CalculateEP5Block($this, $Entry) {
  var $__first__i_i_i_i$s2;
  var $Curr$s2;
  var __stackBase__ = STACKTOP;
  STACKTOP = STACKTOP + 80 | 0;
  var $ref_tmp_i227 = __stackBase__;
  var $ref_tmp_i = __stackBase__ + 8;
  var $agg_tmp_i = __stackBase__ + 16;
  var $Entry_addr = __stackBase__ + 20;
  var $Pre = __stackBase__ + 24;
  var $Curr = __stackBase__ + 40, $Curr$s2 = $Curr >> 2;
  var $AllBlocks = __stackBase__ + 44;
  var $Entries = __stackBase__ + 56;
  var $tmp53 = __stackBase__ + 68;
  var $tmp57 = __stackBase__ + 72;
  HEAP32[$Entry_addr >> 2] = $Entry;
  __ZZN8Relooper9CalculateEP5BlockEN12PreOptimizerC1EPS_($Pre, $this);
  __ZZN8Relooper9CalculateEP5BlockEN12PreOptimizer8FindLiveES1_($Pre, $Entry);
  var $__first__i_i_i_i$s2 = ($this + 20 | 0) >> 2;
  var $cmp26 = (HEAP32[$__first__i_i_i_i$s2] | 0) == 0;
  L285 : do {
    if (!$cmp26) {
      var $__start__i = $this + 16 | 0;
      var $__begin__i_i = $this + 4 | 0;
      var $__tree__i = $Pre + 4 | 0;
      var $agg_tmp2_i_sroa_0_0_tmp3_i_idx = $agg_tmp_i | 0;
      var $1 = $Pre + 8 | 0;
      var $i_027 = 0;
      while (1) {
        var $i_027;
        var $add_i = HEAP32[$__start__i >> 2] + $i_027 | 0;
        HEAP32[$Curr$s2] = HEAP32[HEAP32[HEAP32[$__begin__i_i >> 2] + ($add_i >>> 10 << 2) >> 2] + (($add_i & 1023) << 2) >> 2];
        __ZNSt3__16__treeIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEE4findIS2_EENS_15__tree_iteratorIS2_PNS_11__tree_nodeIS2_PvEEiEERKT_($agg_tmp_i, $__tree__i, $Curr);
        var $cmp_i = (HEAP32[$agg_tmp2_i_sroa_0_0_tmp3_i_idx >> 2] | 0) == ($1 | 0);
        L289 : do {
          if (!$cmp_i) {
            var $8 = HEAP32[$Curr$s2];
            var $9 = HEAP32[$8 >> 2];
            if (($9 | 0) == ($8 + 4 | 0)) {
              break;
            } else {
              var $iter_sroa_0_024 = $9;
            }
            while (1) {
              var $iter_sroa_0_024;
              var $11 = __Znwj(20);
              __ZN6BranchC1EPKcS1_($11, 0, 0);
              HEAP32[__ZNSt3__13mapIP5BlockP6BranchNS_4lessIS2_EENS_9allocatorINS_4pairIKS2_S4_EEEEEixERS9_(HEAP32[$iter_sroa_0_024 + 16 >> 2] + 12 | 0, $Curr) >> 2] = $11;
              var $call_i_i_i = __ZNSt3__111__tree_nextIPNS_16__tree_node_baseIPvEEEET_S5_($iter_sroa_0_024 | 0);
              if (($call_i_i_i | 0) == (HEAP32[$Curr$s2] + 4 | 0)) {
                break L289;
              } else {
                var $iter_sroa_0_024 = $call_i_i_i;
              }
            }
          }
        } while (0);
        var $inc = $i_027 + 1 | 0;
        if ($inc >>> 0 < HEAP32[$__first__i_i_i_i$s2] >>> 0) {
          var $i_027 = $inc;
        } else {
          break L285;
        }
      }
    }
  } while (0);
  __ZZN8Relooper9CalculateEP5BlockEN12PreOptimizer13SplitDeadEndsEv($Pre);
  var $__tree__i_i = $AllBlocks | 0;
  __ZNSt3__16__treeIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEEC1ERKS4_($__tree__i_i);
  var $cmp3618 = (HEAP32[$__first__i_i_i_i$s2] | 0) == 0;
  L298 : do {
    if (!$cmp3618) {
      var $__start__i187 = $this + 16 | 0;
      var $__begin__i_i191 = $this + 4 | 0;
      var $i32_019 = 0;
      while (1) {
        var $i32_019;
        var $add_i188 = HEAP32[$__start__i187 >> 2] + $i32_019 | 0;
        __ZNSt3__16__treeIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEE15__insert_uniqueERKS2_($ref_tmp_i, $__tree__i_i, (($add_i188 & 1023) << 2) + HEAP32[HEAP32[$__begin__i_i191 >> 2] + ($add_i188 >>> 10 << 2) >> 2] | 0);
        var $inc45 = $i32_019 + 1 | 0;
        if ($inc45 >>> 0 < HEAP32[$__first__i_i_i_i$s2] >>> 0) {
          var $i32_019 = $inc45;
        } else {
          break L298;
        }
      }
    }
  } while (0);
  var $__tree__i_i214 = $Entries | 0;
  __ZNSt3__16__treeIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEEC1ERKS4_($__tree__i_i214);
  __ZNSt3__16__treeIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEE15__insert_uniqueERKS2_($ref_tmp_i227, $__tree__i_i214, $Entry_addr);
  __ZZN8Relooper9CalculateEP5BlockEN8AnalyzerC1E_0PS_($tmp53, $this);
  var $Root = $this + 48 | 0;
  HEAP32[$Root >> 2] = __ZZN8Relooper9CalculateEP5BlockEN8Analyzer7ProcessE_0RNSt3__13setIS1_NS3_4lessIS1_EENS3_9allocatorIS1_EEEESA_P5Shape($tmp53, $AllBlocks, $Entries);
  __ZZN8Relooper9CalculateEP5BlockEN13PostOptimizerC1E_1PS_($tmp57, $this);
  __ZZN8Relooper9CalculateEP5BlockEN13PostOptimizer7ProcessE_1P5Shape($tmp57, HEAP32[$Root >> 2]);
  __ZNSt3__13setIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEED1Ev($Entries);
  __ZNSt3__13setIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEED1Ev($AllBlocks);
  __ZZN8Relooper9CalculateEP5BlockEN12PreOptimizerD1Ev($Pre);
  STACKTOP = __stackBase__;
  return;
}
__ZN8Relooper9CalculateEP5Block["X"] = 1;
function __ZZN8Relooper9CalculateEP5BlockEN12PreOptimizerC1EPS_($this, $Parent) {
  __ZZN8Relooper9CalculateEP5BlockEN12PreOptimizerC2EPS_($this, $Parent);
  return;
}
function __ZZN8Relooper9CalculateEP5BlockEN12PreOptimizer8FindLiveES1_($this, $Curr) {
  var __stackBase__ = STACKTOP;
  STACKTOP = STACKTOP + 16 | 0;
  var $agg_tmp_i = __stackBase__ + 8;
  var $Curr_addr = __stackBase__ + 12;
  HEAP32[$Curr_addr >> 2] = $Curr;
  var $__tree__i = $this + 4 | 0;
  __ZNSt3__16__treeIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEE4findIS2_EENS_15__tree_iteratorIS2_PNS_11__tree_nodeIS2_PvEEiEERKT_($agg_tmp_i, $__tree__i, $Curr_addr);
  if ((HEAP32[$agg_tmp_i >> 2] | 0) != ($this + 8 | 0)) {
    STACKTOP = __stackBase__;
    return;
  }
  __ZNSt3__16__treeIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEE15__insert_uniqueERKS2_(__stackBase__, $__tree__i, $Curr_addr);
  var $1 = HEAP32[$Curr_addr >> 2];
  var $2 = HEAP32[$1 >> 2];
  var $__first__i_i_i_i_i10310 = $1 + 4 | 0;
  if (($2 | 0) == ($__first__i_i_i_i_i10310 | 0)) {
    STACKTOP = __stackBase__;
    return;
  }
  var $4 = $__first__i_i_i_i_i10310;
  var $iter_sroa_0_012 = $2;
  while (1) {
    var $iter_sroa_0_012;
    __ZZN8Relooper9CalculateEP5BlockEN12PreOptimizer8FindLiveES1_($this, HEAP32[$iter_sroa_0_012 + 16 >> 2]);
    var $call_i_i_i = __ZNSt3__111__tree_nextIPNS_16__tree_node_baseIPvEEEET_S5_($iter_sroa_0_012 | 0);
    if (($call_i_i_i | 0) == ($4 | 0)) {
      break;
    } else {
      var $iter_sroa_0_012 = $call_i_i_i;
    }
  }
  STACKTOP = __stackBase__;
  return;
}
function __ZZN8Relooper9CalculateEP5BlockEN12PreOptimizer13SplitDeadEndsEv($this) {
  var $7$s2;
  var __stackBase__ = STACKTOP;
  STACKTOP = STACKTOP + 20 | 0;
  var $ref_tmp_i = __stackBase__;
  var $Original = __stackBase__ + 8;
  var $Prior = __stackBase__ + 12;
  var $Split = __stackBase__ + 16;
  var $__tree__i = $this + 4 | 0;
  var $0 = HEAP32[$__tree__i >> 2];
  var $__first__i_i_i_i_i = $this + 8 | 0;
  if (($0 | 0) == ($__first__i_i_i_i_i | 0)) {
    STACKTOP = __stackBase__;
    return;
  }
  var $2 = $__first__i_i_i_i_i;
  var $iter_sroa_0_026 = $0;
  var $TotalCodeSize_027 = 0;
  while (1) {
    var $TotalCodeSize_027;
    var $iter_sroa_0_026;
    var $add = _strlen(HEAP32[HEAP32[$iter_sroa_0_026 + 16 >> 2] + 56 >> 2]) + $TotalCodeSize_027 | 0;
    var $call_i_i125 = __ZNSt3__111__tree_nextIPKNS_16__tree_node_baseIPvEEEET_S6_($iter_sroa_0_026 | 0);
    if (($call_i_i125 | 0) == ($2 | 0)) {
      break;
    } else {
      var $iter_sroa_0_026 = $call_i_i125;
      var $TotalCodeSize_027 = $add;
    }
  }
  var $phitmp = ($add | 0) / 5 & -1;
  var $Parent = $this | 0;
  var $iter6_sroa_0_024 = $0;
  while (1) {
    var $iter6_sroa_0_024;
    var $7 = HEAP32[$iter6_sroa_0_024 + 16 >> 2], $7$s2 = $7 >> 2;
    HEAP32[$Original >> 2] = $7;
    var $8 = HEAP32[$7$s2 + 5];
    var $cmp = $8 >>> 0 < 2;
    L327 : do {
      if (!$cmp) {
        if ((HEAP32[$7$s2 + 2] | 0) != 0) {
          break;
        }
        if ((_strlen(HEAP32[$7$s2 + 14]) * ($8 - 1) | 0) >>> 0 > $phitmp >>> 0) {
          break;
        }
        var $11 = HEAP32[$7$s2 + 3];
        if (($11 | 0) == ($7 + 16 | 0)) {
          break;
        } else {
          var $iter26_sroa_0_022 = $11;
          var $13 = $7;
        }
        while (1) {
          var $13;
          var $iter26_sroa_0_022;
          HEAP32[$Prior >> 2] = HEAP32[$iter26_sroa_0_022 + 16 >> 2];
          var $call36 = __Znwj(68);
          var $15 = $call36;
          __ZN5BlockC1EPKc($15, HEAP32[$13 + 56 >> 2]);
          HEAP32[$Split >> 2] = $15;
          var $17 = __Znwj(20);
          __ZN6BranchC1EPKcS1_($17, 0, 0);
          HEAP32[__ZNSt3__13mapIP5BlockP6BranchNS_4lessIS2_EENS_9allocatorINS_4pairIKS2_S4_EEEEEixERS9_($call36 + 12 | 0, $Prior) >> 2] = $17;
          var $19 = __Znwj(20);
          var $20 = HEAP32[$Prior >> 2];
          var $BranchesOut44 = $20 | 0;
          __ZN6BranchC1EPKcS1_($19, HEAP32[HEAP32[__ZNSt3__13mapIP5BlockP6BranchNS_4lessIS2_EENS_9allocatorINS_4pairIKS2_S4_EEEEEixERS9_($BranchesOut44, $Original) >> 2] + 12 >> 2], HEAP32[HEAP32[__ZNSt3__13mapIP5BlockP6BranchNS_4lessIS2_EENS_9allocatorINS_4pairIKS2_S4_EEEEEixERS9_($BranchesOut44, $Original) >> 2] + 16 >> 2]);
          HEAP32[__ZNSt3__13mapIP5BlockP6BranchNS_4lessIS2_EENS_9allocatorINS_4pairIKS2_S4_EEEEEixERS9_($BranchesOut44, $Split) >> 2] = $19;
          __ZNSt3__16__treeINS_4pairIP5BlockP6BranchEENS_19__map_value_compareIS3_S5_NS_4lessIS3_EELb1EEENS_9allocatorIS6_EEE14__erase_uniqueIS3_EEjRKT_($20 | 0, $Original);
          __ZN8Relooper8AddBlockEP5Block(HEAP32[$Parent >> 2], HEAP32[$Split >> 2]);
          __ZNSt3__16__treeIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEE15__insert_uniqueERKS2_($ref_tmp_i, $__tree__i, $Split);
          var $call_i_i_i = __ZNSt3__111__tree_nextIPNS_16__tree_node_baseIPvEEEET_S5_($iter26_sroa_0_022 | 0);
          var $29 = HEAP32[$Original >> 2];
          if (($call_i_i_i | 0) == ($29 + 16 | 0)) {
            break L327;
          } else {
            var $iter26_sroa_0_022 = $call_i_i_i;
            var $13 = $29;
          }
        }
      }
    } while (0);
    var $call_i_i = __ZNSt3__111__tree_nextIPKNS_16__tree_node_baseIPvEEEET_S6_($iter6_sroa_0_024 | 0);
    if (($call_i_i | 0) == ($2 | 0)) {
      break;
    } else {
      var $iter6_sroa_0_024 = $call_i_i;
    }
  }
  STACKTOP = __stackBase__;
  return;
}
__ZZN8Relooper9CalculateEP5BlockEN12PreOptimizer13SplitDeadEndsEv["X"] = 1;
function __ZZN8Relooper9CalculateEP5BlockEN8AnalyzerC1E_0PS_($this, $Parent) {
  __ZZN8Relooper9CalculateEP5BlockEN8AnalyzerC2E_0PS_($this, $Parent);
  return;
}
function __ZN8Relooper15SetOutputBufferEPci($Buffer, $Size) {
  HEAP32[1311198] = $Buffer;
  HEAP32[1311196] = $Buffer;
  HEAP32[1311195] = $Size;
  return;
}
function __ZN8Relooper12SetAsmJSModeEi($On) {
  HEAP32[1311194] = $On;
  return;
}
function __ZN9LoopShapeD1Ev($this) {
  return;
}
function __ZNSt3__16__treeINS_4pairIPviEENS_19__map_value_compareIS2_iNS_4lessIS2_EELb1EEENS_9allocatorIS3_EEEC2ERKS7_() {
  HEAP32[1310995] = 0;
  HEAP32[1310996] = 0;
  HEAP32[1310994] = 5243980;
  return;
}
function __ZNSt3__16__treeIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEE12__find_equalIS2_EERPNS_16__tree_node_baseIPvEESD_RKT_($this, $__parent, $__v) {
  var label = 0;
  var $__first__i_i_i_i = $this + 4 | 0;
  var $__left__i = $__first__i_i_i_i | 0;
  var $0 = HEAP32[$__left__i >> 2];
  if (($0 | 0) == 0) {
    HEAP32[$__parent >> 2] = $__first__i_i_i_i;
    var $retval_0 = $__left__i;
    var $retval_0;
    return $retval_0;
  }
  var $1 = HEAP32[$__v >> 2];
  var $__nd_0_in = $0;
  while (1) {
    var $__nd_0_in;
    var $3 = HEAP32[$__nd_0_in + 16 >> 2];
    if ($1 >>> 0 < $3 >>> 0) {
      var $__left_ = $__nd_0_in | 0;
      var $4 = HEAP32[$__left_ >> 2];
      if (($4 | 0) == 0) {
        label = 284;
        break;
      } else {
        var $__nd_0_in = $4;
        continue;
      }
    }
    if ($3 >>> 0 >= $1 >>> 0) {
      label = 288;
      break;
    }
    var $__right_ = $__nd_0_in + 4 | 0;
    var $5 = HEAP32[$__right_ >> 2];
    if (($5 | 0) == 0) {
      label = 287;
      break;
    } else {
      var $__nd_0_in = $5;
    }
  }
  if (label == 287) {
    HEAP32[$__parent >> 2] = $__nd_0_in;
    var $retval_0 = $__right_;
    var $retval_0;
    return $retval_0;
  } else if (label == 288) {
    HEAP32[$__parent >> 2] = $__nd_0_in;
    var $retval_0 = $__parent;
    var $retval_0;
    return $retval_0;
  } else if (label == 284) {
    HEAP32[$__parent >> 2] = $__nd_0_in;
    var $retval_0 = $__left_;
    var $retval_0;
    return $retval_0;
  }
}
function __ZNSt3__118__tree_left_rotateIPNS_16__tree_node_baseIPvEEEEvT_($__x) {
  var $__parent_5$s2;
  var $__left_$s2;
  var $__right_ = $__x + 4 | 0;
  var $0 = HEAP32[$__right_ >> 2];
  var $__left_$s2 = ($0 | 0) >> 2;
  var $1 = HEAP32[$__left_$s2];
  HEAP32[$__right_ >> 2] = $1;
  if (($1 | 0) != 0) {
    HEAP32[$1 + 8 >> 2] = $__x;
  }
  var $__parent_5$s2 = ($__x + 8 | 0) >> 2;
  HEAP32[$0 + 8 >> 2] = HEAP32[$__parent_5$s2];
  var $3 = HEAP32[$__parent_5$s2];
  var $__left__i = $3 | 0;
  if ((HEAP32[$__left__i >> 2] | 0) == ($__x | 0)) {
    HEAP32[$__left__i >> 2] = $0;
    HEAP32[$__left_$s2] = $__x;
    HEAP32[$__parent_5$s2] = $0;
    return;
  } else {
    HEAP32[$3 + 4 >> 2] = $0;
    HEAP32[$__left_$s2] = $__x;
    HEAP32[$__parent_5$s2] = $0;
    return;
  }
}
function __ZNSt3__119__tree_right_rotateIPNS_16__tree_node_baseIPvEEEEvT_($__x) {
  var $__parent_5$s2;
  var $__right_$s2;
  var $__left_ = $__x | 0;
  var $0 = HEAP32[$__left_ >> 2];
  var $__right_$s2 = ($0 + 4 | 0) >> 2;
  var $1 = HEAP32[$__right_$s2];
  HEAP32[$__left_ >> 2] = $1;
  if (($1 | 0) != 0) {
    HEAP32[$1 + 8 >> 2] = $__x;
  }
  var $__parent_5$s2 = ($__x + 8 | 0) >> 2;
  HEAP32[$0 + 8 >> 2] = HEAP32[$__parent_5$s2];
  var $3 = HEAP32[$__parent_5$s2];
  var $__left__i = $3 | 0;
  if ((HEAP32[$__left__i >> 2] | 0) == ($__x | 0)) {
    HEAP32[$__left__i >> 2] = $0;
    HEAP32[$__right_$s2] = $__x;
    HEAP32[$__parent_5$s2] = $0;
    return;
  } else {
    HEAP32[$3 + 4 >> 2] = $0;
    HEAP32[$__right_$s2] = $__x;
    HEAP32[$__parent_5$s2] = $0;
    return;
  }
}
function __ZNSt3__16__treeIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEE13__lower_boundIS2_EENS_15__tree_iteratorIS2_PNS_11__tree_nodeIS2_PvEEiEERKT_SD_SD_($agg_result, $__v, $__root, $__result) {
  var $__root_addr_06$s2;
  var $cmp59 = ($__root | 0) == 0;
  L379 : do {
    if ($cmp59) {
      var $__result_addr_0_ph8 = $__result;
    } else {
      var $0 = HEAP32[$__v >> 2];
      var $__root_addr_0_ph10 = $__root;
      var $__result_addr_0_ph11 = $__result;
      while (1) {
        var $__result_addr_0_ph11;
        var $__root_addr_0_ph10;
        var $__root_addr_06 = $__root_addr_0_ph10, $__root_addr_06$s2 = $__root_addr_06 >> 2;
        while (1) {
          var $__root_addr_06;
          if (HEAP32[$__root_addr_06$s2 + 4] >>> 0 >= $0 >>> 0) {
            break;
          }
          var $4 = HEAP32[$__root_addr_06$s2 + 1];
          if (($4 | 0) == 0) {
            var $__result_addr_0_ph8 = $__result_addr_0_ph11;
            break L379;
          } else {
            var $__root_addr_06 = $4, $__root_addr_06$s2 = $__root_addr_06 >> 2;
          }
        }
        var $2 = HEAP32[$__root_addr_06$s2];
        if (($2 | 0) == 0) {
          var $__result_addr_0_ph8 = $__root_addr_06;
          break L379;
        } else {
          var $__root_addr_0_ph10 = $2;
          var $__result_addr_0_ph11 = $__root_addr_06;
        }
      }
    }
  } while (0);
  var $__result_addr_0_ph8;
  HEAP32[$agg_result >> 2] = $__result_addr_0_ph8;
  return;
}
function __ZNSt3__16__treeIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEEC2ERKS4_($this) {
  var $__first__i_i_i = $this + 4 | 0;
  HEAP32[$__first__i_i_i >> 2] = 0;
  HEAP32[$this + 8 >> 2] = 0;
  HEAP32[$this >> 2] = $__first__i_i_i;
  return;
}
function __ZZN8Relooper9CalculateEP5BlockEN8Analyzer7ProcessE_0RNSt3__13setIS1_NS3_4lessIS1_EENS3_9allocatorIS1_EEEESA_P5Shape($this, $Blocks, $InitialEntries) {
  var $Prev_addr_0$s2;
  var $__first__i_i_i_i315$s2;
  var $TempEntries$s2;
  var label = 0;
  var __stackBase__ = STACKTOP;
  STACKTOP = STACKTOP + 64 | 0;
  var $agg_tmp_i823 = __stackBase__;
  var $agg_tmp_i544 = __stackBase__ + 4;
  var $agg_tmp_i471 = __stackBase__ + 8;
  var $TempEntries = __stackBase__ + 12, $TempEntries$s2 = $TempEntries >> 2;
  var $IndependentGroups = __stackBase__ + 36;
  var $Origin = __stackBase__ + 48;
  var $SmallEntry = __stackBase__ + 52;
  var $LargeEntry = __stackBase__ + 56;
  var $Target = __stackBase__ + 60;
  __ZNSt3__16__treeIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEEC1ERKS4_($TempEntries | 0);
  __ZNSt3__16__treeIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEEC1ERKS4_($TempEntries + 12 | 0);
  var $__tree__i_i302 = $IndependentGroups | 0;
  var $__first__i_i_i_i315$s2 = ($IndependentGroups + 8 | 0) >> 2;
  var $__begin_node__i_i_i331 = $IndependentGroups | 0;
  var $__first__i_i_i_i_i = $IndependentGroups + 4 | 0;
  var $0 = $__first__i_i_i_i_i;
  var $agg_tmp2_i820_sroa_0_0_tmp3_i827_idx = $agg_tmp_i823 | 0;
  var $agg_tmp2_i469_sroa_0_0_tmp3_i475_idx = $agg_tmp_i471 | 0;
  var $1 = $__first__i_i_i_i_i;
  var $Prev_addr_0_ph = 0;
  var $Entries_0_ph = $InitialEntries;
  var $CurrTempIndex_0_ph = 0;
  var $Ret_0_ph = 0;
  L390 : while (1) {
    var $Ret_0_ph;
    var $CurrTempIndex_0_ph;
    var $Entries_0_ph;
    var $Prev_addr_0_ph;
    var $retval_0_ph;
    var $Prev_addr_0 = $Prev_addr_0_ph, $Prev_addr_0$s2 = $Prev_addr_0 >> 2;
    var $Entries_0 = $Entries_0_ph;
    var $CurrTempIndex_0 = $CurrTempIndex_0_ph;
    var $Ret_0 = $Ret_0_ph;
    while (1) {
      var $Ret_0;
      var $CurrTempIndex_0;
      var $Entries_0;
      var $Prev_addr_0;
      var $sub = 1 - $CurrTempIndex_0 | 0;
      var $arrayidx = $TempEntries + $sub * 12 | 0;
      __ZNSt3__16__treeIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEE5clearEv($arrayidx | 0);
      var $2 = HEAP32[$Entries_0 + 8 >> 2];
      if (($2 | 0) == 0) {
        var $retval_2 = $Ret_0;
        label = 372;
        break L390;
      } else if (($2 | 0) != 1) {
        break;
      }
      var $4 = HEAP32[HEAP32[$Entries_0 >> 2] + 16 >> 2];
      if ((HEAP32[$4 + 20 >> 2] | 0) == 0) {
        var $call15 = __ZZN8Relooper9CalculateEP5BlockEN8Analyzer10MakeSimpleE_0RNSt3__13setIS1_NS3_4lessIS1_EENS3_9allocatorIS1_EEEES1_SA_($this, $Blocks, $4, $arrayidx);
        if (($Prev_addr_0 | 0) != 0) {
          HEAP32[$Prev_addr_0$s2 + 2] = $call15;
        }
        var $call15_Ret_0 = ($Ret_0 | 0) == 0 ? $call15 : $Ret_0;
        if ((HEAP32[$TempEntries$s2 + ($sub * 3 | 0) + 2] | 0) == 0) {
          var $retval_2 = $call15_Ret_0;
          label = 369;
          break L390;
        } else {
          var $Prev_addr_0 = $call15, $Prev_addr_0$s2 = $Prev_addr_0 >> 2;
          var $Entries_0 = $arrayidx;
          var $CurrTempIndex_0 = $sub;
          var $Ret_0 = $call15_Ret_0;
          continue;
        }
      } else {
        var $call29 = __ZZN8Relooper9CalculateEP5BlockEN8Analyzer8MakeLoopE_0RNSt3__13setIS1_NS3_4lessIS1_EENS3_9allocatorIS1_EEEESA_SA_($this, $Blocks, $Entries_0, $arrayidx);
        if (($Prev_addr_0 | 0) != 0) {
          HEAP32[$Prev_addr_0$s2 + 2] = $call29;
        }
        var $call29_Ret_0 = ($Ret_0 | 0) == 0 ? $call29 : $Ret_0;
        if ((HEAP32[$TempEntries$s2 + ($sub * 3 | 0) + 2] | 0) == 0) {
          var $retval_2 = $call29_Ret_0;
          label = 370;
          break L390;
        } else {
          var $Prev_addr_0 = $call29, $Prev_addr_0$s2 = $Prev_addr_0 >> 2;
          var $Entries_0 = $arrayidx;
          var $CurrTempIndex_0 = $sub;
          var $Ret_0 = $call29_Ret_0;
          continue;
        }
      }
    }
    __ZNSt3__16__treeINS_4pairIP5BlockNS_3setIS3_NS_4lessIS3_EENS_9allocatorIS3_EEEEEENS_19__map_value_compareIS3_S9_S6_Lb1EEENS7_ISA_EEEC1ERKSC_($__tree__i_i302);
    __ZZN8Relooper9CalculateEP5BlockEN8Analyzer21FindIndependentGroupsE_0RNSt3__13setIS1_NS3_4lessIS1_EENS3_9allocatorIS1_EEEESA_RNS3_3mapIS1_S9_S6_NS7_INS3_4pairIKS1_S9_EEEEEE($Entries_0, $IndependentGroups);
    var $11 = HEAP32[$__first__i_i_i_i315$s2];
    do {
      if (($11 | 0) == 0) {
        label = 362;
      } else {
        var $12 = HEAP32[$__begin_node__i_i_i331 >> 2];
        if (($12 | 0) == ($0 | 0)) {
          var $26 = $11;
        } else {
          var $iter_sroa_0_08 = $12;
          while (1) {
            var $iter_sroa_0_08;
            var $13 = HEAP32[$iter_sroa_0_08 + 16 >> 2];
            var $call_i_i_i = __ZNSt3__111__tree_nextIPNS_16__tree_node_baseIPvEEEET_S5_($iter_sroa_0_08 | 0);
            var $15 = $call_i_i_i;
            var $16 = HEAP32[$13 + 12 >> 2];
            var $__first__i_i_i_i_i436 = $13 + 16 | 0;
            var $lnot_i_i4585 = ($16 | 0) == ($__first__i_i_i_i_i436 | 0);
            L413 : do {
              if (!$lnot_i_i4585) {
                var $__tree__i473 = $iter_sroa_0_08 + 20 | 0;
                var $18 = $iter_sroa_0_08 + 24 | 0;
                var $19 = $__first__i_i_i_i_i436;
                var $iterBranch_sroa_0_06 = $16;
                while (1) {
                  var $iterBranch_sroa_0_06;
                  HEAP32[$Origin >> 2] = HEAP32[$iterBranch_sroa_0_06 + 16 >> 2];
                  __ZNSt3__16__treeIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEE4findIS2_EENS_15__tree_iteratorIS2_PNS_11__tree_nodeIS2_PvEEiEERKT_($agg_tmp_i471, $__tree__i473, $Origin);
                  if ((HEAP32[$agg_tmp2_i469_sroa_0_0_tmp3_i475_idx >> 2] | 0) == ($18 | 0)) {
                    break;
                  }
                  var $call_i_i_i564 = __ZNSt3__111__tree_nextIPNS_16__tree_node_baseIPvEEEET_S5_($iterBranch_sroa_0_06 | 0);
                  if (($call_i_i_i564 | 0) == ($19 | 0)) {
                    break L413;
                  } else {
                    var $iterBranch_sroa_0_06 = $call_i_i_i564;
                  }
                }
                __ZNSt3__16__treeINS_4pairIP5BlockNS_3setIS3_NS_4lessIS3_EENS_9allocatorIS3_EEEEEENS_19__map_value_compareIS3_S9_S6_Lb1EEENS7_ISA_EEE5eraseENS_21__tree_const_iteratorISA_PKNS_11__tree_nodeISA_PvEEiEE($agg_tmp_i544, $__tree__i_i302, $iter_sroa_0_08);
              }
            } while (0);
            if (($call_i_i_i | 0) == ($1 | 0)) {
              break;
            } else {
              var $iter_sroa_0_08 = $15;
            }
          }
          var $26 = HEAP32[$__first__i_i_i_i315$s2];
        }
        var $26;
        if (($26 | 0) == 2) {
          var $27 = HEAP32[$__begin_node__i_i_i331 >> 2];
          var $28 = HEAP32[$27 + 16 >> 2];
          HEAP32[$SmallEntry >> 2] = $28;
          var $29 = HEAP32[$27 + 28 >> 2];
          var $call_i_i_i638 = __ZNSt3__111__tree_nextIPNS_16__tree_node_baseIPvEEEET_S5_($27 | 0);
          var $31 = HEAP32[$call_i_i_i638 + 16 >> 2];
          HEAP32[$LargeEntry >> 2] = $31;
          var $32 = HEAP32[$call_i_i_i638 + 28 >> 2];
          var $cmp112 = ($29 | 0) == ($32 | 0);
          L424 : do {
            if (!$cmp112) {
              if (($29 | 0) > ($32 | 0)) {
                HEAP32[$SmallEntry >> 2] = $31;
                HEAP32[$LargeEntry >> 2] = $28;
              }
              var $call122 = __ZNSt3__13mapIP5BlockNS_3setIS2_NS_4lessIS2_EENS_9allocatorIS2_EEEES5_NS6_INS_4pairIKS2_S8_EEEEEixERSA_($IndependentGroups, $SmallEntry);
              var $__tree__i678 = $call122 | 0;
              var $33 = HEAP32[$call122 >> 2];
              var $__first__i_i_i_i_i716 = $call122 + 4 | 0;
              var $34 = $__first__i_i_i_i_i716;
              var $lnot_i11 = ($33 | 0) == ($34 | 0);
              L430 : do {
                if (!$lnot_i11) {
                  var $35 = $__first__i_i_i_i_i716;
                  var $iter124_sroa_0_012 = $33;
                  while (1) {
                    var $iter124_sroa_0_012;
                    var $36 = HEAP32[$iter124_sroa_0_012 + 16 >> 2];
                    var $37 = HEAP32[$36 >> 2];
                    var $__first__i_i_i_i_i787 = $36 + 4 | 0;
                    var $lnot_i_i8099 = ($37 | 0) == ($__first__i_i_i_i_i787 | 0);
                    L434 : do {
                      if (!$lnot_i_i8099) {
                        var $39 = $__first__i_i_i_i_i787;
                        var $iter135_sroa_0_010 = $37;
                        while (1) {
                          var $iter135_sroa_0_010;
                          HEAP32[$Target >> 2] = HEAP32[$iter135_sroa_0_010 + 16 >> 2];
                          __ZNSt3__16__treeIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEE4findIS2_EENS_15__tree_iteratorIS2_PNS_11__tree_nodeIS2_PvEEiEERKT_($agg_tmp_i823, $__tree__i678, $Target);
                          if ((HEAP32[$agg_tmp2_i820_sroa_0_0_tmp3_i827_idx >> 2] | 0) == ($34 | 0)) {
                            break L424;
                          }
                          var $call_i_i_i884 = __ZNSt3__111__tree_nextIPNS_16__tree_node_baseIPvEEEET_S5_($iter135_sroa_0_010 | 0);
                          if (($call_i_i_i884 | 0) == ($39 | 0)) {
                            break L434;
                          } else {
                            var $iter135_sroa_0_010 = $call_i_i_i884;
                          }
                        }
                      }
                    } while (0);
                    var $call_i_i = __ZNSt3__111__tree_nextIPKNS_16__tree_node_baseIPvEEEET_S6_($iter124_sroa_0_012 | 0);
                    if (($call_i_i | 0) == ($35 | 0)) {
                      break L430;
                    } else {
                      var $iter124_sroa_0_012 = $call_i_i;
                    }
                  }
                }
              } while (0);
              __ZNSt3__16__treeINS_4pairIP5BlockNS_3setIS3_NS_4lessIS3_EENS_9allocatorIS3_EEEEEENS_19__map_value_compareIS3_S9_S6_Lb1EEENS7_ISA_EEE14__erase_uniqueIS3_EEjRKT_($__tree__i_i302, $LargeEntry);
            }
          } while (0);
          var $45 = HEAP32[$__first__i_i_i_i315$s2];
        } else {
          var $45 = $26;
        }
        var $45;
        if (($45 | 0) == 0) {
          label = 362;
          break;
        }
        var $call177 = __ZZN8Relooper9CalculateEP5BlockEN8Analyzer12MakeMultipleE_0RNSt3__13setIS1_NS3_4lessIS1_EENS3_9allocatorIS1_EEEESA_RNS3_3mapIS1_S9_S6_NS7_INS3_4pairIKS1_S9_EEEEEEP5ShapeSA_($this, $Blocks, $Entries_0, $IndependentGroups, $Prev_addr_0, $arrayidx);
        if (($Prev_addr_0 | 0) != 0) {
          HEAP32[$Prev_addr_0$s2 + 2] = $call177;
        }
        var $call177_Ret_0 = ($Ret_0 | 0) == 0 ? $call177 : $Ret_0;
        var $tobool186 = (HEAP32[$TempEntries$s2 + ($sub * 3 | 0) + 2] | 0) == 0;
        var $retval_1 = $tobool186 ? $call177_Ret_0 : $retval_0_ph;
        var $Prev_addr_1 = $tobool186 ? $Prev_addr_0 : $call177;
        var $Entries_1 = $tobool186 ? $Entries_0 : $arrayidx;
        var $Ret_5 = $call177_Ret_0;
        var $cleanup_dest_slot_0 = $tobool186 ? 1 : 2;
        break;
      }
    } while (0);
    if (label == 362) {
      label = 0;
      var $call194 = __ZZN8Relooper9CalculateEP5BlockEN8Analyzer8MakeLoopE_0RNSt3__13setIS1_NS3_4lessIS1_EENS3_9allocatorIS1_EEEESA_SA_($this, $Blocks, $Entries_0, $arrayidx);
      if (($Prev_addr_0 | 0) != 0) {
        HEAP32[$Prev_addr_0$s2 + 2] = $call194;
      }
      var $call194_Ret_0 = ($Ret_0 | 0) == 0 ? $call194 : $Ret_0;
      var $tobool203 = (HEAP32[$TempEntries$s2 + ($sub * 3 | 0) + 2] | 0) == 0;
      var $retval_1 = $tobool203 ? $call194_Ret_0 : $retval_0_ph;
      var $Prev_addr_1 = $tobool203 ? $Prev_addr_0 : $call194;
      var $Entries_1 = $tobool203 ? $Entries_0 : $arrayidx;
      var $Ret_5 = $call194_Ret_0;
      var $cleanup_dest_slot_0 = $tobool203 ? 1 : 2;
    }
    var $cleanup_dest_slot_0;
    var $Ret_5;
    var $Entries_1;
    var $Prev_addr_1;
    var $retval_1;
    __ZNSt3__13mapIP5BlockNS_3setIS2_NS_4lessIS2_EENS_9allocatorIS2_EEEES5_NS6_INS_4pairIKS2_S8_EEEEED1Ev($IndependentGroups);
    if (($cleanup_dest_slot_0 | 0) == 2) {
      var $retval_0_ph = $retval_1;
      var $Prev_addr_0_ph = $Prev_addr_1;
      var $Entries_0_ph = $Entries_1;
      var $CurrTempIndex_0_ph = $sub;
      var $Ret_0_ph = $Ret_5;
    } else {
      var $retval_2 = $retval_1;
      label = 371;
      break;
    }
  }
  if (label == 369) {
    var $retval_2;
    var $arraydestroy_element212 = $TempEntries + 12 | 0;
    __ZNSt3__13setIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEED1Ev($arraydestroy_element212);
    var $arraydestroy_element212_1 = $TempEntries | 0;
    __ZNSt3__13setIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEED1Ev($arraydestroy_element212_1);
    STACKTOP = __stackBase__;
    return $retval_2;
  } else if (label == 370) {
    var $retval_2;
    var $arraydestroy_element212 = $TempEntries + 12 | 0;
    __ZNSt3__13setIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEED1Ev($arraydestroy_element212);
    var $arraydestroy_element212_1 = $TempEntries | 0;
    __ZNSt3__13setIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEED1Ev($arraydestroy_element212_1);
    STACKTOP = __stackBase__;
    return $retval_2;
  } else if (label == 371) {
    var $retval_2;
    var $arraydestroy_element212 = $TempEntries + 12 | 0;
    __ZNSt3__13setIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEED1Ev($arraydestroy_element212);
    var $arraydestroy_element212_1 = $TempEntries | 0;
    __ZNSt3__13setIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEED1Ev($arraydestroy_element212_1);
    STACKTOP = __stackBase__;
    return $retval_2;
  } else if (label == 372) {
    var $retval_2;
    var $arraydestroy_element212 = $TempEntries + 12 | 0;
    __ZNSt3__13setIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEED1Ev($arraydestroy_element212);
    var $arraydestroy_element212_1 = $TempEntries | 0;
    __ZNSt3__13setIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEED1Ev($arraydestroy_element212_1);
    STACKTOP = __stackBase__;
    return $retval_2;
  }
}
__ZZN8Relooper9CalculateEP5BlockEN8Analyzer7ProcessE_0RNSt3__13setIS1_NS3_4lessIS1_EENS3_9allocatorIS1_EEEESA_P5Shape["X"] = 1;
function __ZZN8Relooper9CalculateEP5BlockEN13PostOptimizerC1E_1PS_($this, $ParentInit) {
  __ZZN8Relooper9CalculateEP5BlockEN13PostOptimizerC2E_1PS_($this, $ParentInit);
  return;
}
function __ZZN8Relooper9CalculateEP5BlockEN13PostOptimizer7ProcessE_1P5Shape($this, $Root) {
  __ZZN8Relooper9CalculateEP5BlockEN13PostOptimizer19RemoveUnneededFlowsE_1P5ShapeS4_($Root, 0);
  __ZZN8Relooper9CalculateEP5BlockEN13PostOptimizer16FindLabeledLoopsE_1P5Shape($this, $Root);
  return;
}
function __ZNSt3__13setIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEED1Ev($this) {
  __ZNSt3__13setIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEED2Ev($this);
  return;
}
function __ZZN8Relooper9CalculateEP5BlockEN12PreOptimizerD1Ev($this) {
  __ZZN8Relooper9CalculateEP5BlockEN12PreOptimizerD2Ev($this);
  return;
}
function __ZN8Relooper6RenderEv($this) {
  HEAP32[1311198] = HEAP32[1311196];
  var $1 = HEAP32[$this + 48 >> 2];
  FUNCTION_TABLE[HEAP32[HEAP32[$1 >> 2] + 8 >> 2]]($1, 0);
  return;
}
function ___cxx_global_var_init() {
  __ZNSt3__16__treeINS_4pairIPviEENS_19__map_value_compareIS2_iNS_4lessIS2_EELb1EEENS_9allocatorIS3_EEEC1ERKS7_();
  _atexit(54, 5243976, ___dso_handle);
  return;
}
function __ZNSt3__13mapIPviNS_4lessIS1_EENS_9allocatorINS_4pairIKS1_iEEEEED1Ev($this) {
  __ZNSt3__13mapIPviNS_4lessIS1_EENS_9allocatorINS_4pairIKS1_iEEEEED2Ev($this);
  return;
}
function _rl_set_output_buffer($buffer, $size) {
  __ZN8Relooper15SetOutputBufferEPci($buffer, $size);
  return;
}
Module["_rl_set_output_buffer"] = _rl_set_output_buffer;
function _rl_make_output_buffer($size) {
  __ZN8Relooper15SetOutputBufferEPci(_malloc($size), $size);
  return;
}
Module["_rl_make_output_buffer"] = _rl_make_output_buffer;
function _rl_set_asm_js_mode($on) {
  __ZN8Relooper12SetAsmJSModeEi($on);
  return;
}
Module["_rl_set_asm_js_mode"] = _rl_set_asm_js_mode;
function _rl_new_block($text) {
  var $call = __Znwj(68);
  __ZN5BlockC1EPKc($call, $text);
  return $call;
}
Module["_rl_new_block"] = _rl_new_block;
function _rl_delete_block($block) {
  if (($block | 0) == 0) {
    return;
  }
  __ZN5BlockD1Ev($block);
  __ZdlPv($block);
  return;
}
Module["_rl_delete_block"] = _rl_delete_block;
function _rl_block_add_branch_to($from, $to, $condition, $code) {
  __ZN5Block11AddBranchToEPS_PKcS2_($from, $to, $condition, $code);
  return;
}
Module["_rl_block_add_branch_to"] = _rl_block_add_branch_to;
function _rl_new_relooper() {
  var $call = __Znwj(52);
  __ZN8RelooperC1Ev($call);
  return $call;
}
Module["_rl_new_relooper"] = _rl_new_relooper;
function _rl_delete_relooper($relooper) {
  if (($relooper | 0) == 0) {
    return;
  }
  __ZN8RelooperD1Ev($relooper);
  __ZdlPv($relooper);
  return;
}
Module["_rl_delete_relooper"] = _rl_delete_relooper;
function _rl_relooper_add_block($relooper, $block) {
  __ZN8Relooper8AddBlockEP5Block($relooper, $block);
  return;
}
Module["_rl_relooper_add_block"] = _rl_relooper_add_block;
function _rl_relooper_calculate($relooper, $entry1) {
  __ZN8Relooper9CalculateEP5Block($relooper, $entry1);
  return;
}
Module["_rl_relooper_calculate"] = _rl_relooper_calculate;
function _rl_relooper_render($relooper) {
  __ZN8Relooper6RenderEv($relooper);
  return;
}
Module["_rl_relooper_render"] = _rl_relooper_render;
function __ZN13MultipleShapeD1Ev($this) {
  __ZN13MultipleShapeD2Ev($this);
  return;
}
function __ZN13MultipleShapeD0Ev($this) {
  __ZN13MultipleShapeD1Ev($this);
  __ZdlPv($this);
  return;
}
function __ZN9LoopShapeD0Ev($this) {
  __ZdlPv($this);
  return;
}
function __ZNSt3__16__treeINS_4pairIPviEENS_19__map_value_compareIS2_iNS_4lessIS2_EELb1EEENS_9allocatorIS3_EEEC1ERKS7_() {
  __ZNSt3__16__treeINS_4pairIPviEENS_19__map_value_compareIS2_iNS_4lessIS2_EELb1EEENS_9allocatorIS3_EEEC2ERKS7_();
  return;
}
function __ZNSt3__16__treeIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEE15__insert_uniqueERKS2_($agg_result, $this, $__v) {
  var __stackBase__ = STACKTOP;
  STACKTOP = STACKTOP + 16 | 0;
  var $__parent = __stackBase__;
  var $temp_lvalue = __stackBase__ + 4;
  var $call = __ZNSt3__16__treeIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEE12__find_equalIS2_EERPNS_16__tree_node_baseIPvEESD_RKT_($this, $__parent, $__v);
  var $0 = HEAP32[$call >> 2];
  if (($0 | 0) == 0) {
    __ZNSt3__16__treeIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEE16__construct_nodeERKS2_($temp_lvalue, $this, $__v);
    var $__first__i_i_i28_i_i = $temp_lvalue | 0;
    var $2 = HEAP32[$__first__i_i_i28_i_i >> 2];
    HEAP32[$__first__i_i_i28_i_i >> 2] = 0;
    __ZNSt3__16__treeIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEE16__insert_node_atEPNS_16__tree_node_baseIPvEERSB_SB_($this, HEAP32[$__parent >> 2], $call, $2 | 0);
    var $__r_0 = $2;
    var $__inserted_0 = 1;
  } else {
    var $__r_0 = $0;
    var $__inserted_0 = 0;
  }
  var $__inserted_0;
  var $__r_0;
  HEAP32[$agg_result >> 2] = $__r_0;
  HEAP8[$agg_result + 4 | 0] = $__inserted_0;
  STACKTOP = __stackBase__;
  return;
}
function __ZNSt3__16__treeIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEE16__construct_nodeERKS2_($agg_result, $this, $__v) {
  var __stackBase__ = STACKTOP;
  STACKTOP = STACKTOP + 8 | 0;
  var $call_i_i = __Znwj(20);
  var $agg_tmp87_sroa_2_5_agg_tmp_sroa_2_5_idx_idx = __stackBase__ + 4 | 0;
  var $__h_sroa_364 = __stackBase__ | 0;
  HEAP8[$__h_sroa_364] = HEAP8[$agg_tmp87_sroa_2_5_agg_tmp_sroa_2_5_idx_idx];
  HEAP8[$__h_sroa_364 + 1] = HEAP8[$agg_tmp87_sroa_2_5_agg_tmp_sroa_2_5_idx_idx + 1];
  HEAP8[$__h_sroa_364 + 2] = HEAP8[$agg_tmp87_sroa_2_5_agg_tmp_sroa_2_5_idx_idx + 2];
  var $__value_ = $call_i_i + 16 | 0;
  if (($__value_ | 0) != 0) {
    HEAP32[$__value_ >> 2] = HEAP32[$__v >> 2];
  }
  HEAP32[$agg_result >> 2] = $call_i_i;
  HEAP32[$agg_result + 4 >> 2] = $this + 4 | 0;
  HEAP8[$agg_result + 8 | 0] = 1;
  var $agg_tmp3_i_i_i_i236_sroa_2_5__raw_idx = $agg_result + 9 | 0;
  HEAP8[$agg_tmp3_i_i_i_i236_sroa_2_5__raw_idx] = HEAP8[$__h_sroa_364];
  HEAP8[$agg_tmp3_i_i_i_i236_sroa_2_5__raw_idx + 1] = HEAP8[$__h_sroa_364 + 1];
  HEAP8[$agg_tmp3_i_i_i_i236_sroa_2_5__raw_idx + 2] = HEAP8[$__h_sroa_364 + 2];
  STACKTOP = __stackBase__;
  return;
}
function __ZNSt3__16__treeIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEE16__insert_node_atEPNS_16__tree_node_baseIPvEERSB_SB_($this, $__parent, $__child, $__new_node) {
  HEAP32[$__new_node >> 2] = 0;
  HEAP32[$__new_node + 4 >> 2] = 0;
  HEAP32[$__new_node + 8 >> 2] = $__parent;
  HEAP32[$__child >> 2] = $__new_node;
  var $__begin_node__i42 = $this | 0;
  var $1 = HEAP32[HEAP32[$__begin_node__i42 >> 2] >> 2];
  if (($1 | 0) == 0) {
    var $3 = $__new_node;
  } else {
    HEAP32[$__begin_node__i42 >> 2] = $1;
    var $3 = HEAP32[$__child >> 2];
  }
  var $3;
  __ZNSt3__127__tree_balance_after_insertIPNS_16__tree_node_baseIPvEEEEvT_S5_(HEAP32[$this + 4 >> 2], $3);
  var $__first__i_i_i = $this + 8 | 0;
  HEAP32[$__first__i_i_i >> 2] = HEAP32[$__first__i_i_i >> 2] + 1 | 0;
  return;
}
function __ZNSt3__127__tree_balance_after_insertIPNS_16__tree_node_baseIPvEEEEvT_S5_($__root, $__x) {
  var $__parent__i$s2;
  var label = 0;
  var $cmp = ($__x | 0) == ($__root | 0);
  HEAP8[$__x + 12 | 0] = $cmp & 1;
  if ($cmp) {
    return;
  } else {
    var $__x_addr_046 = $__x;
  }
  while (1) {
    var $__x_addr_046;
    var $0 = HEAP32[$__x_addr_046 + 8 >> 2];
    var $__is_black_2 = $0 + 12 | 0;
    if ((HEAP8[$__is_black_2] & 1) << 24 >> 24 != 0) {
      label = 434;
      break;
    }
    var $__parent__i$s2 = ($0 + 8 | 0) >> 2;
    var $3 = HEAP32[$__parent__i$s2];
    var $4 = HEAP32[$3 >> 2];
    if (($0 | 0) == ($4 | 0)) {
      var $5 = HEAP32[$3 + 4 >> 2];
      if (($5 | 0) == 0) {
        label = 420;
        break;
      }
      var $__is_black_9 = $5 + 12 | 0;
      if ((HEAP8[$__is_black_9] & 1) << 24 >> 24 != 0) {
        label = 420;
        break;
      }
      HEAP8[$__is_black_2] = 1;
      HEAP8[$3 + 12 | 0] = ($3 | 0) == ($__root | 0) & 1;
      HEAP8[$__is_black_9] = 1;
    } else {
      if (($4 | 0) == 0) {
        label = 427;
        break;
      }
      var $__is_black_38 = $4 + 12 | 0;
      if ((HEAP8[$__is_black_38] & 1) << 24 >> 24 != 0) {
        label = 427;
        break;
      }
      HEAP8[$__is_black_2] = 1;
      HEAP8[$3 + 12 | 0] = ($3 | 0) == ($__root | 0) & 1;
      HEAP8[$__is_black_38] = 1;
    }
    if (($3 | 0) == ($__root | 0)) {
      label = 431;
      break;
    } else {
      var $__x_addr_046 = $3;
    }
  }
  if (label == 420) {
    if (($__x_addr_046 | 0) == (HEAP32[$0 >> 2] | 0)) {
      var $10 = $0;
      var $9 = $3;
    } else {
      __ZNSt3__118__tree_left_rotateIPNS_16__tree_node_baseIPvEEEEvT_($0);
      var $_pre = HEAP32[$__parent__i$s2];
      var $10 = $_pre;
      var $9 = HEAP32[$_pre + 8 >> 2];
    }
    var $9;
    var $10;
    HEAP8[$10 + 12 | 0] = 1;
    HEAP8[$9 + 12 | 0] = 0;
    __ZNSt3__119__tree_right_rotateIPNS_16__tree_node_baseIPvEEEEvT_($9);
    return;
  } else if (label == 427) {
    if (($__x_addr_046 | 0) == (HEAP32[$0 >> 2] | 0)) {
      __ZNSt3__119__tree_right_rotateIPNS_16__tree_node_baseIPvEEEEvT_($0);
      var $_pre54 = HEAP32[$__parent__i$s2];
      var $15 = $_pre54;
      var $14 = HEAP32[$_pre54 + 8 >> 2];
    } else {
      var $15 = $0;
      var $14 = $3;
    }
    var $14;
    var $15;
    HEAP8[$15 + 12 | 0] = 1;
    HEAP8[$14 + 12 | 0] = 0;
    __ZNSt3__118__tree_left_rotateIPNS_16__tree_node_baseIPvEEEEvT_($14);
    return;
  } else if (label == 431) {
    return;
  } else if (label == 434) {
    return;
  }
}
__ZNSt3__127__tree_balance_after_insertIPNS_16__tree_node_baseIPvEEEEvT_S5_["X"] = 1;
function __ZNSt3__16__treeIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEE4findIS2_EENS_15__tree_iteratorIS2_PNS_11__tree_nodeIS2_PvEEiEERKT_($agg_result, $this, $__v) {
  var __stackBase__ = STACKTOP;
  STACKTOP = STACKTOP + 4 | 0;
  var $__p = __stackBase__;
  var $__first__i_i_i_i = $this + 4 | 0;
  var $0 = $__first__i_i_i_i;
  __ZNSt3__16__treeIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEE13__lower_boundIS2_EENS_15__tree_iteratorIS2_PNS_11__tree_nodeIS2_PvEEiEERKT_SD_SD_($__p, $__v, HEAP32[$__first__i_i_i_i >> 2], $0);
  var $3 = HEAP32[$__p >> 2];
  do {
    if (($3 | 0) != ($0 | 0)) {
      if (HEAP32[$__v >> 2] >>> 0 < HEAP32[$3 + 16 >> 2] >>> 0) {
        break;
      }
      HEAP32[$agg_result >> 2] = $3;
      STACKTOP = __stackBase__;
      return;
    }
  } while (0);
  HEAP32[$agg_result >> 2] = $0;
  STACKTOP = __stackBase__;
  return;
}
function __ZNSt3__16__treeIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEEC1ERKS4_($this) {
  __ZNSt3__16__treeIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEEC2ERKS4_($this);
  return;
}
function __ZNSt3__112__deque_baseIP5BlockNS_9allocatorIS2_EEE3endEv($agg_result, $this) {
  var $add = HEAP32[$this + 16 >> 2] + HEAP32[$this + 20 >> 2] | 0;
  var $2 = HEAP32[$this + 4 >> 2];
  var $add_ptr = ($add >>> 10 << 2) + $2 | 0;
  if ((HEAP32[$this + 8 >> 2] | 0) == ($2 | 0)) {
    var $cond = 0;
  } else {
    var $cond = (($add & 1023) << 2) + HEAP32[$add_ptr >> 2] | 0;
  }
  var $cond;
  HEAP32[$agg_result >> 2] = $add_ptr;
  HEAP32[$agg_result + 4 >> 2] = $cond;
  return;
}
function __ZNSt3__114__split_bufferIPP5BlockRNS_9allocatorIS3_EEE18__construct_at_endINS_13move_iteratorIPS3_EEEENS_9enable_ifIXsr21__is_forward_iteratorIT_EE5valueEvE4typeESD_SD_($this, $__first_0, $__last_0) {
  var $__end_$s2;
  if (($__last_0 | 0) == ($__first_0 | 0)) {
    return;
  }
  var $__end_$s2 = ($this + 8 | 0) >> 2;
  var $1 = $__first_0;
  var $0 = HEAP32[$__end_$s2];
  while (1) {
    var $0;
    var $1;
    if (($0 | 0) == 0) {
      var $3 = 0;
    } else {
      HEAP32[$0 >> 2] = HEAP32[$1 >> 2];
      var $3 = HEAP32[$__end_$s2];
    }
    var $3;
    var $incdec_ptr = $3 + 4 | 0;
    HEAP32[$__end_$s2] = $incdec_ptr;
    var $incdec_ptr_i = $1 + 4 | 0;
    if (($incdec_ptr_i | 0) == ($__last_0 | 0)) {
      break;
    } else {
      var $1 = $incdec_ptr_i;
      var $0 = $incdec_ptr;
    }
  }
  return;
}
function __ZNSt3__111__tree_nextIPNS_16__tree_node_baseIPvEEEET_S5_($__x) {
  var $0 = HEAP32[$__x + 4 >> 2];
  if (($0 | 0) == 0) {
    var $__x_addr_0 = $__x;
    while (1) {
      var $__x_addr_0;
      var $2 = HEAP32[$__x_addr_0 + 8 >> 2];
      if (($__x_addr_0 | 0) == (HEAP32[$2 >> 2] | 0)) {
        var $retval_0 = $2;
        break;
      } else {
        var $__x_addr_0 = $2;
      }
    }
    var $retval_0;
    return $retval_0;
  } else {
    var $__x_addr_i9_0 = $0;
    while (1) {
      var $__x_addr_i9_0;
      var $1 = HEAP32[$__x_addr_i9_0 >> 2];
      if (($1 | 0) == 0) {
        var $retval_0 = $__x_addr_i9_0;
        break;
      } else {
        var $__x_addr_i9_0 = $1;
      }
    }
    var $retval_0;
    return $retval_0;
  }
}
function __ZNSt3__16__treeINS_4pairIP5BlockP5ShapeEENS_19__map_value_compareIS3_S5_NS_4lessIS3_EELb1EEENS_9allocatorIS6_EEE13__lower_boundIS3_EENS_15__tree_iteratorIS6_PNS_11__tree_nodeIS6_PvEEiEERKT_SJ_SJ_($agg_result, $__v, $__root, $__result) {
  var $__root_addr_06$s2;
  var $cmp59 = ($__root | 0) == 0;
  L561 : do {
    if ($cmp59) {
      var $__result_addr_0_ph8 = $__result;
    } else {
      var $0 = HEAP32[$__v >> 2];
      var $__root_addr_0_ph10 = $__root;
      var $__result_addr_0_ph11 = $__result;
      while (1) {
        var $__result_addr_0_ph11;
        var $__root_addr_0_ph10;
        var $__root_addr_06 = $__root_addr_0_ph10, $__root_addr_06$s2 = $__root_addr_06 >> 2;
        while (1) {
          var $__root_addr_06;
          if (HEAP32[$__root_addr_06$s2 + 4] >>> 0 >= $0 >>> 0) {
            break;
          }
          var $4 = HEAP32[$__root_addr_06$s2 + 1];
          if (($4 | 0) == 0) {
            var $__result_addr_0_ph8 = $__result_addr_0_ph11;
            break L561;
          } else {
            var $__root_addr_06 = $4, $__root_addr_06$s2 = $__root_addr_06 >> 2;
          }
        }
        var $2 = HEAP32[$__root_addr_06$s2];
        if (($2 | 0) == 0) {
          var $__result_addr_0_ph8 = $__root_addr_06;
          break L561;
        } else {
          var $__root_addr_0_ph10 = $2;
          var $__result_addr_0_ph11 = $__root_addr_06;
        }
      }
    }
  } while (0);
  var $__result_addr_0_ph8;
  HEAP32[$agg_result >> 2] = $__result_addr_0_ph8;
  return;
}
function __ZNSt3__114__split_bufferIPP5BlockNS_9allocatorIS3_EEE10push_frontERKS3_($this, $__x) {
  var $__first__i_i_i$s2;
  var $__end_$s2;
  var $__first_$s2;
  var $__begin_$s2;
  var __stackBase__ = STACKTOP;
  STACKTOP = STACKTOP + 20 | 0;
  var $__t = __stackBase__;
  var $__begin_$s2 = ($this + 4 | 0) >> 2;
  var $0 = HEAP32[$__begin_$s2];
  var $__first_$s2 = ($this | 0) >> 2;
  do {
    if (($0 | 0) == (HEAP32[$__first_$s2] | 0)) {
      var $__end_$s2 = ($this + 8 | 0) >> 2;
      var $2 = HEAP32[$__end_$s2];
      var $3 = $this + 12 | 0;
      var $__first__i_i_i$s2 = ($3 | 0) >> 2;
      var $4 = HEAP32[$__first__i_i_i$s2];
      var $sub_ptr_lhs_cast = $4;
      if ($2 >>> 0 < $4 >>> 0) {
        var $sub_ptr_rhs_cast = $2;
        var $div = ($sub_ptr_lhs_cast - $sub_ptr_rhs_cast + 4 >> 2 | 0) / 2 & -1;
        var $sub_ptr_sub_i_i = $sub_ptr_rhs_cast - $0 | 0;
        var $add_ptr_i_i = ($div - ($sub_ptr_sub_i_i >> 2) << 2) + $2 | 0;
        _memmove($add_ptr_i_i, $0, $sub_ptr_sub_i_i, 4, 0);
        HEAP32[$__begin_$s2] = $add_ptr_i_i;
        HEAP32[$__end_$s2] = ($div << 2) + HEAP32[$__end_$s2] | 0;
        var $19 = $add_ptr_i_i;
        break;
      } else {
        var $mul = $sub_ptr_lhs_cast - $0 >> 1;
        var $_sroa_speculated = ($mul | 0) == 0 ? 1 : $mul;
        __ZNSt3__114__split_bufferIPP5BlockRNS_9allocatorIS3_EEEC1EjjS6_($__t, $_sroa_speculated, ($_sroa_speculated + 3 | 0) >>> 2, $3);
        __ZNSt3__114__split_bufferIPP5BlockRNS_9allocatorIS3_EEE18__construct_at_endINS_13move_iteratorIPS3_EEEENS_9enable_ifIXsr21__is_forward_iteratorIT_EE5valueEvE4typeESD_SD_($__t, HEAP32[$__begin_$s2], HEAP32[$__end_$s2]);
        var $__first_30 = $__t | 0;
        var $11 = HEAP32[$__first_$s2];
        HEAP32[$__first_$s2] = HEAP32[$__first_30 >> 2];
        HEAP32[$__first_30 >> 2] = $11;
        var $__begin_33 = $__t + 4 | 0;
        var $13 = HEAP32[$__begin_$s2];
        HEAP32[$__begin_$s2] = HEAP32[$__begin_33 >> 2];
        HEAP32[$__begin_33 >> 2] = $13;
        var $__end_36 = $__t + 8 | 0;
        var $15 = HEAP32[$__end_$s2];
        HEAP32[$__end_$s2] = HEAP32[$__end_36 >> 2];
        HEAP32[$__end_36 >> 2] = $15;
        var $__first__i_i_i72 = $__t + 12 | 0;
        var $17 = HEAP32[$__first__i_i_i$s2];
        HEAP32[$__first__i_i_i$s2] = HEAP32[$__first__i_i_i72 >> 2];
        HEAP32[$__first__i_i_i72 >> 2] = $17;
        __ZNSt3__114__split_bufferIPP5BlockRNS_9allocatorIS3_EEED1Ev($__t);
        var $19 = HEAP32[$__begin_$s2];
        break;
      }
    } else {
      var $19 = $0;
    }
  } while (0);
  var $19;
  var $add_ptr45 = $19 - 4 | 0;
  if (($add_ptr45 | 0) == 0) {
    var $21 = $19;
    var $21;
    var $incdec_ptr = $21 - 4 | 0;
    HEAP32[$__begin_$s2] = $incdec_ptr;
    STACKTOP = __stackBase__;
    return;
  }
  HEAP32[$add_ptr45 >> 2] = HEAP32[$__x >> 2];
  var $21 = HEAP32[$__begin_$s2];
  var $21;
  var $incdec_ptr = $21 - 4 | 0;
  HEAP32[$__begin_$s2] = $incdec_ptr;
  STACKTOP = __stackBase__;
  return;
}
__ZNSt3__114__split_bufferIPP5BlockNS_9allocatorIS3_EEE10push_frontERKS3_["X"] = 1;
function __ZNSt3__114__split_bufferIPP5BlockRNS_9allocatorIS3_EEEC1EjjS6_($this, $__cap, $__start, $__a) {
  __ZNSt3__114__split_bufferIPP5BlockRNS_9allocatorIS3_EEEC2EjjS6_($this, $__cap, $__start, $__a);
  return;
}
function __ZNSt3__114__split_bufferIPP5BlockRNS_9allocatorIS3_EEE10push_frontERKS3_($this, $__x) {
  var $__first__i_i_i$s2;
  var $__end_$s2;
  var $__first_$s2;
  var $__begin_$s2;
  var __stackBase__ = STACKTOP;
  STACKTOP = STACKTOP + 20 | 0;
  var $__t = __stackBase__;
  var $__begin_$s2 = ($this + 4 | 0) >> 2;
  var $0 = HEAP32[$__begin_$s2];
  var $__first_$s2 = ($this | 0) >> 2;
  do {
    if (($0 | 0) == (HEAP32[$__first_$s2] | 0)) {
      var $__end_$s2 = ($this + 8 | 0) >> 2;
      var $2 = HEAP32[$__end_$s2];
      var $__first__i_i_i$s2 = ($this + 12 | 0) >> 2;
      var $3 = HEAP32[$__first__i_i_i$s2];
      var $sub_ptr_lhs_cast = $3;
      if ($2 >>> 0 < $3 >>> 0) {
        var $sub_ptr_rhs_cast = $2;
        var $div = ($sub_ptr_lhs_cast - $sub_ptr_rhs_cast + 4 >> 2 | 0) / 2 & -1;
        var $sub_ptr_sub_i_i = $sub_ptr_rhs_cast - $0 | 0;
        var $add_ptr_i_i = ($div - ($sub_ptr_sub_i_i >> 2) << 2) + $2 | 0;
        _memmove($add_ptr_i_i, $0, $sub_ptr_sub_i_i, 4, 0);
        HEAP32[$__begin_$s2] = $add_ptr_i_i;
        HEAP32[$__end_$s2] = ($div << 2) + HEAP32[$__end_$s2] | 0;
        var $17 = $add_ptr_i_i;
        break;
      } else {
        var $mul = $sub_ptr_lhs_cast - $0 >> 1;
        var $_sroa_speculated = ($mul | 0) == 0 ? 1 : $mul;
        __ZNSt3__114__split_bufferIPP5BlockRNS_9allocatorIS3_EEEC1EjjS6_($__t, $_sroa_speculated, ($_sroa_speculated + 3 | 0) >>> 2, HEAP32[$this + 16 >> 2]);
        __ZNSt3__114__split_bufferIPP5BlockRNS_9allocatorIS3_EEE18__construct_at_endINS_13move_iteratorIPS3_EEEENS_9enable_ifIXsr21__is_forward_iteratorIT_EE5valueEvE4typeESD_SD_($__t, HEAP32[$__begin_$s2], HEAP32[$__end_$s2]);
        var $__first_30 = $__t | 0;
        var $9 = HEAP32[$__first_$s2];
        HEAP32[$__first_$s2] = HEAP32[$__first_30 >> 2];
        HEAP32[$__first_30 >> 2] = $9;
        var $__begin_33 = $__t + 4 | 0;
        var $11 = HEAP32[$__begin_$s2];
        HEAP32[$__begin_$s2] = HEAP32[$__begin_33 >> 2];
        HEAP32[$__begin_33 >> 2] = $11;
        var $__end_36 = $__t + 8 | 0;
        var $13 = HEAP32[$__end_$s2];
        HEAP32[$__end_$s2] = HEAP32[$__end_36 >> 2];
        HEAP32[$__end_36 >> 2] = $13;
        var $__first__i_i_i72 = $__t + 12 | 0;
        var $15 = HEAP32[$__first__i_i_i$s2];
        HEAP32[$__first__i_i_i$s2] = HEAP32[$__first__i_i_i72 >> 2];
        HEAP32[$__first__i_i_i72 >> 2] = $15;
        __ZNSt3__114__split_bufferIPP5BlockRNS_9allocatorIS3_EEED1Ev($__t);
        var $17 = HEAP32[$__begin_$s2];
        break;
      }
    } else {
      var $17 = $0;
    }
  } while (0);
  var $17;
  var $add_ptr45 = $17 - 4 | 0;
  if (($add_ptr45 | 0) == 0) {
    var $19 = $17;
    var $19;
    var $incdec_ptr = $19 - 4 | 0;
    HEAP32[$__begin_$s2] = $incdec_ptr;
    STACKTOP = __stackBase__;
    return;
  }
  HEAP32[$add_ptr45 >> 2] = HEAP32[$__x >> 2];
  var $19 = HEAP32[$__begin_$s2];
  var $19;
  var $incdec_ptr = $19 - 4 | 0;
  HEAP32[$__begin_$s2] = $incdec_ptr;
  STACKTOP = __stackBase__;
  return;
}
__ZNSt3__114__split_bufferIPP5BlockRNS_9allocatorIS3_EEE10push_frontERKS3_["X"] = 1;
function __ZNSt3__114__split_bufferIPP5BlockRNS_9allocatorIS3_EEED1Ev($this) {
  __ZNSt3__114__split_bufferIPP5BlockRNS_9allocatorIS3_EEED2Ev($this);
  return;
}
function __ZNSt3__114__split_bufferIPP5BlockRNS_9allocatorIS3_EEED2Ev($this) {
  HEAP32[$this + 8 >> 2] = HEAP32[$this + 4 >> 2];
  var $1 = HEAP32[$this >> 2];
  if (($1 | 0) == 0) {
    return;
  }
  __ZdlPv($1);
  return;
}
function __ZNSt3__114__split_bufferIPP5BlockRNS_9allocatorIS3_EEEC2EjjS6_($this, $__cap, $__start, $__a) {
  var $__first__i_i_i = $this + 12 | 0;
  HEAP32[$__first__i_i_i >> 2] = 0;
  HEAP32[$this + 16 >> 2] = $__a;
  if (($__cap | 0) == 0) {
    var $cond = 0;
  } else {
    var $cond = __Znwj($__cap << 2);
  }
  var $cond;
  HEAP32[$this >> 2] = $cond;
  var $add_ptr = ($__start << 2) + $cond | 0;
  HEAP32[$this + 8 >> 2] = $add_ptr;
  HEAP32[$this + 4 >> 2] = $add_ptr;
  HEAP32[$__first__i_i_i >> 2] = ($__cap << 2) + $cond | 0;
  return;
}
function __ZNSt3__114__split_bufferIPP5ShapeNS_9allocatorIS3_EEED1Ev($this) {
  __ZNSt3__114__split_bufferIPP5ShapeNS_9allocatorIS3_EEED2Ev($this);
  return;
}
function __ZNSt3__114__split_bufferIPP5ShapeNS_9allocatorIS3_EEED2Ev($this) {
  HEAP32[$this + 8 >> 2] = HEAP32[$this + 4 >> 2];
  var $1 = HEAP32[$this >> 2];
  if (($1 | 0) == 0) {
    return;
  }
  __ZdlPv($1);
  return;
}
function __ZNSt3__114__split_bufferIPP5BlockNS_9allocatorIS3_EEED1Ev($this) {
  __ZNSt3__114__split_bufferIPP5BlockNS_9allocatorIS3_EEED2Ev($this);
  return;
}
function __ZNSt3__114__split_bufferIPP5BlockNS_9allocatorIS3_EEED2Ev($this) {
  HEAP32[$this + 8 >> 2] = HEAP32[$this + 4 >> 2];
  var $1 = HEAP32[$this >> 2];
  if (($1 | 0) == 0) {
    return;
  }
  __ZdlPv($1);
  return;
}
function __ZNSt3__16__treeINS_4pairIP5BlockP5ShapeEENS_19__map_value_compareIS3_S5_NS_4lessIS3_EELb1EEENS_9allocatorIS6_EEE4findIS3_EENS_15__tree_iteratorIS6_PNS_11__tree_nodeIS6_PvEEiEERKT_($agg_result, $this, $__v) {
  var __stackBase__ = STACKTOP;
  STACKTOP = STACKTOP + 4 | 0;
  var $__p = __stackBase__;
  var $__first__i_i_i_i = $this + 4 | 0;
  var $0 = $__first__i_i_i_i;
  __ZNSt3__16__treeINS_4pairIP5BlockP5ShapeEENS_19__map_value_compareIS3_S5_NS_4lessIS3_EELb1EEENS_9allocatorIS6_EEE13__lower_boundIS3_EENS_15__tree_iteratorIS6_PNS_11__tree_nodeIS6_PvEEiEERKT_SJ_SJ_($__p, $__v, HEAP32[$__first__i_i_i_i >> 2], $0);
  var $3 = HEAP32[$__p >> 2];
  do {
    if (($3 | 0) != ($0 | 0)) {
      if (HEAP32[$__v >> 2] >>> 0 < HEAP32[$3 + 16 >> 2] >>> 0) {
        break;
      }
      HEAP32[$agg_result >> 2] = $3;
      STACKTOP = __stackBase__;
      return;
    }
  } while (0);
  HEAP32[$agg_result >> 2] = $0;
  STACKTOP = __stackBase__;
  return;
}
function __ZNSt3__15dequeIP5BlockNS_9allocatorIS2_EEE19__add_back_capacityEv($this) {
  var $__begin__i711$s2;
  var $__first__i_i_i_i709$s2;
  var $__end__i704$s2;
  var $__first__i150$s2;
  var $__first__i_i_i_i149$s2;
  var $__begin__i135$s2;
  var $__end__i134$s2;
  var $__first__i$s2;
  var $__first__i_i_i_i$s2;
  var $__end__i127$s2;
  var $__begin__i$s2;
  var __stackBase__ = STACKTOP;
  STACKTOP = STACKTOP + 104 | 0;
  var $__t_i698 = __stackBase__;
  var $__t_i465 = __stackBase__ + 20;
  var $__t_i254 = __stackBase__ + 40;
  var $__t_i125 = __stackBase__ + 60;
  var $ref_tmp21 = __stackBase__ + 80;
  var $__buf = __stackBase__ + 84;
  var $__start__i = $this + 16 | 0;
  var $0 = HEAP32[$__start__i >> 2];
  if ($0 >>> 0 > 1023) {
    HEAP32[$__start__i >> 2] = $0 - 1024 | 0;
    var $__begin__i$s2 = ($this + 4 | 0) >> 2;
    var $1 = HEAP32[$__begin__i$s2];
    var $2 = HEAP32[$1 >> 2];
    var $add_ptr_i = $1 + 4 | 0;
    HEAP32[$__begin__i$s2] = $add_ptr_i;
    var $__end__i127$s2 = ($this + 8 | 0) >> 2;
    var $3 = HEAP32[$__end__i127$s2];
    var $4 = $this + 12 | 0;
    var $__first__i_i_i_i$s2 = ($4 | 0) >> 2;
    do {
      if (($3 | 0) == (HEAP32[$__first__i_i_i_i$s2] | 0)) {
        var $__first__i$s2 = ($this | 0) >> 2;
        var $6 = HEAP32[$__first__i$s2];
        if ($add_ptr_i >>> 0 > $6 >>> 0) {
          var $sub_ptr_lhs_cast_i = $add_ptr_i;
          var $idx_neg_i = ($sub_ptr_lhs_cast_i - $6 + 4 >> 2 | 0) / -2 & -1;
          var $add_ptr_i_sum = $idx_neg_i + 1 | 0;
          var $sub_ptr_sub_i_i_i = $3 - $sub_ptr_lhs_cast_i | 0;
          _memmove(($add_ptr_i_sum << 2) + $1 | 0, $add_ptr_i, $sub_ptr_sub_i_i_i, 4, 0);
          var $add_ptr_i_i_i = (($sub_ptr_sub_i_i_i >> 2) + $add_ptr_i_sum << 2) + $1 | 0;
          HEAP32[$__end__i127$s2] = $add_ptr_i_i_i;
          HEAP32[$__begin__i$s2] = ($idx_neg_i << 2) + HEAP32[$__begin__i$s2] | 0;
          var $21 = $add_ptr_i_i_i;
          break;
        } else {
          var $mul_i = $3 - $6 >> 1;
          var $_sroa_speculated90 = ($mul_i | 0) == 0 ? 1 : $mul_i;
          __ZNSt3__114__split_bufferIPP5BlockRNS_9allocatorIS3_EEEC1EjjS6_($__t_i125, $_sroa_speculated90, $_sroa_speculated90 >>> 2, $4);
          __ZNSt3__114__split_bufferIPP5BlockRNS_9allocatorIS3_EEE18__construct_at_endINS_13move_iteratorIPS3_EEEENS_9enable_ifIXsr21__is_forward_iteratorIT_EE5valueEvE4typeESD_SD_($__t_i125, HEAP32[$__begin__i$s2], HEAP32[$__end__i127$s2]);
          var $__first_30_i = $__t_i125 | 0;
          var $13 = HEAP32[$__first__i$s2];
          HEAP32[$__first__i$s2] = HEAP32[$__first_30_i >> 2];
          HEAP32[$__first_30_i >> 2] = $13;
          var $__begin_33_i = $__t_i125 + 4 | 0;
          var $15 = HEAP32[$__begin__i$s2];
          HEAP32[$__begin__i$s2] = HEAP32[$__begin_33_i >> 2];
          HEAP32[$__begin_33_i >> 2] = $15;
          var $__end_36_i = $__t_i125 + 8 | 0;
          var $17 = HEAP32[$__end__i127$s2];
          HEAP32[$__end__i127$s2] = HEAP32[$__end_36_i >> 2];
          HEAP32[$__end_36_i >> 2] = $17;
          var $__first__i_i_i71_i = $__t_i125 + 12 | 0;
          var $19 = HEAP32[$__first__i_i_i_i$s2];
          HEAP32[$__first__i_i_i_i$s2] = HEAP32[$__first__i_i_i71_i >> 2];
          HEAP32[$__first__i_i_i71_i >> 2] = $19;
          __ZNSt3__114__split_bufferIPP5BlockRNS_9allocatorIS3_EEED1Ev($__t_i125);
          var $21 = HEAP32[$__end__i127$s2];
          break;
        }
      } else {
        var $21 = $3;
      }
    } while (0);
    var $21;
    if (($21 | 0) == 0) {
      var $22 = 0;
    } else {
      HEAP32[$21 >> 2] = $2;
      var $22 = HEAP32[$__end__i127$s2];
    }
    var $22;
    HEAP32[$__end__i127$s2] = $22 + 4 | 0;
    STACKTOP = __stackBase__;
    return;
  }
  var $__map_6 = $this | 0;
  var $__end__i134$s2 = ($this + 8 | 0) >> 2;
  var $23 = HEAP32[$__end__i134$s2];
  var $__begin__i135$s2 = ($this + 4 | 0) >> 2;
  var $sub_ptr_div_i139 = $23 - HEAP32[$__begin__i135$s2] >> 2;
  var $25 = $this + 12 | 0;
  var $__first__i_i_i_i149$s2 = ($25 | 0) >> 2;
  var $26 = HEAP32[$__first__i_i_i_i149$s2];
  var $__first__i150$s2 = ($this | 0) >> 2;
  var $sub_ptr_sub_i153 = $26 - HEAP32[$__first__i150$s2] | 0;
  if ($sub_ptr_div_i139 >>> 0 >= $sub_ptr_sub_i153 >> 2 >>> 0) {
    var $mul = $sub_ptr_sub_i153 >> 1;
    __ZNSt3__114__split_bufferIPP5BlockRNS_9allocatorIS3_EEEC1EjjS6_($__buf, ($mul | 0) == 0 ? 1 : $mul, $sub_ptr_div_i139, $25);
    var $71 = __Znwj(4096);
    var $__end__i704$s2 = ($__buf + 8 | 0) >> 2;
    var $72 = HEAP32[$__end__i704$s2];
    var $__first__i_i_i_i709$s2 = ($__buf + 12 | 0) >> 2;
    do {
      if (($72 | 0) == (HEAP32[$__first__i_i_i_i709$s2] | 0)) {
        var $__begin__i711$s2 = ($__buf + 4 | 0) >> 2;
        var $74 = HEAP32[$__begin__i711$s2];
        var $__first__i712 = $__buf | 0;
        var $75 = HEAP32[$__first__i712 >> 2];
        if ($74 >>> 0 > $75 >>> 0) {
          var $sub_ptr_lhs_cast_i717 = $74;
          var $idx_neg_i726 = ($sub_ptr_lhs_cast_i717 - $75 + 4 >> 2 | 0) / -2 & -1;
          var $add_ptr_i727 = ($idx_neg_i726 << 2) + $74 | 0;
          var $sub_ptr_sub_i_i_i730 = $72 - $sub_ptr_lhs_cast_i717 | 0;
          _memmove($add_ptr_i727, $74, $sub_ptr_sub_i_i_i730, 4, 0);
          var $add_ptr_i_i_i733 = (($sub_ptr_sub_i_i_i730 >> 2) + $idx_neg_i726 << 2) + $74 | 0;
          HEAP32[$__end__i704$s2] = $add_ptr_i_i_i733;
          HEAP32[$__begin__i711$s2] = $add_ptr_i727;
          var $_pre102 = $add_ptr_i_i_i733;
          break;
        }
        var $mul_i749 = $72 - $75 >> 1;
        var $_sroa_speculated = ($mul_i749 | 0) == 0 ? 1 : $mul_i749;
        __ZNSt3__114__split_bufferIPP5BlockRNS_9allocatorIS3_EEEC1EjjS6_($__t_i698, $_sroa_speculated, $_sroa_speculated >>> 2, HEAP32[$__buf + 16 >> 2]);
        __ZNSt3__114__split_bufferIPP5BlockRNS_9allocatorIS3_EEE18__construct_at_endINS_13move_iteratorIPS3_EEEENS_9enable_ifIXsr21__is_forward_iteratorIT_EE5valueEvE4typeESD_SD_($__t_i698, $74, $72);
        var $__first_30_i771 = $__t_i698 | 0;
        HEAP32[$__first__i712 >> 2] = HEAP32[$__first_30_i771 >> 2];
        HEAP32[$__first_30_i771 >> 2] = $75;
        var $__begin_33_i774 = $__t_i698 + 4 | 0;
        HEAP32[$__begin__i711$s2] = HEAP32[$__begin_33_i774 >> 2];
        HEAP32[$__begin_33_i774 >> 2] = $74;
        var $__end_36_i776 = $__t_i698 + 8 | 0;
        var $80 = HEAP32[$__end_36_i776 >> 2];
        HEAP32[$__end__i704$s2] = $80;
        HEAP32[$__end_36_i776 >> 2] = $72;
        var $__first__i_i_i71_i786 = $__t_i698 + 12 | 0;
        HEAP32[$__first__i_i_i_i709$s2] = HEAP32[$__first__i_i_i71_i786 >> 2];
        HEAP32[$__first__i_i_i71_i786 >> 2] = $72;
        __ZNSt3__114__split_bufferIPP5BlockRNS_9allocatorIS3_EEED1Ev($__t_i698);
        var $_pre102 = $80;
      } else {
        var $_pre102 = $72;
      }
    } while (0);
    var $_pre102;
    if (($_pre102 | 0) == 0) {
      var $82 = 0;
    } else {
      HEAP32[$_pre102 >> 2] = $71;
      var $82 = $_pre102;
    }
    var $82;
    HEAP32[$__end__i704$s2] = $82 + 4 | 0;
    var $__i_0 = HEAP32[$__end__i134$s2];
    while (1) {
      var $__i_0;
      if (($__i_0 | 0) == (HEAP32[$__begin__i135$s2] | 0)) {
        break;
      }
      var $incdec_ptr = $__i_0 - 4 | 0;
      __ZNSt3__114__split_bufferIPP5BlockRNS_9allocatorIS3_EEE10push_frontERKS3_($__buf, $incdec_ptr);
      var $__i_0 = $incdec_ptr;
    }
    var $__first_52 = $__buf | 0;
    var $97 = HEAP32[$__first__i150$s2];
    HEAP32[$__first__i150$s2] = HEAP32[$__first_52 >> 2];
    HEAP32[$__first_52 >> 2] = $97;
    var $__begin_55 = $__buf + 4 | 0;
    HEAP32[$__begin__i135$s2] = HEAP32[$__begin_55 >> 2];
    HEAP32[$__begin_55 >> 2] = $__i_0;
    var $100 = HEAP32[$__end__i134$s2];
    HEAP32[$__end__i134$s2] = HEAP32[$__end__i704$s2];
    HEAP32[$__end__i704$s2] = $100;
    var $102 = HEAP32[$__first__i_i_i_i149$s2];
    HEAP32[$__first__i_i_i_i149$s2] = HEAP32[$__first__i_i_i_i709$s2];
    HEAP32[$__first__i_i_i_i709$s2] = $102;
    __ZNSt3__114__split_bufferIPP5BlockRNS_9allocatorIS3_EEED1Ev($__buf);
    STACKTOP = __stackBase__;
    return;
  }
  var $28 = __Znwj(4096);
  if (($26 | 0) == ($23 | 0)) {
    HEAP32[$ref_tmp21 >> 2] = $28;
    __ZNSt3__114__split_bufferIPP5BlockNS_9allocatorIS3_EEE10push_frontERKS3_($__map_6, $ref_tmp21);
    var $49 = HEAP32[$__begin__i135$s2];
    var $50 = HEAP32[$49 >> 2];
    var $add_ptr_i382 = $49 + 4 | 0;
    HEAP32[$__begin__i135$s2] = $add_ptr_i382;
    var $51 = HEAP32[$__end__i134$s2];
    do {
      if (($51 | 0) == (HEAP32[$__first__i_i_i_i149$s2] | 0)) {
        var $53 = HEAP32[$__first__i150$s2];
        if ($add_ptr_i382 >>> 0 > $53 >>> 0) {
          var $sub_ptr_lhs_cast_i484 = $add_ptr_i382;
          var $idx_neg_i493 = ($sub_ptr_lhs_cast_i484 - $53 + 4 >> 2 | 0) / -2 & -1;
          var $add_ptr_i382_sum = $idx_neg_i493 + 1 | 0;
          var $sub_ptr_sub_i_i_i497 = $51 - $sub_ptr_lhs_cast_i484 | 0;
          _memmove(($add_ptr_i382_sum << 2) + $49 | 0, $add_ptr_i382, $sub_ptr_sub_i_i_i497, 4, 0);
          var $add_ptr_i_i_i500 = (($sub_ptr_sub_i_i_i497 >> 2) + $add_ptr_i382_sum << 2) + $49 | 0;
          HEAP32[$__end__i134$s2] = $add_ptr_i_i_i500;
          HEAP32[$__begin__i135$s2] = ($idx_neg_i493 << 2) + HEAP32[$__begin__i135$s2] | 0;
          var $68 = $add_ptr_i_i_i500;
          break;
        } else {
          var $mul_i516 = $51 - $53 >> 1;
          var $_sroa_speculated84 = ($mul_i516 | 0) == 0 ? 1 : $mul_i516;
          __ZNSt3__114__split_bufferIPP5BlockRNS_9allocatorIS3_EEEC1EjjS6_($__t_i465, $_sroa_speculated84, $_sroa_speculated84 >>> 2, $25);
          __ZNSt3__114__split_bufferIPP5BlockRNS_9allocatorIS3_EEE18__construct_at_endINS_13move_iteratorIPS3_EEEENS_9enable_ifIXsr21__is_forward_iteratorIT_EE5valueEvE4typeESD_SD_($__t_i465, HEAP32[$__begin__i135$s2], HEAP32[$__end__i134$s2]);
          var $__first_30_i538 = $__t_i465 | 0;
          var $60 = HEAP32[$__first__i150$s2];
          HEAP32[$__first__i150$s2] = HEAP32[$__first_30_i538 >> 2];
          HEAP32[$__first_30_i538 >> 2] = $60;
          var $__begin_33_i541 = $__t_i465 + 4 | 0;
          var $62 = HEAP32[$__begin__i135$s2];
          HEAP32[$__begin__i135$s2] = HEAP32[$__begin_33_i541 >> 2];
          HEAP32[$__begin_33_i541 >> 2] = $62;
          var $__end_36_i543 = $__t_i465 + 8 | 0;
          var $64 = HEAP32[$__end__i134$s2];
          HEAP32[$__end__i134$s2] = HEAP32[$__end_36_i543 >> 2];
          HEAP32[$__end_36_i543 >> 2] = $64;
          var $__first__i_i_i71_i553 = $__t_i465 + 12 | 0;
          var $66 = HEAP32[$__first__i_i_i_i149$s2];
          HEAP32[$__first__i_i_i_i149$s2] = HEAP32[$__first__i_i_i71_i553 >> 2];
          HEAP32[$__first__i_i_i71_i553 >> 2] = $66;
          __ZNSt3__114__split_bufferIPP5BlockRNS_9allocatorIS3_EEED1Ev($__t_i465);
          var $68 = HEAP32[$__end__i134$s2];
          break;
        }
      } else {
        var $68 = $51;
      }
    } while (0);
    var $68;
    if (($68 | 0) == 0) {
      var $69 = 0;
    } else {
      HEAP32[$68 >> 2] = $50;
      var $69 = HEAP32[$__end__i134$s2];
    }
    var $69;
    HEAP32[$__end__i134$s2] = $69 + 4 | 0;
    STACKTOP = __stackBase__;
    return;
  } else {
    var $29 = HEAP32[$__end__i134$s2];
    do {
      if (($29 | 0) == (HEAP32[$__first__i_i_i_i149$s2] | 0)) {
        var $31 = HEAP32[$__begin__i135$s2];
        var $32 = HEAP32[$__first__i150$s2];
        if ($31 >>> 0 > $32 >>> 0) {
          var $sub_ptr_lhs_cast_i273 = $31;
          var $idx_neg_i282 = ($sub_ptr_lhs_cast_i273 - $32 + 4 >> 2 | 0) / -2 & -1;
          var $sub_ptr_sub_i_i_i286 = $29 - $sub_ptr_lhs_cast_i273 | 0;
          _memmove(($idx_neg_i282 << 2) + $31 | 0, $31, $sub_ptr_sub_i_i_i286, 4, 0);
          var $add_ptr_i_i_i289 = (($sub_ptr_sub_i_i_i286 >> 2) + $idx_neg_i282 << 2) + $31 | 0;
          HEAP32[$__end__i134$s2] = $add_ptr_i_i_i289;
          HEAP32[$__begin__i135$s2] = ($idx_neg_i282 << 2) + HEAP32[$__begin__i135$s2] | 0;
          var $47 = $add_ptr_i_i_i289;
          break;
        } else {
          var $mul_i305 = $29 - $32 >> 1;
          var $_sroa_speculated87 = ($mul_i305 | 0) == 0 ? 1 : $mul_i305;
          __ZNSt3__114__split_bufferIPP5BlockRNS_9allocatorIS3_EEEC1EjjS6_($__t_i254, $_sroa_speculated87, $_sroa_speculated87 >>> 2, $25);
          __ZNSt3__114__split_bufferIPP5BlockRNS_9allocatorIS3_EEE18__construct_at_endINS_13move_iteratorIPS3_EEEENS_9enable_ifIXsr21__is_forward_iteratorIT_EE5valueEvE4typeESD_SD_($__t_i254, HEAP32[$__begin__i135$s2], HEAP32[$__end__i134$s2]);
          var $__first_30_i327 = $__t_i254 | 0;
          var $39 = HEAP32[$__first__i150$s2];
          HEAP32[$__first__i150$s2] = HEAP32[$__first_30_i327 >> 2];
          HEAP32[$__first_30_i327 >> 2] = $39;
          var $__begin_33_i330 = $__t_i254 + 4 | 0;
          var $41 = HEAP32[$__begin__i135$s2];
          HEAP32[$__begin__i135$s2] = HEAP32[$__begin_33_i330 >> 2];
          HEAP32[$__begin_33_i330 >> 2] = $41;
          var $__end_36_i332 = $__t_i254 + 8 | 0;
          var $43 = HEAP32[$__end__i134$s2];
          HEAP32[$__end__i134$s2] = HEAP32[$__end_36_i332 >> 2];
          HEAP32[$__end_36_i332 >> 2] = $43;
          var $__first__i_i_i71_i342 = $__t_i254 + 12 | 0;
          var $45 = HEAP32[$__first__i_i_i_i149$s2];
          HEAP32[$__first__i_i_i_i149$s2] = HEAP32[$__first__i_i_i71_i342 >> 2];
          HEAP32[$__first__i_i_i71_i342 >> 2] = $45;
          __ZNSt3__114__split_bufferIPP5BlockRNS_9allocatorIS3_EEED1Ev($__t_i254);
          var $47 = HEAP32[$__end__i134$s2];
          break;
        }
      } else {
        var $47 = $29;
      }
    } while (0);
    var $47;
    if (($47 | 0) == 0) {
      var $48 = 0;
    } else {
      HEAP32[$47 >> 2] = $28;
      var $48 = HEAP32[$__end__i134$s2];
    }
    var $48;
    HEAP32[$__end__i134$s2] = $48 + 4 | 0;
    STACKTOP = __stackBase__;
    return;
  }
}
__ZNSt3__15dequeIP5BlockNS_9allocatorIS2_EEE19__add_back_capacityEv["X"] = 1;
function __ZNSt3__13mapIP5BlockP6BranchNS_4lessIS2_EENS_9allocatorINS_4pairIKS2_S4_EEEEE16__find_equal_keyERPNS_16__tree_node_baseIPvEERS9_($this, $__parent, $__k) {
  var label = 0;
  var $__first__i_i_i_i = $this + 4 | 0;
  var $__left__i = $__first__i_i_i_i | 0;
  var $0 = HEAP32[$__left__i >> 2];
  if (($0 | 0) == 0) {
    HEAP32[$__parent >> 2] = $__first__i_i_i_i;
    var $retval_0 = $__left__i;
    var $retval_0;
    return $retval_0;
  }
  var $1 = HEAP32[$__k >> 2];
  var $__nd_0_in = $0;
  while (1) {
    var $__nd_0_in;
    var $2 = HEAP32[$__nd_0_in + 16 >> 2];
    if ($1 >>> 0 < $2 >>> 0) {
      var $__left_ = $__nd_0_in | 0;
      var $3 = HEAP32[$__left_ >> 2];
      if (($3 | 0) == 0) {
        label = 566;
        break;
      } else {
        var $__nd_0_in = $3;
        continue;
      }
    }
    if ($2 >>> 0 >= $1 >>> 0) {
      label = 570;
      break;
    }
    var $__right_ = $__nd_0_in + 4 | 0;
    var $4 = HEAP32[$__right_ >> 2];
    if (($4 | 0) == 0) {
      label = 569;
      break;
    } else {
      var $__nd_0_in = $4;
    }
  }
  if (label == 566) {
    HEAP32[$__parent >> 2] = $__nd_0_in;
    var $retval_0 = $__left_;
    var $retval_0;
    return $retval_0;
  } else if (label == 570) {
    HEAP32[$__parent >> 2] = $__nd_0_in;
    var $retval_0 = $__parent;
    var $retval_0;
    return $retval_0;
  } else if (label == 569) {
    HEAP32[$__parent >> 2] = $__nd_0_in;
    var $retval_0 = $__right_;
    var $retval_0;
    return $retval_0;
  }
}
function __ZNSt3__16__treeINS_4pairIP5BlockP6BranchEENS_19__map_value_compareIS3_S5_NS_4lessIS3_EELb1EEENS_9allocatorIS6_EEE13__lower_boundIS3_EENS_15__tree_iteratorIS6_PNS_11__tree_nodeIS6_PvEEiEERKT_SJ_SJ_($agg_result, $__v, $__root, $__result) {
  var $__root_addr_06$s2;
  var $cmp59 = ($__root | 0) == 0;
  L697 : do {
    if ($cmp59) {
      var $__result_addr_0_ph8 = $__result;
    } else {
      var $0 = HEAP32[$__v >> 2];
      var $__root_addr_0_ph10 = $__root;
      var $__result_addr_0_ph11 = $__result;
      while (1) {
        var $__result_addr_0_ph11;
        var $__root_addr_0_ph10;
        var $__root_addr_06 = $__root_addr_0_ph10, $__root_addr_06$s2 = $__root_addr_06 >> 2;
        while (1) {
          var $__root_addr_06;
          if (HEAP32[$__root_addr_06$s2 + 4] >>> 0 >= $0 >>> 0) {
            break;
          }
          var $4 = HEAP32[$__root_addr_06$s2 + 1];
          if (($4 | 0) == 0) {
            var $__result_addr_0_ph8 = $__result_addr_0_ph11;
            break L697;
          } else {
            var $__root_addr_06 = $4, $__root_addr_06$s2 = $__root_addr_06 >> 2;
          }
        }
        var $2 = HEAP32[$__root_addr_06$s2];
        if (($2 | 0) == 0) {
          var $__result_addr_0_ph8 = $__root_addr_06;
          break L697;
        } else {
          var $__root_addr_0_ph10 = $2;
          var $__result_addr_0_ph11 = $__root_addr_06;
        }
      }
    }
  } while (0);
  var $__result_addr_0_ph8;
  HEAP32[$agg_result >> 2] = $__result_addr_0_ph8;
  return;
}
function __ZNSt3__16__treeINS_4pairIP5BlockP6BranchEENS_19__map_value_compareIS3_S5_NS_4lessIS3_EELb1EEENS_9allocatorIS6_EEEC2ERKSA_($this) {
  var $__first__i_i_i = $this + 4 | 0;
  HEAP32[$__first__i_i_i >> 2] = 0;
  HEAP32[$this + 8 >> 2] = 0;
  HEAP32[$this >> 2] = $__first__i_i_i;
  return;
}
function __ZN5Shape8IsSimpleEPS_($It) {
  do {
    if (($It | 0) == 0) {
      var $cond = 0;
    } else {
      if ((HEAP32[$It + 12 >> 2] | 0) != 0) {
        var $cond = 0;
        break;
      }
      var $cond = $It;
    }
  } while (0);
  var $cond;
  return $cond;
}
function __ZN5Shape6IsLoopEPS_($It) {
  do {
    if (($It | 0) == 0) {
      var $cond = 0;
    } else {
      if ((HEAP32[$It + 12 >> 2] | 0) != 2) {
        var $cond = 0;
        break;
      }
      var $cond = $It;
    }
  } while (0);
  var $cond;
  return $cond;
}
function __ZNSt3__13mapIP5BlockP6BranchNS_4lessIS2_EENS_9allocatorINS_4pairIKS2_S4_EEEEE16__construct_nodeERS9_($agg_result, $this, $__k) {
  var $call_i_i = __Znwj(24);
  var $__value_ = $call_i_i + 16 | 0;
  if (($__value_ | 0) != 0) {
    HEAP32[$__value_ >> 2] = HEAP32[$__k >> 2];
  }
  var $second = $call_i_i + 20 | 0;
  if (($second | 0) != 0) {
    HEAP32[$second >> 2] = 0;
  }
  HEAP32[$agg_result >> 2] = $call_i_i;
  HEAP32[$agg_result + 4 >> 2] = $this + 4 | 0;
  HEAP8[$agg_result + 8 | 0] = 1;
  HEAP8[$agg_result + 9 | 0] = 1;
  return;
}
function __ZNSt3__16__treeINS_4pairIP5BlockP6BranchEENS_19__map_value_compareIS3_S5_NS_4lessIS3_EELb1EEENS_9allocatorIS6_EEE16__insert_node_atEPNS_16__tree_node_baseIPvEERSH_SH_($this, $__parent, $__child, $__new_node) {
  HEAP32[$__new_node >> 2] = 0;
  HEAP32[$__new_node + 4 >> 2] = 0;
  HEAP32[$__new_node + 8 >> 2] = $__parent;
  HEAP32[$__child >> 2] = $__new_node;
  var $__begin_node__i42 = $this | 0;
  var $1 = HEAP32[HEAP32[$__begin_node__i42 >> 2] >> 2];
  if (($1 | 0) == 0) {
    var $3 = $__new_node;
  } else {
    HEAP32[$__begin_node__i42 >> 2] = $1;
    var $3 = HEAP32[$__child >> 2];
  }
  var $3;
  __ZNSt3__127__tree_balance_after_insertIPNS_16__tree_node_baseIPvEEEEvT_S5_(HEAP32[$this + 4 >> 2], $3);
  var $__first__i_i_i = $this + 8 | 0;
  HEAP32[$__first__i_i_i >> 2] = HEAP32[$__first__i_i_i >> 2] + 1 | 0;
  return;
}
function __ZNSt3__16__treeINS_4pairIP5BlockP6BranchEENS_19__map_value_compareIS3_S5_NS_4lessIS3_EELb1EEENS_9allocatorIS6_EEE4findIS3_EENS_15__tree_iteratorIS6_PNS_11__tree_nodeIS6_PvEEiEERKT_($agg_result, $this, $__v) {
  var __stackBase__ = STACKTOP;
  STACKTOP = STACKTOP + 4 | 0;
  var $__p = __stackBase__;
  var $__first__i_i_i_i = $this + 4 | 0;
  var $0 = $__first__i_i_i_i;
  __ZNSt3__16__treeINS_4pairIP5BlockP6BranchEENS_19__map_value_compareIS3_S5_NS_4lessIS3_EELb1EEENS_9allocatorIS6_EEE13__lower_boundIS3_EENS_15__tree_iteratorIS6_PNS_11__tree_nodeIS6_PvEEiEERKT_SJ_SJ_($__p, $__v, HEAP32[$__first__i_i_i_i >> 2], $0);
  var $3 = HEAP32[$__p >> 2];
  do {
    if (($3 | 0) != ($0 | 0)) {
      if (HEAP32[$__v >> 2] >>> 0 < HEAP32[$3 + 16 >> 2] >>> 0) {
        break;
      }
      HEAP32[$agg_result >> 2] = $3;
      STACKTOP = __stackBase__;
      return;
    }
  } while (0);
  HEAP32[$agg_result >> 2] = $0;
  STACKTOP = __stackBase__;
  return;
}
function __ZNSt3__16__treeINS_4pairIP5BlockP6BranchEENS_19__map_value_compareIS3_S5_NS_4lessIS3_EELb1EEENS_9allocatorIS6_EEEC1ERKSA_($this) {
  __ZNSt3__16__treeINS_4pairIP5BlockP6BranchEENS_19__map_value_compareIS3_S5_NS_4lessIS3_EELb1EEENS_9allocatorIS6_EEEC2ERKSA_($this);
  return;
}
function __ZN13MultipleShapeD2Ev($this) {
  HEAP32[$this >> 2] = 5244400;
  __ZNSt3__13mapIP5BlockP5ShapeNS_4lessIS2_EENS_9allocatorINS_4pairIKS2_S4_EEEEED1Ev($this + 20 | 0);
  return;
}
function __ZNSt3__13mapIP5BlockP5ShapeNS_4lessIS2_EENS_9allocatorINS_4pairIKS2_S4_EEEEED1Ev($this) {
  __ZNSt3__13mapIP5BlockP5ShapeNS_4lessIS2_EENS_9allocatorINS_4pairIKS2_S4_EEEEED2Ev($this);
  return;
}
function __ZNSt3__13mapIP5BlockP5ShapeNS_4lessIS2_EENS_9allocatorINS_4pairIKS2_S4_EEEEED2Ev($this) {
  __ZNSt3__16__treeINS_4pairIP5BlockP5ShapeEENS_19__map_value_compareIS3_S5_NS_4lessIS3_EELb1EEENS_9allocatorIS6_EEED1Ev(HEAP32[$this + 4 >> 2]);
  return;
}
function __ZNSt3__16__treeINS_4pairIP5BlockP5ShapeEENS_19__map_value_compareIS3_S5_NS_4lessIS3_EELb1EEENS_9allocatorIS6_EEED1Ev($this_0_1_0_0_0_val) {
  __ZNSt3__16__treeINS_4pairIP5BlockP5ShapeEENS_19__map_value_compareIS3_S5_NS_4lessIS3_EELb1EEENS_9allocatorIS6_EEED2Ev($this_0_1_0_0_0_val);
  return;
}
function __ZNSt3__16__treeINS_4pairIP5BlockP5ShapeEENS_19__map_value_compareIS3_S5_NS_4lessIS3_EELb1EEENS_9allocatorIS6_EEED2Ev($this_0_1_0_0_0_val) {
  __ZNSt3__16__treeINS_4pairIP5BlockP5ShapeEENS_19__map_value_compareIS3_S5_NS_4lessIS3_EELb1EEENS_9allocatorIS6_EEE7destroyEPNS_11__tree_nodeIS6_PvEE($this_0_1_0_0_0_val);
  return;
}
function __ZNSt3__16__treeINS_4pairIP5BlockP5ShapeEENS_19__map_value_compareIS3_S5_NS_4lessIS3_EELb1EEENS_9allocatorIS6_EEE7destroyEPNS_11__tree_nodeIS6_PvEE($__nd) {
  if (($__nd | 0) == 0) {
    return;
  } else {
    __ZNSt3__16__treeINS_4pairIP5BlockP5ShapeEENS_19__map_value_compareIS3_S5_NS_4lessIS3_EELb1EEENS_9allocatorIS6_EEE7destroyEPNS_11__tree_nodeIS6_PvEE(HEAP32[$__nd >> 2]);
    __ZNSt3__16__treeINS_4pairIP5BlockP5ShapeEENS_19__map_value_compareIS3_S5_NS_4lessIS3_EELb1EEENS_9allocatorIS6_EEE7destroyEPNS_11__tree_nodeIS6_PvEE(HEAP32[$__nd + 4 >> 2]);
    __ZdlPv($__nd);
    return;
  }
}
function __ZNSt3__13mapIPviNS_4lessIS1_EENS_9allocatorINS_4pairIKS1_iEEEEED2Ev($this) {
  __ZNSt3__16__treeINS_4pairIPviEENS_19__map_value_compareIS2_iNS_4lessIS2_EELb1EEENS_9allocatorIS3_EEED1Ev(HEAP32[$this + 4 >> 2]);
  return;
}
function __ZNSt3__16__treeINS_4pairIPviEENS_19__map_value_compareIS2_iNS_4lessIS2_EELb1EEENS_9allocatorIS3_EEED1Ev($this_0_1_0_0_0_val) {
  __ZNSt3__16__treeINS_4pairIPviEENS_19__map_value_compareIS2_iNS_4lessIS2_EELb1EEENS_9allocatorIS3_EEED2Ev($this_0_1_0_0_0_val);
  return;
}
function __ZNSt3__16__treeINS_4pairIPviEENS_19__map_value_compareIS2_iNS_4lessIS2_EELb1EEENS_9allocatorIS3_EEED2Ev($this_0_1_0_0_0_val) {
  __ZNSt3__16__treeINS_4pairIPviEENS_19__map_value_compareIS2_iNS_4lessIS2_EELb1EEENS_9allocatorIS3_EEE7destroyEPNS_11__tree_nodeIS3_S2_EE($this_0_1_0_0_0_val);
  return;
}
function __ZNSt3__16__treeINS_4pairIPviEENS_19__map_value_compareIS2_iNS_4lessIS2_EELb1EEENS_9allocatorIS3_EEE7destroyEPNS_11__tree_nodeIS3_S2_EE($__nd) {
  if (($__nd | 0) == 0) {
    return;
  } else {
    __ZNSt3__16__treeINS_4pairIPviEENS_19__map_value_compareIS2_iNS_4lessIS2_EELb1EEENS_9allocatorIS3_EEE7destroyEPNS_11__tree_nodeIS3_S2_EE(HEAP32[$__nd >> 2]);
    __ZNSt3__16__treeINS_4pairIPviEENS_19__map_value_compareIS2_iNS_4lessIS2_EELb1EEENS_9allocatorIS3_EEE7destroyEPNS_11__tree_nodeIS3_S2_EE(HEAP32[$__nd + 4 >> 2]);
    __ZdlPv($__nd);
    return;
  }
}
function __ZZN8Relooper9CalculateEP5BlockEN12PreOptimizerD2Ev($this) {
  __ZNSt3__13setIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEED1Ev($this + 4 | 0);
  return;
}
function __ZNSt3__13setIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEED2Ev($this) {
  __ZNSt3__16__treeIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEED1Ev(HEAP32[$this + 4 >> 2]);
  return;
}
function __ZNSt3__16__treeIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEED1Ev($this_0_1_0_0_0_val) {
  __ZNSt3__16__treeIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEED2Ev($this_0_1_0_0_0_val);
  return;
}
function __ZNSt3__16__treeIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEED2Ev($this_0_1_0_0_0_val) {
  __ZNSt3__16__treeIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEE7destroyEPNS_11__tree_nodeIS2_PvEE($this_0_1_0_0_0_val);
  return;
}
function __ZNSt3__16__treeIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEE7destroyEPNS_11__tree_nodeIS2_PvEE($__nd) {
  if (($__nd | 0) == 0) {
    return;
  } else {
    __ZNSt3__16__treeIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEE7destroyEPNS_11__tree_nodeIS2_PvEE(HEAP32[$__nd >> 2]);
    __ZNSt3__16__treeIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEE7destroyEPNS_11__tree_nodeIS2_PvEE(HEAP32[$__nd + 4 >> 2]);
    __ZdlPv($__nd);
    return;
  }
}
function __ZZN8Relooper9CalculateEP5BlockEN13PostOptimizer19RemoveUnneededFlowsE_1P5ShapeS4_($Root, $Natural) {
  var label = 0;
  var $Root_tr = $Root;
  while (1) {
    var $Root_tr;
    var $call = __ZN5Shape8IsSimpleEPS_($Root_tr);
    if (($call | 0) != 0) {
      var $0 = HEAP32[$call + 8 >> 2];
      if (($0 | 0) == 0) {
        break;
      } else {
        var $Root_tr = $0;
        continue;
      }
    }
    var $call21 = __ZN5Shape10IsMultipleEPS_($Root_tr);
    if (($call21 | 0) == 0) {
      var $call40 = __ZN5Shape6IsLoopEPS_($Root_tr);
      if (($call40 | 0) == 0) {
        label = 649;
        break;
      }
      var $24 = HEAP32[$call40 + 20 >> 2];
      __ZZN8Relooper9CalculateEP5BlockEN13PostOptimizer19RemoveUnneededFlowsE_1P5ShapeS4_($24, $24);
      var $Root_tr = HEAP32[$call40 + 8 >> 2];
      continue;
    }
    var $15 = HEAP32[$call21 + 20 >> 2];
    var $__first__i_i_i_i_i153 = $call21 + 24 | 0;
    var $lnot_i_i1824 = ($15 | 0) == ($__first__i_i_i_i_i153 | 0);
    var $Next37_pre = $call21 + 8 | 0;
    L770 : do {
      if (!$lnot_i_i1824) {
        var $17 = $__first__i_i_i_i_i153;
        var $iter25_sroa_0_05 = $15;
        while (1) {
          var $iter25_sroa_0_05;
          __ZZN8Relooper9CalculateEP5BlockEN13PostOptimizer19RemoveUnneededFlowsE_1P5ShapeS4_(HEAP32[$iter25_sroa_0_05 + 20 >> 2], HEAP32[$Next37_pre >> 2]);
          var $call_i_i_i128 = __ZNSt3__111__tree_nextIPNS_16__tree_node_baseIPvEEEET_S5_($iter25_sroa_0_05 | 0);
          if (($call_i_i_i128 | 0) == ($17 | 0)) {
            break L770;
          } else {
            var $iter25_sroa_0_05 = $call_i_i_i128;
          }
        }
      }
    } while (0);
    var $Root_tr = HEAP32[$Next37_pre >> 2];
  }
  if (label == 649) {
    return;
  }
  var $Inner = $call + 16 | 0;
  var $1 = HEAP32[$Inner >> 2];
  var $2 = HEAP32[$1 + 24 >> 2];
  if (($2 | 0) == ($1 + 28 | 0)) {
    return;
  } else {
    var $iter_sroa_0_03 = $2;
  }
  while (1) {
    var $iter_sroa_0_03;
    var $5 = HEAP32[$iter_sroa_0_03 + 20 >> 2];
    var $Type = $5 + 4 | 0;
    do {
      if ((HEAP32[$Type >> 2] | 0) != 0) {
        if ((HEAP32[HEAP32[$iter_sroa_0_03 + 16 >> 2] + 48 >> 2] | 0) != ($Natural | 0)) {
          break;
        }
        HEAP32[$Type >> 2] = 0;
        var $call13 = __ZN5Shape10IsMultipleEPS_(HEAP32[$5 >> 2]);
        if (($call13 | 0) == 0) {
          break;
        }
        var $NeedLoop = $call13 + 32 | 0;
        HEAP32[$NeedLoop >> 2] = HEAP32[$NeedLoop >> 2] - 1 | 0;
      }
    } while (0);
    var $call_i_i_i = __ZNSt3__111__tree_nextIPNS_16__tree_node_baseIPvEEEET_S5_($iter_sroa_0_03 | 0);
    if (($call_i_i_i | 0) == (HEAP32[$Inner >> 2] + 28 | 0)) {
      break;
    } else {
      var $iter_sroa_0_03 = $call_i_i_i;
    }
  }
  return;
}
__ZZN8Relooper9CalculateEP5BlockEN13PostOptimizer19RemoveUnneededFlowsE_1P5ShapeS4_["X"] = 1;
function __ZZN8Relooper9CalculateEP5BlockEN13PostOptimizer16FindLabeledLoopsE_1P5Shape($this, $Root) {
  var $call7$s2;
  var $call$s2;
  var $Closure$s2;
  var __stackBase__ = STACKTOP;
  STACKTOP = STACKTOP + 12 | 0;
  var $ref_tmp = __stackBase__;
  var $ref_tmp72 = __stackBase__ + 4;
  var $ref_tmp101 = __stackBase__ + 8;
  var $Closure$s2 = ($this + 4 | 0) >> 2;
  var $0 = HEAP32[$Closure$s2];
  var $cmp = ($0 | 0) == 0;
  if ($cmp) {
    var $call = __Znwj(24), $call$s2 = $call >> 2;
    HEAP32[$call$s2] = 0;
    HEAP32[$call$s2 + 1] = 0;
    HEAP32[$call$s2 + 2] = 0;
    HEAP32[$call$s2 + 3] = 0;
    HEAP32[$call$s2 + 4] = 0;
    HEAP32[$call$s2 + 5] = 0;
    HEAP32[$Closure$s2] = $call;
    var $1 = $call;
  } else {
    var $1 = $0;
  }
  var $1;
  var $call4 = __ZN5Shape8IsSimpleEPS_($Root);
  var $tobool5 = ($call4 | 0) == 0;
  L792 : do {
    if ($tobool5) {
      var $call66 = __ZN5Shape10IsMultipleEPS_($Root);
      if (($call66 | 0) == 0) {
        var $call98 = __ZN5Shape6IsLoopEPS_($Root);
        if (($call98 | 0) == 0) {
          break;
        }
        HEAP32[$ref_tmp101 >> 2] = $call98 | 0;
        var $c_i396 = $1;
        __ZNSt3__15dequeIP5ShapeNS_9allocatorIS2_EEE9push_backERKS2_($c_i396, $ref_tmp101);
        __ZZN8Relooper9CalculateEP5BlockEN13PostOptimizer16FindLabeledLoopsE_1P5Shape($this, HEAP32[$call98 + 20 >> 2]);
        __ZNSt3__15dequeIP5ShapeNS_9allocatorIS2_EEE8pop_backEv($c_i396);
        var $47 = HEAP32[$Root + 8 >> 2];
        if (($47 | 0) == 0) {
          break;
        }
        __ZZN8Relooper9CalculateEP5BlockEN13PostOptimizer16FindLabeledLoopsE_1P5Shape($this, $47);
        break;
      }
      var $NeedLoop69 = $call66 + 32 | 0;
      if ((HEAP32[$NeedLoop69 >> 2] | 0) != 0) {
        HEAP32[$ref_tmp72 >> 2] = $call66 | 0;
        __ZNSt3__15dequeIP5ShapeNS_9allocatorIS2_EEE9push_backERKS2_($1, $ref_tmp72);
      }
      var $36 = HEAP32[$call66 + 20 >> 2];
      var $__first__i_i_i_i_i321 = $call66 + 24 | 0;
      var $lnot_i_i29754 = ($36 | 0) == ($__first__i_i_i_i_i321 | 0);
      L802 : do {
        if (!$lnot_i_i29754) {
          var $38 = $__first__i_i_i_i_i321;
          var $iter75_sroa_0_055 = $36;
          while (1) {
            var $iter75_sroa_0_055;
            __ZZN8Relooper9CalculateEP5BlockEN13PostOptimizer16FindLabeledLoopsE_1P5Shape($this, HEAP32[$iter75_sroa_0_055 + 20 >> 2]);
            var $call_i_i_i258 = __ZNSt3__111__tree_nextIPNS_16__tree_node_baseIPvEEEET_S5_($iter75_sroa_0_055 | 0);
            if (($call_i_i_i258 | 0) == ($38 | 0)) {
              break L802;
            } else {
              var $iter75_sroa_0_055 = $call_i_i_i258;
            }
          }
        }
      } while (0);
      if ((HEAP32[$NeedLoop69 >> 2] | 0) != 0) {
        __ZNSt3__15dequeIP5ShapeNS_9allocatorIS2_EEE8pop_backEv($1);
      }
      var $44 = HEAP32[$Root + 8 >> 2];
      if (($44 | 0) == 0) {
        break;
      }
      __ZZN8Relooper9CalculateEP5BlockEN13PostOptimizer16FindLabeledLoopsE_1P5Shape($this, $44);
    } else {
      var $Next = $Root + 8 | 0;
      var $call7 = __ZN5Shape10IsMultipleEPS_(HEAP32[$Next >> 2]), $call7$s2 = $call7 >> 2;
      var $tobool8 = ($call7 | 0) != 0;
      L812 : do {
        if ($tobool8) {
          if ((HEAP32[$call7$s2 + 8] | 0) == 0) {
            break;
          }
          HEAP32[$ref_tmp >> 2] = $call7 | 0;
          __ZNSt3__15dequeIP5ShapeNS_9allocatorIS2_EEE9push_backERKS2_($1, $ref_tmp);
          var $5 = HEAP32[$call7$s2 + 5];
          var $__first__i_i_i_i_i = $call7 + 24 | 0;
          if (($5 | 0) == ($__first__i_i_i_i_i | 0)) {
            break;
          }
          var $7 = $__first__i_i_i_i_i;
          var $iter_sroa_0_060 = $5;
          while (1) {
            var $iter_sroa_0_060;
            __ZZN8Relooper9CalculateEP5BlockEN13PostOptimizer16FindLabeledLoopsE_1P5Shape($this, HEAP32[$iter_sroa_0_060 + 20 >> 2]);
            var $call_i_i_i = __ZNSt3__111__tree_nextIPNS_16__tree_node_baseIPvEEEET_S5_($iter_sroa_0_060 | 0);
            if (($call_i_i_i | 0) == ($7 | 0)) {
              break L812;
            } else {
              var $iter_sroa_0_060 = $call_i_i_i;
            }
          }
        }
      } while (0);
      var $Inner = $call4 + 16 | 0;
      var $12 = HEAP32[$Inner >> 2];
      var $13 = HEAP32[$12 + 24 >> 2];
      var $lnot_i_i23857 = ($13 | 0) == ($12 + 28 | 0);
      L819 : do {
        if (!$lnot_i_i23857) {
          var $__first__i_i_i_i_i278 = $1 + 20 | 0;
          var $15 = $1 + 16 | 0;
          var $16 = $1 + 4 | 0;
          var $iter17_sroa_0_058 = $13;
          while (1) {
            var $iter17_sroa_0_058;
            var $18 = HEAP32[$iter17_sroa_0_058 + 20 >> 2];
            do {
              if ((HEAP32[$18 + 4 >> 2] | 0) != 0) {
                var $20 = HEAP32[$__first__i_i_i_i_i278 >> 2];
                if (($20 | 0) == 0) {
                  ___assert_func(5242980, 923, 5244080, 5243188);
                  var $21 = HEAP32[$__first__i_i_i_i_i278 >> 2];
                } else {
                  var $21 = $20;
                }
                var $21;
                var $22 = HEAP32[$18 >> 2];
                var $sub_i_i = $21 - 1 + HEAP32[$15 >> 2] | 0;
                if (($22 | 0) == (HEAP32[HEAP32[HEAP32[$16 >> 2] + ($sub_i_i >>> 10 << 2) >> 2] + (($sub_i_i & 1023) << 2) >> 2] | 0)) {
                  HEAP8[$18 + 8 | 0] = 0;
                  break;
                } else {
                  HEAP8[__ZN5Shape9IsLabeledEPS_($22) + 16 | 0] = 1;
                  HEAP8[$18 + 8 | 0] = 1;
                  break;
                }
              }
            } while (0);
            var $call_i_i_i385 = __ZNSt3__111__tree_nextIPNS_16__tree_node_baseIPvEEEET_S5_($iter17_sroa_0_058 | 0);
            if (($call_i_i_i385 | 0) == (HEAP32[$Inner >> 2] + 28 | 0)) {
              break L819;
            } else {
              var $iter17_sroa_0_058 = $call_i_i_i385;
            }
          }
        }
      } while (0);
      do {
        if ($tobool8) {
          if ((HEAP32[$call7$s2 + 8] | 0) == 0) {
            break;
          }
          __ZNSt3__15dequeIP5ShapeNS_9allocatorIS2_EEE8pop_backEv($1);
          var $32 = HEAP32[$call7$s2 + 2];
          if (($32 | 0) == 0) {
            break L792;
          }
          __ZZN8Relooper9CalculateEP5BlockEN13PostOptimizer16FindLabeledLoopsE_1P5Shape($this, $32);
          break L792;
        }
      } while (0);
      var $33 = HEAP32[$Next >> 2];
      if (($33 | 0) == 0) {
        break;
      }
      __ZZN8Relooper9CalculateEP5BlockEN13PostOptimizer16FindLabeledLoopsE_1P5Shape($this, $33);
    }
  } while (0);
  if (!$cmp) {
    STACKTOP = __stackBase__;
    return;
  }
  var $48 = HEAP32[$Closure$s2];
  if (($48 | 0) == 0) {
    STACKTOP = __stackBase__;
    return;
  }
  __ZNSt3__15stackIP5ShapeNS_5dequeIS2_NS_9allocatorIS2_EEEEED1Ev($48);
  __ZdlPv($48);
  STACKTOP = __stackBase__;
  return;
}
__ZZN8Relooper9CalculateEP5BlockEN13PostOptimizer16FindLabeledLoopsE_1P5Shape["X"] = 1;
function __ZN5Shape9IsLabeledEPS_($It) {
  do {
    if ((__ZN5Shape10IsMultipleEPS_($It) | 0) == 0) {
      if ((__ZN5Shape6IsLoopEPS_($It) | 0) == 0) {
        var $cond = 0;
      } else {
        break;
      }
      var $cond;
      return $cond;
    }
  } while (0);
  var $cond = $It;
  var $cond;
  return $cond;
}
function __ZNSt3__15stackIP5ShapeNS_5dequeIS2_NS_9allocatorIS2_EEEEED1Ev($this) {
  __ZNSt3__15stackIP5ShapeNS_5dequeIS2_NS_9allocatorIS2_EEEEED2Ev($this);
  return;
}
function __ZNSt3__15stackIP5ShapeNS_5dequeIS2_NS_9allocatorIS2_EEEEED2Ev($this) {
  __ZNSt3__15dequeIP5ShapeNS_9allocatorIS2_EEED1Ev($this | 0);
  return;
}
function __ZNSt3__15dequeIP5ShapeNS_9allocatorIS2_EEE8pop_backEv($this) {
  var $__end__i_i_i35$s2;
  var $__first__i_i_i = $this + 20 | 0;
  var $dec = HEAP32[$__first__i_i_i >> 2] - 1 | 0;
  HEAP32[$__first__i_i_i >> 2] = $dec;
  var $__end__i_i_i35$s2 = ($this + 8 | 0) >> 2;
  var $1 = HEAP32[$__end__i_i_i35$s2];
  var $2 = HEAP32[$this + 4 >> 2];
  if (($1 | 0) == ($2 | 0)) {
    var $cond_i_i = 0;
  } else {
    var $cond_i_i = ($1 - $2 << 8) - 1 | 0;
  }
  var $cond_i_i;
  if (($cond_i_i - HEAP32[$this + 16 >> 2] - $dec | 0) >>> 0 <= 2047) {
    return;
  }
  __ZdlPv(HEAP32[$1 - 4 >> 2]);
  HEAP32[$__end__i_i_i35$s2] = HEAP32[$__end__i_i_i35$s2] - 4 | 0;
  return;
}
function __ZNSt3__15dequeIP5ShapeNS_9allocatorIS2_EEE9push_backERKS2_($this, $__v) {
  var $__first__i_i_i_i$s2;
  var __stackBase__ = STACKTOP;
  STACKTOP = STACKTOP + 8 | 0;
  var $tmp = __stackBase__;
  var $1 = HEAP32[$this + 8 >> 2];
  var $2 = HEAP32[$this + 4 >> 2];
  if (($1 | 0) == ($2 | 0)) {
    var $cond_i_i = 0;
  } else {
    var $cond_i_i = ($1 - $2 << 8) - 1 | 0;
  }
  var $cond_i_i;
  var $__first__i_i_i_i$s2 = ($this + 20 | 0) >> 2;
  if (($cond_i_i | 0) == (HEAP32[$__first__i_i_i_i$s2] + HEAP32[$this + 16 >> 2] | 0)) {
    __ZNSt3__15dequeIP5ShapeNS_9allocatorIS2_EEE19__add_back_capacityEv($this);
  }
  __ZNSt3__112__deque_baseIP5ShapeNS_9allocatorIS2_EEE3endEv($tmp, $this | 0);
  var $5 = HEAP32[$tmp + 4 >> 2];
  if (($5 | 0) == 0) {
    var $7 = HEAP32[$__first__i_i_i_i$s2];
    var $inc = $7 + 1 | 0;
    HEAP32[$__first__i_i_i_i$s2] = $inc;
    STACKTOP = __stackBase__;
    return;
  }
  HEAP32[$5 >> 2] = HEAP32[$__v >> 2];
  var $7 = HEAP32[$__first__i_i_i_i$s2];
  var $inc = $7 + 1 | 0;
  HEAP32[$__first__i_i_i_i$s2] = $inc;
  STACKTOP = __stackBase__;
  return;
}
function __ZNSt3__112__deque_baseIP5ShapeNS_9allocatorIS2_EEE3endEv($agg_result, $this) {
  var $add = HEAP32[$this + 16 >> 2] + HEAP32[$this + 20 >> 2] | 0;
  var $2 = HEAP32[$this + 4 >> 2];
  var $add_ptr = ($add >>> 10 << 2) + $2 | 0;
  if ((HEAP32[$this + 8 >> 2] | 0) == ($2 | 0)) {
    var $cond = 0;
  } else {
    var $cond = (($add & 1023) << 2) + HEAP32[$add_ptr >> 2] | 0;
  }
  var $cond;
  HEAP32[$agg_result >> 2] = $add_ptr;
  HEAP32[$agg_result + 4 >> 2] = $cond;
  return;
}
function __ZNSt3__114__split_bufferIPP5ShapeRNS_9allocatorIS3_EEE18__construct_at_endINS_13move_iteratorIPS3_EEEENS_9enable_ifIXsr21__is_forward_iteratorIT_EE5valueEvE4typeESD_SD_($this, $__first_0, $__last_0) {
  var $__end_$s2;
  if (($__last_0 | 0) == ($__first_0 | 0)) {
    return;
  }
  var $__end_$s2 = ($this + 8 | 0) >> 2;
  var $1 = $__first_0;
  var $0 = HEAP32[$__end_$s2];
  while (1) {
    var $0;
    var $1;
    if (($0 | 0) == 0) {
      var $3 = 0;
    } else {
      HEAP32[$0 >> 2] = HEAP32[$1 >> 2];
      var $3 = HEAP32[$__end_$s2];
    }
    var $3;
    var $incdec_ptr = $3 + 4 | 0;
    HEAP32[$__end_$s2] = $incdec_ptr;
    var $incdec_ptr_i = $1 + 4 | 0;
    if (($incdec_ptr_i | 0) == ($__last_0 | 0)) {
      break;
    } else {
      var $1 = $incdec_ptr_i;
      var $0 = $incdec_ptr;
    }
  }
  return;
}
function __ZZN8Relooper9CalculateEP5BlockEN13PostOptimizerC2E_1PS_($this, $ParentInit) {
  HEAP32[$this >> 2] = $ParentInit;
  HEAP32[$this + 4 >> 2] = 0;
  return;
}
function __ZNSt3__114__split_bufferIPP5ShapeNS_9allocatorIS3_EEE10push_frontERKS3_($this, $__x) {
  var $__first__i_i_i$s2;
  var $__end_$s2;
  var $__first_$s2;
  var $__begin_$s2;
  var __stackBase__ = STACKTOP;
  STACKTOP = STACKTOP + 20 | 0;
  var $__t = __stackBase__;
  var $__begin_$s2 = ($this + 4 | 0) >> 2;
  var $0 = HEAP32[$__begin_$s2];
  var $__first_$s2 = ($this | 0) >> 2;
  do {
    if (($0 | 0) == (HEAP32[$__first_$s2] | 0)) {
      var $__end_$s2 = ($this + 8 | 0) >> 2;
      var $2 = HEAP32[$__end_$s2];
      var $3 = $this + 12 | 0;
      var $__first__i_i_i$s2 = ($3 | 0) >> 2;
      var $4 = HEAP32[$__first__i_i_i$s2];
      var $sub_ptr_lhs_cast = $4;
      if ($2 >>> 0 < $4 >>> 0) {
        var $sub_ptr_rhs_cast = $2;
        var $div = ($sub_ptr_lhs_cast - $sub_ptr_rhs_cast + 4 >> 2 | 0) / 2 & -1;
        var $sub_ptr_sub_i_i = $sub_ptr_rhs_cast - $0 | 0;
        var $add_ptr_i_i = ($div - ($sub_ptr_sub_i_i >> 2) << 2) + $2 | 0;
        _memmove($add_ptr_i_i, $0, $sub_ptr_sub_i_i, 4, 0);
        HEAP32[$__begin_$s2] = $add_ptr_i_i;
        HEAP32[$__end_$s2] = ($div << 2) + HEAP32[$__end_$s2] | 0;
        var $19 = $add_ptr_i_i;
        break;
      } else {
        var $mul = $sub_ptr_lhs_cast - $0 >> 1;
        var $_sroa_speculated = ($mul | 0) == 0 ? 1 : $mul;
        __ZNSt3__114__split_bufferIPP5ShapeRNS_9allocatorIS3_EEEC1EjjS6_($__t, $_sroa_speculated, ($_sroa_speculated + 3 | 0) >>> 2, $3);
        __ZNSt3__114__split_bufferIPP5ShapeRNS_9allocatorIS3_EEE18__construct_at_endINS_13move_iteratorIPS3_EEEENS_9enable_ifIXsr21__is_forward_iteratorIT_EE5valueEvE4typeESD_SD_($__t, HEAP32[$__begin_$s2], HEAP32[$__end_$s2]);
        var $__first_30 = $__t | 0;
        var $11 = HEAP32[$__first_$s2];
        HEAP32[$__first_$s2] = HEAP32[$__first_30 >> 2];
        HEAP32[$__first_30 >> 2] = $11;
        var $__begin_33 = $__t + 4 | 0;
        var $13 = HEAP32[$__begin_$s2];
        HEAP32[$__begin_$s2] = HEAP32[$__begin_33 >> 2];
        HEAP32[$__begin_33 >> 2] = $13;
        var $__end_36 = $__t + 8 | 0;
        var $15 = HEAP32[$__end_$s2];
        HEAP32[$__end_$s2] = HEAP32[$__end_36 >> 2];
        HEAP32[$__end_36 >> 2] = $15;
        var $__first__i_i_i72 = $__t + 12 | 0;
        var $17 = HEAP32[$__first__i_i_i$s2];
        HEAP32[$__first__i_i_i$s2] = HEAP32[$__first__i_i_i72 >> 2];
        HEAP32[$__first__i_i_i72 >> 2] = $17;
        __ZNSt3__114__split_bufferIPP5ShapeRNS_9allocatorIS3_EEED1Ev($__t);
        var $19 = HEAP32[$__begin_$s2];
        break;
      }
    } else {
      var $19 = $0;
    }
  } while (0);
  var $19;
  var $add_ptr45 = $19 - 4 | 0;
  if (($add_ptr45 | 0) == 0) {
    var $21 = $19;
    var $21;
    var $incdec_ptr = $21 - 4 | 0;
    HEAP32[$__begin_$s2] = $incdec_ptr;
    STACKTOP = __stackBase__;
    return;
  }
  HEAP32[$add_ptr45 >> 2] = HEAP32[$__x >> 2];
  var $21 = HEAP32[$__begin_$s2];
  var $21;
  var $incdec_ptr = $21 - 4 | 0;
  HEAP32[$__begin_$s2] = $incdec_ptr;
  STACKTOP = __stackBase__;
  return;
}
__ZNSt3__114__split_bufferIPP5ShapeNS_9allocatorIS3_EEE10push_frontERKS3_["X"] = 1;
function __ZNSt3__114__split_bufferIPP5ShapeRNS_9allocatorIS3_EEEC1EjjS6_($this, $__cap, $__start, $__a) {
  __ZNSt3__114__split_bufferIPP5ShapeRNS_9allocatorIS3_EEEC2EjjS6_($this, $__cap, $__start, $__a);
  return;
}
function __ZNSt3__114__split_bufferIPP5ShapeRNS_9allocatorIS3_EEE10push_frontERKS3_($this, $__x) {
  var $__first__i_i_i$s2;
  var $__end_$s2;
  var $__first_$s2;
  var $__begin_$s2;
  var __stackBase__ = STACKTOP;
  STACKTOP = STACKTOP + 20 | 0;
  var $__t = __stackBase__;
  var $__begin_$s2 = ($this + 4 | 0) >> 2;
  var $0 = HEAP32[$__begin_$s2];
  var $__first_$s2 = ($this | 0) >> 2;
  do {
    if (($0 | 0) == (HEAP32[$__first_$s2] | 0)) {
      var $__end_$s2 = ($this + 8 | 0) >> 2;
      var $2 = HEAP32[$__end_$s2];
      var $__first__i_i_i$s2 = ($this + 12 | 0) >> 2;
      var $3 = HEAP32[$__first__i_i_i$s2];
      var $sub_ptr_lhs_cast = $3;
      if ($2 >>> 0 < $3 >>> 0) {
        var $sub_ptr_rhs_cast = $2;
        var $div = ($sub_ptr_lhs_cast - $sub_ptr_rhs_cast + 4 >> 2 | 0) / 2 & -1;
        var $sub_ptr_sub_i_i = $sub_ptr_rhs_cast - $0 | 0;
        var $add_ptr_i_i = ($div - ($sub_ptr_sub_i_i >> 2) << 2) + $2 | 0;
        _memmove($add_ptr_i_i, $0, $sub_ptr_sub_i_i, 4, 0);
        HEAP32[$__begin_$s2] = $add_ptr_i_i;
        HEAP32[$__end_$s2] = ($div << 2) + HEAP32[$__end_$s2] | 0;
        var $17 = $add_ptr_i_i;
        break;
      } else {
        var $mul = $sub_ptr_lhs_cast - $0 >> 1;
        var $_sroa_speculated = ($mul | 0) == 0 ? 1 : $mul;
        __ZNSt3__114__split_bufferIPP5ShapeRNS_9allocatorIS3_EEEC1EjjS6_($__t, $_sroa_speculated, ($_sroa_speculated + 3 | 0) >>> 2, HEAP32[$this + 16 >> 2]);
        __ZNSt3__114__split_bufferIPP5ShapeRNS_9allocatorIS3_EEE18__construct_at_endINS_13move_iteratorIPS3_EEEENS_9enable_ifIXsr21__is_forward_iteratorIT_EE5valueEvE4typeESD_SD_($__t, HEAP32[$__begin_$s2], HEAP32[$__end_$s2]);
        var $__first_30 = $__t | 0;
        var $9 = HEAP32[$__first_$s2];
        HEAP32[$__first_$s2] = HEAP32[$__first_30 >> 2];
        HEAP32[$__first_30 >> 2] = $9;
        var $__begin_33 = $__t + 4 | 0;
        var $11 = HEAP32[$__begin_$s2];
        HEAP32[$__begin_$s2] = HEAP32[$__begin_33 >> 2];
        HEAP32[$__begin_33 >> 2] = $11;
        var $__end_36 = $__t + 8 | 0;
        var $13 = HEAP32[$__end_$s2];
        HEAP32[$__end_$s2] = HEAP32[$__end_36 >> 2];
        HEAP32[$__end_36 >> 2] = $13;
        var $__first__i_i_i72 = $__t + 12 | 0;
        var $15 = HEAP32[$__first__i_i_i$s2];
        HEAP32[$__first__i_i_i$s2] = HEAP32[$__first__i_i_i72 >> 2];
        HEAP32[$__first__i_i_i72 >> 2] = $15;
        __ZNSt3__114__split_bufferIPP5ShapeRNS_9allocatorIS3_EEED1Ev($__t);
        var $17 = HEAP32[$__begin_$s2];
        break;
      }
    } else {
      var $17 = $0;
    }
  } while (0);
  var $17;
  var $add_ptr45 = $17 - 4 | 0;
  if (($add_ptr45 | 0) == 0) {
    var $19 = $17;
    var $19;
    var $incdec_ptr = $19 - 4 | 0;
    HEAP32[$__begin_$s2] = $incdec_ptr;
    STACKTOP = __stackBase__;
    return;
  }
  HEAP32[$add_ptr45 >> 2] = HEAP32[$__x >> 2];
  var $19 = HEAP32[$__begin_$s2];
  var $19;
  var $incdec_ptr = $19 - 4 | 0;
  HEAP32[$__begin_$s2] = $incdec_ptr;
  STACKTOP = __stackBase__;
  return;
}
__ZNSt3__114__split_bufferIPP5ShapeRNS_9allocatorIS3_EEE10push_frontERKS3_["X"] = 1;
function __ZNSt3__114__split_bufferIPP5ShapeRNS_9allocatorIS3_EEED1Ev($this) {
  __ZNSt3__114__split_bufferIPP5ShapeRNS_9allocatorIS3_EEED2Ev($this);
  return;
}
function __ZNSt3__114__split_bufferIPP5ShapeRNS_9allocatorIS3_EEED2Ev($this) {
  HEAP32[$this + 8 >> 2] = HEAP32[$this + 4 >> 2];
  var $1 = HEAP32[$this >> 2];
  if (($1 | 0) == 0) {
    return;
  }
  __ZdlPv($1);
  return;
}
function __ZNSt3__114__split_bufferIPP5ShapeRNS_9allocatorIS3_EEEC2EjjS6_($this, $__cap, $__start, $__a) {
  var $__first__i_i_i = $this + 12 | 0;
  HEAP32[$__first__i_i_i >> 2] = 0;
  HEAP32[$this + 16 >> 2] = $__a;
  if (($__cap | 0) == 0) {
    var $cond = 0;
  } else {
    var $cond = __Znwj($__cap << 2);
  }
  var $cond;
  HEAP32[$this >> 2] = $cond;
  var $add_ptr = ($__start << 2) + $cond | 0;
  HEAP32[$this + 8 >> 2] = $add_ptr;
  HEAP32[$this + 4 >> 2] = $add_ptr;
  HEAP32[$__first__i_i_i >> 2] = ($__cap << 2) + $cond | 0;
  return;
}
function __ZZN8Relooper9CalculateEP5BlockEN8Analyzer10MakeSimpleE_0RNSt3__13setIS1_NS3_4lessIS1_EENS3_9allocatorIS1_EEEES1_SA_($this, $Blocks, $Inner, $NextEntries) {
  var __stackBase__ = STACKTOP;
  STACKTOP = STACKTOP + 24 | 0;
  var $Inner_addr = __stackBase__ + 8;
  var $JustInner = __stackBase__ + 12;
  HEAP32[$Inner_addr >> 2] = $Inner;
  var $call = __Znwj(20);
  __ZN11SimpleShapeC1Ev($call);
  var $1 = $call;
  __ZZN8Relooper9CalculateEP5BlockEN8Analyzer6NoticeE_0P5Shape(HEAP32[$this >> 2], $1);
  HEAP32[$call + 16 >> 2] = $Inner;
  HEAP32[$Inner + 48 >> 2] = $1;
  if (HEAP32[$Blocks + 8 >> 2] >>> 0 <= 1) {
    STACKTOP = __stackBase__;
    return $1;
  }
  __ZNSt3__16__treeIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEE14__erase_uniqueIS2_EEjRKT_($Blocks | 0, $Inner_addr);
  __ZZN8Relooper9CalculateEP5BlockEN8Analyzer12GetBlocksOutE_0S1_RNSt3__13setIS1_NS3_4lessIS1_EENS3_9allocatorIS1_EEEEPS9_(HEAP32[$Inner_addr >> 2], $NextEntries, $Blocks);
  var $__tree__i_i = $JustInner | 0;
  __ZNSt3__16__treeIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEEC1ERKS4_($__tree__i_i);
  __ZNSt3__16__treeIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEE15__insert_uniqueERKS2_(__stackBase__, $__tree__i_i, $Inner_addr);
  var $5 = HEAP32[$NextEntries >> 2];
  var $__first__i_i_i_i_i = $NextEntries + 4 | 0;
  var $lnot_i16 = ($5 | 0) == ($__first__i_i_i_i_i | 0);
  L927 : do {
    if (!$lnot_i16) {
      var $7 = $__first__i_i_i_i_i;
      var $iter_sroa_0_017 = $5;
      while (1) {
        var $iter_sroa_0_017;
        __ZZN8Relooper9CalculateEP5BlockEN8Analyzer9SolipsizeE_0S1_N6Branch8FlowTypeEP5ShapeRNSt3__13setIS1_NS7_4lessIS1_EENS7_9allocatorIS1_EEEE(HEAP32[$iter_sroa_0_017 + 16 >> 2], 0, $1, $JustInner);
        var $call_i_i = __ZNSt3__111__tree_nextIPKNS_16__tree_node_baseIPvEEEET_S6_($iter_sroa_0_017 | 0);
        if (($call_i_i | 0) == ($7 | 0)) {
          break L927;
        } else {
          var $iter_sroa_0_017 = $call_i_i;
        }
      }
    }
  } while (0);
  __ZNSt3__13setIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEED1Ev($JustInner);
  STACKTOP = __stackBase__;
  return $1;
}
__ZZN8Relooper9CalculateEP5BlockEN8Analyzer10MakeSimpleE_0RNSt3__13setIS1_NS3_4lessIS1_EENS3_9allocatorIS1_EEEES1_SA_["X"] = 1;
function __ZNSt3__15dequeIP5ShapeNS_9allocatorIS2_EEE19__add_back_capacityEv($this) {
  var $__begin__i711$s2;
  var $__first__i_i_i_i709$s2;
  var $__end__i704$s2;
  var $__first__i150$s2;
  var $__first__i_i_i_i149$s2;
  var $__begin__i135$s2;
  var $__end__i134$s2;
  var $__first__i$s2;
  var $__first__i_i_i_i$s2;
  var $__end__i127$s2;
  var $__begin__i$s2;
  var __stackBase__ = STACKTOP;
  STACKTOP = STACKTOP + 104 | 0;
  var $__t_i698 = __stackBase__;
  var $__t_i465 = __stackBase__ + 20;
  var $__t_i254 = __stackBase__ + 40;
  var $__t_i125 = __stackBase__ + 60;
  var $ref_tmp21 = __stackBase__ + 80;
  var $__buf = __stackBase__ + 84;
  var $__start__i = $this + 16 | 0;
  var $0 = HEAP32[$__start__i >> 2];
  if ($0 >>> 0 > 1023) {
    HEAP32[$__start__i >> 2] = $0 - 1024 | 0;
    var $__begin__i$s2 = ($this + 4 | 0) >> 2;
    var $1 = HEAP32[$__begin__i$s2];
    var $2 = HEAP32[$1 >> 2];
    var $add_ptr_i = $1 + 4 | 0;
    HEAP32[$__begin__i$s2] = $add_ptr_i;
    var $__end__i127$s2 = ($this + 8 | 0) >> 2;
    var $3 = HEAP32[$__end__i127$s2];
    var $4 = $this + 12 | 0;
    var $__first__i_i_i_i$s2 = ($4 | 0) >> 2;
    do {
      if (($3 | 0) == (HEAP32[$__first__i_i_i_i$s2] | 0)) {
        var $__first__i$s2 = ($this | 0) >> 2;
        var $6 = HEAP32[$__first__i$s2];
        if ($add_ptr_i >>> 0 > $6 >>> 0) {
          var $sub_ptr_lhs_cast_i = $add_ptr_i;
          var $idx_neg_i = ($sub_ptr_lhs_cast_i - $6 + 4 >> 2 | 0) / -2 & -1;
          var $add_ptr_i_sum = $idx_neg_i + 1 | 0;
          var $sub_ptr_sub_i_i_i = $3 - $sub_ptr_lhs_cast_i | 0;
          _memmove(($add_ptr_i_sum << 2) + $1 | 0, $add_ptr_i, $sub_ptr_sub_i_i_i, 4, 0);
          var $add_ptr_i_i_i = (($sub_ptr_sub_i_i_i >> 2) + $add_ptr_i_sum << 2) + $1 | 0;
          HEAP32[$__end__i127$s2] = $add_ptr_i_i_i;
          HEAP32[$__begin__i$s2] = ($idx_neg_i << 2) + HEAP32[$__begin__i$s2] | 0;
          var $21 = $add_ptr_i_i_i;
          break;
        } else {
          var $mul_i = $3 - $6 >> 1;
          var $_sroa_speculated90 = ($mul_i | 0) == 0 ? 1 : $mul_i;
          __ZNSt3__114__split_bufferIPP5ShapeRNS_9allocatorIS3_EEEC1EjjS6_($__t_i125, $_sroa_speculated90, $_sroa_speculated90 >>> 2, $4);
          __ZNSt3__114__split_bufferIPP5ShapeRNS_9allocatorIS3_EEE18__construct_at_endINS_13move_iteratorIPS3_EEEENS_9enable_ifIXsr21__is_forward_iteratorIT_EE5valueEvE4typeESD_SD_($__t_i125, HEAP32[$__begin__i$s2], HEAP32[$__end__i127$s2]);
          var $__first_30_i = $__t_i125 | 0;
          var $13 = HEAP32[$__first__i$s2];
          HEAP32[$__first__i$s2] = HEAP32[$__first_30_i >> 2];
          HEAP32[$__first_30_i >> 2] = $13;
          var $__begin_33_i = $__t_i125 + 4 | 0;
          var $15 = HEAP32[$__begin__i$s2];
          HEAP32[$__begin__i$s2] = HEAP32[$__begin_33_i >> 2];
          HEAP32[$__begin_33_i >> 2] = $15;
          var $__end_36_i = $__t_i125 + 8 | 0;
          var $17 = HEAP32[$__end__i127$s2];
          HEAP32[$__end__i127$s2] = HEAP32[$__end_36_i >> 2];
          HEAP32[$__end_36_i >> 2] = $17;
          var $__first__i_i_i71_i = $__t_i125 + 12 | 0;
          var $19 = HEAP32[$__first__i_i_i_i$s2];
          HEAP32[$__first__i_i_i_i$s2] = HEAP32[$__first__i_i_i71_i >> 2];
          HEAP32[$__first__i_i_i71_i >> 2] = $19;
          __ZNSt3__114__split_bufferIPP5ShapeRNS_9allocatorIS3_EEED1Ev($__t_i125);
          var $21 = HEAP32[$__end__i127$s2];
          break;
        }
      } else {
        var $21 = $3;
      }
    } while (0);
    var $21;
    if (($21 | 0) == 0) {
      var $22 = 0;
    } else {
      HEAP32[$21 >> 2] = $2;
      var $22 = HEAP32[$__end__i127$s2];
    }
    var $22;
    HEAP32[$__end__i127$s2] = $22 + 4 | 0;
    STACKTOP = __stackBase__;
    return;
  }
  var $__map_6 = $this | 0;
  var $__end__i134$s2 = ($this + 8 | 0) >> 2;
  var $23 = HEAP32[$__end__i134$s2];
  var $__begin__i135$s2 = ($this + 4 | 0) >> 2;
  var $sub_ptr_div_i139 = $23 - HEAP32[$__begin__i135$s2] >> 2;
  var $25 = $this + 12 | 0;
  var $__first__i_i_i_i149$s2 = ($25 | 0) >> 2;
  var $26 = HEAP32[$__first__i_i_i_i149$s2];
  var $__first__i150$s2 = ($this | 0) >> 2;
  var $sub_ptr_sub_i153 = $26 - HEAP32[$__first__i150$s2] | 0;
  if ($sub_ptr_div_i139 >>> 0 >= $sub_ptr_sub_i153 >> 2 >>> 0) {
    var $mul = $sub_ptr_sub_i153 >> 1;
    __ZNSt3__114__split_bufferIPP5ShapeRNS_9allocatorIS3_EEEC1EjjS6_($__buf, ($mul | 0) == 0 ? 1 : $mul, $sub_ptr_div_i139, $25);
    var $71 = __Znwj(4096);
    var $__end__i704$s2 = ($__buf + 8 | 0) >> 2;
    var $72 = HEAP32[$__end__i704$s2];
    var $__first__i_i_i_i709$s2 = ($__buf + 12 | 0) >> 2;
    do {
      if (($72 | 0) == (HEAP32[$__first__i_i_i_i709$s2] | 0)) {
        var $__begin__i711$s2 = ($__buf + 4 | 0) >> 2;
        var $74 = HEAP32[$__begin__i711$s2];
        var $__first__i712 = $__buf | 0;
        var $75 = HEAP32[$__first__i712 >> 2];
        if ($74 >>> 0 > $75 >>> 0) {
          var $sub_ptr_lhs_cast_i717 = $74;
          var $idx_neg_i726 = ($sub_ptr_lhs_cast_i717 - $75 + 4 >> 2 | 0) / -2 & -1;
          var $add_ptr_i727 = ($idx_neg_i726 << 2) + $74 | 0;
          var $sub_ptr_sub_i_i_i730 = $72 - $sub_ptr_lhs_cast_i717 | 0;
          _memmove($add_ptr_i727, $74, $sub_ptr_sub_i_i_i730, 4, 0);
          var $add_ptr_i_i_i733 = (($sub_ptr_sub_i_i_i730 >> 2) + $idx_neg_i726 << 2) + $74 | 0;
          HEAP32[$__end__i704$s2] = $add_ptr_i_i_i733;
          HEAP32[$__begin__i711$s2] = $add_ptr_i727;
          var $_pre102 = $add_ptr_i_i_i733;
          break;
        }
        var $mul_i749 = $72 - $75 >> 1;
        var $_sroa_speculated = ($mul_i749 | 0) == 0 ? 1 : $mul_i749;
        __ZNSt3__114__split_bufferIPP5ShapeRNS_9allocatorIS3_EEEC1EjjS6_($__t_i698, $_sroa_speculated, $_sroa_speculated >>> 2, HEAP32[$__buf + 16 >> 2]);
        __ZNSt3__114__split_bufferIPP5ShapeRNS_9allocatorIS3_EEE18__construct_at_endINS_13move_iteratorIPS3_EEEENS_9enable_ifIXsr21__is_forward_iteratorIT_EE5valueEvE4typeESD_SD_($__t_i698, $74, $72);
        var $__first_30_i771 = $__t_i698 | 0;
        HEAP32[$__first__i712 >> 2] = HEAP32[$__first_30_i771 >> 2];
        HEAP32[$__first_30_i771 >> 2] = $75;
        var $__begin_33_i774 = $__t_i698 + 4 | 0;
        HEAP32[$__begin__i711$s2] = HEAP32[$__begin_33_i774 >> 2];
        HEAP32[$__begin_33_i774 >> 2] = $74;
        var $__end_36_i776 = $__t_i698 + 8 | 0;
        var $80 = HEAP32[$__end_36_i776 >> 2];
        HEAP32[$__end__i704$s2] = $80;
        HEAP32[$__end_36_i776 >> 2] = $72;
        var $__first__i_i_i71_i786 = $__t_i698 + 12 | 0;
        HEAP32[$__first__i_i_i_i709$s2] = HEAP32[$__first__i_i_i71_i786 >> 2];
        HEAP32[$__first__i_i_i71_i786 >> 2] = $72;
        __ZNSt3__114__split_bufferIPP5ShapeRNS_9allocatorIS3_EEED1Ev($__t_i698);
        var $_pre102 = $80;
      } else {
        var $_pre102 = $72;
      }
    } while (0);
    var $_pre102;
    if (($_pre102 | 0) == 0) {
      var $82 = 0;
    } else {
      HEAP32[$_pre102 >> 2] = $71;
      var $82 = $_pre102;
    }
    var $82;
    HEAP32[$__end__i704$s2] = $82 + 4 | 0;
    var $__i_0 = HEAP32[$__end__i134$s2];
    while (1) {
      var $__i_0;
      if (($__i_0 | 0) == (HEAP32[$__begin__i135$s2] | 0)) {
        break;
      }
      var $incdec_ptr = $__i_0 - 4 | 0;
      __ZNSt3__114__split_bufferIPP5ShapeRNS_9allocatorIS3_EEE10push_frontERKS3_($__buf, $incdec_ptr);
      var $__i_0 = $incdec_ptr;
    }
    var $__first_52 = $__buf | 0;
    var $97 = HEAP32[$__first__i150$s2];
    HEAP32[$__first__i150$s2] = HEAP32[$__first_52 >> 2];
    HEAP32[$__first_52 >> 2] = $97;
    var $__begin_55 = $__buf + 4 | 0;
    HEAP32[$__begin__i135$s2] = HEAP32[$__begin_55 >> 2];
    HEAP32[$__begin_55 >> 2] = $__i_0;
    var $100 = HEAP32[$__end__i134$s2];
    HEAP32[$__end__i134$s2] = HEAP32[$__end__i704$s2];
    HEAP32[$__end__i704$s2] = $100;
    var $102 = HEAP32[$__first__i_i_i_i149$s2];
    HEAP32[$__first__i_i_i_i149$s2] = HEAP32[$__first__i_i_i_i709$s2];
    HEAP32[$__first__i_i_i_i709$s2] = $102;
    __ZNSt3__114__split_bufferIPP5ShapeRNS_9allocatorIS3_EEED1Ev($__buf);
    STACKTOP = __stackBase__;
    return;
  }
  var $28 = __Znwj(4096);
  if (($26 | 0) == ($23 | 0)) {
    HEAP32[$ref_tmp21 >> 2] = $28;
    __ZNSt3__114__split_bufferIPP5ShapeNS_9allocatorIS3_EEE10push_frontERKS3_($__map_6, $ref_tmp21);
    var $49 = HEAP32[$__begin__i135$s2];
    var $50 = HEAP32[$49 >> 2];
    var $add_ptr_i382 = $49 + 4 | 0;
    HEAP32[$__begin__i135$s2] = $add_ptr_i382;
    var $51 = HEAP32[$__end__i134$s2];
    do {
      if (($51 | 0) == (HEAP32[$__first__i_i_i_i149$s2] | 0)) {
        var $53 = HEAP32[$__first__i150$s2];
        if ($add_ptr_i382 >>> 0 > $53 >>> 0) {
          var $sub_ptr_lhs_cast_i484 = $add_ptr_i382;
          var $idx_neg_i493 = ($sub_ptr_lhs_cast_i484 - $53 + 4 >> 2 | 0) / -2 & -1;
          var $add_ptr_i382_sum = $idx_neg_i493 + 1 | 0;
          var $sub_ptr_sub_i_i_i497 = $51 - $sub_ptr_lhs_cast_i484 | 0;
          _memmove(($add_ptr_i382_sum << 2) + $49 | 0, $add_ptr_i382, $sub_ptr_sub_i_i_i497, 4, 0);
          var $add_ptr_i_i_i500 = (($sub_ptr_sub_i_i_i497 >> 2) + $add_ptr_i382_sum << 2) + $49 | 0;
          HEAP32[$__end__i134$s2] = $add_ptr_i_i_i500;
          HEAP32[$__begin__i135$s2] = ($idx_neg_i493 << 2) + HEAP32[$__begin__i135$s2] | 0;
          var $68 = $add_ptr_i_i_i500;
          break;
        } else {
          var $mul_i516 = $51 - $53 >> 1;
          var $_sroa_speculated84 = ($mul_i516 | 0) == 0 ? 1 : $mul_i516;
          __ZNSt3__114__split_bufferIPP5ShapeRNS_9allocatorIS3_EEEC1EjjS6_($__t_i465, $_sroa_speculated84, $_sroa_speculated84 >>> 2, $25);
          __ZNSt3__114__split_bufferIPP5ShapeRNS_9allocatorIS3_EEE18__construct_at_endINS_13move_iteratorIPS3_EEEENS_9enable_ifIXsr21__is_forward_iteratorIT_EE5valueEvE4typeESD_SD_($__t_i465, HEAP32[$__begin__i135$s2], HEAP32[$__end__i134$s2]);
          var $__first_30_i538 = $__t_i465 | 0;
          var $60 = HEAP32[$__first__i150$s2];
          HEAP32[$__first__i150$s2] = HEAP32[$__first_30_i538 >> 2];
          HEAP32[$__first_30_i538 >> 2] = $60;
          var $__begin_33_i541 = $__t_i465 + 4 | 0;
          var $62 = HEAP32[$__begin__i135$s2];
          HEAP32[$__begin__i135$s2] = HEAP32[$__begin_33_i541 >> 2];
          HEAP32[$__begin_33_i541 >> 2] = $62;
          var $__end_36_i543 = $__t_i465 + 8 | 0;
          var $64 = HEAP32[$__end__i134$s2];
          HEAP32[$__end__i134$s2] = HEAP32[$__end_36_i543 >> 2];
          HEAP32[$__end_36_i543 >> 2] = $64;
          var $__first__i_i_i71_i553 = $__t_i465 + 12 | 0;
          var $66 = HEAP32[$__first__i_i_i_i149$s2];
          HEAP32[$__first__i_i_i_i149$s2] = HEAP32[$__first__i_i_i71_i553 >> 2];
          HEAP32[$__first__i_i_i71_i553 >> 2] = $66;
          __ZNSt3__114__split_bufferIPP5ShapeRNS_9allocatorIS3_EEED1Ev($__t_i465);
          var $68 = HEAP32[$__end__i134$s2];
          break;
        }
      } else {
        var $68 = $51;
      }
    } while (0);
    var $68;
    if (($68 | 0) == 0) {
      var $69 = 0;
    } else {
      HEAP32[$68 >> 2] = $50;
      var $69 = HEAP32[$__end__i134$s2];
    }
    var $69;
    HEAP32[$__end__i134$s2] = $69 + 4 | 0;
    STACKTOP = __stackBase__;
    return;
  } else {
    var $29 = HEAP32[$__end__i134$s2];
    do {
      if (($29 | 0) == (HEAP32[$__first__i_i_i_i149$s2] | 0)) {
        var $31 = HEAP32[$__begin__i135$s2];
        var $32 = HEAP32[$__first__i150$s2];
        if ($31 >>> 0 > $32 >>> 0) {
          var $sub_ptr_lhs_cast_i273 = $31;
          var $idx_neg_i282 = ($sub_ptr_lhs_cast_i273 - $32 + 4 >> 2 | 0) / -2 & -1;
          var $sub_ptr_sub_i_i_i286 = $29 - $sub_ptr_lhs_cast_i273 | 0;
          _memmove(($idx_neg_i282 << 2) + $31 | 0, $31, $sub_ptr_sub_i_i_i286, 4, 0);
          var $add_ptr_i_i_i289 = (($sub_ptr_sub_i_i_i286 >> 2) + $idx_neg_i282 << 2) + $31 | 0;
          HEAP32[$__end__i134$s2] = $add_ptr_i_i_i289;
          HEAP32[$__begin__i135$s2] = ($idx_neg_i282 << 2) + HEAP32[$__begin__i135$s2] | 0;
          var $47 = $add_ptr_i_i_i289;
          break;
        } else {
          var $mul_i305 = $29 - $32 >> 1;
          var $_sroa_speculated87 = ($mul_i305 | 0) == 0 ? 1 : $mul_i305;
          __ZNSt3__114__split_bufferIPP5ShapeRNS_9allocatorIS3_EEEC1EjjS6_($__t_i254, $_sroa_speculated87, $_sroa_speculated87 >>> 2, $25);
          __ZNSt3__114__split_bufferIPP5ShapeRNS_9allocatorIS3_EEE18__construct_at_endINS_13move_iteratorIPS3_EEEENS_9enable_ifIXsr21__is_forward_iteratorIT_EE5valueEvE4typeESD_SD_($__t_i254, HEAP32[$__begin__i135$s2], HEAP32[$__end__i134$s2]);
          var $__first_30_i327 = $__t_i254 | 0;
          var $39 = HEAP32[$__first__i150$s2];
          HEAP32[$__first__i150$s2] = HEAP32[$__first_30_i327 >> 2];
          HEAP32[$__first_30_i327 >> 2] = $39;
          var $__begin_33_i330 = $__t_i254 + 4 | 0;
          var $41 = HEAP32[$__begin__i135$s2];
          HEAP32[$__begin__i135$s2] = HEAP32[$__begin_33_i330 >> 2];
          HEAP32[$__begin_33_i330 >> 2] = $41;
          var $__end_36_i332 = $__t_i254 + 8 | 0;
          var $43 = HEAP32[$__end__i134$s2];
          HEAP32[$__end__i134$s2] = HEAP32[$__end_36_i332 >> 2];
          HEAP32[$__end_36_i332 >> 2] = $43;
          var $__first__i_i_i71_i342 = $__t_i254 + 12 | 0;
          var $45 = HEAP32[$__first__i_i_i_i149$s2];
          HEAP32[$__first__i_i_i_i149$s2] = HEAP32[$__first__i_i_i71_i342 >> 2];
          HEAP32[$__first__i_i_i71_i342 >> 2] = $45;
          __ZNSt3__114__split_bufferIPP5ShapeRNS_9allocatorIS3_EEED1Ev($__t_i254);
          var $47 = HEAP32[$__end__i134$s2];
          break;
        }
      } else {
        var $47 = $29;
      }
    } while (0);
    var $47;
    if (($47 | 0) == 0) {
      var $48 = 0;
    } else {
      HEAP32[$47 >> 2] = $28;
      var $48 = HEAP32[$__end__i134$s2];
    }
    var $48;
    HEAP32[$__end__i134$s2] = $48 + 4 | 0;
    STACKTOP = __stackBase__;
    return;
  }
}
__ZNSt3__15dequeIP5ShapeNS_9allocatorIS2_EEE19__add_back_capacityEv["X"] = 1;
function __ZZN8Relooper9CalculateEP5BlockEN8Analyzer8MakeLoopE_0RNSt3__13setIS1_NS3_4lessIS1_EENS3_9allocatorIS1_EEEESA_SA_($this, $Blocks, $Entries, $NextEntries) {
  var $Curr$s2;
  var __stackBase__ = STACKTOP;
  STACKTOP = STACKTOP + 80 | 0;
  var $ref_tmp_i642 = __stackBase__;
  var $agg_tmp_i613 = __stackBase__ + 8;
  var $agg_tmp_i594 = __stackBase__ + 12;
  var $agg_tmp_i535 = __stackBase__ + 16;
  var $ref_tmp_i337 = __stackBase__ + 20;
  var $ref_tmp_i = __stackBase__ + 28;
  var $agg_tmp_i197 = __stackBase__ + 36;
  var $agg_tmp_i180 = __stackBase__ + 40;
  var $tmp_i_i_i = __stackBase__ + 44;
  var $InnerBlocks = __stackBase__ + 48;
  var $Queue = __stackBase__ + 60;
  var $Curr = __stackBase__ + 72, $Curr$s2 = $Curr >> 2;
  var $Possible = __stackBase__ + 76;
  var $__tree__i_i = $InnerBlocks | 0;
  __ZNSt3__16__treeIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEEC1ERKS4_($__tree__i_i);
  var $__tree__i_i123 = $Queue | 0;
  __ZNSt3__16__treeIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEEC1ERKS7_($__tree__i_i123);
  var $__begin_node__i_i_i_i_i = $Entries | 0;
  var $0 = HEAP32[$__begin_node__i_i_i_i_i >> 2];
  var $__first__i_i_i_i_i_i_i = $Entries + 4 | 0;
  var $1 = $__first__i_i_i_i_i_i_i;
  var $2 = $Queue + 4 | 0;
  var $lnot_i_i_i_i80 = ($0 | 0) == ($1 | 0);
  L991 : do {
    if (!$lnot_i_i_i_i80) {
      var $3 = $__first__i_i_i_i_i_i_i;
      var $agg_tmp18_i_i_sroa_0_081 = $0;
      while (1) {
        var $agg_tmp18_i_i_sroa_0_081;
        __ZNSt3__16__treeIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEE15__insert_uniqueENS_21__tree_const_iteratorIS2_PKNS_11__tree_nodeIS2_PvEEiEERKS2_($tmp_i_i_i, $__tree__i_i123, $2, $agg_tmp18_i_i_sroa_0_081 + 16 | 0);
        var $call_i_i_i_i = __ZNSt3__111__tree_nextIPKNS_16__tree_node_baseIPvEEEET_S6_($agg_tmp18_i_i_sroa_0_081 | 0);
        if (($call_i_i_i_i | 0) == ($3 | 0)) {
          break L991;
        } else {
          var $agg_tmp18_i_i_sroa_0_081 = $call_i_i_i_i;
        }
      }
    }
  } while (0);
  var $__first__i_i_i_i = $Queue + 8 | 0;
  var $cmp79 = (HEAP32[$__first__i_i_i_i >> 2] | 0) == 0;
  L997 : do {
    if (!$cmp79) {
      var $__begin_node__i_i_i = $Queue | 0;
      var $agg_tmp2_i195_sroa_0_0_tmp3_i201_idx = $agg_tmp_i197 | 0;
      var $5 = $InnerBlocks + 4 | 0;
      var $__tree__i257 = $Blocks | 0;
      while (1) {
        var $8 = HEAP32[$__begin_node__i_i_i >> 2];
        HEAP32[$Curr$s2] = HEAP32[$8 + 16 >> 2];
        __ZNSt3__16__treeIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEE5eraseENS_21__tree_const_iteratorIS2_PKNS_11__tree_nodeIS2_PvEEiEE($agg_tmp_i180, $__tree__i_i123, $8);
        __ZNSt3__16__treeIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEE4findIS2_EENS_15__tree_iteratorIS2_PNS_11__tree_nodeIS2_PvEEiEERKT_($agg_tmp_i197, $__tree__i_i, $Curr);
        var $cmp_i = (HEAP32[$agg_tmp2_i195_sroa_0_0_tmp3_i201_idx >> 2] | 0) == ($5 | 0);
        L1001 : do {
          if ($cmp_i) {
            __ZNSt3__16__treeIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEE15__insert_uniqueERKS2_($ref_tmp_i, $__tree__i_i, $Curr);
            __ZNSt3__16__treeIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEE14__erase_uniqueIS2_EEjRKT_($__tree__i257, $Curr);
            var $11 = HEAP32[$Curr$s2];
            var $12 = HEAP32[$11 + 12 >> 2];
            if (($12 | 0) == ($11 + 16 | 0)) {
              break;
            } else {
              var $iter_sroa_0_076 = $12;
            }
            while (1) {
              var $iter_sroa_0_076;
              __ZNSt3__16__treeIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEE15__insert_uniqueERKS2_($ref_tmp_i337, $__tree__i_i123, $iter_sroa_0_076 + 16 | 0);
              var $call_i_i_i = __ZNSt3__111__tree_nextIPNS_16__tree_node_baseIPvEEEET_S5_($iter_sroa_0_076 | 0);
              if (($call_i_i_i | 0) == (HEAP32[$Curr$s2] + 16 | 0)) {
                break L1001;
              } else {
                var $iter_sroa_0_076 = $call_i_i_i;
              }
            }
          }
        } while (0);
        if ((HEAP32[$__first__i_i_i_i >> 2] | 0) == 0) {
          break L997;
        }
      }
    }
  } while (0);
  if ((HEAP32[$InnerBlocks + 8 >> 2] | 0) == 0) {
    ___assert_func(5242980, 488, 5243988, 5243164);
  }
  var $22 = HEAP32[$InnerBlocks >> 2];
  var $__first__i_i_i_i_i428 = $InnerBlocks + 4 | 0;
  var $23 = $__first__i_i_i_i_i428;
  var $lnot_i69 = ($22 | 0) == ($23 | 0);
  L1012 : do {
    if (!$lnot_i69) {
      var $agg_tmp2_i532_sroa_0_0_tmp3_i539_idx = $agg_tmp_i535 | 0;
      var $__tree__i596 = $NextEntries | 0;
      var $agg_tmp2_i591_sroa_0_0_tmp3_i598_idx = $agg_tmp_i594 | 0;
      var $agg_tmp2_i610_sroa_0_0_tmp3_i617_idx = $agg_tmp_i613 | 0;
      var $24 = $__first__i_i_i_i_i428;
      var $iter31_sroa_0_070 = $22;
      while (1) {
        var $iter31_sroa_0_070;
        var $25 = HEAP32[$iter31_sroa_0_070 + 16 >> 2];
        var $26 = HEAP32[$25 >> 2];
        var $__first__i_i_i_i_i499 = $25 + 4 | 0;
        var $lnot_i_i52167 = ($26 | 0) == ($__first__i_i_i_i_i499 | 0);
        L1016 : do {
          if (!$lnot_i_i52167) {
            var $28 = $__first__i_i_i_i_i499;
            var $iter42_sroa_0_068 = $26;
            while (1) {
              var $iter42_sroa_0_068;
              HEAP32[$Possible >> 2] = HEAP32[$iter42_sroa_0_068 + 16 >> 2];
              __ZNSt3__16__treeIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEE4findIS2_EENS_15__tree_iteratorIS2_PNS_11__tree_nodeIS2_PvEEiEERKT_($agg_tmp_i535, $__tree__i_i, $Possible);
              do {
                if ((HEAP32[$agg_tmp2_i532_sroa_0_0_tmp3_i539_idx >> 2] | 0) == ($23 | 0)) {
                  __ZNSt3__16__treeIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEE4findIS2_EENS_15__tree_iteratorIS2_PNS_11__tree_nodeIS2_PvEEiEERKT_($agg_tmp_i594, $__tree__i596, $Possible);
                  var $agg_tmp2_i591_sroa_0_0_copyload33 = HEAP32[$agg_tmp2_i591_sroa_0_0_tmp3_i598_idx >> 2];
                  __ZNSt3__16__treeIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEE4findIS2_EENS_15__tree_iteratorIS2_PNS_11__tree_nodeIS2_PvEEiEERKT_($agg_tmp_i613, $__tree__i596, $Possible);
                  if (($agg_tmp2_i591_sroa_0_0_copyload33 | 0) != (HEAP32[$agg_tmp2_i610_sroa_0_0_tmp3_i617_idx >> 2] | 0)) {
                    break;
                  }
                  __ZNSt3__16__treeIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEE15__insert_uniqueERKS2_($ref_tmp_i642, $__tree__i596, $Possible);
                }
              } while (0);
              var $call_i_i_i672 = __ZNSt3__111__tree_nextIPNS_16__tree_node_baseIPvEEEET_S5_($iter42_sroa_0_068 | 0);
              if (($call_i_i_i672 | 0) == ($28 | 0)) {
                break L1016;
              } else {
                var $iter42_sroa_0_068 = $call_i_i_i672;
              }
            }
          }
        } while (0);
        var $call_i_i = __ZNSt3__111__tree_nextIPKNS_16__tree_node_baseIPvEEEET_S6_($iter31_sroa_0_070 | 0);
        if (($call_i_i | 0) == ($24 | 0)) {
          break L1012;
        } else {
          var $iter31_sroa_0_070 = $call_i_i;
        }
      }
    }
  } while (0);
  var $call78 = __Znwj(24);
  __ZN9LoopShapeC1Ev($call78);
  var $35 = $call78;
  __ZZN8Relooper9CalculateEP5BlockEN8Analyzer6NoticeE_0P5Shape(HEAP32[$this >> 2], $35);
  var $36 = HEAP32[$__begin_node__i_i_i_i_i >> 2];
  var $lnot_i75063 = ($36 | 0) == ($1 | 0);
  L1028 : do {
    if (!$lnot_i75063) {
      var $37 = $__first__i_i_i_i_i_i_i;
      var $iter83_sroa_0_064 = $36;
      while (1) {
        var $iter83_sroa_0_064;
        __ZZN8Relooper9CalculateEP5BlockEN8Analyzer9SolipsizeE_0S1_N6Branch8FlowTypeEP5ShapeRNSt3__13setIS1_NS7_4lessIS1_EENS7_9allocatorIS1_EEEE(HEAP32[$iter83_sroa_0_064 + 16 >> 2], 2, $35, $InnerBlocks);
        var $call_i_i761 = __ZNSt3__111__tree_nextIPKNS_16__tree_node_baseIPvEEEET_S6_($iter83_sroa_0_064 | 0);
        if (($call_i_i761 | 0) == ($37 | 0)) {
          break L1028;
        } else {
          var $iter83_sroa_0_064 = $call_i_i761;
        }
      }
    }
  } while (0);
  var $41 = HEAP32[$NextEntries >> 2];
  var $__first__i_i_i_i_i814 = $NextEntries + 4 | 0;
  var $lnot_i83358 = ($41 | 0) == ($__first__i_i_i_i_i814 | 0);
  L1034 : do {
    if (!$lnot_i83358) {
      var $43 = $__first__i_i_i_i_i814;
      var $iter97_sroa_0_059 = $41;
      while (1) {
        var $iter97_sroa_0_059;
        __ZZN8Relooper9CalculateEP5BlockEN8Analyzer9SolipsizeE_0S1_N6Branch8FlowTypeEP5ShapeRNSt3__13setIS1_NS7_4lessIS1_EENS7_9allocatorIS1_EEEE(HEAP32[$iter97_sroa_0_059 + 16 >> 2], 1, $35, $InnerBlocks);
        var $call_i_i844 = __ZNSt3__111__tree_nextIPKNS_16__tree_node_baseIPvEEEET_S6_($iter97_sroa_0_059 | 0);
        if (($call_i_i844 | 0) == ($43 | 0)) {
          break L1034;
        } else {
          var $iter97_sroa_0_059 = $call_i_i844;
        }
      }
    }
  } while (0);
  HEAP32[$call78 + 20 >> 2] = __ZZN8Relooper9CalculateEP5BlockEN8Analyzer7ProcessE_0RNSt3__13setIS1_NS3_4lessIS1_EENS3_9allocatorIS1_EEEESA_P5Shape($this, $InnerBlocks, $Entries);
  __ZNSt3__13setIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEED1Ev($Queue);
  __ZNSt3__13setIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEED1Ev($InnerBlocks);
  STACKTOP = __stackBase__;
  return $35;
}
__ZZN8Relooper9CalculateEP5BlockEN8Analyzer8MakeLoopE_0RNSt3__13setIS1_NS3_4lessIS1_EENS3_9allocatorIS1_EEEESA_SA_["X"] = 1;
function __ZZN8Relooper9CalculateEP5BlockEN8Analyzer21FindIndependentGroupsE_0RNSt3__13setIS1_NS3_4lessIS1_EENS3_9allocatorIS1_EEEESA_RNS3_3mapIS1_S9_S6_NS7_INS3_4pairIKS1_S9_EEEEEE($Entries, $IndependentGroups) {
  var $__begin_node__i_i_i$s2;
  var $__first__i_i_i_i_i_i$s2;
  var $Owner$s2;
  var __stackBase__ = STACKTOP;
  STACKTOP = STACKTOP + 84 | 0;
  var $ref_tmp_i510 = __stackBase__;
  var $agg_tmp_i414 = __stackBase__ + 8;
  var $ref_tmp_i = __stackBase__ + 12;
  var $Helper = __stackBase__ + 20;
  var $Queue = __stackBase__ + 36;
  var $Entry = __stackBase__ + 48;
  var $Curr = __stackBase__ + 52;
  var $Owner = __stackBase__ + 56, $Owner$s2 = $Owner >> 2;
  var $New = __stackBase__ + 60;
  var $ToInvalidate = __stackBase__ + 64;
  var $Child = __stackBase__ + 76;
  var $Parent = __stackBase__ + 80;
  __ZZZN8Relooper9CalculateEP5BlockEN8Analyzer21FindIndependentGroupsE_0RNSt3__13setIS1_NS3_4lessIS1_EENS3_9allocatorIS1_EEEESA_RNS3_3mapIS1_S9_S6_NS7_INS3_4pairIKS1_S9_EEEEEEEN11HelperClassC1ESH_($Helper, $IndependentGroups);
  var $0 = $Queue;
  HEAP32[$Queue >> 2] = $0;
  var $__next__i_i_i_i_i = $Queue + 4 | 0;
  HEAP32[$__next__i_i_i_i_i >> 2] = $0;
  var $__first__i_i_i_i_i_i$s2 = ($Queue + 8 | 0) >> 2;
  HEAP32[$__first__i_i_i_i_i_i$s2] = 0;
  var $__begin_node__i_i_i$s2 = ($Entries | 0) >> 2;
  var $1 = HEAP32[$__begin_node__i_i_i$s2];
  var $__first__i_i_i_i_i = $Entries + 4 | 0;
  var $2 = $__first__i_i_i_i_i;
  var $lnot_i26 = ($1 | 0) == ($2 | 0);
  L1042 : do {
    if (!$lnot_i26) {
      var $Ownership = $Helper + 4 | 0;
      var $3 = $__first__i_i_i_i_i;
      var $iter_sroa_0_027 = $1;
      while (1) {
        var $iter_sroa_0_027;
        var $5 = HEAP32[$iter_sroa_0_027 + 16 >> 2];
        HEAP32[$Entry >> 2] = $5;
        HEAP32[__ZNSt3__13mapIP5BlockS2_NS_4lessIS2_EENS_9allocatorINS_4pairIKS2_S2_EEEEEixERS7_($Ownership, $Entry) >> 2] = $5;
        __ZNSt3__16__treeIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEE15__insert_uniqueERKS2_($ref_tmp_i, __ZNSt3__13mapIP5BlockNS_3setIS2_NS_4lessIS2_EENS_9allocatorIS2_EEEES5_NS6_INS_4pairIKS2_S8_EEEEEixERSA_($IndependentGroups, $Entry) | 0, $Entry);
        __ZNSt3__14listIP5BlockNS_9allocatorIS2_EEE9push_backERKS2_($Queue, $Entry);
        var $call_i_i224 = __ZNSt3__111__tree_nextIPKNS_16__tree_node_baseIPvEEEET_S6_($iter_sroa_0_027 | 0);
        if (($call_i_i224 | 0) == ($3 | 0)) {
          break;
        } else {
          var $iter_sroa_0_027 = $call_i_i224;
        }
      }
      if ((HEAP32[$__first__i_i_i_i_i_i$s2] | 0) == 0) {
        break;
      }
      var $__tree__i416 = $Ownership | 0;
      var $agg_tmp2_i411_sroa_0_0_tmp3_i418_idx = $agg_tmp_i414 | 0;
      var $4 = $Helper + 8 | 0;
      while (1) {
        HEAP32[$Curr >> 2] = HEAP32[HEAP32[$__next__i_i_i_i_i >> 2] + 8 >> 2];
        __ZNSt3__14listIP5BlockNS_9allocatorIS2_EEE9pop_frontEv($Queue);
        var $12 = HEAP32[__ZNSt3__13mapIP5BlockS2_NS_4lessIS2_EENS_9allocatorINS_4pairIKS2_S2_EEEEEixERS7_($Ownership, $Curr) >> 2];
        HEAP32[$Owner$s2] = $12;
        var $tobool = ($12 | 0) == 0;
        L1055 : do {
          if (!$tobool) {
            var $14 = HEAP32[$Curr >> 2];
            var $15 = HEAP32[$14 >> 2];
            var $__first__i_i_i_i_i38618 = $14 + 4 | 0;
            if (($15 | 0) == ($__first__i_i_i_i_i38618 | 0)) {
              break;
            }
            var $17 = $__first__i_i_i_i_i38618;
            var $iter24_sroa_0_020 = $15;
            while (1) {
              var $iter24_sroa_0_020;
              HEAP32[$New >> 2] = HEAP32[$iter24_sroa_0_020 + 16 >> 2];
              __ZNSt3__16__treeINS_4pairIP5BlockS3_EENS_19__map_value_compareIS3_S3_NS_4lessIS3_EELb1EEENS_9allocatorIS4_EEE4findIS3_EENS_15__tree_iteratorIS4_PNS_11__tree_nodeIS4_PvEEiEERKT_($agg_tmp_i414, $__tree__i416, $New);
              var $agg_tmp2_i411_sroa_0_0_copyload21 = HEAP32[$agg_tmp2_i411_sroa_0_0_tmp3_i418_idx >> 2];
              do {
                if (($agg_tmp2_i411_sroa_0_0_copyload21 | 0) == ($4 | 0)) {
                  var $19 = HEAP32[$Owner$s2];
                  HEAP32[__ZNSt3__13mapIP5BlockS2_NS_4lessIS2_EENS_9allocatorINS_4pairIKS2_S2_EEEEEixERS7_($Ownership, $New) >> 2] = $19;
                  __ZNSt3__16__treeIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEE15__insert_uniqueERKS2_($ref_tmp_i510, __ZNSt3__13mapIP5BlockNS_3setIS2_NS_4lessIS2_EENS_9allocatorIS2_EEEES5_NS6_INS_4pairIKS2_S8_EEEEEixERSA_($IndependentGroups, $Owner) | 0, $New);
                  __ZNSt3__14listIP5BlockNS_9allocatorIS2_EEE9push_backERKS2_($Queue, $New);
                } else {
                  var $21 = HEAP32[$agg_tmp2_i411_sroa_0_0_copyload21 + 20 >> 2];
                  if (($21 | 0) == 0) {
                    break;
                  }
                  if (($21 | 0) == (HEAP32[$Owner$s2] | 0)) {
                    break;
                  }
                  __ZZZN8Relooper9CalculateEP5BlockEN8Analyzer21FindIndependentGroupsE_0RNSt3__13setIS1_NS3_4lessIS1_EENS3_9allocatorIS1_EEEESA_RNS3_3mapIS1_S9_S6_NS7_INS3_4pairIKS1_S9_EEEEEEEN11HelperClass22InvalidateWithChildrenES1_($Helper, HEAP32[$New >> 2]);
                }
              } while (0);
              var $call_i_i_i546 = __ZNSt3__111__tree_nextIPNS_16__tree_node_baseIPvEEEET_S5_($iter24_sroa_0_020 | 0);
              if (($call_i_i_i546 | 0) == ($17 | 0)) {
                break L1055;
              } else {
                var $iter24_sroa_0_020 = $call_i_i_i546;
              }
            }
          }
        } while (0);
        if ((HEAP32[$__first__i_i_i_i_i_i$s2] | 0) == 0) {
          break L1042;
        }
      }
    }
  } while (0);
  var $26 = HEAP32[$__begin_node__i_i_i$s2];
  if (($26 | 0) == ($2 | 0)) {
    __ZNSt3__14listIP5BlockNS_9allocatorIS2_EEED1Ev($Queue);
    __ZZZN8Relooper9CalculateEP5BlockEN8Analyzer21FindIndependentGroupsE_0RNSt3__13setIS1_NS3_4lessIS1_EENS3_9allocatorIS1_EEEESA_RNS3_3mapIS1_S9_S6_NS7_INS3_4pairIKS1_S9_EEEEEEEN11HelperClassD1Ev($Helper);
    STACKTOP = __stackBase__;
    return;
  }
  var $__prev__i_i_i_i_i800 = $ToInvalidate | 0;
  var $27 = $ToInvalidate;
  var $__next__i_i_i_i_i801 = $ToInvalidate + 4 | 0;
  var $__first__i_i_i_i_i_i807 = $ToInvalidate + 8 | 0;
  var $Ownership102 = $Helper + 4 | 0;
  var $28 = $__first__i_i_i_i_i;
  var $iter66_sroa_0_015 = $26;
  while (1) {
    var $iter66_sroa_0_015;
    var $call76 = __ZNSt3__13mapIP5BlockNS_3setIS2_NS_4lessIS2_EENS_9allocatorIS2_EEEES5_NS6_INS_4pairIKS2_S8_EEEEEixERSA_($IndependentGroups, $iter66_sroa_0_015 + 16 | 0);
    HEAP32[$__prev__i_i_i_i_i800 >> 2] = $27;
    HEAP32[$__next__i_i_i_i_i801 >> 2] = $27;
    HEAP32[$__first__i_i_i_i_i_i807 >> 2] = 0;
    var $29 = HEAP32[$call76 >> 2];
    var $__first__i_i_i_i_i738 = $call76 + 4 | 0;
    var $lnot_i7149 = ($29 | 0) == ($__first__i_i_i_i_i738 | 0);
    L1077 : do {
      if (!$lnot_i7149) {
        var $31 = $__first__i_i_i_i_i738;
        var $iter80_sroa_0_010 = $29;
        while (1) {
          var $iter80_sroa_0_010;
          var $32 = HEAP32[$iter80_sroa_0_010 + 16 >> 2];
          HEAP32[$Child >> 2] = $32;
          var $33 = HEAP32[$32 + 12 >> 2];
          var $lnot_i_i6417 = ($33 | 0) == ($32 + 16 | 0);
          L1081 : do {
            if (!$lnot_i_i6417) {
              var $iter91_sroa_0_08 = $33;
              while (1) {
                var $iter91_sroa_0_08;
                HEAP32[$Parent >> 2] = HEAP32[$iter91_sroa_0_08 + 16 >> 2];
                if ((HEAP32[__ZNSt3__13mapIP5BlockS2_NS_4lessIS2_EENS_9allocatorINS_4pairIKS2_S2_EEEEEixERS7_($Ownership102, $Parent) >> 2] | 0) != (HEAP32[__ZNSt3__13mapIP5BlockS2_NS_4lessIS2_EENS_9allocatorINS_4pairIKS2_S2_EEEEEixERS7_($Ownership102, $Child) >> 2] | 0)) {
                  __ZNSt3__14listIP5BlockNS_9allocatorIS2_EEE9push_backERKS2_($ToInvalidate, $Child);
                }
                var $call_i_i_i = __ZNSt3__111__tree_nextIPNS_16__tree_node_baseIPvEEEET_S5_($iter91_sroa_0_08 | 0);
                if (($call_i_i_i | 0) == (HEAP32[$Child >> 2] + 16 | 0)) {
                  break L1081;
                } else {
                  var $iter91_sroa_0_08 = $call_i_i_i;
                }
              }
            }
          } while (0);
          var $call_i_i479 = __ZNSt3__111__tree_nextIPKNS_16__tree_node_baseIPvEEEET_S6_($iter80_sroa_0_010 | 0);
          if (($call_i_i479 | 0) == ($31 | 0)) {
            break L1077;
          } else {
            var $iter80_sroa_0_010 = $call_i_i479;
          }
        }
      }
    } while (0);
    while (1) {
      if ((HEAP32[$__first__i_i_i_i_i_i807 >> 2] | 0) == 0) {
        break;
      }
      var $48 = HEAP32[HEAP32[$__next__i_i_i_i_i801 >> 2] + 8 >> 2];
      __ZNSt3__14listIP5BlockNS_9allocatorIS2_EEE9pop_frontEv($ToInvalidate);
      __ZZZN8Relooper9CalculateEP5BlockEN8Analyzer21FindIndependentGroupsE_0RNSt3__13setIS1_NS3_4lessIS1_EENS3_9allocatorIS1_EEEESA_RNS3_3mapIS1_S9_S6_NS7_INS3_4pairIKS1_S9_EEEEEEEN11HelperClass22InvalidateWithChildrenES1_($Helper, $48);
    }
    __ZNSt3__14listIP5BlockNS_9allocatorIS2_EEED1Ev($ToInvalidate);
    var $call_i_i361 = __ZNSt3__111__tree_nextIPKNS_16__tree_node_baseIPvEEEET_S6_($iter66_sroa_0_015 | 0);
    if (($call_i_i361 | 0) == ($28 | 0)) {
      break;
    } else {
      var $iter66_sroa_0_015 = $call_i_i361;
    }
  }
  var $_pre28 = HEAP32[$__begin_node__i_i_i$s2];
  if (($_pre28 | 0) == ($2 | 0)) {
    __ZNSt3__14listIP5BlockNS_9allocatorIS2_EEED1Ev($Queue);
    __ZZZN8Relooper9CalculateEP5BlockEN8Analyzer21FindIndependentGroupsE_0RNSt3__13setIS1_NS3_4lessIS1_EENS3_9allocatorIS1_EEEESA_RNS3_3mapIS1_S9_S6_NS7_INS3_4pairIKS1_S9_EEEEEEEN11HelperClassD1Ev($Helper);
    STACKTOP = __stackBase__;
    return;
  }
  var $__tree__i236 = $IndependentGroups | 0;
  var $iter137_sroa_0_02 = $_pre28;
  while (1) {
    var $iter137_sroa_0_02;
    var $__value__i283 = $iter137_sroa_0_02 + 16 | 0;
    if ((HEAP32[__ZNSt3__13mapIP5BlockNS_3setIS2_NS_4lessIS2_EENS_9allocatorIS2_EEEES5_NS6_INS_4pairIKS2_S8_EEEEEixERSA_($IndependentGroups, $__value__i283) + 8 >> 2] | 0) == 0) {
      __ZNSt3__16__treeINS_4pairIP5BlockNS_3setIS3_NS_4lessIS3_EENS_9allocatorIS3_EEEEEENS_19__map_value_compareIS3_S9_S6_Lb1EEENS7_ISA_EEE14__erase_uniqueIS3_EEjRKT_($__tree__i236, $__value__i283);
    }
    var $call_i_i = __ZNSt3__111__tree_nextIPKNS_16__tree_node_baseIPvEEEET_S6_($iter137_sroa_0_02 | 0);
    if (($call_i_i | 0) == ($28 | 0)) {
      break;
    } else {
      var $iter137_sroa_0_02 = $call_i_i;
    }
  }
  __ZNSt3__14listIP5BlockNS_9allocatorIS2_EEED1Ev($Queue);
  __ZZZN8Relooper9CalculateEP5BlockEN8Analyzer21FindIndependentGroupsE_0RNSt3__13setIS1_NS3_4lessIS1_EENS3_9allocatorIS1_EEEESA_RNS3_3mapIS1_S9_S6_NS7_INS3_4pairIKS1_S9_EEEEEEEN11HelperClassD1Ev($Helper);
  STACKTOP = __stackBase__;
  return;
}
__ZZN8Relooper9CalculateEP5BlockEN8Analyzer21FindIndependentGroupsE_0RNSt3__13setIS1_NS3_4lessIS1_EENS3_9allocatorIS1_EEEESA_RNS3_3mapIS1_S9_S6_NS7_INS3_4pairIKS1_S9_EEEEEE["X"] = 1;
function __ZNSt3__13mapIP5BlockNS_3setIS2_NS_4lessIS2_EENS_9allocatorIS2_EEEES5_NS6_INS_4pairIKS2_S8_EEEEEixERSA_($this, $__k) {
  var __stackBase__ = STACKTOP;
  STACKTOP = STACKTOP + 16 | 0;
  var $__parent = __stackBase__;
  var $temp_lvalue = __stackBase__ + 4;
  var $call = __ZNSt3__13mapIP5BlockNS_3setIS2_NS_4lessIS2_EENS_9allocatorIS2_EEEES5_NS6_INS_4pairIKS2_S8_EEEEE16__find_equal_keyERPNS_16__tree_node_baseIPvEERSA_($this, $__parent, $__k);
  var $0 = HEAP32[$call >> 2];
  if (($0 | 0) != 0) {
    var $__r_0 = $0;
    var $__r_0;
    var $second = $__r_0 + 20 | 0;
    STACKTOP = __stackBase__;
    return $second;
  }
  __ZNSt3__13mapIP5BlockNS_3setIS2_NS_4lessIS2_EENS_9allocatorIS2_EEEES5_NS6_INS_4pairIKS2_S8_EEEEE16__construct_nodeERSA_($temp_lvalue, $this, $__k);
  var $__first__i_i_i28_i_i = $temp_lvalue | 0;
  var $2 = HEAP32[$__first__i_i_i28_i_i >> 2];
  HEAP32[$__first__i_i_i28_i_i >> 2] = 0;
  __ZNSt3__16__treeINS_4pairIP5BlockNS_3setIS3_NS_4lessIS3_EENS_9allocatorIS3_EEEEEENS_19__map_value_compareIS3_S9_S6_Lb1EEENS7_ISA_EEE16__insert_node_atEPNS_16__tree_node_baseIPvEERSI_SI_($this | 0, HEAP32[$__parent >> 2], $call, $2 | 0);
  var $__r_0 = $2;
  var $__r_0;
  var $second = $__r_0 + 20 | 0;
  STACKTOP = __stackBase__;
  return $second;
}
function __ZZN8Relooper9CalculateEP5BlockEN8Analyzer12MakeMultipleE_0RNSt3__13setIS1_NS3_4lessIS1_EENS3_9allocatorIS1_EEEESA_RNS3_3mapIS1_S9_S6_NS7_INS3_4pairIKS1_S9_EEEEEEP5ShapeSA_($this, $Blocks, $Entries, $IndependentGroups, $Prev, $NextEntries) {
  var $CurrTarget$s2;
  var $CurrInner$s2;
  var $CurrEntry$s2;
  var __stackBase__ = STACKTOP;
  STACKTOP = STACKTOP + 60 | 0;
  var $ref_tmp_i550 = __stackBase__;
  var $agg_tmp_i486 = __stackBase__ + 8;
  var $ref_tmp_i368 = __stackBase__ + 12;
  var $agg_tmp_i308 = __stackBase__ + 20;
  var $ref_tmp_i = __stackBase__ + 24;
  var $CurrEntries = __stackBase__ + 32;
  var $CurrEntry = __stackBase__ + 44, $CurrEntry$s2 = $CurrEntry >> 2;
  var $CurrInner = __stackBase__ + 48, $CurrInner$s2 = $CurrInner >> 2;
  var $CurrTarget = __stackBase__ + 52, $CurrTarget$s2 = $CurrTarget >> 2;
  var $Entry = __stackBase__ + 56;
  var $call = __ZN5Shape8IsSimpleEPS_($Prev);
  var $call3 = __Znwj(36);
  __ZN13MultipleShapeC1Ev($call3);
  var $1 = $call3;
  __ZZN8Relooper9CalculateEP5BlockEN8Analyzer6NoticeE_0P5Shape(HEAP32[$this >> 2], $1);
  var $__tree__i_i = $CurrEntries | 0;
  __ZNSt3__16__treeIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEEC1ERKS4_($__tree__i_i);
  var $__tree__i = $IndependentGroups | 0;
  var $2 = HEAP32[$IndependentGroups >> 2];
  var $__first__i_i_i_i_i = $IndependentGroups + 4 | 0;
  var $3 = $__first__i_i_i_i_i;
  var $lnot_i_i59 = ($2 | 0) == ($3 | 0);
  L1111 : do {
    if (!$lnot_i_i59) {
      var $4 = $call3 + 20 | 0;
      var $__tree__i212 = $Blocks | 0;
      var $agg_tmp2_i305_sroa_0_0_tmp3_i312_idx = $agg_tmp_i308 | 0;
      var $__tree__i370 = $NextEntries | 0;
      var $5 = $__first__i_i_i_i_i;
      if (($call | 0) == 0) {
        var $iter_sroa_0_060_us = $2;
        while (1) {
          var $iter_sroa_0_060_us;
          HEAP32[$CurrEntry$s2] = HEAP32[$iter_sroa_0_060_us + 16 >> 2];
          var $7 = $iter_sroa_0_060_us + 20 | 0;
          __ZNSt3__16__treeIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEE5clearEv($__tree__i_i);
          __ZNSt3__16__treeIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEE15__insert_uniqueERKS2_($ref_tmp_i, $__tree__i_i, $CurrEntry);
          var $__tree__i154_us = $7 | 0;
          var $8 = HEAP32[$7 >> 2];
          var $__first__i_i_i_i_i191_us = $iter_sroa_0_060_us + 24 | 0;
          var $9 = $__first__i_i_i_i_i191_us;
          var $lnot_i55_us = ($8 | 0) == ($9 | 0);
          L1117 : do {
            if (!$lnot_i55_us) {
              var $10 = $__first__i_i_i_i_i191_us;
              var $iter14_sroa_0_056_us = $8;
              while (1) {
                var $iter14_sroa_0_056_us;
                HEAP32[$CurrInner$s2] = HEAP32[$iter14_sroa_0_056_us + 16 >> 2];
                __ZNSt3__16__treeIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEE14__erase_uniqueIS2_EEjRKT_($__tree__i212, $CurrInner);
                var $12 = HEAP32[$CurrInner$s2];
                var $13 = HEAP32[$12 >> 2];
                var $__first__i_i_i_i_i26452_us = $12 + 4 | 0;
                var $lnot_i_i28653_us = ($13 | 0) == ($__first__i_i_i_i_i26452_us | 0);
                L1121 : do {
                  if (!$lnot_i_i28653_us) {
                    var $15 = $__first__i_i_i_i_i26452_us;
                    var $iter26_sroa_0_054_us = $13;
                    while (1) {
                      var $iter26_sroa_0_054_us;
                      HEAP32[$CurrTarget$s2] = HEAP32[$iter26_sroa_0_054_us + 16 >> 2];
                      var $call_i_i_i_us = __ZNSt3__111__tree_nextIPNS_16__tree_node_baseIPvEEEET_S5_($iter26_sroa_0_054_us | 0);
                      __ZNSt3__16__treeIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEE4findIS2_EENS_15__tree_iteratorIS2_PNS_11__tree_nodeIS2_PvEEiEERKT_($agg_tmp_i308, $__tree__i154_us, $CurrTarget);
                      if ((HEAP32[$agg_tmp2_i305_sroa_0_0_tmp3_i312_idx >> 2] | 0) == ($9 | 0)) {
                        __ZNSt3__16__treeIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEE15__insert_uniqueERKS2_($ref_tmp_i368, $__tree__i370, $CurrTarget);
                        __ZZN8Relooper9CalculateEP5BlockEN8Analyzer9SolipsizeE_0S1_N6Branch8FlowTypeEP5ShapeRNSt3__13setIS1_NS7_4lessIS1_EENS7_9allocatorIS1_EEEE(HEAP32[$CurrTarget$s2], 1, $1, $7);
                      }
                      if (($call_i_i_i_us | 0) == ($15 | 0)) {
                        break L1121;
                      } else {
                        var $iter26_sroa_0_054_us = $call_i_i_i_us;
                      }
                    }
                  }
                } while (0);
                var $call_i_i_us = __ZNSt3__111__tree_nextIPKNS_16__tree_node_baseIPvEEEET_S6_($iter14_sroa_0_056_us | 0);
                if (($call_i_i_us | 0) == ($10 | 0)) {
                  break L1117;
                } else {
                  var $iter14_sroa_0_056_us = $call_i_i_us;
                }
              }
            }
          } while (0);
          var $call52_us = __ZZN8Relooper9CalculateEP5BlockEN8Analyzer7ProcessE_0RNSt3__13setIS1_NS3_4lessIS1_EENS3_9allocatorIS1_EEEESA_P5Shape($this, $7, $CurrEntries);
          HEAP32[__ZNSt3__13mapIP5BlockP5ShapeNS_4lessIS2_EENS_9allocatorINS_4pairIKS2_S4_EEEEEixERS9_($4, $CurrEntry) >> 2] = $call52_us;
          HEAP8[HEAP32[$CurrEntry$s2] + 64 | 0] = 1;
          var $call_i_i_i403_us = __ZNSt3__111__tree_nextIPNS_16__tree_node_baseIPvEEEET_S5_($iter_sroa_0_060_us | 0);
          if (($call_i_i_i403_us | 0) == ($5 | 0)) {
            break L1111;
          } else {
            var $iter_sroa_0_060_us = $call_i_i_i403_us;
          }
        }
      } else {
        var $iter_sroa_0_060 = $2;
        while (1) {
          var $iter_sroa_0_060;
          HEAP32[$CurrEntry$s2] = HEAP32[$iter_sroa_0_060 + 16 >> 2];
          var $26 = $iter_sroa_0_060 + 20 | 0;
          __ZNSt3__16__treeIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEE5clearEv($__tree__i_i);
          __ZNSt3__16__treeIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEE15__insert_uniqueERKS2_($ref_tmp_i, $__tree__i_i, $CurrEntry);
          var $__tree__i154 = $26 | 0;
          var $27 = HEAP32[$26 >> 2];
          var $__first__i_i_i_i_i191 = $iter_sroa_0_060 + 24 | 0;
          var $28 = $__first__i_i_i_i_i191;
          var $lnot_i55 = ($27 | 0) == ($28 | 0);
          L1136 : do {
            if (!$lnot_i55) {
              var $29 = $__first__i_i_i_i_i191;
              var $iter14_sroa_0_056 = $27;
              while (1) {
                var $iter14_sroa_0_056;
                HEAP32[$CurrInner$s2] = HEAP32[$iter14_sroa_0_056 + 16 >> 2];
                __ZNSt3__16__treeIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEE14__erase_uniqueIS2_EEjRKT_($__tree__i212, $CurrInner);
                var $31 = HEAP32[$CurrInner$s2];
                var $32 = HEAP32[$31 >> 2];
                var $__first__i_i_i_i_i26452 = $31 + 4 | 0;
                var $lnot_i_i28653 = ($32 | 0) == ($__first__i_i_i_i_i26452 | 0);
                L1140 : do {
                  if (!$lnot_i_i28653) {
                    var $34 = $__first__i_i_i_i_i26452;
                    var $iter26_sroa_0_054 = $32;
                    while (1) {
                      var $iter26_sroa_0_054;
                      HEAP32[$CurrTarget$s2] = HEAP32[$iter26_sroa_0_054 + 16 >> 2];
                      var $call_i_i_i = __ZNSt3__111__tree_nextIPNS_16__tree_node_baseIPvEEEET_S5_($iter26_sroa_0_054 | 0);
                      __ZNSt3__16__treeIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEE4findIS2_EENS_15__tree_iteratorIS2_PNS_11__tree_nodeIS2_PvEEiEERKT_($agg_tmp_i308, $__tree__i154, $CurrTarget);
                      if ((HEAP32[$agg_tmp2_i305_sroa_0_0_tmp3_i312_idx >> 2] | 0) == ($28 | 0)) {
                        __ZNSt3__16__treeIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEE15__insert_uniqueERKS2_($ref_tmp_i368, $__tree__i370, $CurrTarget);
                        __ZZN8Relooper9CalculateEP5BlockEN8Analyzer9SolipsizeE_0S1_N6Branch8FlowTypeEP5ShapeRNSt3__13setIS1_NS7_4lessIS1_EENS7_9allocatorIS1_EEEE(HEAP32[$CurrTarget$s2], 1, $1, $26);
                      }
                      if (($call_i_i_i | 0) == ($34 | 0)) {
                        break L1140;
                      } else {
                        var $iter26_sroa_0_054 = $call_i_i_i;
                      }
                    }
                  }
                } while (0);
                var $call_i_i = __ZNSt3__111__tree_nextIPKNS_16__tree_node_baseIPvEEEET_S6_($iter14_sroa_0_056 | 0);
                if (($call_i_i | 0) == ($29 | 0)) {
                  break L1136;
                } else {
                  var $iter14_sroa_0_056 = $call_i_i;
                }
              }
            }
          } while (0);
          var $call52 = __ZZN8Relooper9CalculateEP5BlockEN8Analyzer7ProcessE_0RNSt3__13setIS1_NS3_4lessIS1_EENS3_9allocatorIS1_EEEESA_P5Shape($this, $26, $CurrEntries);
          HEAP32[__ZNSt3__13mapIP5BlockP5ShapeNS_4lessIS2_EENS_9allocatorINS_4pairIKS2_S4_EEEEEixERS9_($4, $CurrEntry) >> 2] = $call52;
          var $call_i_i_i403 = __ZNSt3__111__tree_nextIPNS_16__tree_node_baseIPvEEEET_S5_($iter_sroa_0_060 | 0);
          if (($call_i_i_i403 | 0) == ($5 | 0)) {
            break L1111;
          } else {
            var $iter_sroa_0_060 = $call_i_i_i403;
          }
        }
      }
    }
  } while (0);
  var $43 = HEAP32[$Entries >> 2];
  var $__first__i_i_i_i_i456 = $Entries + 4 | 0;
  if (($43 | 0) == ($__first__i_i_i_i_i456 | 0)) {
    __ZNSt3__13setIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEED1Ev($CurrEntries);
    STACKTOP = __stackBase__;
    return $1;
  }
  var $agg_tmp2_i483_sroa_0_0_tmp3_i490_idx = $agg_tmp_i486 | 0;
  var $__tree__i552 = $NextEntries | 0;
  var $45 = $__first__i_i_i_i_i456;
  var $iter63_sroa_0_048 = $43;
  while (1) {
    var $iter63_sroa_0_048;
    HEAP32[$Entry >> 2] = HEAP32[$iter63_sroa_0_048 + 16 >> 2];
    __ZNSt3__16__treeINS_4pairIP5BlockNS_3setIS3_NS_4lessIS3_EENS_9allocatorIS3_EEEEEENS_19__map_value_compareIS3_S9_S6_Lb1EEENS7_ISA_EEE4findIS3_EENS_15__tree_iteratorISA_PNS_11__tree_nodeISA_PvEEiEERKT_($agg_tmp_i486, $__tree__i, $Entry);
    if ((HEAP32[$agg_tmp2_i483_sroa_0_0_tmp3_i490_idx >> 2] | 0) == ($3 | 0)) {
      __ZNSt3__16__treeIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEE15__insert_uniqueERKS2_($ref_tmp_i550, $__tree__i552, $Entry);
    }
    var $call_i_i576 = __ZNSt3__111__tree_nextIPKNS_16__tree_node_baseIPvEEEET_S6_($iter63_sroa_0_048 | 0);
    if (($call_i_i576 | 0) == ($45 | 0)) {
      break;
    } else {
      var $iter63_sroa_0_048 = $call_i_i576;
    }
  }
  __ZNSt3__13setIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEED1Ev($CurrEntries);
  STACKTOP = __stackBase__;
  return $1;
}
__ZZN8Relooper9CalculateEP5BlockEN8Analyzer12MakeMultipleE_0RNSt3__13setIS1_NS3_4lessIS1_EENS3_9allocatorIS1_EEEESA_RNS3_3mapIS1_S9_S6_NS7_INS3_4pairIKS1_S9_EEEEEEP5ShapeSA_["X"] = 1;
function __ZNSt3__13mapIP5BlockNS_3setIS2_NS_4lessIS2_EENS_9allocatorIS2_EEEES5_NS6_INS_4pairIKS2_S8_EEEEED1Ev($this) {
  __ZNSt3__13mapIP5BlockNS_3setIS2_NS_4lessIS2_EENS_9allocatorIS2_EEEES5_NS6_INS_4pairIKS2_S8_EEEEED2Ev($this);
  return;
}
function __ZN12LabeledShapeD1Ev($this) {
  return;
}
function __ZN5ShapeD1Ev($this) {
  return;
}
function __ZNSt3__16__treeINS_4pairIP5BlockNS_3setIS3_NS_4lessIS3_EENS_9allocatorIS3_EEEEEENS_19__map_value_compareIS3_S9_S6_Lb1EEENS7_ISA_EEE13__lower_boundIS3_EENS_15__tree_iteratorISA_PNS_11__tree_nodeISA_PvEEiEERKT_SK_SK_($agg_result, $__v, $__root, $__result) {
  var $__root_addr_06$s2;
  var $cmp59 = ($__root | 0) == 0;
  L1166 : do {
    if ($cmp59) {
      var $__result_addr_0_ph8 = $__result;
    } else {
      var $0 = HEAP32[$__v >> 2];
      var $__root_addr_0_ph10 = $__root;
      var $__result_addr_0_ph11 = $__result;
      while (1) {
        var $__result_addr_0_ph11;
        var $__root_addr_0_ph10;
        var $__root_addr_06 = $__root_addr_0_ph10, $__root_addr_06$s2 = $__root_addr_06 >> 2;
        while (1) {
          var $__root_addr_06;
          if (HEAP32[$__root_addr_06$s2 + 4] >>> 0 >= $0 >>> 0) {
            break;
          }
          var $4 = HEAP32[$__root_addr_06$s2 + 1];
          if (($4 | 0) == 0) {
            var $__result_addr_0_ph8 = $__result_addr_0_ph11;
            break L1166;
          } else {
            var $__root_addr_06 = $4, $__root_addr_06$s2 = $__root_addr_06 >> 2;
          }
        }
        var $2 = HEAP32[$__root_addr_06$s2];
        if (($2 | 0) == 0) {
          var $__result_addr_0_ph8 = $__root_addr_06;
          break L1166;
        } else {
          var $__root_addr_0_ph10 = $2;
          var $__result_addr_0_ph11 = $__root_addr_06;
        }
      }
    }
  } while (0);
  var $__result_addr_0_ph8;
  HEAP32[$agg_result >> 2] = $__result_addr_0_ph8;
  return;
}
function __ZNSt3__13mapIP5BlockP5ShapeNS_4lessIS2_EENS_9allocatorINS_4pairIKS2_S4_EEEEE16__find_equal_keyERPNS_16__tree_node_baseIPvEERS9_($this, $__parent, $__k) {
  var label = 0;
  var $__first__i_i_i_i = $this + 4 | 0;
  var $__left__i = $__first__i_i_i_i | 0;
  var $0 = HEAP32[$__left__i >> 2];
  if (($0 | 0) == 0) {
    HEAP32[$__parent >> 2] = $__first__i_i_i_i;
    var $retval_0 = $__left__i;
    var $retval_0;
    return $retval_0;
  }
  var $1 = HEAP32[$__k >> 2];
  var $__nd_0_in = $0;
  while (1) {
    var $__nd_0_in;
    var $2 = HEAP32[$__nd_0_in + 16 >> 2];
    if ($1 >>> 0 < $2 >>> 0) {
      var $__left_ = $__nd_0_in | 0;
      var $3 = HEAP32[$__left_ >> 2];
      if (($3 | 0) == 0) {
        label = 980;
        break;
      } else {
        var $__nd_0_in = $3;
        continue;
      }
    }
    if ($2 >>> 0 >= $1 >>> 0) {
      label = 984;
      break;
    }
    var $__right_ = $__nd_0_in + 4 | 0;
    var $4 = HEAP32[$__right_ >> 2];
    if (($4 | 0) == 0) {
      label = 983;
      break;
    } else {
      var $__nd_0_in = $4;
    }
  }
  if (label == 984) {
    HEAP32[$__parent >> 2] = $__nd_0_in;
    var $retval_0 = $__parent;
    var $retval_0;
    return $retval_0;
  } else if (label == 980) {
    HEAP32[$__parent >> 2] = $__nd_0_in;
    var $retval_0 = $__left_;
    var $retval_0;
    return $retval_0;
  } else if (label == 983) {
    HEAP32[$__parent >> 2] = $__nd_0_in;
    var $retval_0 = $__right_;
    var $retval_0;
    return $retval_0;
  }
}
function __ZNSt3__16__treeINS_4pairIP5BlockP5ShapeEENS_19__map_value_compareIS3_S5_NS_4lessIS3_EELb1EEENS_9allocatorIS6_EEEC2ERKSA_($this) {
  var $__first__i_i_i = $this + 4 | 0;
  HEAP32[$__first__i_i_i >> 2] = 0;
  HEAP32[$this + 8 >> 2] = 0;
  HEAP32[$this >> 2] = $__first__i_i_i;
  return;
}
function __ZN5ShapeC2ENS_9ShapeTypeE($this, $TypeInit) {
  HEAP32[$this >> 2] = 5244376;
  var $1 = HEAP32[1311192];
  HEAP32[1311192] = $1 + 1 | 0;
  HEAP32[$this + 4 >> 2] = $1;
  HEAP32[$this + 8 >> 2] = 0;
  HEAP32[$this + 12 >> 2] = $TypeInit;
  return;
}
function __ZNSt3__13mapIP5BlockNS_3setIS2_NS_4lessIS2_EENS_9allocatorIS2_EEEES5_NS6_INS_4pairIKS2_S8_EEEEED2Ev($this) {
  __ZNSt3__16__treeINS_4pairIP5BlockNS_3setIS3_NS_4lessIS3_EENS_9allocatorIS3_EEEEEENS_19__map_value_compareIS3_S9_S6_Lb1EEENS7_ISA_EEED1Ev(HEAP32[$this + 4 >> 2]);
  return;
}
function __ZNSt3__16__treeINS_4pairIP5BlockNS_3setIS3_NS_4lessIS3_EENS_9allocatorIS3_EEEEEENS_19__map_value_compareIS3_S9_S6_Lb1EEENS7_ISA_EEED1Ev($this_0_1_0_0_0_val) {
  __ZNSt3__16__treeINS_4pairIP5BlockNS_3setIS3_NS_4lessIS3_EENS_9allocatorIS3_EEEEEENS_19__map_value_compareIS3_S9_S6_Lb1EEENS7_ISA_EEED2Ev($this_0_1_0_0_0_val);
  return;
}
function __ZNSt3__16__treeINS_4pairIP5BlockNS_3setIS3_NS_4lessIS3_EENS_9allocatorIS3_EEEEEENS_19__map_value_compareIS3_S9_S6_Lb1EEENS7_ISA_EEED2Ev($this_0_1_0_0_0_val) {
  __ZNSt3__16__treeINS_4pairIP5BlockNS_3setIS3_NS_4lessIS3_EENS_9allocatorIS3_EEEEEENS_19__map_value_compareIS3_S9_S6_Lb1EEENS7_ISA_EEE7destroyEPNS_11__tree_nodeISA_PvEE($this_0_1_0_0_0_val);
  return;
}
function __ZNSt3__16__treeINS_4pairIP5BlockNS_3setIS3_NS_4lessIS3_EENS_9allocatorIS3_EEEEEENS_19__map_value_compareIS3_S9_S6_Lb1EEENS7_ISA_EEE7destroyEPNS_11__tree_nodeISA_PvEE($__nd) {
  if (($__nd | 0) == 0) {
    return;
  } else {
    __ZNSt3__16__treeINS_4pairIP5BlockNS_3setIS3_NS_4lessIS3_EENS_9allocatorIS3_EEEEEENS_19__map_value_compareIS3_S9_S6_Lb1EEENS7_ISA_EEE7destroyEPNS_11__tree_nodeISA_PvEE(HEAP32[$__nd >> 2]);
    __ZNSt3__16__treeINS_4pairIP5BlockNS_3setIS3_NS_4lessIS3_EENS_9allocatorIS3_EEEEEENS_19__map_value_compareIS3_S9_S6_Lb1EEENS7_ISA_EEE7destroyEPNS_11__tree_nodeISA_PvEE(HEAP32[$__nd + 4 >> 2]);
    __ZNSt3__14pairIP5BlockNS_3setIS2_NS_4lessIS2_EENS_9allocatorIS2_EEEEED1Ev($__nd + 16 | 0);
    __ZdlPv($__nd);
    return;
  }
}
function __ZNSt3__14pairIP5BlockNS_3setIS2_NS_4lessIS2_EENS_9allocatorIS2_EEEEED1Ev($this) {
  __ZNSt3__14pairIP5BlockNS_3setIS2_NS_4lessIS2_EENS_9allocatorIS2_EEEEED2Ev($this);
  return;
}
function __ZNSt3__14pairIP5BlockNS_3setIS2_NS_4lessIS2_EENS_9allocatorIS2_EEEEED2Ev($this) {
  __ZNSt3__13setIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEED1Ev($this + 4 | 0);
  return;
}
function __ZN13MultipleShapeC1Ev($this) {
  __ZN13MultipleShapeC2Ev($this);
  return;
}
function __ZZN8Relooper9CalculateEP5BlockEN8Analyzer6NoticeE_0P5Shape($this_0_0_0_val, $New) {
  var __stackBase__ = STACKTOP;
  STACKTOP = STACKTOP + 4 | 0;
  var $New_addr = __stackBase__;
  HEAP32[$New_addr >> 2] = $New;
  __ZNSt3__15dequeIP5ShapeNS_9allocatorIS2_EEE9push_backERKS2_($this_0_0_0_val + 24 | 0, $New_addr);
  STACKTOP = __stackBase__;
  return;
}
function __ZZN8Relooper9CalculateEP5BlockEN8Analyzer9SolipsizeE_0S1_N6Branch8FlowTypeEP5ShapeRNSt3__13setIS1_NS7_4lessIS1_EENS7_9allocatorIS1_EEEE($Target, $Type, $Ancestor, $From) {
  var $Prior$s2;
  var $Target_addr$s2;
  var __stackBase__ = STACKTOP;
  STACKTOP = STACKTOP + 12 | 0;
  var $agg_tmp_i66 = __stackBase__;
  var $Target_addr = __stackBase__ + 4, $Target_addr$s2 = $Target_addr >> 2;
  var $Prior = __stackBase__ + 8, $Prior$s2 = $Prior >> 2;
  HEAP32[$Target_addr$s2] = $Target;
  var $0 = HEAP32[$Target + 12 >> 2];
  if (($0 | 0) == ($Target + 16 | 0)) {
    STACKTOP = __stackBase__;
    return;
  }
  var $__tree__i68 = $From | 0;
  var $agg_tmp2_i63_sroa_0_0_tmp3_i70_idx = $agg_tmp_i66 | 0;
  var $2 = $From + 4 | 0;
  var $iter_sroa_0_03 = $0;
  var $3 = $Target;
  while (1) {
    var $3;
    var $iter_sroa_0_03;
    HEAP32[$Prior$s2] = HEAP32[$iter_sroa_0_03 + 16 >> 2];
    __ZNSt3__16__treeIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEE4findIS2_EENS_15__tree_iteratorIS2_PNS_11__tree_nodeIS2_PvEEiEERKT_($agg_tmp_i66, $__tree__i68, $Prior);
    if ((HEAP32[$agg_tmp2_i63_sroa_0_0_tmp3_i70_idx >> 2] | 0) == ($2 | 0)) {
      var $iter_sroa_0_0_be_in = __ZNSt3__111__tree_nextIPNS_16__tree_node_baseIPvEEEET_S5_($iter_sroa_0_03 | 0);
      var $6 = $3;
    } else {
      var $9 = HEAP32[$iter_sroa_0_03 + 20 >> 2];
      var $11 = HEAP32[__ZNSt3__13mapIP5BlockP6BranchNS_4lessIS2_EENS_9allocatorINS_4pairIKS2_S4_EEEEEixERS9_(HEAP32[$Prior$s2] | 0, $Target_addr) >> 2];
      HEAP32[$11 >> 2] = $Ancestor;
      HEAP32[$11 + 4 >> 2] = $Type;
      var $call14 = __ZN5Shape10IsMultipleEPS_($Ancestor);
      if (($call14 | 0) != 0) {
        var $NeedLoop = $call14 + 32 | 0;
        HEAP32[$NeedLoop >> 2] = HEAP32[$NeedLoop >> 2] + 1 | 0;
      }
      var $call_i_i_i = __ZNSt3__111__tree_nextIPNS_16__tree_node_baseIPvEEEET_S5_($iter_sroa_0_03 | 0);
      var $14 = HEAP32[$Target_addr$s2];
      __ZNSt3__16__treeINS_4pairIP5BlockP6BranchEENS_19__map_value_compareIS3_S5_NS_4lessIS3_EELb1EEENS_9allocatorIS6_EEE14__erase_uniqueIS3_EEjRKT_($14 + 12 | 0, $Prior);
      HEAP32[__ZNSt3__13mapIP5BlockP6BranchNS_4lessIS2_EENS_9allocatorINS_4pairIKS2_S4_EEEEEixERS9_($14 + 36 | 0, $Prior) >> 2] = $9;
      var $15 = HEAP32[$Prior$s2];
      __ZNSt3__16__treeINS_4pairIP5BlockP6BranchEENS_19__map_value_compareIS3_S5_NS_4lessIS3_EELb1EEENS_9allocatorIS6_EEE14__erase_uniqueIS3_EEjRKT_($15 | 0, $Target_addr);
      HEAP32[__ZNSt3__13mapIP5BlockP6BranchNS_4lessIS2_EENS_9allocatorINS_4pairIKS2_S4_EEEEEixERS9_($15 + 24 | 0, $Target_addr) >> 2] = $11;
      var $iter_sroa_0_0_be_in = $call_i_i_i;
      var $6 = HEAP32[$Target_addr$s2];
    }
    var $6;
    var $iter_sroa_0_0_be_in;
    if (($iter_sroa_0_0_be_in | 0) == ($6 + 16 | 0)) {
      break;
    } else {
      var $iter_sroa_0_03 = $iter_sroa_0_0_be_in;
      var $3 = $6;
    }
  }
  STACKTOP = __stackBase__;
  return;
}
__ZZN8Relooper9CalculateEP5BlockEN8Analyzer9SolipsizeE_0S1_N6Branch8FlowTypeEP5ShapeRNSt3__13setIS1_NS7_4lessIS1_EENS7_9allocatorIS1_EEEE["X"] = 1;
function __ZNSt3__13mapIP5BlockP5ShapeNS_4lessIS2_EENS_9allocatorINS_4pairIKS2_S4_EEEEEixERS9_($this, $__k) {
  var __stackBase__ = STACKTOP;
  STACKTOP = STACKTOP + 16 | 0;
  var $__parent = __stackBase__;
  var $temp_lvalue = __stackBase__ + 4;
  var $call = __ZNSt3__13mapIP5BlockP5ShapeNS_4lessIS2_EENS_9allocatorINS_4pairIKS2_S4_EEEEE16__find_equal_keyERPNS_16__tree_node_baseIPvEERS9_($this, $__parent, $__k);
  var $0 = HEAP32[$call >> 2];
  if (($0 | 0) != 0) {
    var $__r_0 = $0;
    var $__r_0;
    var $second = $__r_0 + 20 | 0;
    STACKTOP = __stackBase__;
    return $second;
  }
  __ZNSt3__13mapIP5BlockP5ShapeNS_4lessIS2_EENS_9allocatorINS_4pairIKS2_S4_EEEEE16__construct_nodeERS9_($temp_lvalue, $this, $__k);
  var $__first__i_i_i28_i_i = $temp_lvalue | 0;
  var $2 = HEAP32[$__first__i_i_i28_i_i >> 2];
  HEAP32[$__first__i_i_i28_i_i >> 2] = 0;
  __ZNSt3__16__treeINS_4pairIP5BlockP5ShapeEENS_19__map_value_compareIS3_S5_NS_4lessIS3_EELb1EEENS_9allocatorIS6_EEE16__insert_node_atEPNS_16__tree_node_baseIPvEERSH_SH_($this | 0, HEAP32[$__parent >> 2], $call, $2 | 0);
  var $__r_0 = $2;
  var $__r_0;
  var $second = $__r_0 + 20 | 0;
  STACKTOP = __stackBase__;
  return $second;
}
function __ZNSt3__16__treeINS_4pairIP5BlockNS_3setIS3_NS_4lessIS3_EENS_9allocatorIS3_EEEEEENS_19__map_value_compareIS3_S9_S6_Lb1EEENS7_ISA_EEE4findIS3_EENS_15__tree_iteratorISA_PNS_11__tree_nodeISA_PvEEiEERKT_($agg_result, $this, $__v) {
  var __stackBase__ = STACKTOP;
  STACKTOP = STACKTOP + 4 | 0;
  var $__p = __stackBase__;
  var $__first__i_i_i_i = $this + 4 | 0;
  var $0 = $__first__i_i_i_i;
  __ZNSt3__16__treeINS_4pairIP5BlockNS_3setIS3_NS_4lessIS3_EENS_9allocatorIS3_EEEEEENS_19__map_value_compareIS3_S9_S6_Lb1EEENS7_ISA_EEE13__lower_boundIS3_EENS_15__tree_iteratorISA_PNS_11__tree_nodeISA_PvEEiEERKT_SK_SK_($__p, $__v, HEAP32[$__first__i_i_i_i >> 2], $0);
  var $3 = HEAP32[$__p >> 2];
  do {
    if (($3 | 0) != ($0 | 0)) {
      if (HEAP32[$__v >> 2] >>> 0 < HEAP32[$3 + 16 >> 2] >>> 0) {
        break;
      }
      HEAP32[$agg_result >> 2] = $3;
      STACKTOP = __stackBase__;
      return;
    }
  } while (0);
  HEAP32[$agg_result >> 2] = $0;
  STACKTOP = __stackBase__;
  return;
}
function __ZNSt3__13mapIP5BlockP5ShapeNS_4lessIS2_EENS_9allocatorINS_4pairIKS2_S4_EEEEE16__construct_nodeERS9_($agg_result, $this, $__k) {
  var $call_i_i = __Znwj(24);
  var $__value_ = $call_i_i + 16 | 0;
  if (($__value_ | 0) != 0) {
    HEAP32[$__value_ >> 2] = HEAP32[$__k >> 2];
  }
  var $second = $call_i_i + 20 | 0;
  if (($second | 0) != 0) {
    HEAP32[$second >> 2] = 0;
  }
  HEAP32[$agg_result >> 2] = $call_i_i;
  HEAP32[$agg_result + 4 >> 2] = $this + 4 | 0;
  HEAP8[$agg_result + 8 | 0] = 1;
  HEAP8[$agg_result + 9 | 0] = 1;
  return;
}
function __ZNSt3__16__treeINS_4pairIP5BlockP5ShapeEENS_19__map_value_compareIS3_S5_NS_4lessIS3_EELb1EEENS_9allocatorIS6_EEE16__insert_node_atEPNS_16__tree_node_baseIPvEERSH_SH_($this, $__parent, $__child, $__new_node) {
  HEAP32[$__new_node >> 2] = 0;
  HEAP32[$__new_node + 4 >> 2] = 0;
  HEAP32[$__new_node + 8 >> 2] = $__parent;
  HEAP32[$__child >> 2] = $__new_node;
  var $__begin_node__i42 = $this | 0;
  var $1 = HEAP32[HEAP32[$__begin_node__i42 >> 2] >> 2];
  if (($1 | 0) == 0) {
    var $3 = $__new_node;
  } else {
    HEAP32[$__begin_node__i42 >> 2] = $1;
    var $3 = HEAP32[$__child >> 2];
  }
  var $3;
  __ZNSt3__127__tree_balance_after_insertIPNS_16__tree_node_baseIPvEEEEvT_S5_(HEAP32[$this + 4 >> 2], $3);
  var $__first__i_i_i = $this + 8 | 0;
  HEAP32[$__first__i_i_i >> 2] = HEAP32[$__first__i_i_i >> 2] + 1 | 0;
  return;
}
function __ZNSt3__16__treeINS_4pairIP5BlockP6BranchEENS_19__map_value_compareIS3_S5_NS_4lessIS3_EELb1EEENS_9allocatorIS6_EEE14__erase_uniqueIS3_EEjRKT_($this, $__k) {
  var __stackBase__ = STACKTOP;
  STACKTOP = STACKTOP + 8 | 0;
  var $__i = __stackBase__;
  __ZNSt3__16__treeINS_4pairIP5BlockP6BranchEENS_19__map_value_compareIS3_S5_NS_4lessIS3_EELb1EEENS_9allocatorIS6_EEE4findIS3_EENS_15__tree_iteratorIS6_PNS_11__tree_nodeIS6_PvEEiEERKT_($__i, $this, $__k);
  var $1 = HEAP32[$__i >> 2];
  if (($1 | 0) == ($this + 4 | 0)) {
    STACKTOP = __stackBase__;
    return;
  }
  __ZNSt3__16__treeINS_4pairIP5BlockP6BranchEENS_19__map_value_compareIS3_S5_NS_4lessIS3_EELb1EEENS_9allocatorIS6_EEE5eraseENS_21__tree_const_iteratorIS6_PKNS_11__tree_nodeIS6_PvEEiEE(__stackBase__ + 4, $this, $1);
  STACKTOP = __stackBase__;
  return;
}
function __ZNSt3__16__treeINS_4pairIP5BlockP6BranchEENS_19__map_value_compareIS3_S5_NS_4lessIS3_EELb1EEENS_9allocatorIS6_EEE5eraseENS_21__tree_const_iteratorIS6_PKNS_11__tree_nodeIS6_PvEEiEE($agg_result, $this, $__p_0) {
  var $0 = $__p_0 | 0;
  var $1 = __ZNSt3__111__tree_nextIPNS_16__tree_node_baseIPvEEEET_S5_($0);
  HEAP32[$agg_result >> 2] = $1;
  var $__begin_node__i = $this | 0;
  if ((HEAP32[$__begin_node__i >> 2] | 0) == ($__p_0 | 0)) {
    HEAP32[$__begin_node__i >> 2] = $1;
  }
  var $__first__i_i_i45 = $this + 8 | 0;
  HEAP32[$__first__i_i_i45 >> 2] = HEAP32[$__first__i_i_i45 >> 2] - 1 | 0;
  __ZNSt3__113__tree_removeIPNS_16__tree_node_baseIPvEEEEvT_S5_(HEAP32[$this + 4 >> 2], $0);
  __ZdlPv($__p_0);
  return;
}
function __ZNSt3__113__tree_removeIPNS_16__tree_node_baseIPvEEEEvT_S5_($__root, $__z) {
  var $__w_2$s2;
  var $__w_4$s2;
  var $__parent__i344$s2;
  var $__parent__i376_pre_phi$s2;
  var label = 0;
  var $__left_ = $__z | 0;
  var $0 = HEAP32[$__left_ >> 2];
  do {
    if (($0 | 0) == 0) {
      var $cond123 = $__z;
      var $__left_9128 = $__left_;
      label = 1045;
    } else {
      if ((HEAP32[$__z + 4 >> 2] | 0) == 0) {
        var $cond20108 = $0;
        var $cond124 = $__z;
        var $__left_9127 = $__left_;
        label = 1047;
        break;
      }
      var $call7 = __ZNSt3__111__tree_nextIPNS_16__tree_node_baseIPvEEEET_S5_($__z);
      var $__left_9_phi_trans_insert = $call7 | 0;
      var $_pre = HEAP32[$__left_9_phi_trans_insert >> 2];
      if (($_pre | 0) == 0) {
        var $cond123 = $call7;
        var $__left_9128 = $__left_9_phi_trans_insert;
        label = 1045;
        break;
      } else {
        var $cond20108 = $_pre;
        var $cond124 = $call7;
        var $__left_9127 = $__left_9_phi_trans_insert;
        label = 1047;
        break;
      }
    }
  } while (0);
  do {
    if (label == 1045) {
      var $__left_9128;
      var $cond123;
      var $2 = HEAP32[$cond123 + 4 >> 2];
      if (($2 | 0) != 0) {
        var $cond20108 = $2;
        var $cond124 = $cond123;
        var $__left_9127 = $__left_9128;
        label = 1047;
        break;
      }
      var $cond20109 = 0;
      var $cmp30110 = 0;
      var $__parent__i376_pre_phi = $cond123 + 8 | 0, $__parent__i376_pre_phi$s2 = $__parent__i376_pre_phi >> 2;
      var $cond125 = $cond123;
      var $__left_9126 = $__left_9128;
      break;
    }
  } while (0);
  if (label == 1047) {
    var $__left_9127;
    var $cond124;
    var $cond20108;
    var $__parent_ = $cond124 + 8 | 0;
    HEAP32[$cond20108 + 8 >> 2] = HEAP32[$__parent_ >> 2];
    var $cond20109 = $cond20108;
    var $cmp30110 = 1;
    var $__parent__i376_pre_phi = $__parent_, $__parent__i376_pre_phi$s2 = $__parent__i376_pre_phi >> 2;
    var $cond125 = $cond124;
    var $__left_9126 = $__left_9127;
  }
  var $__left_9126;
  var $cond125;
  var $__parent__i376_pre_phi;
  var $cmp30110;
  var $cond20109;
  var $4 = HEAP32[$__parent__i376_pre_phi$s2];
  var $__left__i377 = $4 | 0;
  do {
    if (($cond125 | 0) == (HEAP32[$__left__i377 >> 2] | 0)) {
      HEAP32[$__left__i377 >> 2] = $cond20109;
      if (($cond125 | 0) == ($__root | 0)) {
        var $__root_addr_0 = $cond20109;
        var $__w_0 = 0;
        break;
      }
      var $__root_addr_0 = $__root;
      var $__w_0 = HEAP32[HEAP32[$__parent__i376_pre_phi$s2] + 4 >> 2];
    } else {
      HEAP32[$4 + 4 >> 2] = $cond20109;
      var $__root_addr_0 = $__root;
      var $__w_0 = HEAP32[HEAP32[$__parent__i376_pre_phi$s2] >> 2];
    }
  } while (0);
  var $__w_0;
  var $__root_addr_0;
  var $__is_black_ = $cond125 + 12 | 0;
  var $tobool = (HEAP8[$__is_black_] & 1) << 24 >> 24 == 0;
  if (($cond125 | 0) == ($__z | 0)) {
    var $__root_addr_1 = $__root_addr_0;
  } else {
    var $__parent_50 = $__z + 8 | 0;
    var $12 = HEAP32[$__parent_50 >> 2];
    HEAP32[$__parent__i376_pre_phi$s2] = $12;
    if ((HEAP32[HEAP32[$__parent_50 >> 2] >> 2] | 0) == ($__z | 0)) {
      HEAP32[$12 >> 2] = $cond125;
    } else {
      HEAP32[$12 + 4 >> 2] = $cond125;
    }
    var $15 = HEAP32[$__left_ >> 2];
    HEAP32[$__left_9126 >> 2] = $15;
    HEAP32[$15 + 8 >> 2] = $cond125;
    var $16 = HEAP32[$__z + 4 >> 2];
    HEAP32[$cond125 + 4 >> 2] = $16;
    if (($16 | 0) != 0) {
      HEAP32[$16 + 8 >> 2] = $cond125;
    }
    HEAP8[$__is_black_] = HEAP8[$__z + 12 | 0] & 1;
    var $__root_addr_1 = ($__root_addr_0 | 0) == ($__z | 0) ? $cond125 : $__root_addr_0;
  }
  var $__root_addr_1;
  if ($tobool | ($__root_addr_1 | 0) == 0) {
    return;
  }
  if ($cmp30110) {
    HEAP8[$cond20109 + 12 | 0] = 1;
    return;
  } else {
    var $__root_addr_2 = $__root_addr_1;
    var $__w_1 = $__w_0;
  }
  while (1) {
    var $__w_1;
    var $__root_addr_2;
    var $__parent__i344$s2 = ($__w_1 + 8 | 0) >> 2;
    var $19 = HEAP32[$__parent__i344$s2];
    var $__is_black_184 = $__w_1 + 12 | 0;
    var $tobool185 = (HEAP8[$__is_black_184] & 1) << 24 >> 24 != 0;
    if (($__w_1 | 0) == (HEAP32[$19 >> 2] | 0)) {
      if ($tobool185) {
        var $__root_addr_6 = $__root_addr_2;
        var $__w_4 = $__w_1, $__w_4$s2 = $__w_4 >> 2;
      } else {
        HEAP8[$__is_black_184] = 1;
        HEAP8[$19 + 12 | 0] = 0;
        __ZNSt3__119__tree_right_rotateIPNS_16__tree_node_baseIPvEEEEvT_(HEAP32[$__parent__i344$s2]);
        var $47 = HEAP32[$__w_1 + 4 >> 2];
        var $__root_addr_6 = ($__root_addr_2 | 0) == ($47 | 0) ? $__w_1 : $__root_addr_2;
        var $__w_4 = HEAP32[$47 >> 2], $__w_4$s2 = $__w_4 >> 2;
      }
      var $__w_4;
      var $__root_addr_6;
      var $49 = HEAP32[$__w_4$s2];
      var $cmp203 = ($49 | 0) == 0;
      if (!$cmp203) {
        var $50 = HEAP8[$49 + 12 | 0];
        if (($50 & 1) << 24 >> 24 == 0) {
          var $61 = $50;
          label = 1092;
          break;
        }
      }
      var $52 = HEAP32[$__w_4$s2 + 1];
      if (($52 | 0) != 0) {
        if ((HEAP8[$52 + 12 | 0] & 1) << 24 >> 24 == 0) {
          label = 1090;
          break;
        }
      }
      HEAP8[$__w_4 + 12 | 0] = 0;
      var $55 = HEAP32[$__w_4$s2 + 2];
      var $__is_black_222 = $55 + 12 | 0;
      if ((HEAP8[$__is_black_222] & 1) << 24 >> 24 == 0 | ($55 | 0) == ($__root_addr_6 | 0)) {
        label = 1087;
        break;
      }
      var $58 = HEAP32[$55 + 8 >> 2];
      var $59 = HEAP32[$58 >> 2];
      if (($55 | 0) != ($59 | 0)) {
        var $__root_addr_2 = $__root_addr_6;
        var $__w_1 = $59;
        continue;
      }
      var $__root_addr_2 = $__root_addr_6;
      var $__w_1 = HEAP32[$58 + 4 >> 2];
      continue;
    }
    if ($tobool185) {
      var $__root_addr_4 = $__root_addr_2;
      var $__w_2 = $__w_1, $__w_2$s2 = $__w_2 >> 2;
    } else {
      HEAP8[$__is_black_184] = 1;
      HEAP8[$19 + 12 | 0] = 0;
      __ZNSt3__118__tree_left_rotateIPNS_16__tree_node_baseIPvEEEEvT_(HEAP32[$__parent__i344$s2]);
      var $24 = HEAP32[$__w_1 >> 2];
      var $__root_addr_4 = ($__root_addr_2 | 0) == ($24 | 0) ? $__w_1 : $__root_addr_2;
      var $__w_2 = HEAP32[$24 + 4 >> 2], $__w_2$s2 = $__w_2 >> 2;
    }
    var $__w_2;
    var $__root_addr_4;
    var $26 = HEAP32[$__w_2$s2];
    if (($26 | 0) != 0) {
      var $__is_black_123 = $26 + 12 | 0;
      if ((HEAP8[$__is_black_123] & 1) << 24 >> 24 == 0) {
        label = 1074;
        break;
      }
    }
    var $29 = HEAP32[$__w_2$s2 + 1];
    if (($29 | 0) != 0) {
      var $30 = HEAP8[$29 + 12 | 0];
      if (($30 & 1) << 24 >> 24 == 0) {
        var $38 = $30;
        label = 1076;
        break;
      }
    }
    HEAP8[$__w_2 + 12 | 0] = 0;
    var $32 = HEAP32[$__w_2$s2 + 2];
    if (($32 | 0) == ($__root_addr_4 | 0)) {
      var $_lcssa = $__root_addr_4;
      label = 1071;
      break;
    }
    if ((HEAP8[$32 + 12 | 0] & 1) << 24 >> 24 == 0) {
      var $_lcssa = $32;
      label = 1071;
      break;
    }
    var $35 = HEAP32[$32 + 8 >> 2];
    var $36 = HEAP32[$35 >> 2];
    if (($32 | 0) != ($36 | 0)) {
      var $__root_addr_2 = $__root_addr_4;
      var $__w_1 = $36;
      continue;
    }
    var $__root_addr_2 = $__root_addr_4;
    var $__w_1 = HEAP32[$35 + 4 >> 2];
  }
  do {
    if (label == 1087) {
      HEAP8[$__is_black_222] = 1;
      return;
    } else if (label == 1090) {
      if ($cmp203) {
        var $63 = $52;
        label = 1094;
        break;
      }
      var $61 = HEAP8[$49 + 12 | 0];
      label = 1092;
      break;
    } else if (label == 1074) {
      var $_pre120 = HEAP32[$__w_2$s2 + 1];
      if (($_pre120 | 0) == 0) {
        var $__is_black_168_pre_phi = $__is_black_123;
        label = 1078;
        break;
      }
      var $38 = HEAP8[$_pre120 + 12 | 0];
      label = 1076;
      break;
    } else if (label == 1071) {
      var $_lcssa;
      HEAP8[$_lcssa + 12 | 0] = 1;
      return;
    }
  } while (0);
  do {
    if (label == 1092) {
      var $61;
      if (($61 & 1) << 24 >> 24 == 0) {
        var $__w_5 = $__w_4;
        label = 1095;
        break;
      }
      var $63 = HEAP32[$__w_4$s2 + 1];
      label = 1094;
      break;
    } else if (label == 1076) {
      var $38;
      if (($38 & 1) << 24 >> 24 == 0) {
        var $__w_3 = $__w_2;
        label = 1079;
        break;
      }
      var $__is_black_168_pre_phi = $26 + 12 | 0;
      label = 1078;
      break;
    }
  } while (0);
  do {
    if (label == 1078) {
      var $__is_black_168_pre_phi;
      HEAP8[$__is_black_168_pre_phi] = 1;
      HEAP8[$__w_2 + 12 | 0] = 0;
      __ZNSt3__119__tree_right_rotateIPNS_16__tree_node_baseIPvEEEEvT_($__w_2);
      var $__w_3 = HEAP32[$__w_2$s2 + 2];
      label = 1079;
      break;
    } else if (label == 1094) {
      var $63;
      HEAP8[$63 + 12 | 0] = 1;
      HEAP8[$__w_4 + 12 | 0] = 0;
      __ZNSt3__118__tree_left_rotateIPNS_16__tree_node_baseIPvEEEEvT_($__w_4);
      var $__w_5 = HEAP32[$__w_4$s2 + 2];
      label = 1095;
      break;
    }
  } while (0);
  if (label == 1079) {
    var $__w_3;
    var $__parent_172 = $__w_3 + 8 | 0;
    var $__is_black_173 = HEAP32[$__parent_172 >> 2] + 12 | 0;
    HEAP8[$__w_3 + 12 | 0] = HEAP8[$__is_black_173] & 1;
    HEAP8[$__is_black_173] = 1;
    HEAP8[HEAP32[$__w_3 + 4 >> 2] + 12 | 0] = 1;
    __ZNSt3__118__tree_left_rotateIPNS_16__tree_node_baseIPvEEEEvT_(HEAP32[$__parent_172 >> 2]);
    return;
  } else if (label == 1095) {
    var $__w_5;
    var $__parent_255 = $__w_5 + 8 | 0;
    var $__is_black_256 = HEAP32[$__parent_255 >> 2] + 12 | 0;
    HEAP8[$__w_5 + 12 | 0] = HEAP8[$__is_black_256] & 1;
    HEAP8[$__is_black_256] = 1;
    HEAP8[HEAP32[$__w_5 >> 2] + 12 | 0] = 1;
    __ZNSt3__119__tree_right_rotateIPNS_16__tree_node_baseIPvEEEEvT_(HEAP32[$__parent_255 >> 2]);
    return;
  }
}
__ZNSt3__113__tree_removeIPNS_16__tree_node_baseIPvEEEEvT_S5_["X"] = 1;
function __ZNSt3__16__treeIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEE14__erase_uniqueIS2_EEjRKT_($this, $__k) {
  var __stackBase__ = STACKTOP;
  STACKTOP = STACKTOP + 8 | 0;
  var $__i = __stackBase__;
  __ZNSt3__16__treeIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEE4findIS2_EENS_15__tree_iteratorIS2_PNS_11__tree_nodeIS2_PvEEiEERKT_($__i, $this, $__k);
  var $1 = HEAP32[$__i >> 2];
  if (($1 | 0) == ($this + 4 | 0)) {
    STACKTOP = __stackBase__;
    return;
  }
  __ZNSt3__16__treeIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEE5eraseENS_21__tree_const_iteratorIS2_PKNS_11__tree_nodeIS2_PvEEiEE(__stackBase__ + 4, $this, $1);
  STACKTOP = __stackBase__;
  return;
}
function __ZNSt3__16__treeIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEE5eraseENS_21__tree_const_iteratorIS2_PKNS_11__tree_nodeIS2_PvEEiEE($agg_result, $this, $__p_0) {
  var $0 = $__p_0 | 0;
  var $1 = __ZNSt3__111__tree_nextIPNS_16__tree_node_baseIPvEEEET_S5_($0);
  HEAP32[$agg_result >> 2] = $1;
  var $__begin_node__i = $this | 0;
  if ((HEAP32[$__begin_node__i >> 2] | 0) == ($__p_0 | 0)) {
    HEAP32[$__begin_node__i >> 2] = $1;
  }
  var $__first__i_i_i45 = $this + 8 | 0;
  HEAP32[$__first__i_i_i45 >> 2] = HEAP32[$__first__i_i_i45 >> 2] - 1 | 0;
  __ZNSt3__113__tree_removeIPNS_16__tree_node_baseIPvEEEEvT_S5_(HEAP32[$this + 4 >> 2], $0);
  __ZdlPv($__p_0);
  return;
}
function __ZN13MultipleShapeC2Ev($this) {
  __ZN12LabeledShapeC2EN5Shape9ShapeTypeE($this | 0, 1);
  HEAP32[$this >> 2] = 5244400;
  __ZNSt3__16__treeINS_4pairIP5BlockP5ShapeEENS_19__map_value_compareIS3_S5_NS_4lessIS3_EELb1EEENS_9allocatorIS6_EEEC1ERKSA_($this + 20 | 0);
  HEAP32[$this + 32 >> 2] = 0;
  return;
}
function __ZN12LabeledShapeC2EN5Shape9ShapeTypeE($this, $TypeInit) {
  __ZN5ShapeC2ENS_9ShapeTypeE($this | 0, $TypeInit);
  HEAP32[$this >> 2] = 5244424;
  HEAP8[$this + 16 | 0] = 0;
  return;
}
function __ZNSt3__16__treeINS_4pairIP5BlockP5ShapeEENS_19__map_value_compareIS3_S5_NS_4lessIS3_EELb1EEENS_9allocatorIS6_EEEC1ERKSA_($this) {
  __ZNSt3__16__treeINS_4pairIP5BlockP5ShapeEENS_19__map_value_compareIS3_S5_NS_4lessIS3_EELb1EEENS_9allocatorIS6_EEEC2ERKSA_($this);
  return;
}
function __ZN12LabeledShapeD0Ev($this) {
  __ZdlPv($this);
  return;
}
function __ZN5ShapeD0Ev($this) {
  __ZdlPv($this);
  return;
}
function __ZNSt3__16__treeINS_4pairIP5BlockNS_3setIS3_NS_4lessIS3_EENS_9allocatorIS3_EEEEEENS_19__map_value_compareIS3_S9_S6_Lb1EEENS7_ISA_EEE14__erase_uniqueIS3_EEjRKT_($this, $__k) {
  var __stackBase__ = STACKTOP;
  STACKTOP = STACKTOP + 8 | 0;
  var $__i = __stackBase__;
  __ZNSt3__16__treeINS_4pairIP5BlockNS_3setIS3_NS_4lessIS3_EENS_9allocatorIS3_EEEEEENS_19__map_value_compareIS3_S9_S6_Lb1EEENS7_ISA_EEE4findIS3_EENS_15__tree_iteratorISA_PNS_11__tree_nodeISA_PvEEiEERKT_($__i, $this, $__k);
  var $1 = HEAP32[$__i >> 2];
  if (($1 | 0) == ($this + 4 | 0)) {
    STACKTOP = __stackBase__;
    return;
  }
  __ZNSt3__16__treeINS_4pairIP5BlockNS_3setIS3_NS_4lessIS3_EENS_9allocatorIS3_EEEEEENS_19__map_value_compareIS3_S9_S6_Lb1EEENS7_ISA_EEE5eraseENS_21__tree_const_iteratorISA_PKNS_11__tree_nodeISA_PvEEiEE(__stackBase__ + 4, $this, $1);
  STACKTOP = __stackBase__;
  return;
}
function __ZNSt3__111__tree_nextIPKNS_16__tree_node_baseIPvEEEET_S6_($__x) {
  var $0 = HEAP32[$__x + 4 >> 2];
  if (($0 | 0) == 0) {
    var $__x_addr_0 = $__x;
    while (1) {
      var $__x_addr_0;
      var $2 = HEAP32[$__x_addr_0 + 8 >> 2];
      if (($__x_addr_0 | 0) == (HEAP32[$2 >> 2] | 0)) {
        var $retval_0 = $2;
        break;
      } else {
        var $__x_addr_0 = $2;
      }
    }
    var $retval_0;
    return $retval_0;
  } else {
    var $__x_addr_i9_0 = $0;
    while (1) {
      var $__x_addr_i9_0;
      var $1 = HEAP32[$__x_addr_i9_0 >> 2];
      if (($1 | 0) == 0) {
        var $retval_0 = $__x_addr_i9_0;
        break;
      } else {
        var $__x_addr_i9_0 = $1;
      }
    }
    var $retval_0;
    return $retval_0;
  }
}
function __ZNSt3__13mapIP5BlockNS_3setIS2_NS_4lessIS2_EENS_9allocatorIS2_EEEES5_NS6_INS_4pairIKS2_S8_EEEEE16__find_equal_keyERPNS_16__tree_node_baseIPvEERSA_($this, $__parent, $__k) {
  var label = 0;
  var $__first__i_i_i_i = $this + 4 | 0;
  var $__left__i = $__first__i_i_i_i | 0;
  var $0 = HEAP32[$__left__i >> 2];
  if (($0 | 0) == 0) {
    HEAP32[$__parent >> 2] = $__first__i_i_i_i;
    var $retval_0 = $__left__i;
    var $retval_0;
    return $retval_0;
  }
  var $1 = HEAP32[$__k >> 2];
  var $__nd_0_in = $0;
  while (1) {
    var $__nd_0_in;
    var $2 = HEAP32[$__nd_0_in + 16 >> 2];
    if ($1 >>> 0 < $2 >>> 0) {
      var $__left_ = $__nd_0_in | 0;
      var $3 = HEAP32[$__left_ >> 2];
      if (($3 | 0) == 0) {
        label = 1131;
        break;
      } else {
        var $__nd_0_in = $3;
        continue;
      }
    }
    if ($2 >>> 0 >= $1 >>> 0) {
      label = 1135;
      break;
    }
    var $__right_ = $__nd_0_in + 4 | 0;
    var $4 = HEAP32[$__right_ >> 2];
    if (($4 | 0) == 0) {
      label = 1134;
      break;
    } else {
      var $__nd_0_in = $4;
    }
  }
  if (label == 1134) {
    HEAP32[$__parent >> 2] = $__nd_0_in;
    var $retval_0 = $__right_;
    var $retval_0;
    return $retval_0;
  } else if (label == 1135) {
    HEAP32[$__parent >> 2] = $__nd_0_in;
    var $retval_0 = $__parent;
    var $retval_0;
    return $retval_0;
  } else if (label == 1131) {
    HEAP32[$__parent >> 2] = $__nd_0_in;
    var $retval_0 = $__left_;
    var $retval_0;
    return $retval_0;
  }
}
function __ZNSt3__16__treeINS_4pairIP5BlockS3_EENS_19__map_value_compareIS3_S3_NS_4lessIS3_EELb1EEENS_9allocatorIS4_EEE13__lower_boundIS3_EENS_15__tree_iteratorIS4_PNS_11__tree_nodeIS4_PvEEiEERKT_SH_SH_($agg_result, $__v, $__root, $__result) {
  var $__root_addr_06$s2;
  var $cmp59 = ($__root | 0) == 0;
  L1379 : do {
    if ($cmp59) {
      var $__result_addr_0_ph8 = $__result;
    } else {
      var $0 = HEAP32[$__v >> 2];
      var $__root_addr_0_ph10 = $__root;
      var $__result_addr_0_ph11 = $__result;
      while (1) {
        var $__result_addr_0_ph11;
        var $__root_addr_0_ph10;
        var $__root_addr_06 = $__root_addr_0_ph10, $__root_addr_06$s2 = $__root_addr_06 >> 2;
        while (1) {
          var $__root_addr_06;
          if (HEAP32[$__root_addr_06$s2 + 4] >>> 0 >= $0 >>> 0) {
            break;
          }
          var $4 = HEAP32[$__root_addr_06$s2 + 1];
          if (($4 | 0) == 0) {
            var $__result_addr_0_ph8 = $__result_addr_0_ph11;
            break L1379;
          } else {
            var $__root_addr_06 = $4, $__root_addr_06$s2 = $__root_addr_06 >> 2;
          }
        }
        var $2 = HEAP32[$__root_addr_06$s2];
        if (($2 | 0) == 0) {
          var $__result_addr_0_ph8 = $__root_addr_06;
          break L1379;
        } else {
          var $__root_addr_0_ph10 = $2;
          var $__result_addr_0_ph11 = $__root_addr_06;
        }
      }
    }
  } while (0);
  var $__result_addr_0_ph8;
  HEAP32[$agg_result >> 2] = $__result_addr_0_ph8;
  return;
}
function __ZNSt3__13mapIP5BlockS2_NS_4lessIS2_EENS_9allocatorINS_4pairIKS2_S2_EEEEE16__find_equal_keyERPNS_16__tree_node_baseIPvEERS7_($this, $__parent, $__k) {
  var label = 0;
  var $__first__i_i_i_i = $this + 4 | 0;
  var $__left__i = $__first__i_i_i_i | 0;
  var $0 = HEAP32[$__left__i >> 2];
  if (($0 | 0) == 0) {
    HEAP32[$__parent >> 2] = $__first__i_i_i_i;
    var $retval_0 = $__left__i;
    var $retval_0;
    return $retval_0;
  }
  var $1 = HEAP32[$__k >> 2];
  var $__nd_0_in = $0;
  while (1) {
    var $__nd_0_in;
    var $2 = HEAP32[$__nd_0_in + 16 >> 2];
    if ($1 >>> 0 < $2 >>> 0) {
      var $__left_ = $__nd_0_in | 0;
      var $3 = HEAP32[$__left_ >> 2];
      if (($3 | 0) == 0) {
        label = 1153;
        break;
      } else {
        var $__nd_0_in = $3;
        continue;
      }
    }
    if ($2 >>> 0 >= $1 >>> 0) {
      label = 1157;
      break;
    }
    var $__right_ = $__nd_0_in + 4 | 0;
    var $4 = HEAP32[$__right_ >> 2];
    if (($4 | 0) == 0) {
      label = 1156;
      break;
    } else {
      var $__nd_0_in = $4;
    }
  }
  if (label == 1157) {
    HEAP32[$__parent >> 2] = $__nd_0_in;
    var $retval_0 = $__parent;
    var $retval_0;
    return $retval_0;
  } else if (label == 1156) {
    HEAP32[$__parent >> 2] = $__nd_0_in;
    var $retval_0 = $__right_;
    var $retval_0;
    return $retval_0;
  } else if (label == 1153) {
    HEAP32[$__parent >> 2] = $__nd_0_in;
    var $retval_0 = $__left_;
    var $retval_0;
    return $retval_0;
  }
}
function __ZNSt3__16__treeINS_4pairIP5BlockS3_EENS_19__map_value_compareIS3_S3_NS_4lessIS3_EELb1EEENS_9allocatorIS4_EEEC2ERKS8_($this) {
  var $__first__i_i_i = $this + 4 | 0;
  HEAP32[$__first__i_i_i >> 2] = 0;
  HEAP32[$this + 8 >> 2] = 0;
  HEAP32[$this >> 2] = $__first__i_i_i;
  return;
}
function __ZNSt3__16__treeINS_4pairIP5BlockNS_3setIS3_NS_4lessIS3_EENS_9allocatorIS3_EEEEEENS_19__map_value_compareIS3_S9_S6_Lb1EEENS7_ISA_EEEC2ERKSC_($this) {
  var $__first__i_i_i = $this + 4 | 0;
  HEAP32[$__first__i_i_i >> 2] = 0;
  HEAP32[$this + 8 >> 2] = 0;
  HEAP32[$this >> 2] = $__first__i_i_i;
  return;
}
function __ZNSt3__111__tree_prevIPKNS_16__tree_node_baseIPvEEEET_S6_($__x) {
  var $0 = HEAP32[$__x >> 2];
  if (($0 | 0) == 0) {
    var $__x_addr_0 = $__x;
    while (1) {
      var $__x_addr_0;
      var $2 = HEAP32[$__x_addr_0 + 8 >> 2];
      if (($__x_addr_0 | 0) == (HEAP32[$2 >> 2] | 0)) {
        var $__x_addr_0 = $2;
      } else {
        var $retval_0 = $2;
        break;
      }
    }
    var $retval_0;
    return $retval_0;
  } else {
    var $__x_addr_i9_0 = $0;
    while (1) {
      var $__x_addr_i9_0;
      var $1 = HEAP32[$__x_addr_i9_0 + 4 >> 2];
      if (($1 | 0) == 0) {
        var $retval_0 = $__x_addr_i9_0;
        break;
      } else {
        var $__x_addr_i9_0 = $1;
      }
    }
    var $retval_0;
    return $retval_0;
  }
}
function __ZNSt3__16__treeIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEEC2ERKS7_($this) {
  var $__first__i_i_i40 = $this + 4 | 0;
  HEAP32[$__first__i_i_i40 >> 2] = 0;
  HEAP32[$this + 8 >> 2] = 0;
  HEAP32[$this >> 2] = $__first__i_i_i40;
  return;
}
function __ZNSt3__16__treeINS_4pairIP5BlockNS_3setIS3_NS_4lessIS3_EENS_9allocatorIS3_EEEEEENS_19__map_value_compareIS3_S9_S6_Lb1EEENS7_ISA_EEE5eraseENS_21__tree_const_iteratorISA_PKNS_11__tree_nodeISA_PvEEiEE($agg_result, $this, $__p_0) {
  var $0 = $__p_0 | 0;
  var $1 = __ZNSt3__111__tree_nextIPNS_16__tree_node_baseIPvEEEET_S5_($0);
  HEAP32[$agg_result >> 2] = $1;
  var $__begin_node__i = $this | 0;
  if ((HEAP32[$__begin_node__i >> 2] | 0) == ($__p_0 | 0)) {
    HEAP32[$__begin_node__i >> 2] = $1;
  }
  var $__first__i_i_i35 = $this + 8 | 0;
  HEAP32[$__first__i_i_i35 >> 2] = HEAP32[$__first__i_i_i35 >> 2] - 1 | 0;
  __ZNSt3__14pairIP5BlockNS_3setIS2_NS_4lessIS2_EENS_9allocatorIS2_EEEEED1Ev($__p_0 + 16 | 0);
  __ZNSt3__113__tree_removeIPNS_16__tree_node_baseIPvEEEEvT_S5_(HEAP32[$this + 4 >> 2], $0);
  __ZdlPv($__p_0);
  return;
}
function __ZNSt3__13mapIP5BlockNS_3setIS2_NS_4lessIS2_EENS_9allocatorIS2_EEEES5_NS6_INS_4pairIKS2_S8_EEEEE16__construct_nodeERSA_($agg_result, $this, $__k) {
  var $call_i_i = __Znwj(32);
  var $__value_ = $call_i_i + 16 | 0;
  if (($__value_ | 0) != 0) {
    HEAP32[$__value_ >> 2] = HEAP32[$__k >> 2];
  }
  var $second = $call_i_i + 20 | 0;
  if (($second | 0) != 0) {
    __ZNSt3__16__treeIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEEC1ERKS4_($second);
  }
  HEAP32[$agg_result >> 2] = $call_i_i;
  HEAP32[$agg_result + 4 >> 2] = $this + 4 | 0;
  HEAP8[$agg_result + 8 | 0] = 1;
  HEAP8[$agg_result + 9 | 0] = 1;
  return;
}
function __ZNSt3__16__treeINS_4pairIP5BlockNS_3setIS3_NS_4lessIS3_EENS_9allocatorIS3_EEEEEENS_19__map_value_compareIS3_S9_S6_Lb1EEENS7_ISA_EEE16__insert_node_atEPNS_16__tree_node_baseIPvEERSI_SI_($this, $__parent, $__child, $__new_node) {
  HEAP32[$__new_node >> 2] = 0;
  HEAP32[$__new_node + 4 >> 2] = 0;
  HEAP32[$__new_node + 8 >> 2] = $__parent;
  HEAP32[$__child >> 2] = $__new_node;
  var $__begin_node__i42 = $this | 0;
  var $1 = HEAP32[HEAP32[$__begin_node__i42 >> 2] >> 2];
  if (($1 | 0) == 0) {
    var $3 = $__new_node;
  } else {
    HEAP32[$__begin_node__i42 >> 2] = $1;
    var $3 = HEAP32[$__child >> 2];
  }
  var $3;
  __ZNSt3__127__tree_balance_after_insertIPNS_16__tree_node_baseIPvEEEEvT_S5_(HEAP32[$this + 4 >> 2], $3);
  var $__first__i_i_i = $this + 8 | 0;
  HEAP32[$__first__i_i_i >> 2] = HEAP32[$__first__i_i_i >> 2] + 1 | 0;
  return;
}
function __ZZZN8Relooper9CalculateEP5BlockEN8Analyzer21FindIndependentGroupsE_0RNSt3__13setIS1_NS3_4lessIS1_EENS3_9allocatorIS1_EEEESA_RNS3_3mapIS1_S9_S6_NS7_INS3_4pairIKS1_S9_EEEEEEEN11HelperClassC1ESH_($this, $IndependentGroupsInit) {
  __ZZZN8Relooper9CalculateEP5BlockEN8Analyzer21FindIndependentGroupsE_0RNSt3__13setIS1_NS3_4lessIS1_EENS3_9allocatorIS1_EEEESA_RNS3_3mapIS1_S9_S6_NS7_INS3_4pairIKS1_S9_EEEEEEEN11HelperClassC2ESH_($this, $IndependentGroupsInit);
  return;
}
function __ZNSt3__13mapIP5BlockS2_NS_4lessIS2_EENS_9allocatorINS_4pairIKS2_S2_EEEEEixERS7_($this, $__k) {
  var __stackBase__ = STACKTOP;
  STACKTOP = STACKTOP + 16 | 0;
  var $__parent = __stackBase__;
  var $temp_lvalue = __stackBase__ + 4;
  var $call = __ZNSt3__13mapIP5BlockS2_NS_4lessIS2_EENS_9allocatorINS_4pairIKS2_S2_EEEEE16__find_equal_keyERPNS_16__tree_node_baseIPvEERS7_($this, $__parent, $__k);
  var $0 = HEAP32[$call >> 2];
  if (($0 | 0) != 0) {
    var $__r_0 = $0;
    var $__r_0;
    var $second = $__r_0 + 20 | 0;
    STACKTOP = __stackBase__;
    return $second;
  }
  __ZNSt3__13mapIP5BlockS2_NS_4lessIS2_EENS_9allocatorINS_4pairIKS2_S2_EEEEE16__construct_nodeERS7_($temp_lvalue, $this, $__k);
  var $__first__i_i_i28_i_i = $temp_lvalue | 0;
  var $2 = HEAP32[$__first__i_i_i28_i_i >> 2];
  HEAP32[$__first__i_i_i28_i_i >> 2] = 0;
  __ZNSt3__16__treeINS_4pairIP5BlockS3_EENS_19__map_value_compareIS3_S3_NS_4lessIS3_EELb1EEENS_9allocatorIS4_EEE16__insert_node_atEPNS_16__tree_node_baseIPvEERSF_SF_($this | 0, HEAP32[$__parent >> 2], $call, $2 | 0);
  var $__r_0 = $2;
  var $__r_0;
  var $second = $__r_0 + 20 | 0;
  STACKTOP = __stackBase__;
  return $second;
}
function __ZNSt3__14listIP5BlockNS_9allocatorIS2_EEE9push_backERKS2_($this, $__x) {
  var $__prev__i$s2;
  var $call_i_i = __Znwj(12);
  var $0 = $call_i_i;
  var $__value_ = $call_i_i + 8 | 0;
  if (($__value_ | 0) != 0) {
    HEAP32[$__value_ >> 2] = HEAP32[$__x >> 2];
  }
  var $__prev__i$s2 = ($this | 0) >> 2;
  HEAP32[HEAP32[$__prev__i$s2] + 4 >> 2] = $0;
  HEAP32[$call_i_i >> 2] = HEAP32[$__prev__i$s2];
  HEAP32[$__prev__i$s2] = $0;
  HEAP32[$call_i_i + 4 >> 2] = $this;
  var $__first__i_i_i96 = $this + 8 | 0;
  HEAP32[$__first__i_i_i96 >> 2] = HEAP32[$__first__i_i_i96 >> 2] + 1 | 0;
  return;
}
function __ZNSt3__14listIP5BlockNS_9allocatorIS2_EEE9pop_frontEv($this) {
  var $0 = HEAP32[$this + 4 >> 2];
  var $__next__i = $0 + 4 | 0;
  var $__prev__i = $0 | 0;
  HEAP32[HEAP32[$__prev__i >> 2] + 4 >> 2] = HEAP32[$__next__i >> 2];
  HEAP32[HEAP32[$__next__i >> 2] >> 2] = HEAP32[$__prev__i >> 2];
  var $__first__i_i_i = $this + 8 | 0;
  HEAP32[$__first__i_i_i >> 2] = HEAP32[$__first__i_i_i >> 2] - 1 | 0;
  __ZdlPv($0);
  return;
}
function __ZZZN8Relooper9CalculateEP5BlockEN8Analyzer21FindIndependentGroupsE_0RNSt3__13setIS1_NS3_4lessIS1_EENS3_9allocatorIS1_EEEESA_RNS3_3mapIS1_S9_S6_NS7_INS3_4pairIKS1_S9_EEEEEEEN11HelperClass22InvalidateWithChildrenES1_($this, $New) {
  var $__first__i_i_i_i_i_i$s2;
  var __stackBase__ = STACKTOP;
  STACKTOP = STACKTOP + 36 | 0;
  var $agg_tmp_i137 = __stackBase__;
  var $agg_tmp_i = __stackBase__ + 4;
  var $New_addr = __stackBase__ + 8;
  var $ToInvalidate = __stackBase__ + 12;
  var $Invalidatee = __stackBase__ + 24;
  var $Owner = __stackBase__ + 28;
  var $Target = __stackBase__ + 32;
  HEAP32[$New_addr >> 2] = $New;
  var $0 = $ToInvalidate;
  HEAP32[$ToInvalidate >> 2] = $0;
  var $__next__i_i_i_i_i = $ToInvalidate + 4 | 0;
  HEAP32[$__next__i_i_i_i_i >> 2] = $0;
  var $__first__i_i_i_i_i_i$s2 = ($ToInvalidate + 8 | 0) >> 2;
  HEAP32[$__first__i_i_i_i_i_i$s2] = 0;
  __ZNSt3__14listIP5BlockNS_9allocatorIS2_EEE9push_backERKS2_($ToInvalidate, $New_addr);
  if ((HEAP32[$__first__i_i_i_i_i_i$s2] | 0) == 0) {
    __ZNSt3__14listIP5BlockNS_9allocatorIS2_EEED1Ev($ToInvalidate);
    STACKTOP = __stackBase__;
    return;
  }
  var $Ownership = $this + 4 | 0;
  var $IndependentGroups = $this | 0;
  var $agg_tmp2_i_sroa_0_0_tmp3_i_idx = $agg_tmp_i | 0;
  var $__tree__i139 = $Ownership | 0;
  var $agg_tmp2_i134_sroa_0_0_tmp3_i141_idx = $agg_tmp_i137 | 0;
  var $2 = $this + 8 | 0;
  while (1) {
    HEAP32[$Invalidatee >> 2] = HEAP32[HEAP32[$__next__i_i_i_i_i >> 2] + 8 >> 2];
    __ZNSt3__14listIP5BlockNS_9allocatorIS2_EEE9pop_frontEv($ToInvalidate);
    HEAP32[$Owner >> 2] = HEAP32[__ZNSt3__13mapIP5BlockS2_NS_4lessIS2_EENS_9allocatorINS_4pairIKS2_S2_EEEEEixERS7_($Ownership, $Invalidatee) >> 2];
    __ZNSt3__16__treeINS_4pairIP5BlockNS_3setIS3_NS_4lessIS3_EENS_9allocatorIS3_EEEEEENS_19__map_value_compareIS3_S9_S6_Lb1EEENS7_ISA_EEE4findIS3_EENS_15__tree_iteratorISA_PNS_11__tree_nodeISA_PvEEiEERKT_($agg_tmp_i, HEAP32[$IndependentGroups >> 2] | 0, $Owner);
    var $ref10 = HEAP32[$IndependentGroups >> 2];
    if ((HEAP32[$agg_tmp2_i_sroa_0_0_tmp3_i_idx >> 2] | 0) != ($ref10 + 4 | 0)) {
      __ZNSt3__16__treeIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEE14__erase_uniqueIS2_EEjRKT_(__ZNSt3__13mapIP5BlockNS_3setIS2_NS_4lessIS2_EENS_9allocatorIS2_EEEES5_NS6_INS_4pairIKS2_S8_EEEEEixERSA_($ref10, $Owner) | 0, $Invalidatee);
    }
    var $tobool = (HEAP32[__ZNSt3__13mapIP5BlockS2_NS_4lessIS2_EENS_9allocatorINS_4pairIKS2_S2_EEEEEixERS7_($Ownership, $Invalidatee) >> 2] | 0) == 0;
    L1456 : do {
      if (!$tobool) {
        HEAP32[__ZNSt3__13mapIP5BlockS2_NS_4lessIS2_EENS_9allocatorINS_4pairIKS2_S2_EEEEEixERS7_($Ownership, $Invalidatee) >> 2] = 0;
        var $9 = HEAP32[$Invalidatee >> 2];
        var $10 = HEAP32[$9 >> 2];
        var $__first__i_i_i_i_i20111 = $9 + 4 | 0;
        if (($10 | 0) == ($__first__i_i_i_i_i20111 | 0)) {
          break;
        }
        var $12 = $__first__i_i_i_i_i20111;
        var $iter_sroa_0_013 = $10;
        while (1) {
          var $iter_sroa_0_013;
          HEAP32[$Target >> 2] = HEAP32[$iter_sroa_0_013 + 16 >> 2];
          __ZNSt3__16__treeINS_4pairIP5BlockS3_EENS_19__map_value_compareIS3_S3_NS_4lessIS3_EELb1EEENS_9allocatorIS4_EEE4findIS3_EENS_15__tree_iteratorIS4_PNS_11__tree_nodeIS4_PvEEiEERKT_($agg_tmp_i137, $__tree__i139, $Target);
          var $agg_tmp2_i134_sroa_0_0_copyload4 = HEAP32[$agg_tmp2_i134_sroa_0_0_tmp3_i141_idx >> 2];
          do {
            if (($agg_tmp2_i134_sroa_0_0_copyload4 | 0) != ($2 | 0)) {
              if ((HEAP32[$agg_tmp2_i134_sroa_0_0_copyload4 + 20 >> 2] | 0) == 0) {
                break;
              }
              __ZNSt3__14listIP5BlockNS_9allocatorIS2_EEE9push_backERKS2_($ToInvalidate, $Target);
            }
          } while (0);
          var $call_i_i_i = __ZNSt3__111__tree_nextIPNS_16__tree_node_baseIPvEEEET_S5_($iter_sroa_0_013 | 0);
          if (($call_i_i_i | 0) == ($12 | 0)) {
            break L1456;
          } else {
            var $iter_sroa_0_013 = $call_i_i_i;
          }
        }
      }
    } while (0);
    if ((HEAP32[$__first__i_i_i_i_i_i$s2] | 0) == 0) {
      break;
    }
  }
  __ZNSt3__14listIP5BlockNS_9allocatorIS2_EEED1Ev($ToInvalidate);
  STACKTOP = __stackBase__;
  return;
}
__ZZZN8Relooper9CalculateEP5BlockEN8Analyzer21FindIndependentGroupsE_0RNSt3__13setIS1_NS3_4lessIS1_EENS3_9allocatorIS1_EEEESA_RNS3_3mapIS1_S9_S6_NS7_INS3_4pairIKS1_S9_EEEEEEEN11HelperClass22InvalidateWithChildrenES1_["X"] = 1;
function __ZNSt3__14listIP5BlockNS_9allocatorIS2_EEED1Ev($this) {
  __ZNSt3__14listIP5BlockNS_9allocatorIS2_EEED2Ev($this);
  return;
}
function __ZZZN8Relooper9CalculateEP5BlockEN8Analyzer21FindIndependentGroupsE_0RNSt3__13setIS1_NS3_4lessIS1_EENS3_9allocatorIS1_EEEESA_RNS3_3mapIS1_S9_S6_NS7_INS3_4pairIKS1_S9_EEEEEEEN11HelperClassD1Ev($this) {
  __ZZZN8Relooper9CalculateEP5BlockEN8Analyzer21FindIndependentGroupsE_0RNSt3__13setIS1_NS3_4lessIS1_EENS3_9allocatorIS1_EEEESA_RNS3_3mapIS1_S9_S6_NS7_INS3_4pairIKS1_S9_EEEEEEEN11HelperClassD2Ev($this);
  return;
}
function __ZZZN8Relooper9CalculateEP5BlockEN8Analyzer21FindIndependentGroupsE_0RNSt3__13setIS1_NS3_4lessIS1_EENS3_9allocatorIS1_EEEESA_RNS3_3mapIS1_S9_S6_NS7_INS3_4pairIKS1_S9_EEEEEEEN11HelperClassD2Ev($this) {
  __ZNSt3__13mapIP5BlockS2_NS_4lessIS2_EENS_9allocatorINS_4pairIKS2_S2_EEEEED1Ev($this + 4 | 0);
  return;
}
function __ZNSt3__13mapIP5BlockS2_NS_4lessIS2_EENS_9allocatorINS_4pairIKS2_S2_EEEEED1Ev($this) {
  __ZNSt3__13mapIP5BlockS2_NS_4lessIS2_EENS_9allocatorINS_4pairIKS2_S2_EEEEED2Ev($this);
  return;
}
function __ZNSt3__13mapIP5BlockS2_NS_4lessIS2_EENS_9allocatorINS_4pairIKS2_S2_EEEEED2Ev($this) {
  __ZNSt3__16__treeINS_4pairIP5BlockS3_EENS_19__map_value_compareIS3_S3_NS_4lessIS3_EELb1EEENS_9allocatorIS4_EEED1Ev(HEAP32[$this + 4 >> 2]);
  return;
}
function __ZNSt3__16__treeINS_4pairIP5BlockS3_EENS_19__map_value_compareIS3_S3_NS_4lessIS3_EELb1EEENS_9allocatorIS4_EEED1Ev($this_0_1_0_0_0_val) {
  __ZNSt3__16__treeINS_4pairIP5BlockS3_EENS_19__map_value_compareIS3_S3_NS_4lessIS3_EELb1EEENS_9allocatorIS4_EEED2Ev($this_0_1_0_0_0_val);
  return;
}
function __ZNSt3__16__treeINS_4pairIP5BlockS3_EENS_19__map_value_compareIS3_S3_NS_4lessIS3_EELb1EEENS_9allocatorIS4_EEED2Ev($this_0_1_0_0_0_val) {
  __ZNSt3__16__treeINS_4pairIP5BlockS3_EENS_19__map_value_compareIS3_S3_NS_4lessIS3_EELb1EEENS_9allocatorIS4_EEE7destroyEPNS_11__tree_nodeIS4_PvEE($this_0_1_0_0_0_val);
  return;
}
function __ZNSt3__16__treeINS_4pairIP5BlockS3_EENS_19__map_value_compareIS3_S3_NS_4lessIS3_EELb1EEENS_9allocatorIS4_EEE7destroyEPNS_11__tree_nodeIS4_PvEE($__nd) {
  if (($__nd | 0) == 0) {
    return;
  } else {
    __ZNSt3__16__treeINS_4pairIP5BlockS3_EENS_19__map_value_compareIS3_S3_NS_4lessIS3_EELb1EEENS_9allocatorIS4_EEE7destroyEPNS_11__tree_nodeIS4_PvEE(HEAP32[$__nd >> 2]);
    __ZNSt3__16__treeINS_4pairIP5BlockS3_EENS_19__map_value_compareIS3_S3_NS_4lessIS3_EELb1EEENS_9allocatorIS4_EEE7destroyEPNS_11__tree_nodeIS4_PvEE(HEAP32[$__nd + 4 >> 2]);
    __ZdlPv($__nd);
    return;
  }
}
function __ZNSt3__14listIP5BlockNS_9allocatorIS2_EEED2Ev($this) {
  __ZNSt3__110__list_impIP5BlockNS_9allocatorIS2_EEED2Ev($this | 0);
  return;
}
function __ZNSt3__110__list_impIP5BlockNS_9allocatorIS2_EEED2Ev($this) {
  __ZNSt3__110__list_impIP5BlockNS_9allocatorIS2_EEE5clearEv($this);
  return;
}
function __ZNSt3__110__list_impIP5BlockNS_9allocatorIS2_EEE5clearEv($this) {
  var $__first__i_i_i_i = $this + 8 | 0;
  if ((HEAP32[$__first__i_i_i_i >> 2] | 0) == 0) {
    return;
  }
  var $1 = HEAP32[$this + 4 >> 2];
  var $2 = $this;
  var $__next__i35 = HEAP32[$this >> 2] + 4 | 0;
  var $__prev__i = $1 | 0;
  HEAP32[HEAP32[$__prev__i >> 2] + 4 >> 2] = HEAP32[$__next__i35 >> 2];
  HEAP32[HEAP32[$__next__i35 >> 2] >> 2] = HEAP32[$__prev__i >> 2];
  HEAP32[$__first__i_i_i_i >> 2] = 0;
  if (($1 | 0) == ($2 | 0)) {
    return;
  } else {
    var $__f_sroa_0_010 = $1;
  }
  while (1) {
    var $__f_sroa_0_010;
    var $8 = HEAP32[$__f_sroa_0_010 + 4 >> 2];
    __ZdlPv($__f_sroa_0_010);
    if (($8 | 0) == ($2 | 0)) {
      break;
    } else {
      var $__f_sroa_0_010 = $8;
    }
  }
  return;
}
function __ZNSt3__16__treeINS_4pairIP5BlockS3_EENS_19__map_value_compareIS3_S3_NS_4lessIS3_EELb1EEENS_9allocatorIS4_EEE4findIS3_EENS_15__tree_iteratorIS4_PNS_11__tree_nodeIS4_PvEEiEERKT_($agg_result, $this, $__v) {
  var __stackBase__ = STACKTOP;
  STACKTOP = STACKTOP + 4 | 0;
  var $__p = __stackBase__;
  var $__first__i_i_i_i = $this + 4 | 0;
  var $0 = $__first__i_i_i_i;
  __ZNSt3__16__treeINS_4pairIP5BlockS3_EENS_19__map_value_compareIS3_S3_NS_4lessIS3_EELb1EEENS_9allocatorIS4_EEE13__lower_boundIS3_EENS_15__tree_iteratorIS4_PNS_11__tree_nodeIS4_PvEEiEERKT_SH_SH_($__p, $__v, HEAP32[$__first__i_i_i_i >> 2], $0);
  var $3 = HEAP32[$__p >> 2];
  do {
    if (($3 | 0) != ($0 | 0)) {
      if (HEAP32[$__v >> 2] >>> 0 < HEAP32[$3 + 16 >> 2] >>> 0) {
        break;
      }
      HEAP32[$agg_result >> 2] = $3;
      STACKTOP = __stackBase__;
      return;
    }
  } while (0);
  HEAP32[$agg_result >> 2] = $0;
  STACKTOP = __stackBase__;
  return;
}
function __ZNSt3__13mapIP5BlockS2_NS_4lessIS2_EENS_9allocatorINS_4pairIKS2_S2_EEEEE16__construct_nodeERS7_($agg_result, $this, $__k) {
  var $call_i_i = __Znwj(24);
  var $__value_ = $call_i_i + 16 | 0;
  if (($__value_ | 0) != 0) {
    HEAP32[$__value_ >> 2] = HEAP32[$__k >> 2];
  }
  var $second = $call_i_i + 20 | 0;
  if (($second | 0) != 0) {
    HEAP32[$second >> 2] = 0;
  }
  HEAP32[$agg_result >> 2] = $call_i_i;
  HEAP32[$agg_result + 4 >> 2] = $this + 4 | 0;
  HEAP8[$agg_result + 8 | 0] = 1;
  HEAP8[$agg_result + 9 | 0] = 1;
  return;
}
function __ZNSt3__16__treeINS_4pairIP5BlockS3_EENS_19__map_value_compareIS3_S3_NS_4lessIS3_EELb1EEENS_9allocatorIS4_EEE16__insert_node_atEPNS_16__tree_node_baseIPvEERSF_SF_($this, $__parent, $__child, $__new_node) {
  HEAP32[$__new_node >> 2] = 0;
  HEAP32[$__new_node + 4 >> 2] = 0;
  HEAP32[$__new_node + 8 >> 2] = $__parent;
  HEAP32[$__child >> 2] = $__new_node;
  var $__begin_node__i42 = $this | 0;
  var $1 = HEAP32[HEAP32[$__begin_node__i42 >> 2] >> 2];
  if (($1 | 0) == 0) {
    var $3 = $__new_node;
  } else {
    HEAP32[$__begin_node__i42 >> 2] = $1;
    var $3 = HEAP32[$__child >> 2];
  }
  var $3;
  __ZNSt3__127__tree_balance_after_insertIPNS_16__tree_node_baseIPvEEEEvT_S5_(HEAP32[$this + 4 >> 2], $3);
  var $__first__i_i_i = $this + 8 | 0;
  HEAP32[$__first__i_i_i >> 2] = HEAP32[$__first__i_i_i >> 2] + 1 | 0;
  return;
}
function __ZZZN8Relooper9CalculateEP5BlockEN8Analyzer21FindIndependentGroupsE_0RNSt3__13setIS1_NS3_4lessIS1_EENS3_9allocatorIS1_EEEESA_RNS3_3mapIS1_S9_S6_NS7_INS3_4pairIKS1_S9_EEEEEEEN11HelperClassC2ESH_($this, $IndependentGroupsInit) {
  HEAP32[$this >> 2] = $IndependentGroupsInit;
  __ZNSt3__16__treeINS_4pairIP5BlockS3_EENS_19__map_value_compareIS3_S3_NS_4lessIS3_EELb1EEENS_9allocatorIS4_EEEC1ERKS8_($this + 4 | 0);
  return;
}
function __ZNSt3__16__treeINS_4pairIP5BlockS3_EENS_19__map_value_compareIS3_S3_NS_4lessIS3_EELb1EEENS_9allocatorIS4_EEEC1ERKS8_($this) {
  __ZNSt3__16__treeINS_4pairIP5BlockS3_EENS_19__map_value_compareIS3_S3_NS_4lessIS3_EELb1EEENS_9allocatorIS4_EEEC2ERKS8_($this);
  return;
}
function __ZNSt3__16__treeINS_4pairIP5BlockNS_3setIS3_NS_4lessIS3_EENS_9allocatorIS3_EEEEEENS_19__map_value_compareIS3_S9_S6_Lb1EEENS7_ISA_EEEC1ERKSC_($this) {
  __ZNSt3__16__treeINS_4pairIP5BlockNS_3setIS3_NS_4lessIS3_EENS_9allocatorIS3_EEEEEENS_19__map_value_compareIS3_S9_S6_Lb1EEENS7_ISA_EEEC2ERKSC_($this);
  return;
}
function __ZN9LoopShapeC1Ev($this) {
  __ZN9LoopShapeC2Ev($this);
  return;
}
function __ZN9LoopShapeC2Ev($this) {
  __ZN12LabeledShapeC2EN5Shape9ShapeTypeE($this | 0, 2);
  HEAP32[$this >> 2] = 5244352;
  HEAP32[$this + 20 >> 2] = 0;
  return;
}
function __ZNSt3__16__treeIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEEC1ERKS7_($this) {
  __ZNSt3__16__treeIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEEC2ERKS7_($this);
  return;
}
function __ZNSt3__16__treeIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEE15__insert_uniqueENS_21__tree_const_iteratorIS2_PKNS_11__tree_nodeIS2_PvEEiEERKS2_($agg_result, $this, $__p_0, $__v) {
  var __stackBase__ = STACKTOP;
  STACKTOP = STACKTOP + 16 | 0;
  var $__parent = __stackBase__;
  var $temp_lvalue = __stackBase__ + 4;
  var $call = __ZNSt3__16__treeIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEE12__find_equalIS2_EERPNS_16__tree_node_baseIPvEENS_21__tree_const_iteratorIS2_PKNS_11__tree_nodeIS2_SA_EEiEESD_RKT_($this, $__p_0, $__parent, $__v);
  var $0 = HEAP32[$call >> 2];
  if (($0 | 0) != 0) {
    var $__r_0 = $0;
    var $__r_0;
    var $__ptr__i_i = $agg_result | 0;
    HEAP32[$__ptr__i_i >> 2] = $__r_0;
    STACKTOP = __stackBase__;
    return;
  }
  __ZNSt3__16__treeIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEE16__construct_nodeERKS2_($temp_lvalue, $this, $__v);
  var $__first__i_i_i28_i_i = $temp_lvalue | 0;
  var $2 = HEAP32[$__first__i_i_i28_i_i >> 2];
  HEAP32[$__first__i_i_i28_i_i >> 2] = 0;
  __ZNSt3__16__treeIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEE16__insert_node_atEPNS_16__tree_node_baseIPvEERSB_SB_($this, HEAP32[$__parent >> 2], $call, $2 | 0);
  var $__r_0 = $2;
  var $__r_0;
  var $__ptr__i_i = $agg_result | 0;
  HEAP32[$__ptr__i_i >> 2] = $__r_0;
  STACKTOP = __stackBase__;
  return;
}
function __ZNSt3__16__treeIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEE12__find_equalIS2_EERPNS_16__tree_node_baseIPvEENS_21__tree_const_iteratorIS2_PKNS_11__tree_nodeIS2_SA_EEiEESD_RKT_($this, $__hint_0, $__parent, $__v) {
  var $__parent$s2 = $__parent >> 2;
  var $__first__i_i_i_i = $this + 4 | 0;
  do {
    if (($__first__i_i_i_i | 0) != ($__hint_0 | 0)) {
      var $1 = HEAP32[$__v >> 2];
      var $2 = HEAP32[$__hint_0 + 16 >> 2];
      if ($1 >>> 0 < $2 >>> 0) {
        break;
      }
      var $12 = $__hint_0 | 0;
      if ($2 >>> 0 >= $1 >>> 0) {
        HEAP32[$__parent$s2] = $12;
        var $retval_0 = $__parent;
        var $retval_0;
        return $retval_0;
      }
      var $call_i_i_i_i = __ZNSt3__111__tree_nextIPKNS_16__tree_node_baseIPvEEEET_S6_($12);
      do {
        if (($call_i_i_i_i | 0) != ($__first__i_i_i_i | 0)) {
          if ($1 >>> 0 < HEAP32[$call_i_i_i_i + 16 >> 2] >>> 0) {
            break;
          }
          var $retval_0 = __ZNSt3__16__treeIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEE12__find_equalIS2_EERPNS_16__tree_node_baseIPvEESD_RKT_($this, $__parent, $__v);
          var $retval_0;
          return $retval_0;
        }
      } while (0);
      var $__right_36 = $__hint_0 + 4 | 0;
      if ((HEAP32[$__right_36 >> 2] | 0) == 0) {
        HEAP32[$__parent$s2] = $12;
        var $retval_0 = $__right_36;
        var $retval_0;
        return $retval_0;
      } else {
        HEAP32[$__parent$s2] = $call_i_i_i_i;
        var $retval_0 = $call_i_i_i_i | 0;
        var $retval_0;
        return $retval_0;
      }
    }
  } while (0);
  do {
    if ((HEAP32[$this >> 2] | 0) == ($__hint_0 | 0)) {
      var $__prior_sroa_0_0 = $__hint_0;
    } else {
      var $call_i = __ZNSt3__111__tree_prevIPKNS_16__tree_node_baseIPvEEEET_S6_($__hint_0 | 0);
      if (HEAP32[$call_i + 16 >> 2] >>> 0 < HEAP32[$__v >> 2] >>> 0) {
        var $__prior_sroa_0_0 = $call_i;
        break;
      }
      var $retval_0 = __ZNSt3__16__treeIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEE12__find_equalIS2_EERPNS_16__tree_node_baseIPvEESD_RKT_($this, $__parent, $__v);
      var $retval_0;
      return $retval_0;
    }
  } while (0);
  var $__prior_sroa_0_0;
  var $__left_ = $__hint_0 | 0;
  if ((HEAP32[$__left_ >> 2] | 0) == 0) {
    HEAP32[$__parent$s2] = $__hint_0 | 0;
    var $retval_0 = $__left_;
    var $retval_0;
    return $retval_0;
  } else {
    HEAP32[$__parent$s2] = $__prior_sroa_0_0 | 0;
    var $retval_0 = $__prior_sroa_0_0 + 4 | 0;
    var $retval_0;
    return $retval_0;
  }
}
__ZNSt3__16__treeIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEE12__find_equalIS2_EERPNS_16__tree_node_baseIPvEENS_21__tree_const_iteratorIS2_PKNS_11__tree_nodeIS2_SA_EEiEESD_RKT_["X"] = 1;
function __ZN11SimpleShapeC1Ev($this) {
  __ZN11SimpleShapeC2Ev($this);
  return;
}
function __ZN11SimpleShapeD1Ev($this) {
  return;
}
function __ZNK10__cxxabiv116__shim_type_info5noop1Ev($this) {
  return;
}
function __ZNK10__cxxabiv116__shim_type_info5noop2Ev($this) {
  return;
}
function __ZN10__cxxabiv18is_equalEPKSt9type_infoS2_b($x, $y) {
  return ($x | 0) == ($y | 0);
}
function __ZN16RelooperRecursorC2EP8Relooper($this, $ParentInit) {
  HEAP32[$this >> 2] = $ParentInit;
  return;
}
function __ZNSt3__112__deque_baseIP5ShapeNS_9allocatorIS2_EEE5beginEv($agg_result, $this_0_0_1_val, $this_0_0_2_val, $this_0_1_val) {
  var $add_ptr = ($this_0_1_val >>> 10 << 2) + $this_0_0_1_val | 0;
  if (($this_0_0_2_val | 0) == ($this_0_0_1_val | 0)) {
    var $cond = 0;
  } else {
    var $cond = (($this_0_1_val & 1023) << 2) + HEAP32[$add_ptr >> 2] | 0;
  }
  var $cond;
  HEAP32[$agg_result >> 2] = $add_ptr;
  HEAP32[$agg_result + 4 >> 2] = $cond;
  return;
}
function __ZNSt3__112__deque_baseIP5BlockNS_9allocatorIS2_EEE5beginEv($agg_result, $this_0_0_1_val, $this_0_0_2_val, $this_0_1_val) {
  var $add_ptr = ($this_0_1_val >>> 10 << 2) + $this_0_0_1_val | 0;
  if (($this_0_0_2_val | 0) == ($this_0_0_1_val | 0)) {
    var $cond = 0;
  } else {
    var $cond = (($this_0_1_val & 1023) << 2) + HEAP32[$add_ptr >> 2] | 0;
  }
  var $cond;
  HEAP32[$agg_result >> 2] = $add_ptr;
  HEAP32[$agg_result + 4 >> 2] = $cond;
  return;
}
function __ZN10ministringC2Ev($this) {
  HEAP32[$this >> 2] = 0;
  HEAP32[$this + 4 >> 2] = 0;
  HEAP32[$this + 8 >> 2] = 0;
  return;
}
function __ZNK10__cxxabiv117__class_type_info24process_found_base_classEPNS_19__dynamic_cast_infoEPvi($info, $adjustedPtr, $path_below) {
  var $dst_ptr_leading_to_static_ptr = $info + 16 | 0;
  var $0 = HEAP32[$dst_ptr_leading_to_static_ptr >> 2];
  if (($0 | 0) == 0) {
    HEAP32[$dst_ptr_leading_to_static_ptr >> 2] = $adjustedPtr;
    HEAP32[$info + 24 >> 2] = $path_below;
    HEAP32[$info + 36 >> 2] = 1;
    return;
  }
  if (($0 | 0) != ($adjustedPtr | 0)) {
    var $number_to_static_ptr11 = $info + 36 | 0;
    HEAP32[$number_to_static_ptr11 >> 2] = HEAP32[$number_to_static_ptr11 >> 2] + 1 | 0;
    HEAP32[$info + 24 >> 2] = 2;
    HEAP8[$info + 54 | 0] = 1;
    return;
  }
  var $path_dst_ptr_to_static_ptr6 = $info + 24 | 0;
  if ((HEAP32[$path_dst_ptr_to_static_ptr6 >> 2] | 0) != 2) {
    return;
  }
  HEAP32[$path_dst_ptr_to_static_ptr6 >> 2] = $path_below;
  return;
}
function __ZNK10__cxxabiv117__class_type_info29process_static_type_above_dstEPNS_19__dynamic_cast_infoEPKvS4_i($info, $dst_ptr, $current_ptr, $path_below) {
  var $info$s2 = $info >> 2;
  HEAP8[$info + 53 | 0] = 1;
  if ((HEAP32[$info$s2 + 1] | 0) != ($current_ptr | 0)) {
    return;
  }
  HEAP8[$info + 52 | 0] = 1;
  var $dst_ptr_leading_to_static_ptr = $info + 16 | 0;
  var $1 = HEAP32[$dst_ptr_leading_to_static_ptr >> 2];
  if (($1 | 0) == 0) {
    HEAP32[$dst_ptr_leading_to_static_ptr >> 2] = $dst_ptr;
    HEAP32[$info$s2 + 6] = $path_below;
    HEAP32[$info$s2 + 9] = 1;
    if (!((HEAP32[$info$s2 + 12] | 0) == 1 & ($path_below | 0) == 1)) {
      return;
    }
    HEAP8[$info + 54 | 0] = 1;
    return;
  }
  if (($1 | 0) != ($dst_ptr | 0)) {
    var $number_to_static_ptr26 = $info + 36 | 0;
    HEAP32[$number_to_static_ptr26 >> 2] = HEAP32[$number_to_static_ptr26 >> 2] + 1 | 0;
    HEAP8[$info + 54 | 0] = 1;
    return;
  }
  var $path_dst_ptr_to_static_ptr12 = $info + 24 | 0;
  var $3 = HEAP32[$path_dst_ptr_to_static_ptr12 >> 2];
  if (($3 | 0) == 2) {
    HEAP32[$path_dst_ptr_to_static_ptr12 >> 2] = $path_below;
    var $4 = $path_below;
  } else {
    var $4 = $3;
  }
  var $4;
  if (!((HEAP32[$info$s2 + 12] | 0) == 1 & ($4 | 0) == 1)) {
    return;
  }
  HEAP8[$info + 54 | 0] = 1;
  return;
}
__ZNK10__cxxabiv117__class_type_info29process_static_type_above_dstEPNS_19__dynamic_cast_infoEPKvS4_i["X"] = 1;
function __ZNK10__cxxabiv117__class_type_info29process_static_type_below_dstEPNS_19__dynamic_cast_infoEPKvi($info, $current_ptr, $path_below) {
  if ((HEAP32[$info + 4 >> 2] | 0) != ($current_ptr | 0)) {
    return;
  }
  var $path_dynamic_ptr_to_static_ptr = $info + 28 | 0;
  if ((HEAP32[$path_dynamic_ptr_to_static_ptr >> 2] | 0) == 1) {
    return;
  }
  HEAP32[$path_dynamic_ptr_to_static_ptr >> 2] = $path_below;
  return;
}
function __ZZN8Relooper9CalculateEP5BlockEN8Analyzer12GetBlocksOutE_0S1_RNSt3__13setIS1_NS3_4lessIS1_EENS3_9allocatorIS1_EEEEPS9_($Source, $Entries, $LimitTo) {
  var __stackBase__ = STACKTOP;
  STACKTOP = STACKTOP + 12 | 0;
  var $agg_tmp_i102 = __stackBase__;
  var $ref_tmp_i = __stackBase__ + 4;
  var $0 = HEAP32[$Source >> 2];
  var $__first__i_i_i_i_i = $Source + 4 | 0;
  if (($0 | 0) == ($__first__i_i_i_i_i | 0)) {
    STACKTOP = __stackBase__;
    return;
  }
  var $__tree__i47 = $Entries | 0;
  var $__tree__i104 = $LimitTo | 0;
  var $agg_tmp2_i100_sroa_0_0_tmp3_i106_idx = $agg_tmp_i102 | 0;
  var $2 = $LimitTo + 4 | 0;
  var $3 = $__first__i_i_i_i_i;
  if (($LimitTo | 0) == 0) {
    var $iter_sroa_0_02_us = $0;
    while (1) {
      var $iter_sroa_0_02_us;
      __ZNSt3__16__treeIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEE15__insert_uniqueERKS2_($ref_tmp_i, $__tree__i47, $iter_sroa_0_02_us + 16 | 0);
      var $call_i_i_i_us = __ZNSt3__111__tree_nextIPNS_16__tree_node_baseIPvEEEET_S5_($iter_sroa_0_02_us | 0);
      if (($call_i_i_i_us | 0) == ($3 | 0)) {
        break;
      } else {
        var $iter_sroa_0_02_us = $call_i_i_i_us;
      }
    }
    STACKTOP = __stackBase__;
    return;
  } else {
    var $iter_sroa_0_02 = $0;
  }
  while (1) {
    var $iter_sroa_0_02;
    var $first = $iter_sroa_0_02 + 16 | 0;
    __ZNSt3__16__treeIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEE4findIS2_EENS_15__tree_iteratorIS2_PNS_11__tree_nodeIS2_PvEEiEERKT_($agg_tmp_i102, $__tree__i104, $first);
    if ((HEAP32[$agg_tmp2_i100_sroa_0_0_tmp3_i106_idx >> 2] | 0) != ($2 | 0)) {
      __ZNSt3__16__treeIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEE15__insert_uniqueERKS2_($ref_tmp_i, $__tree__i47, $first);
    }
    var $call_i_i_i = __ZNSt3__111__tree_nextIPNS_16__tree_node_baseIPvEEEET_S5_($iter_sroa_0_02 | 0);
    if (($call_i_i_i | 0) == ($3 | 0)) {
      break;
    } else {
      var $iter_sroa_0_02 = $call_i_i_i;
    }
  }
  STACKTOP = __stackBase__;
  return;
}
function __ZN11SimpleShapeC2Ev($this) {
  __ZN5ShapeC2ENS_9ShapeTypeE($this | 0, 0);
  HEAP32[$this >> 2] = 5244448;
  HEAP32[$this + 16 >> 2] = 0;
  return;
}
function __ZN11SimpleShapeD0Ev($this) {
  __ZdlPv($this);
  return;
}
function __ZN11SimpleShape6RenderEb($this, $InLoop) {
  __ZN5Block6RenderEb(HEAP32[$this + 16 >> 2], $InLoop);
  var $1 = HEAP32[$this + 8 >> 2];
  if (($1 | 0) == 0) {
    return;
  }
  FUNCTION_TABLE[HEAP32[HEAP32[$1 >> 2] + 8 >> 2]]($1, $InLoop);
  return;
}
function __ZNSt3__16__treeIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEE5clearEv($this) {
  var $__first__i_i_i_i = $this + 4 | 0;
  var $__left__i = $__first__i_i_i_i | 0;
  __ZNSt3__16__treeIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEE7destroyEPNS_11__tree_nodeIS2_PvEE(HEAP32[$__left__i >> 2]);
  HEAP32[$this + 8 >> 2] = 0;
  HEAP32[$this >> 2] = $__first__i_i_i_i;
  HEAP32[$__left__i >> 2] = 0;
  return;
}
function __ZZN8Relooper9CalculateEP5BlockEN8AnalyzerC2E_0PS_($this, $Parent) {
  __ZN16RelooperRecursorC2EP8Relooper($this | 0, $Parent);
  return;
}
function __ZZN8Relooper9CalculateEP5BlockEN12PreOptimizerC2EPS_($this, $Parent) {
  __ZN16RelooperRecursorC2EP8Relooper($this | 0, $Parent);
  __ZNSt3__16__treeIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEEC1ERKS4_($this + 4 | 0);
  return;
}
function __ZNSt3__15dequeIP5ShapeNS_9allocatorIS2_EEED2Ev($this) {
  __ZNSt3__112__deque_baseIP5ShapeNS_9allocatorIS2_EEED2Ev($this | 0);
  return;
}
function __ZNSt3__112__deque_baseIP5ShapeNS_9allocatorIS2_EEED2Ev($this) {
  __ZNSt3__112__deque_baseIP5ShapeNS_9allocatorIS2_EEE5clearEv($this);
  var $__map_ = $this | 0;
  var $0 = HEAP32[$this + 4 >> 2];
  var $1 = HEAP32[$this + 8 >> 2];
  if (($0 | 0) == ($1 | 0)) {
    __ZNSt3__114__split_bufferIPP5ShapeNS_9allocatorIS3_EEED1Ev($__map_);
    return;
  } else {
    var $__i_04 = $0;
  }
  while (1) {
    var $__i_04;
    __ZdlPv(HEAP32[$__i_04 >> 2]);
    var $incdec_ptr = $__i_04 + 4 | 0;
    if (($incdec_ptr | 0) == ($1 | 0)) {
      break;
    } else {
      var $__i_04 = $incdec_ptr;
    }
  }
  __ZNSt3__114__split_bufferIPP5ShapeNS_9allocatorIS3_EEED1Ev($__map_);
  return;
}
function __ZNSt3__112__deque_baseIP5ShapeNS_9allocatorIS2_EEE5clearEv($this) {
  var $this_idx2$s2;
  var $this_idx1$s2;
  var $this_idx$s2;
  var __stackBase__ = STACKTOP;
  STACKTOP = STACKTOP + 16 | 0;
  var $__i = __stackBase__;
  var $__e = __stackBase__ + 8;
  var $this_idx$s2 = ($this + 4 | 0) >> 2;
  var $this_idx1$s2 = ($this + 8 | 0) >> 2;
  var $this_idx2$s2 = ($this + 16 | 0) >> 2;
  __ZNSt3__112__deque_baseIP5ShapeNS_9allocatorIS2_EEE5beginEv($__i, HEAP32[$this_idx$s2], HEAP32[$this_idx1$s2], HEAP32[$this_idx2$s2]);
  __ZNSt3__112__deque_baseIP5ShapeNS_9allocatorIS2_EEE3endEv($__e, $this);
  var $__ptr__i_i = $__i + 4 | 0;
  var $0 = HEAP32[$__ptr__i_i >> 2];
  var $1 = HEAP32[$__e + 4 >> 2];
  if (($0 | 0) != ($1 | 0)) {
    var $__m_iter__i = $__i | 0;
    var $_pre = HEAP32[$__m_iter__i >> 2];
    var $4 = $0;
    var $3 = $_pre;
    var $2 = HEAP32[$_pre >> 2];
    while (1) {
      var $2;
      var $3;
      var $4;
      var $incdec_ptr_i = $4 + 4 | 0;
      if (($incdec_ptr_i - $2 | 0) == 4096) {
        var $incdec_ptr3_i = $3 + 4 | 0;
        HEAP32[$__m_iter__i >> 2] = $incdec_ptr3_i;
        var $8 = HEAP32[$incdec_ptr3_i >> 2];
        var $7 = $8;
        var $6 = $incdec_ptr3_i;
        var $5 = $8;
      } else {
        var $7 = $incdec_ptr_i;
        var $6 = $3;
        var $5 = $2;
      }
      var $5;
      var $6;
      var $7;
      if (($7 | 0) == ($1 | 0)) {
        break;
      } else {
        var $4 = $7;
        var $3 = $6;
        var $2 = $5;
      }
    }
    HEAP32[$__ptr__i_i >> 2] = $1;
  }
  HEAP32[$this + 20 >> 2] = 0;
  var $10 = HEAP32[$this_idx$s2];
  var $sub_ptr_div_i455 = HEAP32[$this_idx1$s2] - $10 >> 2;
  var $cmp6 = $sub_ptr_div_i455 >>> 0 > 2;
  L1647 : do {
    if ($cmp6) {
      var $11 = $10;
      while (1) {
        var $11;
        __ZdlPv(HEAP32[$11 >> 2]);
        var $add_ptr_i = HEAP32[$this_idx$s2] + 4 | 0;
        HEAP32[$this_idx$s2] = $add_ptr_i;
        var $sub_ptr_div_i45 = HEAP32[$this_idx1$s2] - $add_ptr_i >> 2;
        if ($sub_ptr_div_i45 >>> 0 > 2) {
          var $11 = $add_ptr_i;
        } else {
          var $sub_ptr_div_i45_lcssa = $sub_ptr_div_i45;
          break L1647;
        }
      }
    } else {
      var $sub_ptr_div_i45_lcssa = $sub_ptr_div_i455;
    }
  } while (0);
  var $sub_ptr_div_i45_lcssa;
  if (($sub_ptr_div_i45_lcssa | 0) == 1) {
    HEAP32[$this_idx2$s2] = 512;
    STACKTOP = __stackBase__;
    return;
  } else if (($sub_ptr_div_i45_lcssa | 0) == 2) {
    HEAP32[$this_idx2$s2] = 1024;
    STACKTOP = __stackBase__;
    return;
  } else {
    STACKTOP = __stackBase__;
    return;
  }
}
__ZNSt3__112__deque_baseIP5ShapeNS_9allocatorIS2_EEE5clearEv["X"] = 1;
function __ZNSt3__15dequeIP5BlockNS_9allocatorIS2_EEED2Ev($this) {
  __ZNSt3__112__deque_baseIP5BlockNS_9allocatorIS2_EEED2Ev($this | 0);
  return;
}
function __ZNSt3__112__deque_baseIP5BlockNS_9allocatorIS2_EEED2Ev($this) {
  __ZNSt3__112__deque_baseIP5BlockNS_9allocatorIS2_EEE5clearEv($this);
  var $__map_ = $this | 0;
  var $0 = HEAP32[$this + 4 >> 2];
  var $1 = HEAP32[$this + 8 >> 2];
  if (($0 | 0) == ($1 | 0)) {
    __ZNSt3__114__split_bufferIPP5BlockNS_9allocatorIS3_EEED1Ev($__map_);
    return;
  } else {
    var $__i_04 = $0;
  }
  while (1) {
    var $__i_04;
    __ZdlPv(HEAP32[$__i_04 >> 2]);
    var $incdec_ptr = $__i_04 + 4 | 0;
    if (($incdec_ptr | 0) == ($1 | 0)) {
      break;
    } else {
      var $__i_04 = $incdec_ptr;
    }
  }
  __ZNSt3__114__split_bufferIPP5BlockNS_9allocatorIS3_EEED1Ev($__map_);
  return;
}
function __ZNSt3__112__deque_baseIP5BlockNS_9allocatorIS2_EEE5clearEv($this) {
  var $this_idx2$s2;
  var $this_idx1$s2;
  var $this_idx$s2;
  var __stackBase__ = STACKTOP;
  STACKTOP = STACKTOP + 16 | 0;
  var $__i = __stackBase__;
  var $__e = __stackBase__ + 8;
  var $this_idx$s2 = ($this + 4 | 0) >> 2;
  var $this_idx1$s2 = ($this + 8 | 0) >> 2;
  var $this_idx2$s2 = ($this + 16 | 0) >> 2;
  __ZNSt3__112__deque_baseIP5BlockNS_9allocatorIS2_EEE5beginEv($__i, HEAP32[$this_idx$s2], HEAP32[$this_idx1$s2], HEAP32[$this_idx2$s2]);
  __ZNSt3__112__deque_baseIP5BlockNS_9allocatorIS2_EEE3endEv($__e, $this);
  var $__ptr__i_i = $__i + 4 | 0;
  var $0 = HEAP32[$__ptr__i_i >> 2];
  var $1 = HEAP32[$__e + 4 >> 2];
  if (($0 | 0) != ($1 | 0)) {
    var $__m_iter__i = $__i | 0;
    var $_pre = HEAP32[$__m_iter__i >> 2];
    var $4 = $0;
    var $3 = $_pre;
    var $2 = HEAP32[$_pre >> 2];
    while (1) {
      var $2;
      var $3;
      var $4;
      var $incdec_ptr_i = $4 + 4 | 0;
      if (($incdec_ptr_i - $2 | 0) == 4096) {
        var $incdec_ptr3_i = $3 + 4 | 0;
        HEAP32[$__m_iter__i >> 2] = $incdec_ptr3_i;
        var $8 = HEAP32[$incdec_ptr3_i >> 2];
        var $7 = $8;
        var $6 = $incdec_ptr3_i;
        var $5 = $8;
      } else {
        var $7 = $incdec_ptr_i;
        var $6 = $3;
        var $5 = $2;
      }
      var $5;
      var $6;
      var $7;
      if (($7 | 0) == ($1 | 0)) {
        break;
      } else {
        var $4 = $7;
        var $3 = $6;
        var $2 = $5;
      }
    }
    HEAP32[$__ptr__i_i >> 2] = $1;
  }
  HEAP32[$this + 20 >> 2] = 0;
  var $10 = HEAP32[$this_idx$s2];
  var $sub_ptr_div_i455 = HEAP32[$this_idx1$s2] - $10 >> 2;
  var $cmp6 = $sub_ptr_div_i455 >>> 0 > 2;
  L1674 : do {
    if ($cmp6) {
      var $11 = $10;
      while (1) {
        var $11;
        __ZdlPv(HEAP32[$11 >> 2]);
        var $add_ptr_i = HEAP32[$this_idx$s2] + 4 | 0;
        HEAP32[$this_idx$s2] = $add_ptr_i;
        var $sub_ptr_div_i45 = HEAP32[$this_idx1$s2] - $add_ptr_i >> 2;
        if ($sub_ptr_div_i45 >>> 0 > 2) {
          var $11 = $add_ptr_i;
        } else {
          var $sub_ptr_div_i45_lcssa = $sub_ptr_div_i45;
          break L1674;
        }
      }
    } else {
      var $sub_ptr_div_i45_lcssa = $sub_ptr_div_i455;
    }
  } while (0);
  var $sub_ptr_div_i45_lcssa;
  if (($sub_ptr_div_i45_lcssa | 0) == 2) {
    HEAP32[$this_idx2$s2] = 1024;
    STACKTOP = __stackBase__;
    return;
  } else if (($sub_ptr_div_i45_lcssa | 0) == 1) {
    HEAP32[$this_idx2$s2] = 512;
    STACKTOP = __stackBase__;
    return;
  } else {
    STACKTOP = __stackBase__;
    return;
  }
}
__ZNSt3__112__deque_baseIP5BlockNS_9allocatorIS2_EEE5clearEv["X"] = 1;
function __ZN10ministringD2Ev($this_0_1_val) {
  if (($this_0_1_val | 0) == 0) {
    return;
  }
  _free($this_0_1_val);
  return;
}
function __ZNSt3__13mapIP5BlockP6BranchNS_4lessIS2_EENS_9allocatorINS_4pairIKS2_S4_EEEEED2Ev($this) {
  __ZNSt3__16__treeINS_4pairIP5BlockP6BranchEENS_19__map_value_compareIS3_S5_NS_4lessIS3_EELb1EEENS_9allocatorIS6_EEED1Ev(HEAP32[$this + 4 >> 2]);
  return;
}
function __ZNSt3__16__treeINS_4pairIP5BlockP6BranchEENS_19__map_value_compareIS3_S5_NS_4lessIS3_EELb1EEENS_9allocatorIS6_EEED1Ev($this_0_1_0_0_0_val) {
  __ZNSt3__16__treeINS_4pairIP5BlockP6BranchEENS_19__map_value_compareIS3_S5_NS_4lessIS3_EELb1EEENS_9allocatorIS6_EEED2Ev($this_0_1_0_0_0_val);
  return;
}
function __ZNSt3__16__treeINS_4pairIP5BlockP6BranchEENS_19__map_value_compareIS3_S5_NS_4lessIS3_EELb1EEENS_9allocatorIS6_EEED2Ev($this_0_1_0_0_0_val) {
  __ZNSt3__16__treeINS_4pairIP5BlockP6BranchEENS_19__map_value_compareIS3_S5_NS_4lessIS3_EELb1EEENS_9allocatorIS6_EEE7destroyEPNS_11__tree_nodeIS6_PvEE($this_0_1_0_0_0_val);
  return;
}
function __ZNSt3__16__treeINS_4pairIP5BlockP6BranchEENS_19__map_value_compareIS3_S5_NS_4lessIS3_EELb1EEENS_9allocatorIS6_EEE7destroyEPNS_11__tree_nodeIS6_PvEE($__nd) {
  if (($__nd | 0) == 0) {
    return;
  } else {
    __ZNSt3__16__treeINS_4pairIP5BlockP6BranchEENS_19__map_value_compareIS3_S5_NS_4lessIS3_EELb1EEENS_9allocatorIS6_EEE7destroyEPNS_11__tree_nodeIS6_PvEE(HEAP32[$__nd >> 2]);
    __ZNSt3__16__treeINS_4pairIP5BlockP6BranchEENS_19__map_value_compareIS3_S5_NS_4lessIS3_EELb1EEENS_9allocatorIS6_EEE7destroyEPNS_11__tree_nodeIS6_PvEE(HEAP32[$__nd + 4 >> 2]);
    __ZdlPv($__nd);
    return;
  }
}
function __GLOBAL__I_a() {
  ___cxx_global_var_init();
  return;
}
function __ZN10__cxxabiv116__shim_type_infoD2Ev($this) {
  __ZNSt9type_infoD2Ev($this | 0);
  return;
}
function __ZN10__cxxabiv117__class_type_infoD0Ev($this) {
  __ZN10__cxxabiv117__class_type_infoD1Ev($this);
  __ZdlPv($this);
  return;
}
function __ZN10__cxxabiv117__class_type_infoD1Ev($this) {
  __ZN10__cxxabiv117__class_type_infoD2Ev($this);
  return;
}
function __ZN10__cxxabiv117__class_type_infoD2Ev($this) {
  __ZN10__cxxabiv116__shim_type_infoD2Ev($this | 0);
  return;
}
function __ZN10__cxxabiv120__si_class_type_infoD0Ev($this) {
  __ZN10__cxxabiv120__si_class_type_infoD1Ev($this);
  __ZdlPv($this);
  return;
}
function __ZN10__cxxabiv120__si_class_type_infoD1Ev($this) {
  __ZN10__cxxabiv120__si_class_type_infoD2Ev($this);
  return;
}
function __ZN10__cxxabiv120__si_class_type_infoD2Ev($this) {
  __ZN10__cxxabiv117__class_type_infoD2Ev($this | 0);
  return;
}
function __ZNK10__cxxabiv117__class_type_info9can_catchEPKNS_16__shim_type_infoERPv($this, $thrown_type, $adjustedPtr) {
  var $info$s2;
  var __stackBase__ = STACKTOP;
  STACKTOP = STACKTOP + 56 | 0;
  var $info = __stackBase__, $info$s2 = $info >> 2;
  do {
    if (__ZN10__cxxabiv18is_equalEPKSt9type_infoS2_b($this | 0, $thrown_type | 0)) {
      var $retval_0 = 1;
    } else {
      if (($thrown_type | 0) == 0) {
        var $retval_0 = 0;
        break;
      }
      var $4 = ___dynamic_cast($thrown_type, 5244684);
      var $5 = $4;
      if (($4 | 0) == 0) {
        var $retval_0 = 0;
        break;
      }
      var $6 = $info;
      for (var $$dest = $6 >> 2, $$stop = $$dest + 14; $$dest < $$stop; $$dest++) {
        HEAP32[$$dest] = 0;
      }
      HEAP32[$info$s2] = $5;
      HEAP32[$info$s2 + 2] = $this;
      HEAP32[$info$s2 + 3] = -1;
      HEAP32[$info$s2 + 12] = 1;
      FUNCTION_TABLE[HEAP32[HEAP32[$4 >> 2] + 28 >> 2]]($5, $info, HEAP32[$adjustedPtr >> 2], 1);
      if ((HEAP32[$info$s2 + 6] | 0) != 1) {
        var $retval_0 = 0;
        break;
      }
      HEAP32[$adjustedPtr >> 2] = HEAP32[$info$s2 + 4];
      var $retval_0 = 1;
    }
  } while (0);
  var $retval_0;
  STACKTOP = __stackBase__;
  return $retval_0;
}
function __ZNK10__cxxabiv117__class_type_info27has_unambiguous_public_baseEPNS_19__dynamic_cast_infoEPvi($this, $info, $adjustedPtr, $path_below) {
  if (!__ZN10__cxxabiv18is_equalEPKSt9type_infoS2_b($this | 0, HEAP32[$info + 8 >> 2] | 0)) {
    return;
  }
  __ZNK10__cxxabiv117__class_type_info24process_found_base_classEPNS_19__dynamic_cast_infoEPvi($info, $adjustedPtr, $path_below);
  return;
}
function __ZNK10__cxxabiv120__si_class_type_info27has_unambiguous_public_baseEPNS_19__dynamic_cast_infoEPvi($this, $info, $adjustedPtr, $path_below) {
  if (__ZN10__cxxabiv18is_equalEPKSt9type_infoS2_b($this | 0, HEAP32[$info + 8 >> 2] | 0)) {
    __ZNK10__cxxabiv117__class_type_info24process_found_base_classEPNS_19__dynamic_cast_infoEPvi($info, $adjustedPtr, $path_below);
    return;
  } else {
    var $3 = HEAP32[$this + 8 >> 2];
    FUNCTION_TABLE[HEAP32[HEAP32[$3 >> 2] + 28 >> 2]]($3, $info, $adjustedPtr, $path_below);
    return;
  }
}
function ___dynamic_cast($static_ptr, $dst_type) {
  var $info$s2;
  var __stackBase__ = STACKTOP;
  STACKTOP = STACKTOP + 56 | 0;
  var $info = __stackBase__, $info$s2 = $info >> 2;
  var $1 = HEAP32[$static_ptr >> 2];
  var $add_ptr = $static_ptr + HEAP32[$1 - 8 >> 2] | 0;
  var $4 = HEAP32[$1 - 4 >> 2];
  var $5 = $4;
  HEAP32[$info$s2] = $dst_type;
  HEAP32[$info$s2 + 1] = $static_ptr;
  HEAP32[$info$s2 + 2] = 5244696;
  HEAP32[$info$s2 + 3] = -1;
  var $dst_ptr_leading_to_static_ptr = $info + 16 | 0;
  var $dst_ptr_not_leading_to_static_ptr = $info + 20 | 0;
  var $path_dst_ptr_to_static_ptr = $info + 24 | 0;
  var $path_dynamic_ptr_to_static_ptr = $info + 28 | 0;
  var $path_dynamic_ptr_to_dst_ptr = $info + 32 | 0;
  var $number_to_dst_ptr = $info + 40 | 0;
  var $call = __ZN10__cxxabiv18is_equalEPKSt9type_infoS2_b($4, $dst_type | 0);
  var $8 = $dst_ptr_leading_to_static_ptr;
  for (var $$dest = $8 >> 2, $$stop = $$dest + 9; $$dest < $$stop; $$dest++) {
    HEAP32[$$dest] = 0;
  }
  HEAP16[$8 + 36 >> 1] = 0;
  HEAP8[$8 + 38] = 0;
  if ($call) {
    HEAP32[$info$s2 + 12] = 1;
    FUNCTION_TABLE[HEAP32[HEAP32[$4 >> 2] + 20 >> 2]]($5, $info, $add_ptr, $add_ptr, 1, 0);
    STACKTOP = __stackBase__;
    return (HEAP32[$path_dst_ptr_to_static_ptr >> 2] | 0) == 1 ? $add_ptr : 0;
  }
  FUNCTION_TABLE[HEAP32[HEAP32[$4 >> 2] + 24 >> 2]]($5, $info, $add_ptr, 1, 0);
  var $14 = HEAP32[$info$s2 + 9];
  if (($14 | 0) == 0) {
    if ((HEAP32[$number_to_dst_ptr >> 2] | 0) != 1) {
      var $dst_ptr_0 = 0;
      var $dst_ptr_0;
      STACKTOP = __stackBase__;
      return $dst_ptr_0;
    }
    if ((HEAP32[$path_dynamic_ptr_to_static_ptr >> 2] | 0) != 1) {
      var $dst_ptr_0 = 0;
      var $dst_ptr_0;
      STACKTOP = __stackBase__;
      return $dst_ptr_0;
    }
    var $dst_ptr_0 = (HEAP32[$path_dynamic_ptr_to_dst_ptr >> 2] | 0) == 1 ? HEAP32[$dst_ptr_not_leading_to_static_ptr >> 2] : 0;
    var $dst_ptr_0;
    STACKTOP = __stackBase__;
    return $dst_ptr_0;
  } else if (($14 | 0) == 1) {
    do {
      if ((HEAP32[$path_dst_ptr_to_static_ptr >> 2] | 0) != 1) {
        if ((HEAP32[$number_to_dst_ptr >> 2] | 0) != 0) {
          var $dst_ptr_0 = 0;
          var $dst_ptr_0;
          STACKTOP = __stackBase__;
          return $dst_ptr_0;
        }
        if ((HEAP32[$path_dynamic_ptr_to_static_ptr >> 2] | 0) != 1) {
          var $dst_ptr_0 = 0;
          var $dst_ptr_0;
          STACKTOP = __stackBase__;
          return $dst_ptr_0;
        }
        if ((HEAP32[$path_dynamic_ptr_to_dst_ptr >> 2] | 0) == 1) {
          break;
        } else {
          var $dst_ptr_0 = 0;
        }
        var $dst_ptr_0;
        STACKTOP = __stackBase__;
        return $dst_ptr_0;
      }
    } while (0);
    var $dst_ptr_0 = HEAP32[$dst_ptr_leading_to_static_ptr >> 2];
    var $dst_ptr_0;
    STACKTOP = __stackBase__;
    return $dst_ptr_0;
  } else {
    var $dst_ptr_0 = 0;
    var $dst_ptr_0;
    STACKTOP = __stackBase__;
    return $dst_ptr_0;
  }
}
___dynamic_cast["X"] = 1;
function __ZNK10__cxxabiv120__si_class_type_info16search_below_dstEPNS_19__dynamic_cast_infoEPKvib($this, $info, $current_ptr, $path_below, $use_strcmp) {
  var $is_dst_type_derived_from_static_type$s2;
  var $info$s2 = $info >> 2;
  var label = 0;
  var $0 = $this | 0;
  if (__ZN10__cxxabiv18is_equalEPKSt9type_infoS2_b($0, HEAP32[$info$s2 + 2] | 0)) {
    __ZNK10__cxxabiv117__class_type_info29process_static_type_below_dstEPNS_19__dynamic_cast_infoEPKvi($info, $current_ptr, $path_below);
    return;
  }
  if (!__ZN10__cxxabiv18is_equalEPKSt9type_infoS2_b($0, HEAP32[$info$s2] | 0)) {
    var $18 = HEAP32[$this + 8 >> 2];
    FUNCTION_TABLE[HEAP32[HEAP32[$18 >> 2] + 24 >> 2]]($18, $info, $current_ptr, $path_below, $use_strcmp);
    return;
  }
  do {
    if ((HEAP32[$info$s2 + 4] | 0) != ($current_ptr | 0)) {
      var $dst_ptr_not_leading_to_static_ptr = $info + 20 | 0;
      if ((HEAP32[$dst_ptr_not_leading_to_static_ptr >> 2] | 0) == ($current_ptr | 0)) {
        break;
      }
      HEAP32[$info$s2 + 8] = $path_below;
      var $is_dst_type_derived_from_static_type$s2 = ($info + 44 | 0) >> 2;
      if ((HEAP32[$is_dst_type_derived_from_static_type$s2] | 0) == 4) {
        return;
      }
      var $found_our_static_ptr = $info + 52 | 0;
      HEAP8[$found_our_static_ptr] = 0;
      var $found_any_static_type = $info + 53 | 0;
      HEAP8[$found_any_static_type] = 0;
      var $8 = HEAP32[$this + 8 >> 2];
      FUNCTION_TABLE[HEAP32[HEAP32[$8 >> 2] + 20 >> 2]]($8, $info, $current_ptr, $current_ptr, 1, $use_strcmp);
      do {
        if ((HEAP8[$found_any_static_type] & 1) << 24 >> 24 == 0) {
          var $is_dst_type_derived_from_static_type13_032 = 0;
          label = 1467;
        } else {
          if ((HEAP8[$found_our_static_ptr] & 1) << 24 >> 24 == 0) {
            var $is_dst_type_derived_from_static_type13_032 = 1;
            label = 1467;
            break;
          } else {
            break;
          }
        }
      } while (0);
      L1767 : do {
        if (label == 1467) {
          var $is_dst_type_derived_from_static_type13_032;
          HEAP32[$dst_ptr_not_leading_to_static_ptr >> 2] = $current_ptr;
          var $number_to_dst_ptr = $info + 40 | 0;
          HEAP32[$number_to_dst_ptr >> 2] = HEAP32[$number_to_dst_ptr >> 2] + 1 | 0;
          do {
            if ((HEAP32[$info$s2 + 9] | 0) == 1) {
              if ((HEAP32[$info$s2 + 6] | 0) != 2) {
                label = 1470;
                break;
              }
              HEAP8[$info + 54 | 0] = 1;
              if ($is_dst_type_derived_from_static_type13_032) {
                break L1767;
              } else {
                break;
              }
            } else {
              label = 1470;
            }
          } while (0);
          if (label == 1470) {
            if ($is_dst_type_derived_from_static_type13_032) {
              break;
            }
          }
          HEAP32[$is_dst_type_derived_from_static_type$s2] = 4;
          return;
        }
      } while (0);
      HEAP32[$is_dst_type_derived_from_static_type$s2] = 3;
      return;
    }
  } while (0);
  if (($path_below | 0) != 1) {
    return;
  }
  HEAP32[$info$s2 + 8] = 1;
  return;
}
__ZNK10__cxxabiv120__si_class_type_info16search_below_dstEPNS_19__dynamic_cast_infoEPKvib["X"] = 1;
function __ZNK10__cxxabiv117__class_type_info16search_below_dstEPNS_19__dynamic_cast_infoEPKvib($this, $info, $current_ptr, $path_below, $use_strcmp) {
  var $info$s2 = $info >> 2;
  var $0 = $this | 0;
  if (__ZN10__cxxabiv18is_equalEPKSt9type_infoS2_b($0, HEAP32[$info$s2 + 2] | 0)) {
    __ZNK10__cxxabiv117__class_type_info29process_static_type_below_dstEPNS_19__dynamic_cast_infoEPKvi($info, $current_ptr, $path_below);
    return;
  }
  if (!__ZN10__cxxabiv18is_equalEPKSt9type_infoS2_b($0, HEAP32[$info$s2] | 0)) {
    return;
  }
  do {
    if ((HEAP32[$info$s2 + 4] | 0) != ($current_ptr | 0)) {
      var $dst_ptr_not_leading_to_static_ptr = $info + 20 | 0;
      if ((HEAP32[$dst_ptr_not_leading_to_static_ptr >> 2] | 0) == ($current_ptr | 0)) {
        break;
      }
      HEAP32[$info$s2 + 8] = $path_below;
      HEAP32[$dst_ptr_not_leading_to_static_ptr >> 2] = $current_ptr;
      var $number_to_dst_ptr = $info + 40 | 0;
      HEAP32[$number_to_dst_ptr >> 2] = HEAP32[$number_to_dst_ptr >> 2] + 1 | 0;
      do {
        if ((HEAP32[$info$s2 + 9] | 0) == 1) {
          if ((HEAP32[$info$s2 + 6] | 0) != 2) {
            break;
          }
          HEAP8[$info + 54 | 0] = 1;
        }
      } while (0);
      HEAP32[$info$s2 + 11] = 4;
      return;
    }
  } while (0);
  if (($path_below | 0) != 1) {
    return;
  }
  HEAP32[$info$s2 + 8] = 1;
  return;
}
function __ZNK10__cxxabiv120__si_class_type_info16search_above_dstEPNS_19__dynamic_cast_infoEPKvS4_ib($this, $info, $dst_ptr, $current_ptr, $path_below, $use_strcmp) {
  if (__ZN10__cxxabiv18is_equalEPKSt9type_infoS2_b($this | 0, HEAP32[$info + 8 >> 2] | 0)) {
    __ZNK10__cxxabiv117__class_type_info29process_static_type_above_dstEPNS_19__dynamic_cast_infoEPKvS4_i($info, $dst_ptr, $current_ptr, $path_below);
    return;
  } else {
    var $3 = HEAP32[$this + 8 >> 2];
    FUNCTION_TABLE[HEAP32[HEAP32[$3 >> 2] + 20 >> 2]]($3, $info, $dst_ptr, $current_ptr, $path_below, $use_strcmp);
    return;
  }
}
function __ZNK10__cxxabiv117__class_type_info16search_above_dstEPNS_19__dynamic_cast_infoEPKvS4_ib($this, $info, $dst_ptr, $current_ptr, $path_below, $use_strcmp) {
  if (!__ZN10__cxxabiv18is_equalEPKSt9type_infoS2_b($this | 0, HEAP32[$info + 8 >> 2] | 0)) {
    return;
  }
  __ZNK10__cxxabiv117__class_type_info29process_static_type_above_dstEPNS_19__dynamic_cast_infoEPKvS4_i($info, $dst_ptr, $current_ptr, $path_below);
  return;
}
function _malloc($bytes) {
  do {
    if ($bytes >>> 0 < 245) {
      if ($bytes >>> 0 < 11) {
        var $cond = 16;
      } else {
        var $cond = $bytes + 11 & -8;
      }
      var $cond;
      var $shr = $cond >>> 3;
      var $0 = HEAP32[1310877];
      var $shr3 = $0 >>> ($shr >>> 0);
      if (($shr3 & 3 | 0) != 0) {
        var $add8 = ($shr3 & 1 ^ 1) + $shr | 0;
        var $shl = $add8 << 1;
        var $1 = ($shl << 2) + 5243548 | 0;
        var $2 = ($shl + 2 << 2) + 5243548 | 0;
        var $3 = HEAP32[$2 >> 2];
        var $fd9 = $3 + 8 | 0;
        var $4 = HEAP32[$fd9 >> 2];
        do {
          if (($1 | 0) == ($4 | 0)) {
            HEAP32[1310877] = $0 & (1 << $add8 ^ -1);
          } else {
            if ($4 >>> 0 < HEAP32[1310881] >>> 0) {
              _abort();
            } else {
              HEAP32[$2 >> 2] = $4;
              HEAP32[$4 + 12 >> 2] = $1;
              break;
            }
          }
        } while (0);
        var $shl20 = $add8 << 3;
        HEAP32[$3 + 4 >> 2] = $shl20 | 3;
        var $8 = $3 + ($shl20 | 4) | 0;
        HEAP32[$8 >> 2] = HEAP32[$8 >> 2] | 1;
        var $mem_0 = $fd9;
        var $mem_0;
        return $mem_0;
      }
      if ($cond >>> 0 <= HEAP32[1310879] >>> 0) {
        var $nb_0 = $cond;
        break;
      }
      if (($shr3 | 0) == 0) {
        if ((HEAP32[1310878] | 0) == 0) {
          var $nb_0 = $cond;
          break;
        }
        var $call = _tmalloc_small($cond);
        if (($call | 0) == 0) {
          var $nb_0 = $cond;
          break;
        } else {
          var $mem_0 = $call;
        }
        var $mem_0;
        return $mem_0;
      }
      var $shl37 = 2 << $shr;
      var $and41 = $shr3 << $shr & ($shl37 | -$shl37);
      var $sub44 = ($and41 & -$and41) - 1 | 0;
      var $and46 = $sub44 >>> 12 & 16;
      var $shr47 = $sub44 >>> ($and46 >>> 0);
      var $and49 = $shr47 >>> 5 & 8;
      var $shr51 = $shr47 >>> ($and49 >>> 0);
      var $and53 = $shr51 >>> 2 & 4;
      var $shr55 = $shr51 >>> ($and53 >>> 0);
      var $and57 = $shr55 >>> 1 & 2;
      var $shr59 = $shr55 >>> ($and57 >>> 0);
      var $and61 = $shr59 >>> 1 & 1;
      var $add64 = ($and49 | $and46 | $and53 | $and57 | $and61) + ($shr59 >>> ($and61 >>> 0)) | 0;
      var $shl65 = $add64 << 1;
      var $12 = ($shl65 << 2) + 5243548 | 0;
      var $13 = ($shl65 + 2 << 2) + 5243548 | 0;
      var $14 = HEAP32[$13 >> 2];
      var $fd69 = $14 + 8 | 0;
      var $15 = HEAP32[$fd69 >> 2];
      do {
        if (($12 | 0) == ($15 | 0)) {
          HEAP32[1310877] = $0 & (1 << $add64 ^ -1);
        } else {
          if ($15 >>> 0 < HEAP32[1310881] >>> 0) {
            _abort();
          } else {
            HEAP32[$13 >> 2] = $15;
            HEAP32[$15 + 12 >> 2] = $12;
            break;
          }
        }
      } while (0);
      var $shl87 = $add64 << 3;
      var $sub88 = $shl87 - $cond | 0;
      HEAP32[$14 + 4 >> 2] = $cond | 3;
      var $18 = $14;
      var $19 = $18 + $cond | 0;
      HEAP32[$18 + ($cond | 4) >> 2] = $sub88 | 1;
      HEAP32[$18 + $shl87 >> 2] = $sub88;
      var $21 = HEAP32[1310879];
      if (($21 | 0) != 0) {
        var $22 = HEAP32[1310882];
        var $shr99 = $21 >>> 3;
        var $shl100 = $shr99 << 1;
        var $23 = ($shl100 << 2) + 5243548 | 0;
        var $24 = HEAP32[1310877];
        var $shl103 = 1 << $shr99;
        do {
          if (($24 & $shl103 | 0) == 0) {
            HEAP32[1310877] = $24 | $shl103;
            var $F102_0 = $23;
            var $_pre_phi = ($shl100 + 2 << 2) + 5243548 | 0;
          } else {
            var $25 = ($shl100 + 2 << 2) + 5243548 | 0;
            var $26 = HEAP32[$25 >> 2];
            if ($26 >>> 0 >= HEAP32[1310881] >>> 0) {
              var $F102_0 = $26;
              var $_pre_phi = $25;
              break;
            }
            _abort();
          }
        } while (0);
        var $_pre_phi;
        var $F102_0;
        HEAP32[$_pre_phi >> 2] = $22;
        HEAP32[$F102_0 + 12 >> 2] = $22;
        HEAP32[$22 + 8 >> 2] = $F102_0;
        HEAP32[$22 + 12 >> 2] = $23;
      }
      HEAP32[1310879] = $sub88;
      HEAP32[1310882] = $19;
      var $mem_0 = $fd69;
      var $mem_0;
      return $mem_0;
    } else {
      if ($bytes >>> 0 > 4294967231) {
        var $nb_0 = -1;
        break;
      }
      var $and143 = $bytes + 11 & -8;
      if ((HEAP32[1310878] | 0) == 0) {
        var $nb_0 = $and143;
        break;
      }
      var $call147 = _tmalloc_large($and143);
      if (($call147 | 0) == 0) {
        var $nb_0 = $and143;
        break;
      } else {
        var $mem_0 = $call147;
      }
      var $mem_0;
      return $mem_0;
    }
  } while (0);
  var $nb_0;
  var $32 = HEAP32[1310879];
  if ($nb_0 >>> 0 > $32 >>> 0) {
    var $41 = HEAP32[1310880];
    if ($nb_0 >>> 0 < $41 >>> 0) {
      var $sub186 = $41 - $nb_0 | 0;
      HEAP32[1310880] = $sub186;
      var $42 = HEAP32[1310883];
      var $43 = $42;
      HEAP32[1310883] = $43 + $nb_0 | 0;
      HEAP32[$nb_0 + ($43 + 4) >> 2] = $sub186 | 1;
      HEAP32[$42 + 4 >> 2] = $nb_0 | 3;
      var $mem_0 = $42 + 8 | 0;
      var $mem_0;
      return $mem_0;
    } else {
      var $mem_0 = _sys_alloc($nb_0);
      var $mem_0;
      return $mem_0;
    }
  } else {
    var $sub158 = $32 - $nb_0 | 0;
    var $33 = HEAP32[1310882];
    if ($sub158 >>> 0 > 15) {
      var $34 = $33;
      HEAP32[1310882] = $34 + $nb_0 | 0;
      HEAP32[1310879] = $sub158;
      HEAP32[$nb_0 + ($34 + 4) >> 2] = $sub158 | 1;
      HEAP32[$34 + $32 >> 2] = $sub158;
      HEAP32[$33 + 4 >> 2] = $nb_0 | 3;
    } else {
      HEAP32[1310879] = 0;
      HEAP32[1310882] = 0;
      HEAP32[$33 + 4 >> 2] = $32 | 3;
      var $38 = $32 + ($33 + 4) | 0;
      HEAP32[$38 >> 2] = HEAP32[$38 >> 2] | 1;
    }
    var $mem_0 = $33 + 8 | 0;
    var $mem_0;
    return $mem_0;
  }
}
_malloc["X"] = 1;
function _tmalloc_small($nb) {
  var $R_1$s2;
  var $v_0$s2;
  var $0 = HEAP32[1310878];
  var $sub2 = ($0 & -$0) - 1 | 0;
  var $and3 = $sub2 >>> 12 & 16;
  var $shr4 = $sub2 >>> ($and3 >>> 0);
  var $and6 = $shr4 >>> 5 & 8;
  var $shr7 = $shr4 >>> ($and6 >>> 0);
  var $and9 = $shr7 >>> 2 & 4;
  var $shr11 = $shr7 >>> ($and9 >>> 0);
  var $and13 = $shr11 >>> 1 & 2;
  var $shr15 = $shr11 >>> ($and13 >>> 0);
  var $and17 = $shr15 >>> 1 & 1;
  var $1 = HEAP32[(($and6 | $and3 | $and9 | $and13 | $and17) + ($shr15 >>> ($and17 >>> 0)) << 2) + 5243812 >> 2];
  var $t_0 = $1;
  var $v_0 = $1, $v_0$s2 = $v_0 >> 2;
  var $rsize_0 = (HEAP32[$1 + 4 >> 2] & -8) - $nb | 0;
  while (1) {
    var $rsize_0;
    var $v_0;
    var $t_0;
    var $3 = HEAP32[$t_0 + 16 >> 2];
    if (($3 | 0) == 0) {
      var $4 = HEAP32[$t_0 + 20 >> 2];
      if (($4 | 0) == 0) {
        break;
      } else {
        var $cond5 = $4;
      }
    } else {
      var $cond5 = $3;
    }
    var $cond5;
    var $sub31 = (HEAP32[$cond5 + 4 >> 2] & -8) - $nb | 0;
    var $cmp32 = $sub31 >>> 0 < $rsize_0 >>> 0;
    var $t_0 = $cond5;
    var $v_0 = $cmp32 ? $cond5 : $v_0, $v_0$s2 = $v_0 >> 2;
    var $rsize_0 = $cmp32 ? $sub31 : $rsize_0;
  }
  var $6 = $v_0;
  var $7 = HEAP32[1310881];
  if ($6 >>> 0 < $7 >>> 0) {
    _abort();
  }
  var $add_ptr = $6 + $nb | 0;
  var $8 = $add_ptr;
  if ($6 >>> 0 >= $add_ptr >>> 0) {
    _abort();
  }
  var $9 = HEAP32[$v_0$s2 + 6];
  var $10 = HEAP32[$v_0$s2 + 3];
  var $cmp40 = ($10 | 0) == ($v_0 | 0);
  L1885 : do {
    if ($cmp40) {
      var $arrayidx55 = $v_0 + 20 | 0;
      var $13 = HEAP32[$arrayidx55 >> 2];
      do {
        if (($13 | 0) == 0) {
          var $arrayidx59 = $v_0 + 16 | 0;
          var $14 = HEAP32[$arrayidx59 >> 2];
          if (($14 | 0) == 0) {
            var $R_1 = 0, $R_1$s2 = $R_1 >> 2;
            break L1885;
          } else {
            var $R_0 = $14;
            var $RP_0 = $arrayidx59;
            break;
          }
        } else {
          var $R_0 = $13;
          var $RP_0 = $arrayidx55;
        }
      } while (0);
      while (1) {
        var $RP_0;
        var $R_0;
        var $arrayidx65 = $R_0 + 20 | 0;
        var $15 = HEAP32[$arrayidx65 >> 2];
        if (($15 | 0) != 0) {
          var $R_0 = $15;
          var $RP_0 = $arrayidx65;
          continue;
        }
        var $arrayidx69 = $R_0 + 16 | 0;
        var $16 = HEAP32[$arrayidx69 >> 2];
        if (($16 | 0) == 0) {
          break;
        } else {
          var $R_0 = $16;
          var $RP_0 = $arrayidx69;
        }
      }
      if ($RP_0 >>> 0 < $7 >>> 0) {
        _abort();
      } else {
        HEAP32[$RP_0 >> 2] = 0;
        var $R_1 = $R_0, $R_1$s2 = $R_1 >> 2;
        break;
      }
    } else {
      var $11 = HEAP32[$v_0$s2 + 2];
      if ($11 >>> 0 < $7 >>> 0) {
        _abort();
      } else {
        HEAP32[$11 + 12 >> 2] = $10;
        HEAP32[$10 + 8 >> 2] = $11;
        var $R_1 = $10, $R_1$s2 = $R_1 >> 2;
        break;
      }
    }
  } while (0);
  var $R_1;
  var $cmp84 = ($9 | 0) == 0;
  L1901 : do {
    if (!$cmp84) {
      var $index = $v_0 + 28 | 0;
      var $arrayidx88 = (HEAP32[$index >> 2] << 2) + 5243812 | 0;
      do {
        if (($v_0 | 0) == (HEAP32[$arrayidx88 >> 2] | 0)) {
          HEAP32[$arrayidx88 >> 2] = $R_1;
          if (($R_1 | 0) != 0) {
            break;
          }
          HEAP32[1310878] = HEAP32[1310878] & (1 << HEAP32[$index >> 2] ^ -1);
          break L1901;
        } else {
          if ($9 >>> 0 < HEAP32[1310881] >>> 0) {
            _abort();
          }
          var $arrayidx107 = $9 + 16 | 0;
          if ((HEAP32[$arrayidx107 >> 2] | 0) == ($v_0 | 0)) {
            HEAP32[$arrayidx107 >> 2] = $R_1;
          } else {
            HEAP32[$9 + 20 >> 2] = $R_1;
          }
          if (($R_1 | 0) == 0) {
            break L1901;
          }
        }
      } while (0);
      if ($R_1 >>> 0 < HEAP32[1310881] >>> 0) {
        _abort();
      }
      HEAP32[$R_1$s2 + 6] = $9;
      var $27 = HEAP32[$v_0$s2 + 4];
      do {
        if (($27 | 0) != 0) {
          if ($27 >>> 0 < HEAP32[1310881] >>> 0) {
            _abort();
          } else {
            HEAP32[$R_1$s2 + 4] = $27;
            HEAP32[$27 + 24 >> 2] = $R_1;
            break;
          }
        }
      } while (0);
      var $30 = HEAP32[$v_0$s2 + 5];
      if (($30 | 0) == 0) {
        break;
      }
      if ($30 >>> 0 < HEAP32[1310881] >>> 0) {
        _abort();
      } else {
        HEAP32[$R_1$s2 + 5] = $30;
        HEAP32[$30 + 24 >> 2] = $R_1;
        break;
      }
    }
  } while (0);
  if ($rsize_0 >>> 0 < 16) {
    var $add171 = $rsize_0 + $nb | 0;
    HEAP32[$v_0$s2 + 1] = $add171 | 3;
    var $33 = $add171 + ($6 + 4) | 0;
    HEAP32[$33 >> 2] = HEAP32[$33 >> 2] | 1;
    var $add_ptr219 = $v_0 + 8 | 0;
    var $44 = $add_ptr219;
    return $44;
  }
  HEAP32[$v_0$s2 + 1] = $nb | 3;
  HEAP32[$nb + ($6 + 4) >> 2] = $rsize_0 | 1;
  HEAP32[$6 + $rsize_0 + $nb >> 2] = $rsize_0;
  var $36 = HEAP32[1310879];
  if (($36 | 0) != 0) {
    var $37 = HEAP32[1310882];
    var $shr188 = $36 >>> 3;
    var $shl189 = $shr188 << 1;
    var $38 = ($shl189 << 2) + 5243548 | 0;
    var $39 = HEAP32[1310877];
    var $shl192 = 1 << $shr188;
    do {
      if (($39 & $shl192 | 0) == 0) {
        HEAP32[1310877] = $39 | $shl192;
        var $F191_0 = $38;
        var $_pre_phi = ($shl189 + 2 << 2) + 5243548 | 0;
      } else {
        var $40 = ($shl189 + 2 << 2) + 5243548 | 0;
        var $41 = HEAP32[$40 >> 2];
        if ($41 >>> 0 >= HEAP32[1310881] >>> 0) {
          var $F191_0 = $41;
          var $_pre_phi = $40;
          break;
        }
        _abort();
      }
    } while (0);
    var $_pre_phi;
    var $F191_0;
    HEAP32[$_pre_phi >> 2] = $37;
    HEAP32[$F191_0 + 12 >> 2] = $37;
    HEAP32[$37 + 8 >> 2] = $F191_0;
    HEAP32[$37 + 12 >> 2] = $38;
  }
  HEAP32[1310879] = $rsize_0;
  HEAP32[1310882] = $8;
  var $add_ptr219 = $v_0 + 8 | 0;
  var $44 = $add_ptr219;
  return $44;
}
_tmalloc_small["X"] = 1;
function _sys_alloc($nb) {
  var $sp_039$s2;
  var label = 0;
  if ((HEAP32[1310720] | 0) == 0) {
    _init_mparams();
  }
  var $tobool11 = (HEAP32[1310987] & 4 | 0) == 0;
  L1946 : do {
    if ($tobool11) {
      var $2 = HEAP32[1310883];
      do {
        if (($2 | 0) == 0) {
          label = 1612;
        } else {
          var $call15 = _segment_holding($2);
          if (($call15 | 0) == 0) {
            label = 1612;
            break;
          }
          var $8 = HEAP32[1310722];
          var $and50 = $nb + 47 - HEAP32[1310880] + $8 & -$8;
          if ($and50 >>> 0 >= 2147483647) {
            var $tsize_091517_ph = 0;
            break;
          }
          var $call53 = _sbrk($and50);
          var $cmp55 = ($call53 | 0) == (HEAP32[$call15 >> 2] + HEAP32[$call15 + 4 >> 2] | 0);
          var $tbase_0 = $cmp55 ? $call53 : -1;
          var $tsize_0 = $cmp55 ? $and50 : 0;
          var $br_0 = $call53;
          var $asize_1 = $and50;
          label = 1619;
          break;
        }
      } while (0);
      do {
        if (label == 1612) {
          var $call18 = _sbrk(0);
          if (($call18 | 0) == -1) {
            var $tsize_091517_ph = 0;
            break;
          }
          var $4 = HEAP32[1310722];
          var $and23 = $4 + ($nb + 47) & -$4;
          var $5 = $call18;
          var $6 = HEAP32[1310721];
          var $sub24 = $6 - 1 | 0;
          if (($sub24 & $5 | 0) == 0) {
            var $asize_0 = $and23;
          } else {
            var $asize_0 = $and23 - $5 + ($sub24 + $5 & -$6) | 0;
          }
          var $asize_0;
          if ($asize_0 >>> 0 >= 2147483647) {
            var $tsize_091517_ph = 0;
            break;
          }
          var $call38 = _sbrk($asize_0);
          var $cmp39 = ($call38 | 0) == ($call18 | 0);
          var $tbase_0 = $cmp39 ? $call18 : -1;
          var $tsize_0 = $cmp39 ? $asize_0 : 0;
          var $br_0 = $call38;
          var $asize_1 = $asize_0;
          label = 1619;
          break;
        }
      } while (0);
      L1959 : do {
        if (label == 1619) {
          var $asize_1;
          var $br_0;
          var $tsize_0;
          var $tbase_0;
          var $sub82 = -$asize_1 | 0;
          if (($tbase_0 | 0) != -1) {
            var $tsize_227 = $tsize_0;
            var $tbase_228 = $tbase_0;
            label = 1632;
            break L1946;
          }
          do {
            if (($br_0 | 0) != -1 & $asize_1 >>> 0 < 2147483647) {
              if ($asize_1 >>> 0 >= ($nb + 48 | 0) >>> 0) {
                var $asize_2 = $asize_1;
                break;
              }
              var $11 = HEAP32[1310722];
              var $and74 = $nb + 47 - $asize_1 + $11 & -$11;
              if ($and74 >>> 0 >= 2147483647) {
                var $asize_2 = $asize_1;
                break;
              }
              if ((_sbrk($and74) | 0) == -1) {
                _sbrk($sub82);
                var $tsize_091517_ph = $tsize_0;
                break L1959;
              } else {
                var $asize_2 = $and74 + $asize_1 | 0;
                break;
              }
            } else {
              var $asize_2 = $asize_1;
            }
          } while (0);
          var $asize_2;
          if (($br_0 | 0) != -1) {
            var $tsize_227 = $asize_2;
            var $tbase_228 = $br_0;
            label = 1632;
            break L1946;
          }
          HEAP32[1310987] = HEAP32[1310987] | 4;
          var $tsize_122 = $tsize_0;
          label = 1629;
          break L1946;
        }
      } while (0);
      var $tsize_091517_ph;
      HEAP32[1310987] = HEAP32[1310987] | 4;
      var $tsize_122 = $tsize_091517_ph;
      label = 1629;
      break;
    } else {
      var $tsize_122 = 0;
      label = 1629;
    }
  } while (0);
  do {
    if (label == 1629) {
      var $tsize_122;
      var $14 = HEAP32[1310722];
      var $and103 = $14 + ($nb + 47) & -$14;
      if ($and103 >>> 0 >= 2147483647) {
        break;
      }
      var $call108 = _sbrk($and103);
      var $call109 = _sbrk(0);
      if (!(($call109 | 0) != -1 & ($call108 | 0) != -1 & $call108 >>> 0 < $call109 >>> 0)) {
        break;
      }
      var $sub_ptr_sub = $call109 - $call108 | 0;
      var $cmp117 = $sub_ptr_sub >>> 0 > ($nb + 40 | 0) >>> 0;
      var $call108_tbase_1 = $cmp117 ? $call108 : -1;
      if (($call108_tbase_1 | 0) == -1) {
        break;
      } else {
        var $tsize_227 = $cmp117 ? $sub_ptr_sub : $tsize_122;
        var $tbase_228 = $call108_tbase_1;
        label = 1632;
        break;
      }
    }
  } while (0);
  do {
    if (label == 1632) {
      var $tbase_228;
      var $tsize_227;
      var $add125 = HEAP32[1310985] + $tsize_227 | 0;
      HEAP32[1310985] = $add125;
      if ($add125 >>> 0 > HEAP32[1310986] >>> 0) {
        HEAP32[1310986] = $add125;
      }
      var $17 = HEAP32[1310883];
      var $cmp132 = ($17 | 0) == 0;
      L1981 : do {
        if ($cmp132) {
          var $18 = HEAP32[1310881];
          if (($18 | 0) == 0 | $tbase_228 >>> 0 < $18 >>> 0) {
            HEAP32[1310881] = $tbase_228;
          }
          HEAP32[1310988] = $tbase_228;
          HEAP32[1310989] = $tsize_227;
          HEAP32[1310991] = 0;
          HEAP32[1310886] = HEAP32[1310720];
          HEAP32[1310885] = -1;
          _init_bins();
          _init_top($tbase_228, $tsize_227 - 40 | 0);
        } else {
          var $sp_039 = 5243952, $sp_039$s2 = $sp_039 >> 2;
          while (1) {
            var $sp_039;
            var $21 = HEAP32[$sp_039$s2];
            var $size162 = $sp_039 + 4 | 0;
            var $22 = HEAP32[$size162 >> 2];
            if (($tbase_228 | 0) == ($21 + $22 | 0)) {
              label = 1640;
              break;
            }
            var $23 = HEAP32[$sp_039$s2 + 2];
            if (($23 | 0) == 0) {
              break;
            } else {
              var $sp_039 = $23, $sp_039$s2 = $sp_039 >> 2;
            }
          }
          do {
            if (label == 1640) {
              if ((HEAP32[$sp_039$s2 + 3] & 8 | 0) != 0) {
                break;
              }
              var $25 = $17;
              if (!($25 >>> 0 >= $21 >>> 0 & $25 >>> 0 < $tbase_228 >>> 0)) {
                break;
              }
              HEAP32[$size162 >> 2] = $22 + $tsize_227 | 0;
              _init_top(HEAP32[1310883], HEAP32[1310880] + $tsize_227 | 0);
              break L1981;
            }
          } while (0);
          if ($tbase_228 >>> 0 < HEAP32[1310881] >>> 0) {
            HEAP32[1310881] = $tbase_228;
          }
          var $add_ptr201 = $tbase_228 + $tsize_227 | 0;
          var $sp_135 = 5243952;
          while (1) {
            var $sp_135;
            var $base200 = $sp_135 | 0;
            if ((HEAP32[$base200 >> 2] | 0) == ($add_ptr201 | 0)) {
              label = 1648;
              break;
            }
            var $30 = HEAP32[$sp_135 + 8 >> 2];
            if (($30 | 0) == 0) {
              break;
            } else {
              var $sp_135 = $30;
            }
          }
          do {
            if (label == 1648) {
              if ((HEAP32[$sp_135 + 12 >> 2] & 8 | 0) != 0) {
                break;
              }
              HEAP32[$base200 >> 2] = $tbase_228;
              var $size219 = $sp_135 + 4 | 0;
              HEAP32[$size219 >> 2] = HEAP32[$size219 >> 2] + $tsize_227 | 0;
              var $retval_0 = _prepend_alloc($tbase_228, $add_ptr201, $nb);
              var $retval_0;
              return $retval_0;
            }
          } while (0);
          _add_segment($tbase_228, $tsize_227);
        }
      } while (0);
      var $33 = HEAP32[1310880];
      if ($33 >>> 0 <= $nb >>> 0) {
        break;
      }
      var $sub230 = $33 - $nb | 0;
      HEAP32[1310880] = $sub230;
      var $34 = HEAP32[1310883];
      var $35 = $34;
      HEAP32[1310883] = $35 + $nb | 0;
      HEAP32[$nb + ($35 + 4) >> 2] = $sub230 | 1;
      HEAP32[$34 + 4 >> 2] = $nb | 3;
      var $retval_0 = $34 + 8 | 0;
      var $retval_0;
      return $retval_0;
    }
  } while (0);
  HEAP32[___errno_location() >> 2] = 12;
  var $retval_0 = 0;
  var $retval_0;
  return $retval_0;
}
_sys_alloc["X"] = 1;
function _tmalloc_large($nb) {
  var $R_1$s2;
  var $10$s2;
  var $t_221$s2;
  var $v_3_lcssa$s2;
  var $t_0$s2;
  var $nb$s2 = $nb >> 2;
  var label = 0;
  var $sub = -$nb | 0;
  var $shr = $nb >>> 8;
  do {
    if (($shr | 0) == 0) {
      var $idx_0 = 0;
    } else {
      if ($nb >>> 0 > 16777215) {
        var $idx_0 = 31;
        break;
      }
      var $and = ($shr + 1048320 | 0) >>> 16 & 8;
      var $shl = $shr << $and;
      var $and8 = ($shl + 520192 | 0) >>> 16 & 4;
      var $shl9 = $shl << $and8;
      var $and12 = ($shl9 + 245760 | 0) >>> 16 & 2;
      var $add17 = 14 - ($and8 | $and | $and12) + ($shl9 << $and12 >>> 15) | 0;
      var $idx_0 = $nb >>> (($add17 + 7 | 0) >>> 0) & 1 | $add17 << 1;
    }
  } while (0);
  var $idx_0;
  var $0 = HEAP32[($idx_0 << 2) + 5243812 >> 2];
  var $cmp24 = ($0 | 0) == 0;
  L2015 : do {
    if ($cmp24) {
      var $v_2 = 0;
      var $rsize_2 = $sub;
      var $t_1 = 0;
    } else {
      if (($idx_0 | 0) == 31) {
        var $cond = 0;
      } else {
        var $cond = 25 - ($idx_0 >>> 1) | 0;
      }
      var $cond;
      var $v_0 = 0;
      var $rsize_0 = $sub;
      var $t_0 = $0, $t_0$s2 = $t_0 >> 2;
      var $sizebits_0 = $nb << $cond;
      var $rst_0 = 0;
      while (1) {
        var $rst_0;
        var $sizebits_0;
        var $t_0;
        var $rsize_0;
        var $v_0;
        var $and32 = HEAP32[$t_0$s2 + 1] & -8;
        var $sub33 = $and32 - $nb | 0;
        if ($sub33 >>> 0 < $rsize_0 >>> 0) {
          if (($and32 | 0) == ($nb | 0)) {
            var $v_2 = $t_0;
            var $rsize_2 = $sub33;
            var $t_1 = $t_0;
            break L2015;
          } else {
            var $v_1 = $t_0;
            var $rsize_1 = $sub33;
          }
        } else {
          var $v_1 = $v_0;
          var $rsize_1 = $rsize_0;
        }
        var $rsize_1;
        var $v_1;
        var $2 = HEAP32[$t_0$s2 + 5];
        var $3 = HEAP32[(($sizebits_0 >>> 31 << 2) + 16 >> 2) + $t_0$s2];
        var $rst_1 = ($2 | 0) == 0 | ($2 | 0) == ($3 | 0) ? $rst_0 : $2;
        if (($3 | 0) == 0) {
          var $v_2 = $v_1;
          var $rsize_2 = $rsize_1;
          var $t_1 = $rst_1;
          break L2015;
        } else {
          var $v_0 = $v_1;
          var $rsize_0 = $rsize_1;
          var $t_0 = $3, $t_0$s2 = $t_0 >> 2;
          var $sizebits_0 = $sizebits_0 << 1;
          var $rst_0 = $rst_1;
        }
      }
    }
  } while (0);
  var $t_1;
  var $rsize_2;
  var $v_2;
  do {
    if (($t_1 | 0) == 0 & ($v_2 | 0) == 0) {
      var $shl59 = 2 << $idx_0;
      var $and63 = HEAP32[1310878] & ($shl59 | -$shl59);
      if (($and63 | 0) == 0) {
        var $retval_0 = 0;
        var $retval_0;
        return $retval_0;
      } else {
        var $sub69 = ($and63 & -$and63) - 1 | 0;
        var $and72 = $sub69 >>> 12 & 16;
        var $shr74 = $sub69 >>> ($and72 >>> 0);
        var $and76 = $shr74 >>> 5 & 8;
        var $shr78 = $shr74 >>> ($and76 >>> 0);
        var $and80 = $shr78 >>> 2 & 4;
        var $shr82 = $shr78 >>> ($and80 >>> 0);
        var $and84 = $shr82 >>> 1 & 2;
        var $shr86 = $shr82 >>> ($and84 >>> 0);
        var $and88 = $shr86 >>> 1 & 1;
        var $t_2_ph = HEAP32[(($and76 | $and72 | $and80 | $and84 | $and88) + ($shr86 >>> ($and88 >>> 0)) << 2) + 5243812 >> 2];
        break;
      }
    } else {
      var $t_2_ph = $t_1;
    }
  } while (0);
  var $t_2_ph;
  var $cmp9620 = ($t_2_ph | 0) == 0;
  L2032 : do {
    if ($cmp9620) {
      var $rsize_3_lcssa = $rsize_2;
      var $v_3_lcssa = $v_2, $v_3_lcssa$s2 = $v_3_lcssa >> 2;
    } else {
      var $t_221 = $t_2_ph, $t_221$s2 = $t_221 >> 2;
      var $rsize_322 = $rsize_2;
      var $v_323 = $v_2;
      while (1) {
        var $v_323;
        var $rsize_322;
        var $t_221;
        var $sub100 = (HEAP32[$t_221$s2 + 1] & -8) - $nb | 0;
        var $cmp101 = $sub100 >>> 0 < $rsize_322 >>> 0;
        var $sub100_rsize_3 = $cmp101 ? $sub100 : $rsize_322;
        var $t_2_v_3 = $cmp101 ? $t_221 : $v_323;
        var $7 = HEAP32[$t_221$s2 + 4];
        if (($7 | 0) != 0) {
          var $t_221 = $7, $t_221$s2 = $t_221 >> 2;
          var $rsize_322 = $sub100_rsize_3;
          var $v_323 = $t_2_v_3;
          continue;
        }
        var $8 = HEAP32[$t_221$s2 + 5];
        if (($8 | 0) == 0) {
          var $rsize_3_lcssa = $sub100_rsize_3;
          var $v_3_lcssa = $t_2_v_3, $v_3_lcssa$s2 = $v_3_lcssa >> 2;
          break L2032;
        } else {
          var $t_221 = $8, $t_221$s2 = $t_221 >> 2;
          var $rsize_322 = $sub100_rsize_3;
          var $v_323 = $t_2_v_3;
        }
      }
    }
  } while (0);
  var $v_3_lcssa;
  var $rsize_3_lcssa;
  if (($v_3_lcssa | 0) == 0) {
    var $retval_0 = 0;
    var $retval_0;
    return $retval_0;
  }
  if ($rsize_3_lcssa >>> 0 >= (HEAP32[1310879] - $nb | 0) >>> 0) {
    var $retval_0 = 0;
    var $retval_0;
    return $retval_0;
  }
  var $10 = $v_3_lcssa, $10$s2 = $10 >> 2;
  var $11 = HEAP32[1310881];
  if ($10 >>> 0 < $11 >>> 0) {
    _abort();
  }
  var $add_ptr = $10 + $nb | 0;
  var $12 = $add_ptr;
  if ($10 >>> 0 >= $add_ptr >>> 0) {
    _abort();
  }
  var $13 = HEAP32[$v_3_lcssa$s2 + 6];
  var $14 = HEAP32[$v_3_lcssa$s2 + 3];
  var $cmp127 = ($14 | 0) == ($v_3_lcssa | 0);
  L2049 : do {
    if ($cmp127) {
      var $arrayidx143 = $v_3_lcssa + 20 | 0;
      var $17 = HEAP32[$arrayidx143 >> 2];
      do {
        if (($17 | 0) == 0) {
          var $arrayidx147 = $v_3_lcssa + 16 | 0;
          var $18 = HEAP32[$arrayidx147 >> 2];
          if (($18 | 0) == 0) {
            var $R_1 = 0, $R_1$s2 = $R_1 >> 2;
            break L2049;
          } else {
            var $R_0 = $18;
            var $RP_0 = $arrayidx147;
            break;
          }
        } else {
          var $R_0 = $17;
          var $RP_0 = $arrayidx143;
        }
      } while (0);
      while (1) {
        var $RP_0;
        var $R_0;
        var $arrayidx153 = $R_0 + 20 | 0;
        var $19 = HEAP32[$arrayidx153 >> 2];
        if (($19 | 0) != 0) {
          var $R_0 = $19;
          var $RP_0 = $arrayidx153;
          continue;
        }
        var $arrayidx157 = $R_0 + 16 | 0;
        var $20 = HEAP32[$arrayidx157 >> 2];
        if (($20 | 0) == 0) {
          break;
        } else {
          var $R_0 = $20;
          var $RP_0 = $arrayidx157;
        }
      }
      if ($RP_0 >>> 0 < $11 >>> 0) {
        _abort();
      } else {
        HEAP32[$RP_0 >> 2] = 0;
        var $R_1 = $R_0, $R_1$s2 = $R_1 >> 2;
        break;
      }
    } else {
      var $15 = HEAP32[$v_3_lcssa$s2 + 2];
      if ($15 >>> 0 < $11 >>> 0) {
        _abort();
      } else {
        HEAP32[$15 + 12 >> 2] = $14;
        HEAP32[$14 + 8 >> 2] = $15;
        var $R_1 = $14, $R_1$s2 = $R_1 >> 2;
        break;
      }
    }
  } while (0);
  var $R_1;
  var $cmp172 = ($13 | 0) == 0;
  L2065 : do {
    if ($cmp172) {
      var $v_3_lcssa2 = $v_3_lcssa;
    } else {
      var $index = $v_3_lcssa + 28 | 0;
      var $arrayidx176 = (HEAP32[$index >> 2] << 2) + 5243812 | 0;
      do {
        if (($v_3_lcssa | 0) == (HEAP32[$arrayidx176 >> 2] | 0)) {
          HEAP32[$arrayidx176 >> 2] = $R_1;
          if (($R_1 | 0) != 0) {
            break;
          }
          HEAP32[1310878] = HEAP32[1310878] & (1 << HEAP32[$index >> 2] ^ -1);
          var $v_3_lcssa2 = $v_3_lcssa;
          break L2065;
        } else {
          if ($13 >>> 0 < HEAP32[1310881] >>> 0) {
            _abort();
          }
          var $arrayidx196 = $13 + 16 | 0;
          if ((HEAP32[$arrayidx196 >> 2] | 0) == ($v_3_lcssa | 0)) {
            HEAP32[$arrayidx196 >> 2] = $R_1;
          } else {
            HEAP32[$13 + 20 >> 2] = $R_1;
          }
          if (($R_1 | 0) == 0) {
            var $v_3_lcssa2 = $v_3_lcssa;
            break L2065;
          }
        }
      } while (0);
      if ($R_1 >>> 0 < HEAP32[1310881] >>> 0) {
        _abort();
      }
      HEAP32[$R_1$s2 + 6] = $13;
      var $31 = HEAP32[$v_3_lcssa$s2 + 4];
      do {
        if (($31 | 0) != 0) {
          if ($31 >>> 0 < HEAP32[1310881] >>> 0) {
            _abort();
          } else {
            HEAP32[$R_1$s2 + 4] = $31;
            HEAP32[$31 + 24 >> 2] = $R_1;
            break;
          }
        }
      } while (0);
      var $34 = HEAP32[$v_3_lcssa$s2 + 5];
      if (($34 | 0) == 0) {
        var $v_3_lcssa2 = $v_3_lcssa;
        break;
      }
      if ($34 >>> 0 < HEAP32[1310881] >>> 0) {
        _abort();
      } else {
        HEAP32[$R_1$s2 + 5] = $34;
        HEAP32[$34 + 24 >> 2] = $R_1;
        var $v_3_lcssa2 = $v_3_lcssa;
        break;
      }
    }
  } while (0);
  var $v_3_lcssa2;
  do {
    if ($rsize_3_lcssa >>> 0 < 16) {
      var $add260 = $rsize_3_lcssa + $nb | 0;
      HEAP32[$v_3_lcssa2 + 4 >> 2] = $add260 | 3;
      var $37 = $add260 + ($10 + 4) | 0;
      HEAP32[$37 >> 2] = HEAP32[$37 >> 2] | 1;
    } else {
      HEAP32[$v_3_lcssa2 + 4 >> 2] = $nb | 3;
      HEAP32[$nb$s2 + ($10$s2 + 1)] = $rsize_3_lcssa | 1;
      HEAP32[($rsize_3_lcssa >> 2) + $10$s2 + $nb$s2] = $rsize_3_lcssa;
      var $shr275 = $rsize_3_lcssa >>> 3;
      if ($rsize_3_lcssa >>> 0 < 256) {
        var $shl280 = $shr275 << 1;
        var $40 = ($shl280 << 2) + 5243548 | 0;
        var $41 = HEAP32[1310877];
        var $shl283 = 1 << $shr275;
        do {
          if (($41 & $shl283 | 0) == 0) {
            HEAP32[1310877] = $41 | $shl283;
            var $F282_0 = $40;
            var $_pre_phi = ($shl280 + 2 << 2) + 5243548 | 0;
          } else {
            var $42 = ($shl280 + 2 << 2) + 5243548 | 0;
            var $43 = HEAP32[$42 >> 2];
            if ($43 >>> 0 >= HEAP32[1310881] >>> 0) {
              var $F282_0 = $43;
              var $_pre_phi = $42;
              break;
            }
            _abort();
          }
        } while (0);
        var $_pre_phi;
        var $F282_0;
        HEAP32[$_pre_phi >> 2] = $12;
        HEAP32[$F282_0 + 12 >> 2] = $12;
        HEAP32[$nb$s2 + ($10$s2 + 2)] = $F282_0;
        HEAP32[$nb$s2 + ($10$s2 + 3)] = $40;
        break;
      }
      var $48 = $add_ptr;
      var $shr310 = $rsize_3_lcssa >>> 8;
      do {
        if (($shr310 | 0) == 0) {
          var $I308_0 = 0;
        } else {
          if ($rsize_3_lcssa >>> 0 > 16777215) {
            var $I308_0 = 31;
            break;
          }
          var $and323 = ($shr310 + 1048320 | 0) >>> 16 & 8;
          var $shl325 = $shr310 << $and323;
          var $and328 = ($shl325 + 520192 | 0) >>> 16 & 4;
          var $shl330 = $shl325 << $and328;
          var $and333 = ($shl330 + 245760 | 0) >>> 16 & 2;
          var $add338 = 14 - ($and328 | $and323 | $and333) + ($shl330 << $and333 >>> 15) | 0;
          var $I308_0 = $rsize_3_lcssa >>> (($add338 + 7 | 0) >>> 0) & 1 | $add338 << 1;
        }
      } while (0);
      var $I308_0;
      var $arrayidx347 = ($I308_0 << 2) + 5243812 | 0;
      HEAP32[$nb$s2 + ($10$s2 + 7)] = $I308_0;
      HEAP32[$nb$s2 + ($10$s2 + 5)] = 0;
      HEAP32[$nb$s2 + ($10$s2 + 4)] = 0;
      var $51 = HEAP32[1310878];
      var $shl354 = 1 << $I308_0;
      if (($51 & $shl354 | 0) == 0) {
        HEAP32[1310878] = $51 | $shl354;
        HEAP32[$arrayidx347 >> 2] = $48;
        HEAP32[$nb$s2 + ($10$s2 + 6)] = $arrayidx347;
        HEAP32[$nb$s2 + ($10$s2 + 3)] = $48;
        HEAP32[$nb$s2 + ($10$s2 + 2)] = $48;
        break;
      }
      if (($I308_0 | 0) == 31) {
        var $cond375 = 0;
      } else {
        var $cond375 = 25 - ($I308_0 >>> 1) | 0;
      }
      var $cond375;
      var $K365_0 = $rsize_3_lcssa << $cond375;
      var $T_0 = HEAP32[$arrayidx347 >> 2];
      while (1) {
        var $T_0;
        var $K365_0;
        if ((HEAP32[$T_0 + 4 >> 2] & -8 | 0) == ($rsize_3_lcssa | 0)) {
          break;
        }
        var $arrayidx386 = ($K365_0 >>> 31 << 2) + $T_0 + 16 | 0;
        var $58 = HEAP32[$arrayidx386 >> 2];
        if (($58 | 0) == 0) {
          label = 1727;
          break;
        } else {
          var $K365_0 = $K365_0 << 1;
          var $T_0 = $58;
        }
      }
      if (label == 1727) {
        if ($arrayidx386 >>> 0 < HEAP32[1310881] >>> 0) {
          _abort();
        } else {
          HEAP32[$arrayidx386 >> 2] = $48;
          HEAP32[$nb$s2 + ($10$s2 + 6)] = $T_0;
          HEAP32[$nb$s2 + ($10$s2 + 3)] = $48;
          HEAP32[$nb$s2 + ($10$s2 + 2)] = $48;
          break;
        }
      }
      var $fd405 = $T_0 + 8 | 0;
      var $64 = HEAP32[$fd405 >> 2];
      var $66 = HEAP32[1310881];
      if ($T_0 >>> 0 < $66 >>> 0) {
        _abort();
      }
      if ($64 >>> 0 < $66 >>> 0) {
        _abort();
      } else {
        HEAP32[$64 + 12 >> 2] = $48;
        HEAP32[$fd405 >> 2] = $48;
        HEAP32[$nb$s2 + ($10$s2 + 2)] = $64;
        HEAP32[$nb$s2 + ($10$s2 + 3)] = $T_0;
        HEAP32[$nb$s2 + ($10$s2 + 6)] = 0;
        break;
      }
    }
  } while (0);
  var $retval_0 = $v_3_lcssa2 + 8 | 0;
  var $retval_0;
  return $retval_0;
}
_tmalloc_large["X"] = 1;
function _release_unused_segments() {
  var $sp_0_in = 5243960;
  while (1) {
    var $sp_0_in;
    var $sp_0 = HEAP32[$sp_0_in >> 2];
    if (($sp_0 | 0) == 0) {
      break;
    } else {
      var $sp_0_in = $sp_0 + 8 | 0;
    }
  }
  HEAP32[1310885] = -1;
  return;
}
function _sys_trim() {
  var $size$s2;
  if ((HEAP32[1310720] | 0) == 0) {
    _init_mparams();
  }
  var $1 = HEAP32[1310883];
  if (($1 | 0) == 0) {
    return;
  }
  var $2 = HEAP32[1310880];
  do {
    if ($2 >>> 0 > 40) {
      var $3 = HEAP32[1310722];
      var $mul = (Math.floor((($2 - 41 + $3 | 0) >>> 0) / ($3 >>> 0)) - 1) * $3 | 0;
      var $call10 = _segment_holding($1);
      if ((HEAP32[$call10 + 12 >> 2] & 8 | 0) != 0) {
        break;
      }
      var $call20 = _sbrk(0);
      var $size$s2 = ($call10 + 4 | 0) >> 2;
      if (($call20 | 0) != (HEAP32[$call10 >> 2] + HEAP32[$size$s2] | 0)) {
        break;
      }
      var $call24 = _sbrk(-($mul >>> 0 > 2147483646 ? -2147483648 - $3 | 0 : $mul) | 0);
      var $call25 = _sbrk(0);
      if (!(($call24 | 0) != -1 & $call25 >>> 0 < $call20 >>> 0)) {
        break;
      }
      var $sub_ptr_sub = $call20 - $call25 | 0;
      if (($call20 | 0) == ($call25 | 0)) {
        break;
      }
      HEAP32[$size$s2] = HEAP32[$size$s2] - $sub_ptr_sub | 0;
      HEAP32[1310985] = HEAP32[1310985] - $sub_ptr_sub | 0;
      _init_top(HEAP32[1310883], HEAP32[1310880] - $sub_ptr_sub | 0);
      return;
    }
  } while (0);
  if (HEAP32[1310880] >>> 0 <= HEAP32[1310884] >>> 0) {
    return;
  }
  HEAP32[1310884] = -1;
  return;
}
_sys_trim["X"] = 1;
function _free($mem) {
  var $R288_1$s2;
  var $51$s2;
  var $R_1$s2;
  var $p_0$s2;
  var $47$s2;
  var $add_ptr_sum215$s2;
  var $and5$s2;
  var $mem$s2 = $mem >> 2;
  var label = 0;
  if (($mem | 0) == 0) {
    return;
  }
  var $add_ptr = $mem - 8 | 0;
  var $0 = $add_ptr;
  var $1 = HEAP32[1310881];
  if ($add_ptr >>> 0 < $1 >>> 0) {
    _abort();
  }
  var $3 = HEAP32[$mem - 4 >> 2];
  var $and = $3 & 3;
  if (($and | 0) == 1) {
    _abort();
  }
  var $and5 = $3 & -8, $and5$s2 = $and5 >> 2;
  var $add_ptr6 = $mem + ($and5 - 8) | 0;
  var $4 = $add_ptr6;
  var $tobool9 = ($3 & 1 | 0) == 0;
  L2164 : do {
    if ($tobool9) {
      var $5 = HEAP32[$add_ptr >> 2];
      if (($and | 0) == 0) {
        return;
      }
      var $add_ptr_sum215 = -8 - $5 | 0, $add_ptr_sum215$s2 = $add_ptr_sum215 >> 2;
      var $add_ptr16 = $mem + $add_ptr_sum215 | 0;
      var $6 = $add_ptr16;
      var $add17 = $5 + $and5 | 0;
      if ($add_ptr16 >>> 0 < $1 >>> 0) {
        _abort();
      }
      if (($6 | 0) == (HEAP32[1310882] | 0)) {
        var $47$s2 = ($mem + ($and5 - 4) | 0) >> 2;
        if ((HEAP32[$47$s2] & 3 | 0) != 3) {
          var $p_0 = $6, $p_0$s2 = $p_0 >> 2;
          var $psize_0 = $add17;
          break;
        }
        HEAP32[1310879] = $add17;
        HEAP32[$47$s2] = HEAP32[$47$s2] & -2;
        HEAP32[$add_ptr_sum215$s2 + ($mem$s2 + 1)] = $add17 | 1;
        HEAP32[$add_ptr6 >> 2] = $add17;
        return;
      }
      var $shr = $5 >>> 3;
      if ($5 >>> 0 < 256) {
        var $9 = HEAP32[$add_ptr_sum215$s2 + ($mem$s2 + 2)];
        var $11 = HEAP32[$add_ptr_sum215$s2 + ($mem$s2 + 3)];
        if (($9 | 0) == ($11 | 0)) {
          HEAP32[1310877] = HEAP32[1310877] & (1 << $shr ^ -1);
          var $p_0 = $6, $p_0$s2 = $p_0 >> 2;
          var $psize_0 = $add17;
          break;
        }
        var $13 = ($shr << 3) + 5243548 | 0;
        if (($9 | 0) != ($13 | 0) & $9 >>> 0 < $1 >>> 0) {
          _abort();
        }
        if (($11 | 0) == ($13 | 0) | $11 >>> 0 >= $1 >>> 0) {
          HEAP32[$9 + 12 >> 2] = $11;
          HEAP32[$11 + 8 >> 2] = $9;
          var $p_0 = $6, $p_0$s2 = $p_0 >> 2;
          var $psize_0 = $add17;
          break;
        } else {
          _abort();
        }
      }
      var $16 = $add_ptr16;
      var $18 = HEAP32[$add_ptr_sum215$s2 + ($mem$s2 + 6)];
      var $20 = HEAP32[$add_ptr_sum215$s2 + ($mem$s2 + 3)];
      var $cmp57 = ($20 | 0) == ($16 | 0);
      L2189 : do {
        if ($cmp57) {
          var $24 = $add_ptr_sum215 + ($mem + 20) | 0;
          var $25 = HEAP32[$24 >> 2];
          do {
            if (($25 | 0) == 0) {
              var $arrayidx78 = $add_ptr_sum215 + ($mem + 16) | 0;
              var $26 = HEAP32[$arrayidx78 >> 2];
              if (($26 | 0) == 0) {
                var $R_1 = 0, $R_1$s2 = $R_1 >> 2;
                break L2189;
              } else {
                var $R_0 = $26;
                var $RP_0 = $arrayidx78;
                break;
              }
            } else {
              var $R_0 = $25;
              var $RP_0 = $24;
            }
          } while (0);
          while (1) {
            var $RP_0;
            var $R_0;
            var $arrayidx83 = $R_0 + 20 | 0;
            var $27 = HEAP32[$arrayidx83 >> 2];
            if (($27 | 0) != 0) {
              var $R_0 = $27;
              var $RP_0 = $arrayidx83;
              continue;
            }
            var $arrayidx88 = $R_0 + 16 | 0;
            var $28 = HEAP32[$arrayidx88 >> 2];
            if (($28 | 0) == 0) {
              break;
            } else {
              var $R_0 = $28;
              var $RP_0 = $arrayidx88;
            }
          }
          if ($RP_0 >>> 0 < $1 >>> 0) {
            _abort();
          } else {
            HEAP32[$RP_0 >> 2] = 0;
            var $R_1 = $R_0, $R_1$s2 = $R_1 >> 2;
            break;
          }
        } else {
          var $22 = HEAP32[$add_ptr_sum215$s2 + ($mem$s2 + 2)];
          if ($22 >>> 0 < $1 >>> 0) {
            _abort();
          } else {
            HEAP32[$22 + 12 >> 2] = $20;
            HEAP32[$20 + 8 >> 2] = $22;
            var $R_1 = $20, $R_1$s2 = $R_1 >> 2;
            break;
          }
        }
      } while (0);
      var $R_1;
      if (($18 | 0) == 0) {
        var $p_0 = $6, $p_0$s2 = $p_0 >> 2;
        var $psize_0 = $add17;
        break;
      }
      var $30 = $add_ptr_sum215 + ($mem + 28) | 0;
      var $arrayidx104 = (HEAP32[$30 >> 2] << 2) + 5243812 | 0;
      do {
        if (($16 | 0) == (HEAP32[$arrayidx104 >> 2] | 0)) {
          HEAP32[$arrayidx104 >> 2] = $R_1;
          if (($R_1 | 0) != 0) {
            break;
          }
          HEAP32[1310878] = HEAP32[1310878] & (1 << HEAP32[$30 >> 2] ^ -1);
          var $p_0 = $6, $p_0$s2 = $p_0 >> 2;
          var $psize_0 = $add17;
          break L2164;
        } else {
          if ($18 >>> 0 < HEAP32[1310881] >>> 0) {
            _abort();
          }
          var $arrayidx123 = $18 + 16 | 0;
          if ((HEAP32[$arrayidx123 >> 2] | 0) == ($16 | 0)) {
            HEAP32[$arrayidx123 >> 2] = $R_1;
          } else {
            HEAP32[$18 + 20 >> 2] = $R_1;
          }
          if (($R_1 | 0) == 0) {
            var $p_0 = $6, $p_0$s2 = $p_0 >> 2;
            var $psize_0 = $add17;
            break L2164;
          }
        }
      } while (0);
      if ($R_1 >>> 0 < HEAP32[1310881] >>> 0) {
        _abort();
      }
      HEAP32[$R_1$s2 + 6] = $18;
      var $40 = HEAP32[$add_ptr_sum215$s2 + ($mem$s2 + 4)];
      do {
        if (($40 | 0) != 0) {
          if ($40 >>> 0 < HEAP32[1310881] >>> 0) {
            _abort();
          } else {
            HEAP32[$R_1$s2 + 4] = $40;
            HEAP32[$40 + 24 >> 2] = $R_1;
            break;
          }
        }
      } while (0);
      var $44 = HEAP32[$add_ptr_sum215$s2 + ($mem$s2 + 5)];
      if (($44 | 0) == 0) {
        var $p_0 = $6, $p_0$s2 = $p_0 >> 2;
        var $psize_0 = $add17;
        break;
      }
      if ($44 >>> 0 < HEAP32[1310881] >>> 0) {
        _abort();
      } else {
        HEAP32[$R_1$s2 + 5] = $44;
        HEAP32[$44 + 24 >> 2] = $R_1;
        var $p_0 = $6, $p_0$s2 = $p_0 >> 2;
        var $psize_0 = $add17;
        break;
      }
    } else {
      var $p_0 = $0, $p_0$s2 = $p_0 >> 2;
      var $psize_0 = $and5;
    }
  } while (0);
  var $psize_0;
  var $p_0;
  var $51 = $p_0, $51$s2 = $51 >> 2;
  if ($51 >>> 0 >= $add_ptr6 >>> 0) {
    _abort();
  }
  var $52 = $mem + ($and5 - 4) | 0;
  var $53 = HEAP32[$52 >> 2];
  if (($53 & 1 | 0) == 0) {
    _abort();
  }
  do {
    if (($53 & 2 | 0) == 0) {
      if (($4 | 0) == (HEAP32[1310883] | 0)) {
        var $add217 = HEAP32[1310880] + $psize_0 | 0;
        HEAP32[1310880] = $add217;
        HEAP32[1310883] = $p_0;
        HEAP32[$p_0$s2 + 1] = $add217 | 1;
        if (($p_0 | 0) == (HEAP32[1310882] | 0)) {
          HEAP32[1310882] = 0;
          HEAP32[1310879] = 0;
        }
        if ($add217 >>> 0 <= HEAP32[1310884] >>> 0) {
          return;
        }
        _sys_trim();
        return;
      }
      if (($4 | 0) == (HEAP32[1310882] | 0)) {
        var $add232 = HEAP32[1310879] + $psize_0 | 0;
        HEAP32[1310879] = $add232;
        HEAP32[1310882] = $p_0;
        HEAP32[$p_0$s2 + 1] = $add232 | 1;
        HEAP32[($add232 >> 2) + $51$s2] = $add232;
        return;
      }
      var $add240 = ($53 & -8) + $psize_0 | 0;
      var $shr241 = $53 >>> 3;
      var $cmp242 = $53 >>> 0 < 256;
      L2255 : do {
        if ($cmp242) {
          var $61 = HEAP32[$mem$s2 + $and5$s2];
          var $63 = HEAP32[(($and5 | 4) >> 2) + $mem$s2];
          if (($61 | 0) == ($63 | 0)) {
            HEAP32[1310877] = HEAP32[1310877] & (1 << $shr241 ^ -1);
            break;
          }
          var $65 = ($shr241 << 3) + 5243548 | 0;
          do {
            if (($61 | 0) != ($65 | 0)) {
              if ($61 >>> 0 >= HEAP32[1310881] >>> 0) {
                break;
              }
              _abort();
            }
          } while (0);
          do {
            if (($63 | 0) != ($65 | 0)) {
              if ($63 >>> 0 >= HEAP32[1310881] >>> 0) {
                break;
              }
              _abort();
            }
          } while (0);
          HEAP32[$61 + 12 >> 2] = $63;
          HEAP32[$63 + 8 >> 2] = $61;
        } else {
          var $70 = $add_ptr6;
          var $72 = HEAP32[$and5$s2 + ($mem$s2 + 4)];
          var $74 = HEAP32[(($and5 | 4) >> 2) + $mem$s2];
          var $cmp290 = ($74 | 0) == ($70 | 0);
          L2269 : do {
            if ($cmp290) {
              var $79 = $and5 + ($mem + 12) | 0;
              var $80 = HEAP32[$79 >> 2];
              do {
                if (($80 | 0) == 0) {
                  var $arrayidx313 = $and5 + ($mem + 8) | 0;
                  var $81 = HEAP32[$arrayidx313 >> 2];
                  if (($81 | 0) == 0) {
                    var $R288_1 = 0, $R288_1$s2 = $R288_1 >> 2;
                    break L2269;
                  } else {
                    var $R288_0 = $81;
                    var $RP306_0 = $arrayidx313;
                    break;
                  }
                } else {
                  var $R288_0 = $80;
                  var $RP306_0 = $79;
                }
              } while (0);
              while (1) {
                var $RP306_0;
                var $R288_0;
                var $arrayidx320 = $R288_0 + 20 | 0;
                var $82 = HEAP32[$arrayidx320 >> 2];
                if (($82 | 0) != 0) {
                  var $R288_0 = $82;
                  var $RP306_0 = $arrayidx320;
                  continue;
                }
                var $arrayidx325 = $R288_0 + 16 | 0;
                var $83 = HEAP32[$arrayidx325 >> 2];
                if (($83 | 0) == 0) {
                  break;
                } else {
                  var $R288_0 = $83;
                  var $RP306_0 = $arrayidx325;
                }
              }
              if ($RP306_0 >>> 0 < HEAP32[1310881] >>> 0) {
                _abort();
              } else {
                HEAP32[$RP306_0 >> 2] = 0;
                var $R288_1 = $R288_0, $R288_1$s2 = $R288_1 >> 2;
                break;
              }
            } else {
              var $76 = HEAP32[$mem$s2 + $and5$s2];
              if ($76 >>> 0 < HEAP32[1310881] >>> 0) {
                _abort();
              } else {
                HEAP32[$76 + 12 >> 2] = $74;
                HEAP32[$74 + 8 >> 2] = $76;
                var $R288_1 = $74, $R288_1$s2 = $R288_1 >> 2;
                break;
              }
            }
          } while (0);
          var $R288_1;
          if (($72 | 0) == 0) {
            break;
          }
          var $86 = $and5 + ($mem + 20) | 0;
          var $arrayidx345 = (HEAP32[$86 >> 2] << 2) + 5243812 | 0;
          do {
            if (($70 | 0) == (HEAP32[$arrayidx345 >> 2] | 0)) {
              HEAP32[$arrayidx345 >> 2] = $R288_1;
              if (($R288_1 | 0) != 0) {
                break;
              }
              HEAP32[1310878] = HEAP32[1310878] & (1 << HEAP32[$86 >> 2] ^ -1);
              break L2255;
            } else {
              if ($72 >>> 0 < HEAP32[1310881] >>> 0) {
                _abort();
              }
              var $arrayidx364 = $72 + 16 | 0;
              if ((HEAP32[$arrayidx364 >> 2] | 0) == ($70 | 0)) {
                HEAP32[$arrayidx364 >> 2] = $R288_1;
              } else {
                HEAP32[$72 + 20 >> 2] = $R288_1;
              }
              if (($R288_1 | 0) == 0) {
                break L2255;
              }
            }
          } while (0);
          if ($R288_1 >>> 0 < HEAP32[1310881] >>> 0) {
            _abort();
          }
          HEAP32[$R288_1$s2 + 6] = $72;
          var $96 = HEAP32[$and5$s2 + ($mem$s2 + 2)];
          do {
            if (($96 | 0) != 0) {
              if ($96 >>> 0 < HEAP32[1310881] >>> 0) {
                _abort();
              } else {
                HEAP32[$R288_1$s2 + 4] = $96;
                HEAP32[$96 + 24 >> 2] = $R288_1;
                break;
              }
            }
          } while (0);
          var $100 = HEAP32[$and5$s2 + ($mem$s2 + 3)];
          if (($100 | 0) == 0) {
            break;
          }
          if ($100 >>> 0 < HEAP32[1310881] >>> 0) {
            _abort();
          } else {
            HEAP32[$R288_1$s2 + 5] = $100;
            HEAP32[$100 + 24 >> 2] = $R288_1;
            break;
          }
        }
      } while (0);
      HEAP32[$p_0$s2 + 1] = $add240 | 1;
      HEAP32[($add240 >> 2) + $51$s2] = $add240;
      if (($p_0 | 0) != (HEAP32[1310882] | 0)) {
        var $psize_1 = $add240;
        break;
      }
      HEAP32[1310879] = $add240;
      return;
    } else {
      HEAP32[$52 >> 2] = $53 & -2;
      HEAP32[$p_0$s2 + 1] = $psize_0 | 1;
      HEAP32[($psize_0 >> 2) + $51$s2] = $psize_0;
      var $psize_1 = $psize_0;
    }
  } while (0);
  var $psize_1;
  var $shr443 = $psize_1 >>> 3;
  if ($psize_1 >>> 0 < 256) {
    var $shl450 = $shr443 << 1;
    var $104 = ($shl450 << 2) + 5243548 | 0;
    var $105 = HEAP32[1310877];
    var $shl453 = 1 << $shr443;
    do {
      if (($105 & $shl453 | 0) == 0) {
        HEAP32[1310877] = $105 | $shl453;
        var $F452_0 = $104;
        var $_pre_phi = ($shl450 + 2 << 2) + 5243548 | 0;
      } else {
        var $106 = ($shl450 + 2 << 2) + 5243548 | 0;
        var $107 = HEAP32[$106 >> 2];
        if ($107 >>> 0 >= HEAP32[1310881] >>> 0) {
          var $F452_0 = $107;
          var $_pre_phi = $106;
          break;
        }
        _abort();
      }
    } while (0);
    var $_pre_phi;
    var $F452_0;
    HEAP32[$_pre_phi >> 2] = $p_0;
    HEAP32[$F452_0 + 12 >> 2] = $p_0;
    HEAP32[$p_0$s2 + 2] = $F452_0;
    HEAP32[$p_0$s2 + 3] = $104;
    return;
  }
  var $110 = $p_0;
  var $shr477 = $psize_1 >>> 8;
  do {
    if (($shr477 | 0) == 0) {
      var $I476_0 = 0;
    } else {
      if ($psize_1 >>> 0 > 16777215) {
        var $I476_0 = 31;
        break;
      }
      var $and487 = ($shr477 + 1048320 | 0) >>> 16 & 8;
      var $shl488 = $shr477 << $and487;
      var $and491 = ($shl488 + 520192 | 0) >>> 16 & 4;
      var $shl493 = $shl488 << $and491;
      var $and496 = ($shl493 + 245760 | 0) >>> 16 & 2;
      var $add501 = 14 - ($and491 | $and487 | $and496) + ($shl493 << $and496 >>> 15) | 0;
      var $I476_0 = $psize_1 >>> (($add501 + 7 | 0) >>> 0) & 1 | $add501 << 1;
    }
  } while (0);
  var $I476_0;
  var $arrayidx509 = ($I476_0 << 2) + 5243812 | 0;
  HEAP32[$p_0$s2 + 7] = $I476_0;
  HEAP32[$p_0$s2 + 5] = 0;
  HEAP32[$p_0$s2 + 4] = 0;
  var $112 = HEAP32[1310878];
  var $shl515 = 1 << $I476_0;
  do {
    if (($112 & $shl515 | 0) == 0) {
      HEAP32[1310878] = $112 | $shl515;
      HEAP32[$arrayidx509 >> 2] = $110;
      HEAP32[$p_0$s2 + 6] = $arrayidx509;
      HEAP32[$p_0$s2 + 3] = $p_0;
      HEAP32[$p_0$s2 + 2] = $p_0;
    } else {
      if (($I476_0 | 0) == 31) {
        var $cond = 0;
      } else {
        var $cond = 25 - ($I476_0 >>> 1) | 0;
      }
      var $cond;
      var $K525_0 = $psize_1 << $cond;
      var $T_0 = HEAP32[$arrayidx509 >> 2];
      while (1) {
        var $T_0;
        var $K525_0;
        if ((HEAP32[$T_0 + 4 >> 2] & -8 | 0) == ($psize_1 | 0)) {
          break;
        }
        var $arrayidx541 = ($K525_0 >>> 31 << 2) + $T_0 + 16 | 0;
        var $115 = HEAP32[$arrayidx541 >> 2];
        if (($115 | 0) == 0) {
          label = 1880;
          break;
        } else {
          var $K525_0 = $K525_0 << 1;
          var $T_0 = $115;
        }
      }
      if (label == 1880) {
        if ($arrayidx541 >>> 0 < HEAP32[1310881] >>> 0) {
          _abort();
        } else {
          HEAP32[$arrayidx541 >> 2] = $110;
          HEAP32[$p_0$s2 + 6] = $T_0;
          HEAP32[$p_0$s2 + 3] = $p_0;
          HEAP32[$p_0$s2 + 2] = $p_0;
          break;
        }
      }
      var $fd559 = $T_0 + 8 | 0;
      var $118 = HEAP32[$fd559 >> 2];
      var $120 = HEAP32[1310881];
      if ($T_0 >>> 0 < $120 >>> 0) {
        _abort();
      }
      if ($118 >>> 0 < $120 >>> 0) {
        _abort();
      } else {
        HEAP32[$118 + 12 >> 2] = $110;
        HEAP32[$fd559 >> 2] = $110;
        HEAP32[$p_0$s2 + 2] = $118;
        HEAP32[$p_0$s2 + 3] = $T_0;
        HEAP32[$p_0$s2 + 6] = 0;
        break;
      }
    }
  } while (0);
  var $dec = HEAP32[1310885] - 1 | 0;
  HEAP32[1310885] = $dec;
  if (($dec | 0) != 0) {
    return;
  }
  _release_unused_segments();
  return;
}
_free["X"] = 1;
function _mmap_resize($oldp, $nb) {
  var $and = HEAP32[$oldp + 4 >> 2] & -8;
  do {
    if ($nb >>> 0 < 256) {
      var $retval_0 = 0;
    } else {
      if ($and >>> 0 >= ($nb + 4 | 0) >>> 0) {
        if (($and - $nb | 0) >>> 0 <= HEAP32[1310722] << 1 >>> 0) {
          var $retval_0 = $oldp;
          break;
        }
      }
      var $retval_0 = 0;
    }
  } while (0);
  var $retval_0;
  return $retval_0;
}
function _segment_holding($addr) {
  var $sp_0$s2;
  var label = 0;
  var $sp_0 = 5243952, $sp_0$s2 = $sp_0 >> 2;
  while (1) {
    var $sp_0;
    var $0 = HEAP32[$sp_0$s2];
    if ($0 >>> 0 <= $addr >>> 0) {
      if (($0 + HEAP32[$sp_0$s2 + 1] | 0) >>> 0 > $addr >>> 0) {
        var $retval_0 = $sp_0;
        label = 1922;
        break;
      }
    }
    var $2 = HEAP32[$sp_0$s2 + 2];
    if (($2 | 0) == 0) {
      var $retval_0 = 0;
      label = 1923;
      break;
    } else {
      var $sp_0 = $2, $sp_0$s2 = $sp_0 >> 2;
    }
  }
  if (label == 1922) {
    var $retval_0;
    return $retval_0;
  } else if (label == 1923) {
    var $retval_0;
    return $retval_0;
  }
}
function _init_top($p, $psize) {
  var $0 = $p;
  var $1 = $p + 8 | 0;
  if (($1 & 7 | 0) == 0) {
    var $cond = 0;
  } else {
    var $cond = -$1 & 7;
  }
  var $cond;
  var $sub5 = $psize - $cond | 0;
  HEAP32[1310883] = $0 + $cond | 0;
  HEAP32[1310880] = $sub5;
  HEAP32[$cond + ($0 + 4) >> 2] = $sub5 | 1;
  HEAP32[$psize + ($0 + 4) >> 2] = 40;
  HEAP32[1310884] = HEAP32[1310724];
  return;
}
function _init_bins() {
  var $i_02 = 0;
  while (1) {
    var $i_02;
    var $shl = $i_02 << 1;
    var $0 = ($shl << 2) + 5243548 | 0;
    HEAP32[($shl + 3 << 2) + 5243548 >> 2] = $0;
    HEAP32[($shl + 2 << 2) + 5243548 >> 2] = $0;
    var $inc = $i_02 + 1 | 0;
    if (($inc | 0) == 32) {
      break;
    } else {
      var $i_02 = $inc;
    }
  }
  return;
}
function _realloc($oldmem, $bytes) {
  if (($oldmem | 0) == 0) {
    var $retval_0 = _malloc($bytes);
  } else {
    var $retval_0 = _internal_realloc($oldmem, $bytes);
  }
  var $retval_0;
  return $retval_0;
}
function _internal_realloc($oldmem, $bytes) {
  var $5$s2;
  var $1$s2;
  var label = 0;
  if ($bytes >>> 0 > 4294967231) {
    HEAP32[___errno_location() >> 2] = 12;
    var $retval_0 = 0;
    var $retval_0;
    return $retval_0;
  }
  var $add_ptr = $oldmem - 8 | 0;
  var $0 = $add_ptr;
  var $1$s2 = ($oldmem - 4 | 0) >> 2;
  var $2 = HEAP32[$1$s2];
  var $and = $2 & -8;
  var $add_ptr_sum = $and - 8 | 0;
  var $3 = $oldmem + $add_ptr_sum | 0;
  if ($add_ptr >>> 0 < HEAP32[1310881] >>> 0) {
    _abort();
  }
  var $and4 = $2 & 3;
  if (!(($and4 | 0) != 1 & ($add_ptr_sum | 0) > -8)) {
    _abort();
  }
  var $5$s2 = ($oldmem + ($and - 4) | 0) >> 2;
  if ((HEAP32[$5$s2] & 1 | 0) == 0) {
    _abort();
  }
  if ($bytes >>> 0 < 11) {
    var $cond = 16;
  } else {
    var $cond = $bytes + 11 & -8;
  }
  var $cond;
  do {
    if (($and4 | 0) == 0) {
      var $newp_0 = _mmap_resize($0, $cond);
      var $extra_0 = 0;
      label = 1950;
      break;
    } else {
      if ($and >>> 0 >= $cond >>> 0) {
        var $sub = $and - $cond | 0;
        if ($sub >>> 0 <= 15) {
          var $newp_0 = $0;
          var $extra_0 = 0;
          label = 1950;
          break;
        }
        HEAP32[$1$s2] = $cond | $2 & 1 | 2;
        HEAP32[$oldmem + ($cond - 4) >> 2] = $sub | 3;
        HEAP32[$5$s2] = HEAP32[$5$s2] | 1;
        var $newp_0 = $0;
        var $extra_0 = $oldmem + $cond | 0;
        label = 1950;
        break;
      }
      if (($3 | 0) != (HEAP32[1310883] | 0)) {
        break;
      }
      var $add43 = HEAP32[1310880] + $and | 0;
      if ($add43 >>> 0 <= $cond >>> 0) {
        break;
      }
      var $sub48 = $add43 - $cond | 0;
      HEAP32[$1$s2] = $cond | $2 & 1 | 2;
      HEAP32[$oldmem + ($cond - 4) >> 2] = $sub48 | 1;
      HEAP32[1310883] = $oldmem + ($cond - 8) | 0;
      HEAP32[1310880] = $sub48;
      var $newp_0 = $0;
      var $extra_0 = 0;
      label = 1950;
      break;
    }
  } while (0);
  do {
    if (label == 1950) {
      var $extra_0;
      var $newp_0;
      if (($newp_0 | 0) == 0) {
        break;
      }
      if (($extra_0 | 0) != 0) {
        _free($extra_0);
      }
      var $retval_0 = $newp_0 + 8 | 0;
      var $retval_0;
      return $retval_0;
    }
  } while (0);
  var $call74 = _malloc($bytes);
  if (($call74 | 0) == 0) {
    var $retval_0 = 0;
    var $retval_0;
    return $retval_0;
  }
  var $sub81 = $and - ((HEAP32[$1$s2] & 3 | 0) == 0 ? 8 : 4) | 0;
  _memcpy($call74, $oldmem, $sub81 >>> 0 < $bytes >>> 0 ? $sub81 : $bytes);
  _free($oldmem);
  var $retval_0 = $call74;
  var $retval_0;
  return $retval_0;
}
_internal_realloc["X"] = 1;
function _init_mparams() {
  if ((HEAP32[1310720] | 0) != 0) {
    return;
  }
  var $call = _sysconf(8);
  if (($call - 1 & $call | 0) != 0) {
    _abort();
  }
  HEAP32[1310722] = $call;
  HEAP32[1310721] = $call;
  HEAP32[1310723] = -1;
  HEAP32[1310724] = 2097152;
  HEAP32[1310725] = 0;
  HEAP32[1310987] = 0;
  HEAP32[1310720] = _time(0) & -16 ^ 1431655768;
  return;
}
function _prepend_alloc($newbase, $oldbase, $nb) {
  var $R_1$s2;
  var $add_ptr4_sum$s2;
  var $cond15$s2;
  var $oldbase$s2 = $oldbase >> 2;
  var $newbase$s2 = $newbase >> 2;
  var label = 0;
  var $0 = $newbase + 8 | 0;
  if (($0 & 7 | 0) == 0) {
    var $cond = 0;
  } else {
    var $cond = -$0 & 7;
  }
  var $cond;
  var $2 = $oldbase + 8 | 0;
  if (($2 & 7 | 0) == 0) {
    var $cond15 = 0, $cond15$s2 = $cond15 >> 2;
  } else {
    var $cond15 = -$2 & 7, $cond15$s2 = $cond15 >> 2;
  }
  var $cond15;
  var $add_ptr16 = $oldbase + $cond15 | 0;
  var $4 = $add_ptr16;
  var $add_ptr4_sum = $cond + $nb | 0, $add_ptr4_sum$s2 = $add_ptr4_sum >> 2;
  var $add_ptr17 = $newbase + $add_ptr4_sum | 0;
  var $5 = $add_ptr17;
  var $sub18 = $add_ptr16 - ($newbase + $cond) - $nb | 0;
  HEAP32[($cond + 4 >> 2) + $newbase$s2] = $nb | 3;
  if (($4 | 0) == (HEAP32[1310883] | 0)) {
    var $add = HEAP32[1310880] + $sub18 | 0;
    HEAP32[1310880] = $add;
    HEAP32[1310883] = $5;
    HEAP32[$add_ptr4_sum$s2 + ($newbase$s2 + 1)] = $add | 1;
    var $add_ptr4_sum1415 = $cond | 8;
    var $add_ptr353 = $newbase + $add_ptr4_sum1415 | 0;
    return $add_ptr353;
  }
  if (($4 | 0) == (HEAP32[1310882] | 0)) {
    var $add26 = HEAP32[1310879] + $sub18 | 0;
    HEAP32[1310879] = $add26;
    HEAP32[1310882] = $5;
    HEAP32[$add_ptr4_sum$s2 + ($newbase$s2 + 1)] = $add26 | 1;
    HEAP32[($add26 >> 2) + $newbase$s2 + $add_ptr4_sum$s2] = $add26;
    var $add_ptr4_sum1415 = $cond | 8;
    var $add_ptr353 = $newbase + $add_ptr4_sum1415 | 0;
    return $add_ptr353;
  }
  var $14 = HEAP32[$cond15$s2 + ($oldbase$s2 + 1)];
  if (($14 & 3 | 0) == 1) {
    var $and37 = $14 & -8;
    var $shr = $14 >>> 3;
    var $cmp38 = $14 >>> 0 < 256;
    L2446 : do {
      if ($cmp38) {
        var $16 = HEAP32[(($cond15 | 8) >> 2) + $oldbase$s2];
        var $18 = HEAP32[$cond15$s2 + ($oldbase$s2 + 3)];
        if (($16 | 0) == ($18 | 0)) {
          HEAP32[1310877] = HEAP32[1310877] & (1 << $shr ^ -1);
          break;
        }
        var $20 = ($shr << 3) + 5243548 | 0;
        do {
          if (($16 | 0) != ($20 | 0)) {
            if ($16 >>> 0 >= HEAP32[1310881] >>> 0) {
              break;
            }
            _abort();
          }
        } while (0);
        do {
          if (($18 | 0) != ($20 | 0)) {
            if ($18 >>> 0 >= HEAP32[1310881] >>> 0) {
              break;
            }
            _abort();
          }
        } while (0);
        HEAP32[$16 + 12 >> 2] = $18;
        HEAP32[$18 + 8 >> 2] = $16;
      } else {
        var $25 = $add_ptr16;
        var $27 = HEAP32[(($cond15 | 24) >> 2) + $oldbase$s2];
        var $29 = HEAP32[$cond15$s2 + ($oldbase$s2 + 3)];
        var $cmp61 = ($29 | 0) == ($25 | 0);
        L2448 : do {
          if ($cmp61) {
            var $add_ptr16_sum56 = $cond15 | 16;
            var $34 = $add_ptr16_sum56 + ($oldbase + 4) | 0;
            var $35 = HEAP32[$34 >> 2];
            do {
              if (($35 | 0) == 0) {
                var $arrayidx81 = $oldbase + $add_ptr16_sum56 | 0;
                var $36 = HEAP32[$arrayidx81 >> 2];
                if (($36 | 0) == 0) {
                  var $R_1 = 0, $R_1$s2 = $R_1 >> 2;
                  break L2448;
                } else {
                  var $R_0 = $36;
                  var $RP_0 = $arrayidx81;
                  break;
                }
              } else {
                var $R_0 = $35;
                var $RP_0 = $34;
              }
            } while (0);
            while (1) {
              var $RP_0;
              var $R_0;
              var $arrayidx86 = $R_0 + 20 | 0;
              var $37 = HEAP32[$arrayidx86 >> 2];
              if (($37 | 0) != 0) {
                var $R_0 = $37;
                var $RP_0 = $arrayidx86;
                continue;
              }
              var $arrayidx91 = $R_0 + 16 | 0;
              var $38 = HEAP32[$arrayidx91 >> 2];
              if (($38 | 0) == 0) {
                break;
              } else {
                var $R_0 = $38;
                var $RP_0 = $arrayidx91;
              }
            }
            if ($RP_0 >>> 0 < HEAP32[1310881] >>> 0) {
              _abort();
            } else {
              HEAP32[$RP_0 >> 2] = 0;
              var $R_1 = $R_0, $R_1$s2 = $R_1 >> 2;
              break;
            }
          } else {
            var $31 = HEAP32[(($cond15 | 8) >> 2) + $oldbase$s2];
            if ($31 >>> 0 < HEAP32[1310881] >>> 0) {
              _abort();
            } else {
              HEAP32[$31 + 12 >> 2] = $29;
              HEAP32[$29 + 8 >> 2] = $31;
              var $R_1 = $29, $R_1$s2 = $R_1 >> 2;
              break;
            }
          }
        } while (0);
        var $R_1;
        if (($27 | 0) == 0) {
          break;
        }
        var $41 = $cond15 + ($oldbase + 28) | 0;
        var $arrayidx108 = (HEAP32[$41 >> 2] << 2) + 5243812 | 0;
        do {
          if (($25 | 0) == (HEAP32[$arrayidx108 >> 2] | 0)) {
            HEAP32[$arrayidx108 >> 2] = $R_1;
            if (($R_1 | 0) != 0) {
              break;
            }
            HEAP32[1310878] = HEAP32[1310878] & (1 << HEAP32[$41 >> 2] ^ -1);
            break L2446;
          } else {
            if ($27 >>> 0 < HEAP32[1310881] >>> 0) {
              _abort();
            }
            var $arrayidx128 = $27 + 16 | 0;
            if ((HEAP32[$arrayidx128 >> 2] | 0) == ($25 | 0)) {
              HEAP32[$arrayidx128 >> 2] = $R_1;
            } else {
              HEAP32[$27 + 20 >> 2] = $R_1;
            }
            if (($R_1 | 0) == 0) {
              break L2446;
            }
          }
        } while (0);
        if ($R_1 >>> 0 < HEAP32[1310881] >>> 0) {
          _abort();
        }
        HEAP32[$R_1$s2 + 6] = $27;
        var $add_ptr16_sum2627 = $cond15 | 16;
        var $51 = HEAP32[($add_ptr16_sum2627 >> 2) + $oldbase$s2];
        do {
          if (($51 | 0) != 0) {
            if ($51 >>> 0 < HEAP32[1310881] >>> 0) {
              _abort();
            } else {
              HEAP32[$R_1$s2 + 4] = $51;
              HEAP32[$51 + 24 >> 2] = $R_1;
              break;
            }
          }
        } while (0);
        var $55 = HEAP32[($add_ptr16_sum2627 + 4 >> 2) + $oldbase$s2];
        if (($55 | 0) == 0) {
          break;
        }
        if ($55 >>> 0 < HEAP32[1310881] >>> 0) {
          _abort();
        } else {
          HEAP32[$R_1$s2 + 5] = $55;
          HEAP32[$55 + 24 >> 2] = $R_1;
          break;
        }
      }
    } while (0);
    var $oldfirst_0 = $oldbase + ($and37 | $cond15) | 0;
    var $qsize_0 = $and37 + $sub18 | 0;
  } else {
    var $oldfirst_0 = $4;
    var $qsize_0 = $sub18;
  }
  var $qsize_0;
  var $oldfirst_0;
  var $head193 = $oldfirst_0 + 4 | 0;
  HEAP32[$head193 >> 2] = HEAP32[$head193 >> 2] & -2;
  HEAP32[$add_ptr4_sum$s2 + ($newbase$s2 + 1)] = $qsize_0 | 1;
  HEAP32[($qsize_0 >> 2) + $newbase$s2 + $add_ptr4_sum$s2] = $qsize_0;
  var $shr199 = $qsize_0 >>> 3;
  if ($qsize_0 >>> 0 < 256) {
    var $shl206 = $shr199 << 1;
    var $61 = ($shl206 << 2) + 5243548 | 0;
    var $62 = HEAP32[1310877];
    var $shl211 = 1 << $shr199;
    do {
      if (($62 & $shl211 | 0) == 0) {
        HEAP32[1310877] = $62 | $shl211;
        var $F209_0 = $61;
        var $_pre_phi = ($shl206 + 2 << 2) + 5243548 | 0;
      } else {
        var $63 = ($shl206 + 2 << 2) + 5243548 | 0;
        var $64 = HEAP32[$63 >> 2];
        if ($64 >>> 0 >= HEAP32[1310881] >>> 0) {
          var $F209_0 = $64;
          var $_pre_phi = $63;
          break;
        }
        _abort();
      }
    } while (0);
    var $_pre_phi;
    var $F209_0;
    HEAP32[$_pre_phi >> 2] = $5;
    HEAP32[$F209_0 + 12 >> 2] = $5;
    HEAP32[$add_ptr4_sum$s2 + ($newbase$s2 + 2)] = $F209_0;
    HEAP32[$add_ptr4_sum$s2 + ($newbase$s2 + 3)] = $61;
    var $add_ptr4_sum1415 = $cond | 8;
    var $add_ptr353 = $newbase + $add_ptr4_sum1415 | 0;
    return $add_ptr353;
  }
  var $69 = $add_ptr17;
  var $shr238 = $qsize_0 >>> 8;
  do {
    if (($shr238 | 0) == 0) {
      var $I237_0 = 0;
    } else {
      if ($qsize_0 >>> 0 > 16777215) {
        var $I237_0 = 31;
        break;
      }
      var $and249 = ($shr238 + 1048320 | 0) >>> 16 & 8;
      var $shl250 = $shr238 << $and249;
      var $and253 = ($shl250 + 520192 | 0) >>> 16 & 4;
      var $shl255 = $shl250 << $and253;
      var $and258 = ($shl255 + 245760 | 0) >>> 16 & 2;
      var $add263 = 14 - ($and253 | $and249 | $and258) + ($shl255 << $and258 >>> 15) | 0;
      var $I237_0 = $qsize_0 >>> (($add263 + 7 | 0) >>> 0) & 1 | $add263 << 1;
    }
  } while (0);
  var $I237_0;
  var $arrayidx272 = ($I237_0 << 2) + 5243812 | 0;
  HEAP32[$add_ptr4_sum$s2 + ($newbase$s2 + 7)] = $I237_0;
  HEAP32[$add_ptr4_sum$s2 + ($newbase$s2 + 5)] = 0;
  HEAP32[$add_ptr4_sum$s2 + ($newbase$s2 + 4)] = 0;
  var $72 = HEAP32[1310878];
  var $shl279 = 1 << $I237_0;
  if (($72 & $shl279 | 0) == 0) {
    HEAP32[1310878] = $72 | $shl279;
    HEAP32[$arrayidx272 >> 2] = $69;
    HEAP32[$add_ptr4_sum$s2 + ($newbase$s2 + 6)] = $arrayidx272;
    HEAP32[$add_ptr4_sum$s2 + ($newbase$s2 + 3)] = $69;
    HEAP32[$add_ptr4_sum$s2 + ($newbase$s2 + 2)] = $69;
    var $add_ptr4_sum1415 = $cond | 8;
    var $add_ptr353 = $newbase + $add_ptr4_sum1415 | 0;
    return $add_ptr353;
  }
  if (($I237_0 | 0) == 31) {
    var $cond300 = 0;
  } else {
    var $cond300 = 25 - ($I237_0 >>> 1) | 0;
  }
  var $cond300;
  var $K290_0 = $qsize_0 << $cond300;
  var $T_0 = HEAP32[$arrayidx272 >> 2];
  while (1) {
    var $T_0;
    var $K290_0;
    if ((HEAP32[$T_0 + 4 >> 2] & -8 | 0) == ($qsize_0 | 0)) {
      break;
    }
    var $arrayidx310 = ($K290_0 >>> 31 << 2) + $T_0 + 16 | 0;
    var $79 = HEAP32[$arrayidx310 >> 2];
    if (($79 | 0) == 0) {
      label = 2037;
      break;
    } else {
      var $K290_0 = $K290_0 << 1;
      var $T_0 = $79;
    }
  }
  if (label == 2037) {
    if ($arrayidx310 >>> 0 < HEAP32[1310881] >>> 0) {
      _abort();
    }
    HEAP32[$arrayidx310 >> 2] = $69;
    HEAP32[$add_ptr4_sum$s2 + ($newbase$s2 + 6)] = $T_0;
    HEAP32[$add_ptr4_sum$s2 + ($newbase$s2 + 3)] = $69;
    HEAP32[$add_ptr4_sum$s2 + ($newbase$s2 + 2)] = $69;
    var $add_ptr4_sum1415 = $cond | 8;
    var $add_ptr353 = $newbase + $add_ptr4_sum1415 | 0;
    return $add_ptr353;
  }
  var $fd329 = $T_0 + 8 | 0;
  var $85 = HEAP32[$fd329 >> 2];
  var $87 = HEAP32[1310881];
  if ($T_0 >>> 0 < $87 >>> 0) {
    _abort();
  }
  if ($85 >>> 0 < $87 >>> 0) {
    _abort();
  }
  HEAP32[$85 + 12 >> 2] = $69;
  HEAP32[$fd329 >> 2] = $69;
  HEAP32[$add_ptr4_sum$s2 + ($newbase$s2 + 2)] = $85;
  HEAP32[$add_ptr4_sum$s2 + ($newbase$s2 + 3)] = $T_0;
  HEAP32[$add_ptr4_sum$s2 + ($newbase$s2 + 6)] = 0;
  var $add_ptr4_sum1415 = $cond | 8;
  var $add_ptr353 = $newbase + $add_ptr4_sum1415 | 0;
  return $add_ptr353;
}
_prepend_alloc["X"] = 1;
function __ZNKSt9bad_alloc4whatEv($this) {
  return 5243056;
}
function __ZSt15get_new_handlerv() {
  return tempValue = HEAP32[1311197], HEAP32[1311197] = tempValue, tempValue;
}
function _memcpy($dest, $src, $n) {
  var $0 = $dest;
  if ((($src ^ $0) & 3 | 0) == 0) {
    var $tobool329 = ($n | 0) == 0;
    var $or_cond30 = ($0 & 3 | 0) == 0 | $tobool329;
    L2546 : do {
      if ($or_cond30) {
        var $n_addr_0_lcssa = $n;
        var $d_0_lcssa = $dest;
        var $s_0_lcssa = $src;
        var $tobool3_lcssa = $tobool329;
      } else {
        var $n_addr_031 = $n;
        var $d_032 = $dest;
        var $s_033 = $src;
        while (1) {
          var $s_033;
          var $d_032;
          var $n_addr_031;
          var $incdec_ptr = $s_033 + 1 | 0;
          var $incdec_ptr4 = $d_032 + 1 | 0;
          HEAP8[$d_032] = HEAP8[$s_033];
          var $dec = $n_addr_031 - 1 | 0;
          var $tobool3 = ($dec | 0) == 0;
          if (($incdec_ptr4 & 3 | 0) == 0 | $tobool3) {
            var $n_addr_0_lcssa = $dec;
            var $d_0_lcssa = $incdec_ptr4;
            var $s_0_lcssa = $incdec_ptr;
            var $tobool3_lcssa = $tobool3;
            break L2546;
          } else {
            var $n_addr_031 = $dec;
            var $d_032 = $incdec_ptr4;
            var $s_033 = $incdec_ptr;
          }
        }
      }
    } while (0);
    var $tobool3_lcssa;
    var $s_0_lcssa;
    var $d_0_lcssa;
    var $n_addr_0_lcssa;
    if ($tobool3_lcssa) {
      return $dest;
    }
    var $6 = $d_0_lcssa;
    var $7 = $s_0_lcssa;
    var $cmp821 = $n_addr_0_lcssa >>> 0 > 3;
    L2553 : do {
      if ($cmp821) {
        var $n_addr_122 = $n_addr_0_lcssa;
        var $wd_023 = $6;
        var $ws_024 = $7;
        while (1) {
          var $ws_024;
          var $wd_023;
          var $n_addr_122;
          var $incdec_ptr10 = $ws_024 + 4 | 0;
          var $incdec_ptr11 = $wd_023 + 4 | 0;
          HEAP32[$wd_023 >> 2] = HEAP32[$ws_024 >> 2];
          var $sub = $n_addr_122 - 4 | 0;
          if ($sub >>> 0 > 3) {
            var $n_addr_122 = $sub;
            var $wd_023 = $incdec_ptr11;
            var $ws_024 = $incdec_ptr10;
          } else {
            var $n_addr_1_lcssa = $sub;
            var $wd_0_lcssa = $incdec_ptr11;
            var $ws_0_lcssa = $incdec_ptr10;
            break L2553;
          }
        }
      } else {
        var $n_addr_1_lcssa = $n_addr_0_lcssa;
        var $wd_0_lcssa = $6;
        var $ws_0_lcssa = $7;
      }
    } while (0);
    var $ws_0_lcssa;
    var $wd_0_lcssa;
    var $n_addr_1_lcssa;
    var $s_1_ph = $ws_0_lcssa;
    var $d_1_ph = $wd_0_lcssa;
    var $n_addr_2_ph = $n_addr_1_lcssa;
  } else {
    var $s_1_ph = $src;
    var $d_1_ph = $dest;
    var $n_addr_2_ph = $n;
  }
  var $n_addr_2_ph;
  var $d_1_ph;
  var $s_1_ph;
  if (($n_addr_2_ph | 0) == 0) {
    return $dest;
  } else {
    var $n_addr_218 = $n_addr_2_ph;
    var $d_119 = $d_1_ph;
    var $s_120 = $s_1_ph;
  }
  while (1) {
    var $s_120;
    var $d_119;
    var $n_addr_218;
    HEAP8[$d_119] = HEAP8[$s_120];
    var $dec20 = $n_addr_218 - 1 | 0;
    if (($dec20 | 0) == 0) {
      break;
    } else {
      var $n_addr_218 = $dec20;
      var $d_119 = $d_119 + 1 | 0;
      var $s_120 = $s_120 + 1 | 0;
    }
  }
  return $dest;
}
Module["_memcpy"] = _memcpy;
_memcpy["X"] = 1;
function __ZNSt9bad_allocC2Ev($this) {
  HEAP32[$this >> 2] = 5244328;
  return;
}
function __ZNSt9bad_allocC1Ev($this) {
  __ZNSt9bad_allocC2Ev($this);
  return;
}
function __ZNSt9bad_allocD1Ev($this) {
  __ZNSt9bad_allocD2Ev($this);
  return;
}
function __ZdlPv($ptr) {
  if (($ptr | 0) == 0) {
    return;
  }
  _free($ptr);
  return;
}
function __ZNSt9bad_allocD0Ev($this) {
  __ZNSt9bad_allocD1Ev($this);
  __ZdlPv($this);
  return;
}
function __ZNSt9bad_allocD2Ev($this) {
  return;
}
function _add_segment($tbase, $tsize) {
  var $add_ptr14$s2;
  var $0$s2;
  var label = 0;
  var $0 = HEAP32[1310883], $0$s2 = $0 >> 2;
  var $1 = $0;
  var $call = _segment_holding($1);
  var $2 = HEAP32[$call >> 2];
  var $3 = HEAP32[$call + 4 >> 2];
  var $add_ptr = $2 + $3 | 0;
  var $4 = $2 + ($3 - 39) | 0;
  if (($4 & 7 | 0) == 0) {
    var $cond = 0;
  } else {
    var $cond = -$4 & 7;
  }
  var $cond;
  var $add_ptr7 = $2 + ($3 - 47) + $cond | 0;
  var $cond13 = $add_ptr7 >>> 0 < ($0 + 16 | 0) >>> 0 ? $1 : $add_ptr7;
  var $add_ptr14 = $cond13 + 8 | 0, $add_ptr14$s2 = $add_ptr14 >> 2;
  _init_top($tbase, $tsize - 40 | 0);
  HEAP32[$cond13 + 4 >> 2] = 27;
  HEAP32[$add_ptr14$s2] = HEAP32[1310988];
  HEAP32[$add_ptr14$s2 + 1] = HEAP32[1310989];
  HEAP32[$add_ptr14$s2 + 2] = HEAP32[1310990];
  HEAP32[$add_ptr14$s2 + 3] = HEAP32[1310991];
  HEAP32[1310988] = $tbase;
  HEAP32[1310989] = $tsize;
  HEAP32[1310991] = 0;
  HEAP32[1310990] = $add_ptr14;
  var $9 = $cond13 + 28 | 0;
  HEAP32[$9 >> 2] = 7;
  var $cmp2712 = ($cond13 + 32 | 0) >>> 0 < $add_ptr >>> 0;
  L2577 : do {
    if ($cmp2712) {
      var $add_ptr2413 = $9;
      while (1) {
        var $add_ptr2413;
        var $11 = $add_ptr2413 + 4 | 0;
        HEAP32[$11 >> 2] = 7;
        if (($add_ptr2413 + 8 | 0) >>> 0 < $add_ptr >>> 0) {
          var $add_ptr2413 = $11;
        } else {
          break L2577;
        }
      }
    }
  } while (0);
  if (($cond13 | 0) == ($1 | 0)) {
    return;
  }
  var $sub_ptr_sub = $cond13 - $0 | 0;
  var $14 = $sub_ptr_sub + ($1 + 4) | 0;
  HEAP32[$14 >> 2] = HEAP32[$14 >> 2] & -2;
  HEAP32[$0$s2 + 1] = $sub_ptr_sub | 1;
  HEAP32[$1 + $sub_ptr_sub >> 2] = $sub_ptr_sub;
  var $shr = $sub_ptr_sub >>> 3;
  if ($sub_ptr_sub >>> 0 < 256) {
    var $shl = $shr << 1;
    var $16 = ($shl << 2) + 5243548 | 0;
    var $17 = HEAP32[1310877];
    var $shl39 = 1 << $shr;
    do {
      if (($17 & $shl39 | 0) == 0) {
        HEAP32[1310877] = $17 | $shl39;
        var $F_0 = $16;
        var $_pre_phi = ($shl + 2 << 2) + 5243548 | 0;
      } else {
        var $18 = ($shl + 2 << 2) + 5243548 | 0;
        var $19 = HEAP32[$18 >> 2];
        if ($19 >>> 0 >= HEAP32[1310881] >>> 0) {
          var $F_0 = $19;
          var $_pre_phi = $18;
          break;
        }
        _abort();
      }
    } while (0);
    var $_pre_phi;
    var $F_0;
    HEAP32[$_pre_phi >> 2] = $0;
    HEAP32[$F_0 + 12 >> 2] = $0;
    HEAP32[$0$s2 + 2] = $F_0;
    HEAP32[$0$s2 + 3] = $16;
    return;
  }
  var $22 = $0;
  var $shr58 = $sub_ptr_sub >>> 8;
  do {
    if (($shr58 | 0) == 0) {
      var $I57_0 = 0;
    } else {
      if ($sub_ptr_sub >>> 0 > 16777215) {
        var $I57_0 = 31;
        break;
      }
      var $and69 = ($shr58 + 1048320 | 0) >>> 16 & 8;
      var $shl70 = $shr58 << $and69;
      var $and73 = ($shl70 + 520192 | 0) >>> 16 & 4;
      var $shl75 = $shl70 << $and73;
      var $and78 = ($shl75 + 245760 | 0) >>> 16 & 2;
      var $add83 = 14 - ($and73 | $and69 | $and78) + ($shl75 << $and78 >>> 15) | 0;
      var $I57_0 = $sub_ptr_sub >>> (($add83 + 7 | 0) >>> 0) & 1 | $add83 << 1;
    }
  } while (0);
  var $I57_0;
  var $arrayidx91 = ($I57_0 << 2) + 5243812 | 0;
  HEAP32[$0$s2 + 7] = $I57_0;
  HEAP32[$0$s2 + 5] = 0;
  HEAP32[$0$s2 + 4] = 0;
  var $24 = HEAP32[1310878];
  var $shl95 = 1 << $I57_0;
  if (($24 & $shl95 | 0) == 0) {
    HEAP32[1310878] = $24 | $shl95;
    HEAP32[$arrayidx91 >> 2] = $22;
    HEAP32[$0$s2 + 6] = $arrayidx91;
    HEAP32[$0$s2 + 3] = $0;
    HEAP32[$0$s2 + 2] = $0;
    return;
  }
  if (($I57_0 | 0) == 31) {
    var $cond115 = 0;
  } else {
    var $cond115 = 25 - ($I57_0 >>> 1) | 0;
  }
  var $cond115;
  var $K105_0 = $sub_ptr_sub << $cond115;
  var $T_0 = HEAP32[$arrayidx91 >> 2];
  while (1) {
    var $T_0;
    var $K105_0;
    if ((HEAP32[$T_0 + 4 >> 2] & -8 | 0) == ($sub_ptr_sub | 0)) {
      break;
    }
    var $arrayidx126 = ($K105_0 >>> 31 << 2) + $T_0 + 16 | 0;
    var $27 = HEAP32[$arrayidx126 >> 2];
    if (($27 | 0) == 0) {
      label = 2101;
      break;
    } else {
      var $K105_0 = $K105_0 << 1;
      var $T_0 = $27;
    }
  }
  if (label == 2101) {
    if ($arrayidx126 >>> 0 < HEAP32[1310881] >>> 0) {
      _abort();
    }
    HEAP32[$arrayidx126 >> 2] = $22;
    HEAP32[$0$s2 + 6] = $T_0;
    HEAP32[$0$s2 + 3] = $0;
    HEAP32[$0$s2 + 2] = $0;
    return;
  }
  var $fd145 = $T_0 + 8 | 0;
  var $30 = HEAP32[$fd145 >> 2];
  var $32 = HEAP32[1310881];
  if ($T_0 >>> 0 < $32 >>> 0) {
    _abort();
  }
  if ($30 >>> 0 < $32 >>> 0) {
    _abort();
  }
  HEAP32[$30 + 12 >> 2] = $22;
  HEAP32[$fd145 >> 2] = $22;
  HEAP32[$0$s2 + 2] = $30;
  HEAP32[$0$s2 + 3] = $T_0;
  HEAP32[$0$s2 + 6] = 0;
  return;
}
_add_segment["X"] = 1;
function __Znwj($size) {
  var label = 0;
  var $_size = ($size | 0) == 0 ? 1 : $size;
  while (1) {
    var $call = _malloc($_size);
    if (($call | 0) != 0) {
      label = 2125;
      break;
    }
    var $call2 = __ZSt15get_new_handlerv();
    if (($call2 | 0) == 0) {
      break;
    }
    FUNCTION_TABLE[$call2]();
  }
  if (label == 2125) {
    return $call;
  }
  var $exception = ___cxa_allocate_exception(4);
  __ZNSt9bad_allocC1Ev($exception);
  ___cxa_throw($exception, 5244660, 42);
}





// Warning: printing of i64 values may be slightly rounded! No deep i64 math used, so precise i64 code not included
var i64Math = null;

// === Auto-generated postamble setup entry stuff ===

Module.callMain = function callMain(args) {
  var argc = args.length+1;
  function pad() {
    for (var i = 0; i < 4-1; i++) {
      argv.push(0);
    }
  }
  var argv = [allocate(intArrayFromString("/bin/this.program"), 'i8', ALLOC_STATIC) ];
  pad();
  for (var i = 0; i < argc-1; i = i + 1) {
    argv.push(allocate(intArrayFromString(args[i]), 'i8', ALLOC_STATIC));
    pad();
  }
  argv.push(0);
  argv = allocate(argv, 'i32', ALLOC_STATIC);

  return Module['_main'](argc, argv, 0);
}




function run(args) {
  args = args || Module['arguments'];

  if (runDependencies > 0) {
    Module.printErr('run() called, but dependencies remain, so not running');
    return 0;
  }

  if (Module['preRun']) {
    if (typeof Module['preRun'] == 'function') Module['preRun'] = [Module['preRun']];
    var toRun = Module['preRun'];
    Module['preRun'] = [];
    for (var i = toRun.length-1; i >= 0; i--) {
      toRun[i]();
    }
    if (runDependencies > 0) {
      // a preRun added a dependency, run will be called later
      return 0;
    }
  }

  function doRun() {
    var ret = 0;
    calledRun = true;
    if (Module['_main']) {
      preMain();
      ret = Module.callMain(args);
      if (!Module['noExitRuntime']) {
        exitRuntime();
      }
    }
    if (Module['postRun']) {
      if (typeof Module['postRun'] == 'function') Module['postRun'] = [Module['postRun']];
      while (Module['postRun'].length > 0) {
        Module['postRun'].pop()();
      }
    }
    return ret;
  }

  if (Module['setStatus']) {
    Module['setStatus']('Running...');
    setTimeout(function() {
      setTimeout(function() {
        Module['setStatus']('');
      }, 1);
      doRun();
    }, 1);
    return 0;
  } else {
    return doRun();
  }
}
Module['run'] = Module.run = run;

// {{PRE_RUN_ADDITIONS}}

if (Module['preInit']) {
  if (typeof Module['preInit'] == 'function') Module['preInit'] = [Module['preInit']];
  while (Module['preInit'].length > 0) {
    Module['preInit'].pop()();
  }
}

initRuntime();

var shouldRunNow = true;
if (Module['noInitialRun']) {
  shouldRunNow = false;
}

if (shouldRunNow) {
  var ret = run();
}

// {{POST_RUN_ADDITIONS}}






  // {{MODULE_ADDITIONS}}


// EMSCRIPTEN_GENERATED_FUNCTIONS: ["_sys_trim","__ZNSt3__14pairIP5BlockNS_3setIS2_NS_4lessIS2_EENS_9allocatorIS2_EEEEED2Ev","__ZNSt3__13mapIP5BlockNS_3setIS2_NS_4lessIS2_EENS_9allocatorIS2_EEEES5_NS6_INS_4pairIKS2_S8_EEEEE16__find_equal_keyERPNS_16__tree_node_baseIPvEERSA_","__ZN9LoopShapeD1Ev","___cxx_global_var_init","__ZN13MultipleShape6RenderEb","__ZNK10__cxxabiv117__class_type_info16search_above_dstEPNS_19__dynamic_cast_infoEPKvS4_ib","__ZNSt3__114__split_bufferIPP5BlockRNS_9allocatorIS3_EEEC2EjjS6_","__ZNSt3__112__deque_baseIP5BlockNS_9allocatorIS2_EEED2Ev","__ZNSt3__16__treeINS_4pairIP5BlockS3_EENS_19__map_value_compareIS3_S3_NS_4lessIS3_EELb1EEENS_9allocatorIS4_EEED2Ev","__ZNSt9bad_allocC2Ev","__ZN13MultipleShapeC1Ev","_rl_make_output_buffer","__ZNSt3__13mapIP5BlockS2_NS_4lessIS2_EENS_9allocatorINS_4pairIKS2_S2_EEEEEixERS7_","__ZN12LabeledShapeD0Ev","__ZN8RelooperD2Ev","__ZN16RelooperRecursorC2EP8Relooper","__ZNK10__cxxabiv117__class_type_info9can_catchEPKNS_16__shim_type_infoERPv","__ZNSt3__114__split_bufferIPP5ShapeRNS_9allocatorIS3_EEEC1EjjS6_","__ZNSt3__114__split_bufferIPP5BlockRNS_9allocatorIS3_EEED1Ev","__ZNSt3__16__treeINS_4pairIP5BlockP6BranchEENS_19__map_value_compareIS3_S5_NS_4lessIS3_EELb1EEENS_9allocatorIS6_EEE5eraseENS_21__tree_const_iteratorIS6_PKNS_11__tree_nodeIS6_PvEEiEE","_rl_relooper_add_block","__ZNSt3__16__treeIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEE13__lower_boundIS2_EENS_15__tree_iteratorIS2_PNS_11__tree_nodeIS2_PvEEiEERKT_SD_SD_","__ZNKSt9bad_alloc4whatEv","__ZNSt3__111__tree_nextIPNS_16__tree_node_baseIPvEEEET_S5_","__ZN5ShapeD0Ev","_init_mparams","__ZNSt3__16__treeIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEE12__find_equalIS2_EERPNS_16__tree_node_baseIPvEENS_21__tree_const_iteratorIS2_PKNS_11__tree_nodeIS2_SA_EEiEESD_RKT_","__ZN10ministringD2Ev","_memcpy","__ZNSt3__15dequeIP5ShapeNS_9allocatorIS2_EEE19__add_back_capacityEv","__ZNK10__cxxabiv117__class_type_info24process_found_base_classEPNS_19__dynamic_cast_infoEPvi","_rl_delete_relooper","__ZNSt3__13setIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEED2Ev","__ZNK10__cxxabiv120__si_class_type_info16search_above_dstEPNS_19__dynamic_cast_infoEPKvS4_ib","__ZN13MultipleShapeD2Ev","__ZNSt3__16__treeINS_4pairIP5BlockNS_3setIS3_NS_4lessIS3_EENS_9allocatorIS3_EEEEEENS_19__map_value_compareIS3_S9_S6_Lb1EEENS7_ISA_EEE4findIS3_EENS_15__tree_iteratorISA_PNS_11__tree_nodeISA_PvEEiEERKT_","__ZNSt3__16__treeIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEEC2ERKS4_","__ZNSt3__114__split_bufferIPP5ShapeRNS_9allocatorIS3_EEEC2EjjS6_","__ZN5BlockC2EPKc","__ZN8Relooper8AddBlockEP5Block","__ZNSt3__114__split_bufferIPP5BlockNS_9allocatorIS3_EEED1Ev","__ZN5Shape9IsLabeledEPS_","__ZNSt3__16__treeINS_4pairIP5BlockP5ShapeEENS_19__map_value_compareIS3_S5_NS_4lessIS3_EELb1EEENS_9allocatorIS6_EEE4findIS3_EENS_15__tree_iteratorIS6_PNS_11__tree_nodeIS6_PvEEiEERKT_","__ZN10__cxxabiv116__shim_type_infoD2Ev","__ZNSt3__16__treeINS_4pairIP5BlockP5ShapeEENS_19__map_value_compareIS3_S5_NS_4lessIS3_EELb1EEENS_9allocatorIS6_EEEC1ERKSA_","_release_unused_segments","__ZNSt3__14listIP5BlockNS_9allocatorIS2_EEED1Ev","__ZN10ministringpLEPKc","__ZNSt3__15dequeIP5ShapeNS_9allocatorIS2_EEED1Ev","__ZNSt3__16__treeINS_4pairIPviEENS_19__map_value_compareIS2_iNS_4lessIS2_EELb1EEENS_9allocatorIS3_EEED1Ev","__ZNSt3__15dequeIP5ShapeNS_9allocatorIS2_EEE9push_backERKS2_","__ZNSt3__112__deque_baseIP5BlockNS_9allocatorIS2_EEE5clearEv","__ZN5Block11AddBranchToEPS_PKcS2_","__ZN5BlockD2Ev","__ZN13MultipleShape16RenderLoopPrefixEv","__ZL13PrintIndentedPKcz","__ZNSt9bad_allocD0Ev","__ZZZN8Relooper9CalculateEP5BlockEN8Analyzer21FindIndependentGroupsE_0RNSt3__13setIS1_NS3_4lessIS1_EENS3_9allocatorIS1_EEEESA_RNS3_3mapIS1_S9_S6_NS7_INS3_4pairIKS1_S9_EEEEEEEN11HelperClassD1Ev","__ZNSt3__16__treeIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEEC1ERKS4_","__ZNSt3__114__split_bufferIPP5ShapeRNS_9allocatorIS3_EEE10push_frontERKS3_","__ZN6Branch6RenderEP5Blockb","__ZNSt3__16__treeIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEED1Ev","__ZNSt3__13mapIP5BlockP5ShapeNS_4lessIS2_EENS_9allocatorINS_4pairIKS2_S4_EEEEED2Ev","_rl_new_relooper","__ZN5Shape10IsMultipleEPS_","__ZNK10__cxxabiv116__shim_type_info5noop2Ev","__ZNSt3__16__treeINS_4pairIP5BlockNS_3setIS3_NS_4lessIS3_EENS_9allocatorIS3_EEEEEENS_19__map_value_compareIS3_S9_S6_Lb1EEENS7_ISA_EEEC1ERKSC_","__ZNSt3__16__treeINS_4pairIP5BlockNS_3setIS3_NS_4lessIS3_EENS_9allocatorIS3_EEEEEENS_19__map_value_compareIS3_S9_S6_Lb1EEENS7_ISA_EEE5eraseENS_21__tree_const_iteratorISA_PKNS_11__tree_nodeISA_PvEEiEE","__ZNSt3__111__tree_prevIPKNS_16__tree_node_baseIPvEEEET_S6_","__ZN13MultipleShapeD0Ev","__ZN10__cxxabiv120__si_class_type_infoD2Ev","__ZNSt3__16__treeIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEED2Ev","_rl_new_block","__ZNSt3__13mapIP5BlockP6BranchNS_4lessIS2_EENS_9allocatorINS_4pairIKS2_S4_EEEEED2Ev","__ZZN8Relooper9CalculateEP5BlockEN12PreOptimizer13SplitDeadEndsEv","__ZZN8Relooper9CalculateEP5BlockEN8Analyzer6NoticeE_0P5Shape","__ZZN8Relooper9CalculateEP5BlockEN8Analyzer9SolipsizeE_0S1_N6Branch8FlowTypeEP5ShapeRNSt3__13setIS1_NS7_4lessIS1_EENS7_9allocatorIS1_EEEE","__ZN10__cxxabiv117__class_type_infoD2Ev","__ZN8Relooper6RenderEv","__ZZZN8Relooper9CalculateEP5BlockEN8Analyzer21FindIndependentGroupsE_0RNSt3__13setIS1_NS3_4lessIS1_EENS3_9allocatorIS1_EEEESA_RNS3_3mapIS1_S9_S6_NS7_INS3_4pairIKS1_S9_EEEEEEEN11HelperClass22InvalidateWithChildrenES1_","_rl_relooper_calculate","_rl_set_output_buffer","__ZNSt3__16__treeINS_4pairIP5BlockP6BranchEENS_19__map_value_compareIS3_S5_NS_4lessIS3_EELb1EEENS_9allocatorIS6_EEEC1ERKSA_","_tmalloc_small","__ZN12LabeledShapeC2EN5Shape9ShapeTypeE","__ZNSt3__13mapIP5BlockS2_NS_4lessIS2_EENS_9allocatorINS_4pairIKS2_S2_EEEEE16__find_equal_keyERPNS_16__tree_node_baseIPvEERS7_","__ZN5Shape8IsSimpleEPS_","__ZZN8Relooper9CalculateEP5BlockEN12PreOptimizerC2EPS_","__ZNK10__cxxabiv117__class_type_info27has_unambiguous_public_baseEPNS_19__dynamic_cast_infoEPvi","__ZSt15get_new_handlerv","__ZNK10__cxxabiv120__si_class_type_info16search_below_dstEPNS_19__dynamic_cast_infoEPKvib","_realloc","__ZNSt3__15dequeIP5BlockNS_9allocatorIS2_EEED1Ev","_tmalloc_large","__ZZN8Relooper9CalculateEP5BlockEN13PostOptimizerC2E_1PS_","__ZNSt9bad_allocD1Ev","__ZN13MultipleShapeC2Ev","__ZNSt3__16__treeINS_4pairIP5BlockP5ShapeEENS_19__map_value_compareIS3_S5_NS_4lessIS3_EELb1EEENS_9allocatorIS6_EEE16__insert_node_atEPNS_16__tree_node_baseIPvEERSH_SH_","__ZN8RelooperC1Ev","__ZNSt3__112__deque_baseIP5ShapeNS_9allocatorIS2_EEED2Ev","_rl_set_asm_js_mode","__ZNSt3__16__treeIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEE14__erase_uniqueIS2_EEjRKT_","__ZNSt3__15dequeIP5BlockNS_9allocatorIS2_EEED2Ev","__ZNSt3__113__tree_removeIPNS_16__tree_node_baseIPvEEEEvT_S5_","__ZZN8Relooper9CalculateEP5BlockEN8Analyzer7ProcessE_0RNSt3__13setIS1_NS3_4lessIS1_EENS3_9allocatorIS1_EEEESA_P5Shape","__ZNSt3__114__split_bufferIPP5BlockNS_9allocatorIS3_EEED2Ev","__ZN5BlockD1Ev","__ZNSt3__112__deque_baseIP5ShapeNS_9allocatorIS2_EEE3endEv","__ZNSt3__114__split_bufferIPP5BlockRNS_9allocatorIS3_EEED2Ev","_init_top","__ZZN8Relooper9CalculateEP5BlockEN13PostOptimizerC1E_1PS_","__ZNSt3__16__treeIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEE7destroyEPNS_11__tree_nodeIS2_PvEE","__ZN8Relooper12SetAsmJSModeEi","__ZNSt3__114__split_bufferIPP5ShapeRNS_9allocatorIS3_EEED1Ev","__ZNSt3__16__treeINS_4pairIP5BlockS3_EENS_19__map_value_compareIS3_S3_NS_4lessIS3_EELb1EEENS_9allocatorIS4_EEE4findIS3_EENS_15__tree_iteratorIS4_PNS_11__tree_nodeIS4_PvEEiEERKT_","__ZN10__cxxabiv117__class_type_infoD0Ev","__ZNSt3__114__split_bufferIPP5BlockRNS_9allocatorIS3_EEEC1EjjS6_","__ZNSt3__13mapIP5BlockP5ShapeNS_4lessIS2_EENS_9allocatorINS_4pairIKS2_S4_EEEEEixERS9_","__ZNSt3__114__split_bufferIPP5BlockRNS_9allocatorIS3_EEE18__construct_at_endINS_13move_iteratorIPS3_EEEENS_9enable_ifIXsr21__is_forward_iteratorIT_EE5valueEvE4typeESD_SD_","__ZNSt3__13mapIPviNS_4lessIS1_EENS_9allocatorINS_4pairIKS1_iEEEEED2Ev","__ZN10__cxxabiv120__si_class_type_infoD1Ev","__ZNSt3__16__treeINS_4pairIP5BlockNS_3setIS3_NS_4lessIS3_EENS_9allocatorIS3_EEEEEENS_19__map_value_compareIS3_S9_S6_Lb1EEENS7_ISA_EEE7destroyEPNS_11__tree_nodeISA_PvEE","__Znwj","__ZN6BranchC1EPKcS1_","__GLOBAL__I_a","__ZdlPv","__ZNSt3__16__treeINS_4pairIP5BlockS3_EENS_19__map_value_compareIS3_S3_NS_4lessIS3_EELb1EEENS_9allocatorIS4_EEE13__lower_boundIS3_EENS_15__tree_iteratorIS4_PNS_11__tree_nodeIS4_PvEEiEERKT_SH_SH_","__ZNSt3__16__treeINS_4pairIPviEENS_19__map_value_compareIS2_iNS_4lessIS2_EELb1EEENS_9allocatorIS3_EEE7destroyEPNS_11__tree_nodeIS3_S2_EE","_free","__ZNK10__cxxabiv117__class_type_info16search_below_dstEPNS_19__dynamic_cast_infoEPKvib","__ZNSt3__15dequeIP5ShapeNS_9allocatorIS2_EEED2Ev","__ZN5Block6RenderEb","__ZN13MultipleShapeD1Ev","__ZNSt3__16__treeIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEE15__insert_uniqueERKS2_","__ZZN8Relooper9CalculateEP5BlockEN12PreOptimizerD1Ev","__ZNSt3__16__treeINS_4pairIP5BlockNS_3setIS3_NS_4lessIS3_EENS_9allocatorIS3_EEEEEENS_19__map_value_compareIS3_S9_S6_Lb1EEENS7_ISA_EEED1Ev","__ZN9LoopShapeC1Ev","__ZNSt3__13mapIP5BlockS2_NS_4lessIS2_EENS_9allocatorINS_4pairIKS2_S2_EEEEED2Ev","__ZN10__cxxabiv117__class_type_infoD1Ev","__ZL11PutIndentedPKc","__ZNSt3__16__treeIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEE12__find_equalIS2_EERPNS_16__tree_node_baseIPvEESD_RKT_","__ZNSt3__114__split_bufferIPP5ShapeNS_9allocatorIS3_EEE10push_frontERKS3_","__ZNSt3__16__treeINS_4pairIP5BlockP5ShapeEENS_19__map_value_compareIS3_S5_NS_4lessIS3_EELb1EEENS_9allocatorIS6_EEE7destroyEPNS_11__tree_nodeIS6_PvEE","__ZN11SimpleShapeC2Ev","__ZZN8Relooper9CalculateEP5BlockEN8Analyzer10MakeSimpleE_0RNSt3__13setIS1_NS3_4lessIS1_EENS3_9allocatorIS1_EEEES1_SA_","_mmap_resize","__ZNSt3__114__split_bufferIPP5ShapeNS_9allocatorIS3_EEED1Ev","__ZNSt3__16__treeINS_4pairIP5BlockNS_3setIS3_NS_4lessIS3_EENS_9allocatorIS3_EEEEEENS_19__map_value_compareIS3_S9_S6_Lb1EEENS7_ISA_EEE13__lower_boundIS3_EENS_15__tree_iteratorISA_PNS_11__tree_nodeISA_PvEEiEERKT_SK_SK_","__ZNSt3__16__treeINS_4pairIP5BlockP6BranchEENS_19__map_value_compareIS3_S5_NS_4lessIS3_EELb1EEENS_9allocatorIS6_EEE7destroyEPNS_11__tree_nodeIS6_PvEE","__ZNSt3__13mapIP5BlockNS_3setIS2_NS_4lessIS2_EENS_9allocatorIS2_EEEES5_NS6_INS_4pairIKS2_S8_EEEEED1Ev","__ZNSt3__13mapIP5BlockP6BranchNS_4lessIS2_EENS_9allocatorINS_4pairIKS2_S4_EEEEEixERS9_","__ZNSt3__15dequeIP5BlockNS_9allocatorIS2_EEE19__add_back_capacityEv","__ZNSt3__112__deque_baseIP5ShapeNS_9allocatorIS2_EEE5clearEv","_prepend_alloc","__ZNSt3__119__tree_right_rotateIPNS_16__tree_node_baseIPvEEEEvT_","__ZNSt3__13mapIP5BlockP5ShapeNS_4lessIS2_EENS_9allocatorINS_4pairIKS2_S4_EEEEE16__find_equal_keyERPNS_16__tree_node_baseIPvEERS9_","__ZZN8Relooper9CalculateEP5BlockEN12PreOptimizerD2Ev","__ZZN8Relooper9CalculateEP5BlockEN8AnalyzerC1E_0PS_","_init_bins","__ZN10__cxxabiv18is_equalEPKSt9type_infoS2_b","__ZNK10__cxxabiv117__class_type_info29process_static_type_below_dstEPNS_19__dynamic_cast_infoEPKvi","__ZZN8Relooper9CalculateEP5BlockEN8AnalyzerC2E_0PS_","___dynamic_cast","__ZNSt3__16__treeINS_4pairIP5BlockS3_EENS_19__map_value_compareIS3_S3_NS_4lessIS3_EELb1EEENS_9allocatorIS4_EEED1Ev","__ZN13MultipleShape17RenderLoopPostfixEv","__ZZN8Relooper9CalculateEP5BlockEN13PostOptimizer7ProcessE_1P5Shape","__ZZZN8Relooper9CalculateEP5BlockEN8Analyzer21FindIndependentGroupsE_0RNSt3__13setIS1_NS3_4lessIS1_EENS3_9allocatorIS1_EEEESA_RNS3_3mapIS1_S9_S6_NS7_INS3_4pairIKS1_S9_EEEEEEEN11HelperClassC1ESH_","__ZN9LoopShape6RenderEb","__ZNSt3__16__treeIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEEC2ERKS7_","__ZNSt3__16__treeINS_4pairIP5BlockP6BranchEENS_19__map_value_compareIS3_S5_NS_4lessIS3_EELb1EEENS_9allocatorIS6_EEEC2ERKSA_","__ZNSt3__16__treeIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEE4findIS2_EENS_15__tree_iteratorIS2_PNS_11__tree_nodeIS2_PvEEiEERKT_","__ZN6BranchD2Ev","__ZNK10__cxxabiv120__si_class_type_info27has_unambiguous_public_baseEPNS_19__dynamic_cast_infoEPvi","__ZNSt9bad_allocC1Ev","__ZNSt3__16__treeINS_4pairIP5BlockP6BranchEENS_19__map_value_compareIS3_S5_NS_4lessIS3_EELb1EEENS_9allocatorIS6_EEED2Ev","__ZNSt3__16__treeINS_4pairIP5BlockP5ShapeEENS_19__map_value_compareIS3_S5_NS_4lessIS3_EELb1EEENS_9allocatorIS6_EEED2Ev","__ZNSt3__16__treeINS_4pairIP5BlockP6BranchEENS_19__map_value_compareIS3_S5_NS_4lessIS3_EELb1EEENS_9allocatorIS6_EEE16__insert_node_atEPNS_16__tree_node_baseIPvEERSH_SH_","__ZNSt3__16__treeIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEE5clearEv","__ZNSt3__13mapIP5BlockS2_NS_4lessIS2_EENS_9allocatorINS_4pairIKS2_S2_EEEEE16__construct_nodeERS7_","__ZN11SimpleShapeD0Ev","__ZNSt3__16__treeIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEE15__insert_uniqueENS_21__tree_const_iteratorIS2_PKNS_11__tree_nodeIS2_PvEEiEERKS2_","__ZNSt3__16__treeINS_4pairIP5BlockNS_3setIS3_NS_4lessIS3_EENS_9allocatorIS3_EEEEEENS_19__map_value_compareIS3_S9_S6_Lb1EEENS7_ISA_EEED2Ev","__ZNSt3__127__tree_balance_after_insertIPNS_16__tree_node_baseIPvEEEEvT_S5_","__ZN9LoopShapeC2Ev","__ZNSt3__16__treeINS_4pairIP5BlockP5ShapeEENS_19__map_value_compareIS3_S5_NS_4lessIS3_EELb1EEENS_9allocatorIS6_EEE13__lower_boundIS3_EENS_15__tree_iteratorIS6_PNS_11__tree_nodeIS6_PvEEiEERKT_SJ_SJ_","__ZNSt3__13setIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEED1Ev","__ZNSt3__13mapIP5BlockP6BranchNS_4lessIS2_EENS_9allocatorINS_4pairIKS2_S4_EEEEE16__find_equal_keyERPNS_16__tree_node_baseIPvEERS9_","__ZN11SimpleShape6RenderEb","__ZNSt3__112__deque_baseIP5ShapeNS_9allocatorIS2_EEE5beginEv","__ZN8RelooperC2Ev","__ZZN8Relooper9CalculateEP5BlockEN12PreOptimizerC1EPS_","__ZN8Relooper15SetOutputBufferEPci","__ZZN8Relooper9CalculateEP5BlockEN8Analyzer12GetBlocksOutE_0S1_RNSt3__13setIS1_NS3_4lessIS1_EENS3_9allocatorIS1_EEEEPS9_","__ZN5BlockC1EPKc","__ZZN8Relooper9CalculateEP5BlockEN13PostOptimizer16FindLabeledLoopsE_1P5Shape","_rl_relooper_render","__ZNSt3__114__split_bufferIPP5ShapeRNS_9allocatorIS3_EEE18__construct_at_endINS_13move_iteratorIPS3_EEEENS_9enable_ifIXsr21__is_forward_iteratorIT_EE5valueEvE4typeESD_SD_","__ZNSt3__114__split_bufferIPP5BlockNS_9allocatorIS3_EEE10push_frontERKS3_","__ZN12LabeledShapeD1Ev","_malloc","__ZNSt3__14pairIP5BlockNS_3setIS2_NS_4lessIS2_EENS_9allocatorIS2_EEEEED1Ev","__ZNSt3__16__treeINS_4pairIP5BlockP6BranchEENS_19__map_value_compareIS3_S5_NS_4lessIS3_EELb1EEENS_9allocatorIS6_EEE4findIS3_EENS_15__tree_iteratorIS6_PNS_11__tree_nodeIS6_PvEEiEERKT_","__ZZN8Relooper9CalculateEP5BlockEN8Analyzer8MakeLoopE_0RNSt3__13setIS1_NS3_4lessIS1_EENS3_9allocatorIS1_EEEESA_SA_","__ZN8RelooperD1Ev","__ZNSt3__15stackIP5ShapeNS_5dequeIS2_NS_9allocatorIS2_EEEEED1Ev","__ZNSt3__14listIP5BlockNS_9allocatorIS2_EEE9push_backERKS2_","__ZNSt3__13mapIPviNS_4lessIS1_EENS_9allocatorINS_4pairIKS1_iEEEEED1Ev","__ZN5ShapeD1Ev","__ZN6BranchC2EPKcS1_","__ZNSt3__114__split_bufferIPP5ShapeRNS_9allocatorIS3_EEED2Ev","__ZZN8Relooper9CalculateEP5BlockEN8Analyzer21FindIndependentGroupsE_0RNSt3__13setIS1_NS3_4lessIS1_EENS3_9allocatorIS1_EEEESA_RNS3_3mapIS1_S9_S6_NS7_INS3_4pairIKS1_S9_EEEEEE","__ZNSt3__13mapIP5BlockP5ShapeNS_4lessIS2_EENS_9allocatorINS_4pairIKS2_S4_EEEEE16__construct_nodeERS9_","__ZN5Shape6IsLoopEPS_","__ZNSt3__13mapIP5BlockNS_3setIS2_NS_4lessIS2_EENS_9allocatorIS2_EEEES5_NS6_INS_4pairIKS2_S8_EEEEEixERSA_","__ZN8Relooper9CalculateEP5Block","__ZNSt3__112__deque_baseIP5BlockNS_9allocatorIS2_EEE5beginEv","__ZZN8Relooper9CalculateEP5BlockEN13PostOptimizer19RemoveUnneededFlowsE_1P5ShapeS4_","__ZN11SimpleShapeC1Ev","__ZNSt3__111__tree_nextIPKNS_16__tree_node_baseIPvEEEET_S6_","__ZNSt3__16__treeINS_4pairIP5BlockP6BranchEENS_19__map_value_compareIS3_S5_NS_4lessIS3_EELb1EEENS_9allocatorIS6_EEE13__lower_boundIS3_EENS_15__tree_iteratorIS6_PNS_11__tree_nodeIS6_PvEEiEERKT_SJ_SJ_","__ZN10ministring4sizeEv","__ZNSt3__13mapIP5BlockP5ShapeNS_4lessIS2_EENS_9allocatorINS_4pairIKS2_S4_EEEEED1Ev","__ZNSt3__112__deque_baseIP5BlockNS_9allocatorIS2_EEE3endEv","__ZNSt3__16__treeIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEEC1ERKS7_","__ZNSt3__13mapIP5BlockS2_NS_4lessIS2_EENS_9allocatorINS_4pairIKS2_S2_EEEEED1Ev","__ZZZN8Relooper9CalculateEP5BlockEN8Analyzer21FindIndependentGroupsE_0RNSt3__13setIS1_NS3_4lessIS1_EENS3_9allocatorIS1_EEEESA_RNS3_3mapIS1_S9_S6_NS7_INS3_4pairIKS1_S9_EEEEEEEN11HelperClassC2ESH_","__ZNSt3__13mapIP5BlockNS_3setIS2_NS_4lessIS2_EENS_9allocatorIS2_EEEES5_NS6_INS_4pairIKS2_S8_EEEEE16__construct_nodeERSA_","__ZNSt3__16__treeINS_4pairIP5BlockP5ShapeEENS_19__map_value_compareIS3_S5_NS_4lessIS3_EELb1EEENS_9allocatorIS6_EEEC2ERKSA_","__ZNSt3__16__treeIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEE16__insert_node_atEPNS_16__tree_node_baseIPvEERSB_SB_","__ZNSt3__15dequeIP5BlockNS_9allocatorIS2_EEE9push_backERKS2_","__ZNSt3__16__treeINS_4pairIPviEENS_19__map_value_compareIS2_iNS_4lessIS2_EELb1EEENS_9allocatorIS3_EEEC2ERKS7_","_rl_block_add_branch_to","__ZNK10__cxxabiv117__class_type_info29process_static_type_above_dstEPNS_19__dynamic_cast_infoEPKvS4_i","__ZNSt3__16__treeINS_4pairIP5BlockP5ShapeEENS_19__map_value_compareIS3_S5_NS_4lessIS3_EELb1EEENS_9allocatorIS6_EEED1Ev","__ZZN8Relooper9CalculateEP5BlockEN8Analyzer12MakeMultipleE_0RNSt3__13setIS1_NS3_4lessIS1_EENS3_9allocatorIS1_EEEESA_RNS3_3mapIS1_S9_S6_NS7_INS3_4pairIKS1_S9_EEEEEEP5ShapeSA_","__ZNSt3__15dequeIP5ShapeNS_9allocatorIS2_EEE8pop_backEv","__ZNSt3__110__list_impIP5BlockNS_9allocatorIS2_EEED2Ev","_internal_realloc","__ZNSt3__16__treeINS_4pairIPviEENS_19__map_value_compareIS2_iNS_4lessIS2_EELb1EEENS_9allocatorIS3_EEEC1ERKS7_","__ZNSt3__14listIP5BlockNS_9allocatorIS2_EEED2Ev","__ZN10ministring5c_strEv","__ZN9LoopShapeD0Ev","__ZNSt3__16__treeINS_4pairIP5BlockNS_3setIS3_NS_4lessIS3_EENS_9allocatorIS3_EEEEEENS_19__map_value_compareIS3_S9_S6_Lb1EEENS7_ISA_EEE14__erase_uniqueIS3_EEjRKT_","__ZNSt3__118__tree_left_rotateIPNS_16__tree_node_baseIPvEEEEvT_","__ZNSt3__14listIP5BlockNS_9allocatorIS2_EEE9pop_frontEv","__ZN8Indenter8UnindentEv","__ZN8Indenter6IndentEv","_sys_alloc","__ZNSt3__110__list_impIP5BlockNS_9allocatorIS2_EEE5clearEv","__ZN11SimpleShapeD1Ev","__ZNSt3__16__treeINS_4pairIP5BlockS3_EENS_19__map_value_compareIS3_S3_NS_4lessIS3_EELb1EEENS_9allocatorIS4_EEEC1ERKS8_","_segment_holding","__ZNSt3__16__treeINS_4pairIP5BlockS3_EENS_19__map_value_compareIS3_S3_NS_4lessIS3_EELb1EEENS_9allocatorIS4_EEEC2ERKS8_","__ZN10ministringC2Ev","__ZNK10__cxxabiv116__shim_type_info5noop1Ev","__ZNSt3__13mapIP5BlockNS_3setIS2_NS_4lessIS2_EENS_9allocatorIS2_EEEES5_NS6_INS_4pairIKS2_S8_EEEEED2Ev","__ZNSt3__16__treeINS_4pairIP5BlockS3_EENS_19__map_value_compareIS3_S3_NS_4lessIS3_EELb1EEENS_9allocatorIS4_EEE7destroyEPNS_11__tree_nodeIS4_PvEE","__ZNSt9bad_allocD2Ev","__ZNSt3__16__treeINS_4pairIPviEENS_19__map_value_compareIS2_iNS_4lessIS2_EELb1EEENS_9allocatorIS3_EEED2Ev","__ZNSt3__16__treeINS_4pairIP5BlockP6BranchEENS_19__map_value_compareIS3_S5_NS_4lessIS3_EELb1EEENS_9allocatorIS6_EEE14__erase_uniqueIS3_EEjRKT_","__ZNSt3__16__treeINS_4pairIP5BlockNS_3setIS3_NS_4lessIS3_EENS_9allocatorIS3_EEEEEENS_19__map_value_compareIS3_S9_S6_Lb1EEENS7_ISA_EEE16__insert_node_atEPNS_16__tree_node_baseIPvEERSI_SI_","_add_segment","__ZNSt3__114__split_bufferIPP5BlockRNS_9allocatorIS3_EEE10push_frontERKS3_","__ZN10ministringC1Ev","__ZNSt3__114__split_bufferIPP5ShapeNS_9allocatorIS3_EEED2Ev","__ZNSt3__16__treeIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEE16__construct_nodeERKS2_","__ZN6BranchD1Ev","__ZN5ShapeC2ENS_9ShapeTypeE","__ZNSt3__16__treeIP5BlockNS_4lessIS2_EENS_9allocatorIS2_EEE5eraseENS_21__tree_const_iteratorIS2_PKNS_11__tree_nodeIS2_PvEEiEE","__ZNSt3__16__treeINS_4pairIP5BlockP6BranchEENS_19__map_value_compareIS3_S5_NS_4lessIS3_EELb1EEENS_9allocatorIS6_EEED1Ev","__ZN10__cxxabiv120__si_class_type_infoD0Ev","__ZZZN8Relooper9CalculateEP5BlockEN8Analyzer21FindIndependentGroupsE_0RNSt3__13setIS1_NS3_4lessIS1_EENS3_9allocatorIS1_EEEESA_RNS3_3mapIS1_S9_S6_NS7_INS3_4pairIKS1_S9_EEEEEEEN11HelperClassD2Ev","__ZNSt3__15stackIP5ShapeNS_5dequeIS2_NS_9allocatorIS2_EEEEED2Ev","__ZNSt3__13mapIP5BlockP6BranchNS_4lessIS2_EENS_9allocatorINS_4pairIKS2_S4_EEEEED1Ev","__ZNSt3__13mapIP5BlockP6BranchNS_4lessIS2_EENS_9allocatorINS_4pairIKS2_S4_EEEEE16__construct_nodeERS9_","_rl_delete_block","__ZNSt3__16__treeINS_4pairIP5BlockS3_EENS_19__map_value_compareIS3_S3_NS_4lessIS3_EELb1EEENS_9allocatorIS4_EEE16__insert_node_atEPNS_16__tree_node_baseIPvEERSF_SF_","__ZNSt3__16__treeINS_4pairIP5BlockNS_3setIS3_NS_4lessIS3_EENS_9allocatorIS3_EEEEEENS_19__map_value_compareIS3_S9_S6_Lb1EEENS7_ISA_EEEC2ERKSC_","__ZZN8Relooper9CalculateEP5BlockEN12PreOptimizer8FindLiveES1_","__ZN10ministringD1Ev"]



  var RBUFFER_SIZE = 20*1024*1024;
  var rbuffer = _malloc(RBUFFER_SIZE);
  _rl_set_output_buffer(rbuffer, RBUFFER_SIZE);

  var TBUFFER_SIZE = 10*1024*1024;
  var tbuffer = _malloc(TBUFFER_SIZE);

  var RelooperGlue = {};
  RelooperGlue['init'] = function() {
    this.r = _rl_new_relooper();
  },
  RelooperGlue['addBlock'] = function(text) {
    assert(this.r);
    assert(text.length+1 < TBUFFER_SIZE);
    writeStringToMemory(text, tbuffer);
    var b = _rl_new_block(tbuffer);
    _rl_relooper_add_block(this.r, b);
    return b;
  };
  RelooperGlue['addBranch'] = function(from, to, condition, code) {
    assert(this.r);
    if (condition) {
      assert(condition.length+1 < TBUFFER_SIZE/2);
      writeStringToMemory(condition, tbuffer);
      condition = tbuffer;
    } else {
      condition = 0; // allow undefined, null, etc. as inputs
    }
    if (code) {
      assert(code.length+1 < TBUFFER_SIZE/2);
      writeStringToMemory(code, tbuffer + TBUFFER_SIZE/2);
      code = tbuffer + TBUFFER_SIZE/2;
    } else {
      code = 0; // allow undefined, null, etc. as inputs
    }
    _rl_block_add_branch_to(from, to, condition, code);
  };
  RelooperGlue['render'] = function(entry) {
    assert(this.r);
    assert(entry);
    _rl_relooper_calculate(this.r, entry);
    _rl_relooper_render(this.r);
    var ret = Pointer_stringify(rbuffer);
    _rl_delete_relooper(this.r);
    this.r = 0;
    return ret;
  };
  RelooperGlue['setDebug'] = function(debug) {
    _rl_set_debugging(+!!debug);
  };
  RelooperGlue['setAsmJSMode'] = function(on) {
    _rl_set_asm_js_mode(on);
  };

  Module['Relooper'] = RelooperGlue;




  return Module.Relooper;
})();
