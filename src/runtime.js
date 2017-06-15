//"use strict";

// Implementation details for the 'runtime environment' we generate in
// JavaScript. The Runtime object itself is used both during compilation,
// and is available at runtime (dynamic compilation). The RuntimeGenerator
// helps to create the Runtime object (written so that the Runtime object
// itself is as optimized as possible - no unneeded runtime checks).

var RuntimeGenerator = {
  alloc: function(size, type, sep, ignoreAlign) {
    sep = sep || ';';
    var ret = type + 'TOP';
    ret += sep + type + 'TOP = (' + type + 'TOP + ' + size + ')|0';
    if ({{{ STACK_ALIGN }}} > 1 && !ignoreAlign) {
      ret += sep + RuntimeGenerator.alignMemory(type + 'TOP', {{{ STACK_ALIGN }}});
    }
    return ret;
  },

  // An allocation that lives as long as the current function call
  stackAlloc: function(size, sep) {
    sep = sep || ';';
    var ret = RuntimeGenerator.alloc(size, 'STACK', sep, (isNumber(size) && parseInt(size) % {{{ STACK_ALIGN }}} == 0));
    if (ASSERTIONS || STACK_OVERFLOW_CHECK >= 2) {
      ret += sep + '(assert(' + asmCoercion('(STACKTOP|0) < (STACK_MAX|0)', 'i32') + ')|0)';
    }
    return ret;
  },

  stackEnter: function(initial, force) {
    if (initial === 0 && SKIP_STACK_IN_SMALL && !force) return '';
    var ret = 'var sp=0;sp=STACKTOP';
    if (initial > 0) ret += ';STACKTOP=(STACKTOP+' + initial + ')|0';
    assert(initial % Runtime.STACK_ALIGN == 0);
    if (ASSERTIONS && Runtime.STACK_ALIGN == 4) {
      ret += '; (assert(' + asmCoercion('!(STACKTOP&3)', 'i32') + ')|0)';
    }
    if (ASSERTIONS) {
      ret += '; (assert(' + asmCoercion('(STACKTOP|0) < (STACK_MAX|0)', 'i32') + ')|0)';
    }
    return ret;
  },

  stackExit: function(initial, force) {
    if (initial === 0 && SKIP_STACK_IN_SMALL && !force) return '';
    return 'STACKTOP=sp';
  },

  // An allocation that cannot normally be free'd (except through sbrk, which once
  // called, takes control of STATICTOP)
  staticAlloc: function(size) {
    if (ASSERTIONS) size = '(assert(!staticSealed),' + size + ')'; // static area must not be sealed
#if USE_PTHREADS
    if (typeof ENVIRONMENT_IS_PTHREAD !== 'undefined' && ENVIRONMENT_IS_PTHREAD) throw 'Runtime.staticAlloc is not available in pthreads!'; // This is because each worker has its own copy of STATICTOP, of which main thread is authoritative.
#endif
    var ret = RuntimeGenerator.alloc(size, 'STATIC');
    return ret;
  },

  // allocation on the top of memory, adjusted dynamically by sbrk
  dynamicAlloc: function(size) {
    var ret = '';
    if (ASSERTIONS) ret += 'assert(DYNAMICTOP_PTR);'; // dynamic area must be ready
    ret += 'var ret = HEAP32[DYNAMICTOP_PTR>>2];'
      + 'var end = (((ret + size + 15)|0) & -16);'
      + 'HEAP32[DYNAMICTOP_PTR>>2] = end;'
      + 'if (end >= TOTAL_MEMORY) {'
      +   'var success = enlargeMemory();'
      +     'if (!success) {'
      +       'HEAP32[DYNAMICTOP_PTR>>2] = ret;'
      +       'return 0;'
      +    '}'
      +  '}'
      + 'return ret;';
    return ret;
  },

  forceAlign: function(target, quantum) {
    quantum = quantum || {{{ QUANTUM_SIZE }}};
    if (quantum == 1) return target;
    if (isNumber(target) && isNumber(quantum)) {
      return Math.ceil(target/quantum)*quantum;
    } else if (isNumber(quantum) && isPowerOfTwo(quantum)) {
      return '(((' +target + ')+' + (quantum-1) + ')&' + -quantum + ')';
    }
    return 'Math.ceil((' + target + ')/' + quantum + ')*' + quantum;
  },

  alignMemory: function(target, quantum) {
    if (typeof quantum !== 'number') {
      quantum = '(quantum ? quantum : {{{ STACK_ALIGN }}})';
    }
    return target + ' = ' + RuntimeGenerator.forceAlign(target, quantum);
  },

  // Given two 32-bit unsigned parts of an emulated 64-bit number, combine them into a JS number (double).
  // Rounding is inevitable if the number is large. This is a particular problem for small negative numbers
  // (-1 will be rounded!), so handle negatives separately and carefully
  makeBigInt: function(low, high, unsigned) {
    var unsignedRet = '(' + asmCoercion(makeSignOp(low, 'i32', 'un', 1, 1), 'double') + '+(' + asmCoercion(makeSignOp(high, 'i32', 'un', 1, 1), 'double') + '*' + asmEnsureFloat(4294967296, 'double') + '))';
    var signedRet = '(' + asmCoercion(makeSignOp(low, 'i32', 'un', 1, 1), 'double') + '+(' + asmCoercion(makeSignOp(high, 'i32', 're', 1, 1), 'double') + '*' + asmEnsureFloat(4294967296, 'double') + '))';
    if (typeof unsigned === 'string') return '(' + unsigned + ' ? ' + unsignedRet + ' : ' + signedRet + ')';
    return unsigned ? unsignedRet : signedRet;
  }
};

function unInline(name_, params, isExpression) {
  if (isExpression) {
    var src = '(function(' + params + ') { var ret = ' + RuntimeGenerator[name_].apply(null, params) + '; return ret; })';
  } else {
    var src = '(function(' + params + ') { ' + RuntimeGenerator[name_].apply(null, params) + '})';
  }
  var ret = eval(src);
  return ret;
}

var Compiletime = {
  isPointerType: isPointerType,
  isStructType: isStructType,

  isNumberType: function(type) {
    return type in Compiletime.INT_TYPES || type in Compiletime.FLOAT_TYPES;
  },

  INT_TYPES: set('i1', 'i8', 'i16', 'i32', 'i64'),
  FLOAT_TYPES: set('float', 'double'),
};

var Runtime = {
  // When a 64 bit long is returned from a compiled function the least significant
  // 32 bit word is passed in the return value, but the most significant 32 bit
  // word is placed in tempRet0. This provides an accessor for that value.
  setTempRet0: function(value) {
    tempRet0 = value;
    return value;
  },
  getTempRet0: function() {
    return tempRet0;
  },
  stackSave: function() {
    return STACKTOP;
  },
  stackRestore: function(stackTop) {
    STACKTOP = stackTop;
  },

  //! Returns the size of a type, as C/C++ would have it (in 32-bit), in bytes.
  //! @param type The type, by name.
  getNativeTypeSize: function(type) {
    switch (type) {
      case 'i1': case 'i8': return 1;
      case 'i16': return 2;
      case 'i32': return 4;
      case 'i64': return 8;
      case 'float': return 4;
      case 'double': return 8;
      default: {
        if (type[type.length-1] === '*') {
          return Runtime.QUANTUM_SIZE; // A pointer
        } else if (type[0] === 'i') {
          var bits = parseInt(type.substr(1));
          assert(bits % 8 === 0);
          return bits/8;
        } else {
          return 0;
        }
      }
    }
  },

  //! Returns the size of a structure field, as C/C++ would have it (in 32-bit,
  //! for now).
  //! @param type The type, by name.
  getNativeFieldSize: function(type) {
    return Math.max(Runtime.getNativeTypeSize(type), Runtime.QUANTUM_SIZE);
  },

  STACK_ALIGN: {{{ STACK_ALIGN }}},

  // This must be called before reading a double or i64 vararg. It will bump the pointer properly.
  // It also does an assert on i32 values, so it's nice to call it before all varargs calls.
  prepVararg: function(ptr, type) {
    if (type === 'double' || type === 'i64') {
      // move so the load is aligned
      if (ptr & 7) {
        assert((ptr & 7) === 4);
        ptr += 4;
      }
    } else {
      assert((ptr & 3) === 0);
    }
    return ptr;
  },

  // type can be a native type or a struct (or null, for structs we only look at size here)
  getAlignSize: function(type, size, vararg) {
    // we align i64s and doubles on 64-bit boundaries, unlike x86
    if (!vararg && (type == 'i64' || type == 'double')) return 8;
    if (!type) return Math.min(size, 8); // align structures internally to 64 bits
    return Math.min(size || (type ? Runtime.getNativeFieldSize(type) : 0), Runtime.QUANTUM_SIZE);
  },

  dynCall: function(sig, ptr, args) {
    if (args && args.length) {
#if ASSERTIONS
      assert(args.length == sig.length-1);
#endif
#if ASSERTIONS
      assert(('dynCall_' + sig) in Module, 'bad function pointer type - no table for sig \'' + sig + '\'');
#endif
      return Module['dynCall_' + sig].apply(null, [ptr].concat(args));
    } else {
#if ASSERTIONS
      assert(sig.length == 1);
#endif
#if ASSERTIONS
      assert(('dynCall_' + sig) in Module, 'bad function pointer type - no table for sig \'' + sig + '\'');
#endif
      return Module['dynCall_' + sig].call(null, ptr);
    }
  },

#if EMULATED_FUNCTION_POINTERS
  getFunctionTables: function(module) {
    if (!module) module = Module;
    var tables = {};
    for (var t in module) {
      if (/^FUNCTION_TABLE_.*/.test(t)) {
        var table = module[t];
        if (typeof table === 'object') tables[t.substr('FUNCTION_TABLE_'.length)] = table;
      }
    }
    return tables;
  },

  alignFunctionTables: function(module) {
    var tables = Runtime.getFunctionTables(module);
    var maxx = 0;
    for (var sig in tables) {
      maxx = Math.max(maxx, tables[sig].length);
    }
    assert(maxx >= 0);
    for (var sig in tables) {
      var table = tables[sig];
      while (table.length < maxx) table.push(0);
    }
    return maxx;
  },

  registerFunctions: function(sigs, newModule) {
    sigs.forEach(function(sig) {
      if (!Module['FUNCTION_TABLE_' + sig]) {
        Module['FUNCTION_TABLE_' + sig] = [];
      }
    });
    var oldMaxx = Runtime.alignFunctionTables(); // align the new tables we may have just added
    var newMaxx = Runtime.alignFunctionTables(newModule);
    var maxx = oldMaxx + newMaxx;
    sigs.forEach(function(sig) {
      var newTable = newModule['FUNCTION_TABLE_' + sig];
      var oldTable = Module['FUNCTION_TABLE_' + sig];
      assert(newTable !== oldTable);
      assert(oldTable.length === oldMaxx);
      for (var i = 0; i < newTable.length; i++) {
        oldTable.push(newTable[i]);
      }
      assert(oldTable.length === maxx);
    });
    assert(maxx === Runtime.alignFunctionTables()); // align the ones we didn't touch
  },
#endif

  functionPointers: new Array(RESERVED_FUNCTION_POINTERS),

  addFunction: function(func) {
#if EMULATED_FUNCTION_POINTERS == 0
    for (var i = 0; i < Runtime.functionPointers.length; i++) {
      if (!Runtime.functionPointers[i]) {
        Runtime.functionPointers[i] = func;
        return {{{ FUNCTION_POINTER_ALIGNMENT }}}*(1 + i);
      }
    }
    throw 'Finished up all reserved function pointers. Use a higher value for RESERVED_FUNCTION_POINTERS.';
#else
#if BINARYEN
    // we can simply appent to the wasm table
    var table = Module['wasmTable'];
    var ret = table.length;
    table.grow(1);
    table.set(ret, func);
    return ret;
#else
    Runtime.alignFunctionTables(); // XXX we should rely on this being an invariant
    var tables = Runtime.getFunctionTables();
    var ret = -1;
    for (var sig in tables) {
      var table = tables[sig];
      if (ret < 0) ret = table.length;
      else assert(ret === table.length);
      table.push(func);
    }
    return ret;
#endif
#endif
  },

  removeFunction: function(index) {
#if EMULATED_FUNCTION_POINTERS == 0
    Runtime.functionPointers[(index-{{{ FUNCTION_POINTER_ALIGNMENT }}})/{{{ FUNCTION_POINTER_ALIGNMENT }}}] = null;
#else
    Runtime.alignFunctionTables(); // XXX we should rely on this being an invariant
    var tables = Runtime.getFunctionTables();
    for (var sig in tables) {
      tables[sig][index] = null;
    }
#endif
  },

#if RELOCATABLE
  loadedDynamicLibraries: [],

  loadDynamicLibrary: function(lib) {
    var libModule;
#if BINARYEN
    var bin;
    if (lib.buffer) {
      // we were provided the binary, in a typed array
      bin = lib;
    } else {
      // load the binary synchronously
      bin = Module['readBinary'](lib);
    }
    libModule = Runtime.loadWebAssemblyModule(bin);
#else
    var src = Module['read'](lib);
    libModule = eval(src)(
      Runtime.alignFunctionTables(),
      Module
    );
#endif
    // add symbols into global namespace TODO: weak linking etc.
    for (var sym in libModule) {
      if (!Module.hasOwnProperty(sym)) {
        Module[sym] = libModule[sym];
      }
#if ASSERTIONS == 2
      else if (sym[0] === '_') {
        var curr = Module[sym], next = libModule[sym];
        // don't warn on functions - might be odr, linkonce_odr, etc.
        if (!(typeof curr === 'function' && typeof next === 'function')) {
          Module.printErr("warning: trying to dynamically load symbol '" + sym + "' (from '" + lib + "') that already exists (duplicate symbol? or weak linking, which isn't supported yet?)"); // + [curr, ' vs ', next]);
        }
      }
#endif
    }
    Runtime.loadedDynamicLibraries.push(libModule);
  },

#if BINARYEN
  // Loads a side module from binary data
  loadWebAssemblyModule: function(binary) {
    var int32View = new Uint32Array(new Uint8Array(binary.subarray(0, 24)).buffer);
    assert(int32View[0] == 0x6d736100, 'need to see wasm magic number'); // \0wasm
    // we should see the dylink section right after the magic number and wasm version
    assert(binary[8] === 0, 'need the dylink section to be first')
    var next = 9;
    function getLEB() {
      var ret = 0;
      var mul = 1;
      while (1) {
        var byte = binary[next++];
        ret += ((byte & 0x7f) * mul);
        mul *= 0x80;
        if (!(byte & 0x80)) break;
      }
      return ret;
    }
    var sectionSize = getLEB();
    assert(binary[next] === 6);                 next++; // size of "dylink" string
    assert(binary[next] === 'd'.charCodeAt(0)); next++;
    assert(binary[next] === 'y'.charCodeAt(0)); next++;
    assert(binary[next] === 'l'.charCodeAt(0)); next++;
    assert(binary[next] === 'i'.charCodeAt(0)); next++;
    assert(binary[next] === 'n'.charCodeAt(0)); next++;
    assert(binary[next] === 'k'.charCodeAt(0)); next++;
    var memorySize = getLEB();
    var tableSize = getLEB();
    var env = Module['asmLibraryArg'];
    // TODO: use only memoryBase and tableBase, need to update asm.js backend
    var table = Module['wasmTable'];
    var oldTableSize = table.length;
    env['memoryBase'] = env['gb'] = Runtime.alignMemory(getMemory(memorySize + Runtime.STACK_ALIGN), Runtime.STACK_ALIGN); // TODO: add to cleanups
    env['tableBase'] = env['fb'] = oldTableSize;
    var originalTable = table;
    table.grow(tableSize);
    assert(table === originalTable);
    // zero-initialize memory and table TODO: in some cases we can tell it is already zero initialized
    for (var i = env['memoryBase']; i < env['memoryBase'] + memorySize; i++) {
      HEAP8[i] = 0;
    }
    for (var i = env['tableBase']; i < env['tableBase'] + tableSize; i++) {
      table.set(i, null);
    }
    // copy currently exported symbols so the new module can import them
    for (var x in Module) {
      if (!(x in env)) {
        env[x] = Module[x];
      }
    }
    var info = {
      global: Module['asmGlobalArg'],
      env: env
    };
#if ASSERTIONS
    var oldTable = [];
    for (var i = 0; i < oldTableSize; i++) {
      oldTable.push(table.get(i));
    }
#endif
    // create a module from the instance
    var instance = new WebAssembly.Instance(new WebAssembly.Module(binary), info);
#if ASSERTIONS
    // the table should be unchanged
    assert(table === originalTable);
    assert(table === Module['wasmTable']);
    if (instance.exports['table']) {
      assert(table === instance.exports['table']);
    }
    // the old part of the table should be unchanged
    for (var i = 0; i < oldTableSize; i++) {
      assert(table.get(i) === oldTable[i], 'old table entries must remain the same');
    }
    // verify that the new table region was filled in
    for (var i = 0; i < tableSize; i++) {
      assert(table.get(oldTableSize + i) !== undefined, 'table entry was not filled in');
    }
#endif
    var exports = {};
    for (var e in instance.exports) {
      var value = instance.exports[e];
      if (typeof value === 'number') {
        // relocate it - modules export the absolute value, they can't relocate before they export
        value = value + env['memoryBase'];
      }
      exports[e] = value;
    }
    // initialize the module
    var init = exports['__post_instantiate'];
    if (init) {
      if (runtimeInitialized) {
        init();
      } else {
        // we aren't ready to run compiled code yet
        __ATINIT__.push(init);
      }
    }
    return exports;
  },
#endif
#endif

  warnOnce: function(text) {
    if (!Runtime.warnOnce.shown) Runtime.warnOnce.shown = {};
    if (!Runtime.warnOnce.shown[text]) {
      Runtime.warnOnce.shown[text] = 1;
      Module.printErr(text);
    }
  },

  funcWrappers: {},

  getFuncWrapper: function(func, sig) {
    assert(sig);
    if (!Runtime.funcWrappers[sig]) {
      Runtime.funcWrappers[sig] = {};
    }
    var sigCache = Runtime.funcWrappers[sig];
    if (!sigCache[func]) {
      // optimize away arguments usage in common cases
      if (sig.length === 1) {
        sigCache[func] = function dynCall_wrapper() {
          return Runtime.dynCall(sig, func);
        };
      } else if (sig.length === 2) {
        sigCache[func] = function dynCall_wrapper(arg) {
          return Runtime.dynCall(sig, func, [arg]);
        };
      } else {
        // general case
        sigCache[func] = function dynCall_wrapper() {
          return Runtime.dynCall(sig, func, Array.prototype.slice.call(arguments));
        };
      }
    }
    return sigCache[func];
  },

#if RETAIN_COMPILER_SETTINGS
  compilerSettings: {},
#endif

  getCompilerSetting: function(name) {
#if RETAIN_COMPILER_SETTINGS == 0
    throw 'You must build with -s RETAIN_COMPILER_SETTINGS=1 for Runtime.getCompilerSetting or emscripten_get_compiler_setting to work';
#else
    if (!(name in Runtime.compilerSettings)) return 'invalid compiler setting: ' + name;
    return Runtime.compilerSettings[name];
#endif
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

Runtime.stackAlloc = unInline('stackAlloc', ['size'], true);
Runtime.staticAlloc = unInline('staticAlloc', ['size'], true);
Runtime.dynamicAlloc = unInline('dynamicAlloc', ['size'], false);
Runtime.alignMemory = unInline('alignMemory', ['size', 'quantum'], true);
Runtime.makeBigInt = unInline('makeBigInt', ['low', 'high', 'unsigned'], true);

if (MAIN_MODULE || SIDE_MODULE) {
  Runtime.tempRet0 = 0;
  Runtime.getTempRet0 = function() {
    return Runtime.tempRet0;
  };
  Runtime.setTempRet0 = function(x) {
    Runtime.tempRet0 = x;
    return x;
  };
}

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
function unSign(value, bits, ignore) {
  if (value >= 0) {
    return value;
  }
  return bits <= 32 ? 2*Math.abs(1 << (bits-1)) + value // Need some trickery, since if bits == 32, we are right at the limit of the bits JS uses in bitshifts
                    : Math.pow(2, bits)         + value;
}

// Converts a value we have as unsigned, into a signed value. For
// example, 200 in a uint8 would be a negative number.
function reSign(value, bits, ignore) {
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

// The address globals begin at. Very low in memory, for code size and optimization opportunities.
// Above 0 is static memory, starting with globals.
// Then the stack.
// Then 'dynamic' memory for sbrk.
Runtime.GLOBAL_BASE = {{{ GLOBAL_BASE }}};

if (RETAIN_COMPILER_SETTINGS) {
  var blacklist = set('STRUCT_INFO');
  for (var x in this) {
    try {
      if (x[0] !== '_' && !(x in blacklist) && x == x.toUpperCase() && (typeof this[x] === 'number' || typeof this[x] === 'string' || this.isArray())) Runtime.compilerSettings[x] = this[x];
    } catch(e){}
  }
}

