//"use strict";

// Implementation details for the 'runtime environment' we generate in
// JavaScript. The Runtime object itself is used both during compilation,
// and is available at runtime (dynamic compilation). The RuntimeGenerator
// helps to create the Runtime object (written so that the Runtime object
// itself is as optimized as possible - no unneeded runtime checks).

var RuntimeGenerator = {
  alloc: function(size, type, init, sep, ignoreAlign) {
    sep = sep || ';';
    var ret = type + 'TOP';
    if (init) {
      ret += sep + '_memset(' + type + 'TOP, 0, ' + size + ')';
    }
    ret += sep + type + 'TOP += ' + size;
    if ({{{ QUANTUM_SIZE }}} > 1 && !ignoreAlign) {
      ret += sep + RuntimeGenerator.alignMemory(type + 'TOP', {{{ QUANTUM_SIZE }}});
    }
    return ret;
  },

  // An allocation that lives as long as the current function call
  stackAlloc: function(size, sep) {
    sep = sep || ';';
    if (USE_TYPED_ARRAYS === 2) 'STACKTOP += STACKTOP % ' + ({{{ QUANTUM_SIZE }}} - (isNumber(size) ? Math.min(size, {{{ QUANTUM_SIZE }}}) : {{{ QUANTUM_SIZE }}})) + sep;
    //                                                               The stack is always QUANTUM SIZE aligned, so we may not need to force alignment here
    var ret = RuntimeGenerator.alloc(size, 'STACK', INIT_STACK, sep, USE_TYPED_ARRAYS != 2 || (isNumber(size) && parseInt(size) % {{{ QUANTUM_SIZE }}} == 0));
    if (ASSERTIONS) {
      ret += sep + 'assert(STACKTOP < STACK_ROOT + STACK_MAX, "Ran out of stack")';
    }
    return ret;
  },

  stackEnter: function(initial, force) {
    if (initial === 0 && SKIP_STACK_IN_SMALL && !force) return '';
    var ret = 'var __stackBase__  = STACKTOP';
    if (initial > 0) ret += '; STACKTOP += ' + initial;
    if (USE_TYPED_ARRAYS == 2) {
      assert(initial % QUANTUM_SIZE == 0);
      if (ASSERTIONS) {
        ret += '; assert(STACKTOP % {{{ QUANTUM_SIZE }}} == 0, "Stack is unaligned")';
      }
    }
    if (ASSERTIONS) {
      ret += '; assert(STACKTOP < STACK_MAX, "Ran out of stack")';
    }
    if (INIT_STACK) {
      ret += '; _memset(__stackBase__, 0, ' + initial + ')';
    }
    return ret;
  },

  stackExit: function(initial, force) {
    if (initial === 0 && SKIP_STACK_IN_SMALL && !force) return '';
    var ret = '';
    if (SAFE_HEAP) {
      ret += 'for (var i = __stackBase__; i < STACKTOP; i++) SAFE_HEAP_CLEAR(i);';
    }
    return ret += 'STACKTOP = __stackBase__';
  },

  // An allocation that cannot normally be free'd (except through sbrk, which once
  // called, takes control of STATICTOP)
  staticAlloc: function(size) {
    var ret = RuntimeGenerator.alloc(size, 'STATIC', INIT_HEAP);
    if (USE_TYPED_ARRAYS) ret += '; if (STATICTOP >= TOTAL_MEMORY) enlargeMemory();'
    return ret;
  },

  alignMemory: function(target, quantum) {
    if (typeof quantum !== 'number') {
      quantum = '(quantum ? quantum : {{{ QUANTUM_SIZE }}})';
    }
    return target + ' = ' + Runtime.forceAlign(target, quantum);
  },

  // Given two 32-bit unsigned parts of an emulated 64-bit number, combine them into a JS number (double).
  // Rounding is inevitable if the number is large. This is a particular problem for small negative numbers
  // (-1 will be rounded!), so handle negatives separately and carefully
  makeBigInt: function(low, high, unsigned) {
    var unsignedRet = '(' + makeSignOp(low, 'i32', 'un', 1, 1) + '+(' + makeSignOp(high, 'i32', 'un', 1, 1) + '*4294967296))';
    var signedRet = '(' + makeSignOp(low, 'i32', 'un', 1, 1) + '+(' + makeSignOp(high, 'i32', 're', 1, 1) + '*4294967296))';
    if (typeof unsigned === 'string') return '(' + unsigned + ' ? ' + unsignedRet + ' : ' + signedRet + ')';
    return unsigned ? unsignedRet : signedRet;
  }
};

function unInline(name_, params) {
  var src = '(function ' + name_ + '(' + params + ') { var ret = ' + RuntimeGenerator[name_].apply(null, params) + '; return ret; })';
  var ret = eval(src);
  return ret;
}

var Runtime = {
  stackSave: function() {
    return STACKTOP;
  },
  stackRestore: function(stackTop) {
    STACKTOP = stackTop;
  },

  forceAlign: function(target, quantum) {
    quantum = quantum || {{{ QUANTUM_SIZE }}};
    if (quantum == 1) return target;
    if (isNumber(target) && isNumber(quantum)) {
      return Math.ceil(target/quantum)*quantum;
    } else if (isNumber(quantum) && isPowerOfTwo(quantum)) {
      var logg = log2(quantum);
      return '((((' +target + ')+' + (quantum-1) + ')>>' + logg + ')<<' + logg + ')';
    }
    return 'Math.ceil((' + target + ')/' + quantum + ')*' + quantum;
  },

  isNumberType: function(type) {
    return type in Runtime.INT_TYPES || type in Runtime.FLOAT_TYPES;
  },

  isPointerType: isPointerType,
  isStructType: isStructType,

  INT_TYPES: set('i1', 'i8', 'i16', 'i32', 'i64'),
  FLOAT_TYPES: set('float', 'double'),

  // Mirrors processMathop's treatment of constants (which we optimize directly)
  bitshift64: function(low, high, op, bits) {
    var ander = Math.pow(2, bits)-1;
    if (bits < 32) {
      switch (op) {
        case 'shl':
          return [low << bits, (high << bits) | ((low&(ander << (32 - bits))) >>> (32 - bits))];
        case 'ashr':
          return [(((low >>> bits ) | ((high&ander) << (32 - bits))) >> 0) >>> 0, (high >> bits) >>> 0];
        case 'lshr':
          return [((low >>> bits) | ((high&ander) << (32 - bits))) >>> 0, high >>> bits];
      }
    } else if (bits == 32) {
      switch (op) {
        case 'shl':
          return [0, low];
        case 'ashr':
          return [high, (high|0) < 0 ? ander : 0];
        case 'lshr':
          return [high, 0];
      }
    } else { // bits > 32
      switch (op) {
        case 'shl':
          return [0, low << (bits - 32)];
        case 'ashr':
          return [(high >> (bits - 32)) >>> 0, (high|0) < 0 ? ander : 0];
        case 'lshr':
          return [high >>>  (bits - 32) , 0];
      }
    }
    abort('unknown bitshift64 op: ' + [value, op, bits]);
  },

  // Imprecise bitops utilities
  or64: function(x, y) {
    var l = (x | 0) | (y | 0);
    var h = (Math.round(x / 4294967296) | Math.round(y / 4294967296)) * 4294967296;
    return l + h;
  },
  and64: function(x, y) {
    var l = (x | 0) & (y | 0);
    var h = (Math.round(x / 4294967296) & Math.round(y / 4294967296)) * 4294967296;
    return l + h;
  },
  xor64: function(x, y) {
    var l = (x | 0) ^ (y | 0);
    var h = (Math.round(x / 4294967296) ^ Math.round(y / 4294967296)) * 4294967296;
    return l + h;
  },

  //! Returns the size of a type, as C/C++ would have it (in 32-bit, for now), in bytes.
  //! @param type The type, by name.
  getNativeTypeSize: function(type, quantumSize) {
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

  //! Returns the size of a structure field, as C/C++ would have it (in 32-bit,
  //! for now).
  //! @param type The type, by name.
  getNativeFieldSize: function(type) {
    return Math.max(Runtime.getNativeTypeSize(type), Runtime.QUANTUM_SIZE);
  },

  dedup: dedup,

  set: set,

  // Calculate aligned size, just like C structs should be. TODO: Consider
  // requesting that compilation be done with #pragma pack(push) /n #pragma pack(1),
  // which would remove much of the complexity here.
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

  // Given details about a structure, returns its alignment. For example,
  // generateStructInfo(
  //    [
  //      ['i32', 'field1'],
  //      ['i8', 'field2']
  //    ]
  // ) will return
  //    { field1: 0, field2: 4 } (depending on QUANTUM_SIZE)
  //
  // Instead of [type, name], you can also provide just [name]. In that case
  // it will use type information present in LLVM bitcode. (It is safer to
  // specify the type though, as it will then check the type.) You must then
  // also specify the second parameter to generateStructInfo, which is the
  // LLVM structure name.
  //
  // Note that LLVM optimizations can remove some of the debug info generated
  // by -g.
  //
  // Note that you will need the full %struct.* name here at compile time,
  // but not at runtime. The reason is that during compilation we cannot
  // simplify the type names yet. At runtime, you can provide either the short
  // or the full name.
  //
  // When providing a typeName, you can generate information for nested
  // structs, for example, struct = ['field1', { field2: ['sub1', 'sub2', 'sub3'] }, 'field3']
  // which repesents a structure whose 2nd field is another structure.
  generateStructInfo: function(struct, typeName, offset) {
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

  addFunction: function(func) {
    var ret = FUNCTION_TABLE.length;
    FUNCTION_TABLE.push(func);
    FUNCTION_TABLE.push(0);
    return ret;
  },

  warnOnce: function(text) {
    if (!Runtime.warnOnce.shown) Runtime.warnOnce.shown = {};
    if (!Runtime.warnOnce.shown[text]) {
      Runtime.warnOnce.shown[text] = 1;
      Module.printErr(text);
    }
  },

  funcWrappers: {},

  getFuncWrapper: function(func) {
    if (!Runtime.funcWrappers[func]) {
      Runtime.funcWrappers[func] = function() {
        FUNCTION_TABLE[func].apply(null, arguments);
      };
    }
    return Runtime.funcWrappers[func];
  },

  // Returns a processor of UTF.
  // processCChar() receives characters from a C-like UTF representation and returns JS string fragments.
  // processJSString() receives a JS string and returns a C-like UTF representation in an array
  UTF8Processor: function() {
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

#if RUNTIME_DEBUG
  debug: true, // Switch to false at runtime to disable logging at the right times

  printObjectList: [],

  prettyPrint: function(arg) {
    if (typeof arg == 'undefined') return '!UNDEFINED!';
    if (typeof arg == 'boolean') arg = arg + 0;
    if (!arg) return arg;
    var index = Runtime.printObjectList.indexOf(arg);
    if (index >= 0) return '<' + arg + '|' + index + '>';
    if (arg.toString() == '[object HTMLImageElement]') {
      return arg + '\n\n';
    }
    if (arg.byteLength) {
      return '{' + Array.prototype.slice.call(arg, 0, Math.min(arg.length, 400)) + '}'; // Useful for correct arrays, less so for compiled arrays, see the code below for that
      var buf = new ArrayBuffer(32);
      var i8buf = new Int8Array(buf);
      var i16buf = new Int16Array(buf);
      var f32buf = new Float32Array(buf);
      switch(arg.toString()) {
        case '[object Uint8Array]':
          i8buf.set(arg.subarray(0, 32));
          break;
        case '[object Float32Array]':
          f32buf.set(arg.subarray(0, 5));
          break;
        case '[object Uint16Array]':
          i16buf.set(arg.subarray(0, 16));
          break;
        default:
          alert('unknown array for debugging: ' + arg);
          throw 'see alert';
      }
      var ret = '{' + arg.byteLength + ':\n';
      var arr = Array.prototype.slice.call(i8buf);
      ret += 'i8:' + arr.toString().replace(/,/g, ',') + '\n';
      arr = Array.prototype.slice.call(f32buf, 0, 8);
      ret += 'f32:' + arr.toString().replace(/,/g, ',') + '}';
      return ret;
    }
    if (typeof arg == 'object') {
      Runtime.printObjectList.push(arg);
      return '<' + arg + '|' + (Runtime.printObjectList.length-1) + '>';
    }
    if (typeof arg == 'number') {
      if (arg > 0) return '0x' + arg.toString(16) + ' (' + arg + ')';
    }
    return arg;
  }
#endif
};

Runtime.stackAlloc = unInline('stackAlloc', ['size']);
Runtime.staticAlloc = unInline('staticAlloc', ['size']);
Runtime.alignMemory = unInline('alignMemory', ['size', 'quantum']);
Runtime.makeBigInt = unInline('makeBigInt', ['low', 'high', 'unsigned']);

function getRuntime() {
  var ret = 'var Runtime = {\n';
  for (i in Runtime) {
    var item = Runtime[i];
    ret += '  ' + i + ': ';
    if (typeof item === 'function') {
      ret += item.toString();
    } else {
      ret += JSON.stringify(item);
    }
    ret += ',\n';
  }
  return ret + '  __dummy__: 0\n}\n';
}

// Additional runtime elements, that need preprocessing

// Converts a value we have as signed, into an unsigned value. For
// example, -1 in int32 would be a very large number as unsigned.
function unSign(value, bits, ignore, sig) {
  if (value >= 0) {
#if CHECK_SIGNS
    if (!ignore) CorrectionsMonitor.note('UnSign', 1, sig);
#endif
    return value;
  }
#if CHECK_SIGNS
  if (!ignore) CorrectionsMonitor.note('UnSign', 0, sig);
#endif
  return bits <= 32 ? 2*Math.abs(1 << (bits-1)) + value // Need some trickery, since if bits == 32, we are right at the limit of the bits JS uses in bitshifts
                    : Math.pow(2, bits)         + value;
  // TODO: clean up previous line
}

// Converts a value we have as unsigned, into a signed value. For
// example, 200 in a uint8 would be a negative number.
function reSign(value, bits, ignore, sig) {
  if (value <= 0) {
#if CHECK_SIGNS
    if (!ignore) CorrectionsMonitor.note('ReSign', 1, sig);
#endif
    return value;
  }
  var half = bits <= 32 ? Math.abs(1 << (bits-1)) // abs is needed if bits == 32
                        : Math.pow(2, bits-1);
#if CHECK_SIGNS
  var noted = false;
#endif
  if (value >= half && (bits <= 32 || value > half)) { // for huge values, we can hit the precision limit and always get true here. so don't do that
                                                       // but, in general there is no perfect solution here. With 64-bit ints, we get rounding and errors
                                                       // TODO: In i64 mode 1, resign the two parts separately and safely
#if CHECK_SIGNS
    if (!ignore) {
      CorrectionsMonitor.note('ReSign', 0, sig);
      noted = true;
    }
#endif
    value = -2*half + value; // Cannot bitshift half, as it may be at the limit of the bits JS uses in bitshifts
  }
#if CHECK_SIGNS
  // If this is a 32-bit value, then it should be corrected at this point. And,
  // without CHECK_SIGNS, we would just do the |0 shortcut, so check that that
  // would indeed give the exact same result.
  if (bits === 32 && (value|0) !== value && typeof value !== 'boolean') {
    if (!ignore) {
      CorrectionsMonitor.note('ReSign', 0, sig);
      noted = true;
    }
  }
  if (!noted) CorrectionsMonitor.note('ReSign', 1, sig);
#endif
  return value;
}

// Just a stub. We don't care about noting compile-time corrections. But they are called.
var CorrectionsMonitor = {
  note: function(){}
};

