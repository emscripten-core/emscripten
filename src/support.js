// Copyright 2017 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

// {{PREAMBLE_ADDITIONS}}

var STACK_ALIGN = {{{ STACK_ALIGN }}};

#if ASSERTIONS
// stack management, and other functionality that is provided by the compiled code,
// should not be used before it is ready
stackSave = stackRestore = stackAlloc = function() {
  abort('cannot use the stack before compiled code is ready to run, and has provided stack access');
};
#endif

function staticAlloc(size) {
#if ASSERTIONS
  assert(!staticSealed);
#endif
  var ret = STATICTOP;
  STATICTOP = (STATICTOP + size + 15) & -16;
#if ASSERTIONS
  assert(STATICTOP < TOTAL_MEMORY, 'not enough memory for static allocation - increase TOTAL_MEMORY');
#endif
  return ret;
}

function dynamicAlloc(size) {
#if ASSERTIONS
  assert(DYNAMICTOP_PTR);
#endif
  var ret = HEAP32[DYNAMICTOP_PTR>>2];
  var end = (ret + size + 15) & -16;
  HEAP32[DYNAMICTOP_PTR>>2] = end;
  if (end >= TOTAL_MEMORY) {
    var success = enlargeMemory();
    if (!success) {
      HEAP32[DYNAMICTOP_PTR>>2] = ret;
      return 0;
    }
  }
  return ret;
}

{{{ alignMemory }}}

{{{ getNativeTypeSize }}}

function warnOnce(text) {
  if (!warnOnce.shown) warnOnce.shown = {};
  if (!warnOnce.shown[text]) {
    warnOnce.shown[text] = 1;
    err(text);
  }
}

var asm2wasmImports = { // special asm2wasm imports
    "f64-rem": function(x, y) {
        return x % y;
    },
    "debugger": function() {
        debugger;
    }
#if NEED_ALL_ASM2WASM_IMPORTS
    ,
    "f64-to-int": function(x) {
        return x | 0;
    },
    "i32s-div": function(x, y) {
        return ((x | 0) / (y | 0)) | 0;
    },
    "i32u-div": function(x, y) {
        return ((x >>> 0) / (y >>> 0)) >>> 0;
    },
    "i32s-rem": function(x, y) {
        return ((x | 0) % (y | 0)) | 0;
    },
    "i32u-rem": function(x, y) {
        return ((x >>> 0) % (y >>> 0)) >>> 0;
    }
#endif // NEED_ALL_ASM2WASM_IMPORTS
};

#if RELOCATABLE

function loadDynamicLibrary(lib) {
  var libModule;
#if WASM
  var bin;
  if (lib.buffer) {
    // we were provided the binary, in a typed array
    bin = lib;
  } else {
    // load the binary synchronously
    bin = Module['readBinary'](lib);
  }
  libModule = loadWebAssemblyModule(bin);
#else
  var src = Module['read'](lib);
  libModule = eval(src)(
    alignFunctionTables(),
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
        err("warning: trying to dynamically load symbol '" + sym + "' (from '" + lib + "') that already exists (duplicate symbol? or weak linking, which isn't supported yet?)"); // + [curr, ' vs ', next]);
      }
    }
#endif
  }
}

#if WASM
// Loads a side module from binary data
function loadWebAssemblyModule(binary, loadAsync) {
  var int32View = new Uint32Array(new Uint8Array(binary.subarray(0, 24)).buffer);
  assert(int32View[0] == 0x6d736100, 'need to see wasm magic number'); // \0asm
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
  var memoryAlign = getLEB();
  var tableSize = getLEB();
  var tableAlign = getLEB();
  // alignments are powers of 2
  memoryAlign = Math.pow(2, memoryAlign);
  tableAlign = Math.pow(2, tableAlign);
  // finalize alignments and verify them
  memoryAlign = Math.max(memoryAlign, STACK_ALIGN); // we at least need stack alignment
  assert(tableAlign === 1);
  // prepare memory
  var memoryStart = alignMemory(getMemory(memorySize + memoryAlign), memoryAlign); // TODO: add to cleanups
  // The static area consists of explicitly initialized data, followed by zero-initialized data.
  // The latter may need zeroing out if the MAIN_MODULE has already used this memory area before
  // dlopen'ing the SIDE_MODULE.  Since we don't know the size of the explicitly initialized data
  // here, we just zero the whole thing, which is suboptimal, but should at least resolve bugs
  // from uninitialized memory.
  for (var i = memoryStart; i < memoryStart + memorySize; ++i) HEAP8[i] = 0;
  // prepare env imports
  var env = Module['asmLibraryArg'];
  // TODO: use only __memory_base and __table_base, need to update asm.js backend
  var table = Module['wasmTable'];
  var oldTableSize = table.length;
  env['__memory_base'] = env['gb'] = memoryStart;
  env['__table_base'] = env['fb'] = oldTableSize;
  var originalTable = table;
  table.grow(tableSize);
  assert(table === originalTable);
  // zero-initialize memory and table
  // TODO: in some cases we can tell it is already zero initialized
  for (var i = env['__memory_base']; i < env['__memory_base'] + memorySize; i++) {
    HEAP8[i] = 0;
  }
  for (var i = env['__table_base']; i < env['__table_base'] + tableSize; i++) {
    table.set(i, null);
  }
  // copy currently exported symbols so the new module can import them
  for (var x in Module) {
    if (!(x in env)) {
      env[x] = Module[x];
    }
  }
  // wasm dynamic libraries are pure wasm, so they cannot assist in
  // their own loading. When side module A wants to import something
  // provided by a side module B that is loaded later, we need to
  // add a layer of indirection, but worse, we can't even tell what
  // to add the indirection for, without inspecting what A's imports
  // are. To do that here, we use a JS proxy (another option would
  // be to inspect the binary directly).
  var proxyHandler = {
    'get': function(obj, prop) {
      if (prop in obj) {
        return obj[prop]; // already present
      }
      if (prop.startsWith('g$')) {
        // a global. the g$ function returns the global address.
        var name = prop.substr(2); // without g$ prefix
        return env[prop] = function() {
#if ASSERTIONS
          assert(Module[name], 'missing linked global ' + name);
#endif
          return Module[name];
        };
      }
      if (prop.startsWith('invoke_')) {
        // A missing invoke, i.e., an invoke for a function type
        // present in the dynamic library but not in the main JS,
        // and the dynamic library cannot provide JS for it. Use
        // the generic "X" invoke for it.
        return env[prop] = invoke_X;
      }
      // if not a global, then a function - call it indirectly
      return env[prop] = function() {
#if ASSERTIONS
        assert(Module[prop], 'missing linked function ' + prop);
#endif
        return Module[prop].apply(null, arguments);
      };
    }
  };
  var info = {
    global: {
      'NaN': NaN,
      'Infinity': Infinity,
    },
    'global.Math': Math,
    env: new Proxy(env, proxyHandler),
    'asm2wasm': asm2wasmImports
  };
#if ASSERTIONS
  var oldTable = [];
  for (var i = 0; i < oldTableSize; i++) {
    oldTable.push(table.get(i));
  }
#endif

  function postInstantiation(instance) {
    var exports = {};
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
    for (var e in instance.exports) {
      var value = instance.exports[e];
      if (typeof value === 'object') {
        // a breaking change in the wasm spec, globals are now objects
        // https://github.com/WebAssembly/mutable-global/issues/1
        value = value.value;
      }
      if (typeof value === 'number') {
        // relocate it - modules export the absolute value, they can't relocate before they export
#if EMULATE_FUNCTION_POINTER_CASTS
        // it may be a function pointer
        if (e.substr(0, 3) == 'fp$' && typeof instance.exports[e.substr(3)] === 'function') {
          value = value + env['__table_base'];
        } else {
#endif
          value = value + env['__memory_base'];
#if EMULATE_FUNCTION_POINTER_CASTS
        }
#endif
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
  }

  if (loadAsync) {
    return WebAssembly.instantiate(binary, info).then(function(result) {
      return postInstantiation(result.instance);
    });
  } else {
    var instance = new WebAssembly.Instance(new WebAssembly.Module(binary), info);
    return postInstantiation(instance);
  }
}
Module['loadWebAssemblyModule'] = loadWebAssemblyModule;

#endif // WASM
#endif // RELOCATABLE

#if EMULATED_FUNCTION_POINTERS
#if WASM == 0
function getFunctionTables(module) {
  if (!module) module = Module;
  var tables = {};
  for (var t in module) {
    if (/^FUNCTION_TABLE_.*/.test(t)) {
      var table = module[t];
      if (typeof table === 'object') tables[t.substr('FUNCTION_TABLE_'.length)] = table;
    }
  }
  return tables;
}

function alignFunctionTables(module) {
  var tables = getFunctionTables(module);
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
}
#endif // WASM == 0

#if RELOCATABLE
// register functions from a new module being loaded
function registerFunctions(sigs, newModule) {
  sigs.forEach(function(sig) {
    if (!Module['FUNCTION_TABLE_' + sig]) {
      Module['FUNCTION_TABLE_' + sig] = [];
    }
  });
  var oldMaxx = alignFunctionTables(); // align the new tables we may have just added
  var newMaxx = alignFunctionTables(newModule);
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
  assert(maxx === alignFunctionTables()); // align the ones we didn't touch
}
// export this so side modules can use it
Module['registerFunctions'] = registerFunctions;
#endif // RELOCATABLE
#endif // EMULATED_FUNCTION_POINTERS

#if WASM_BACKEND_WITH_RESERVED_FUNCTION_POINTERS
var jsCallStartIndex = {{{ JSCALL_START_INDEX }}};
var jsCallSigOrder = {{{ JSON.stringify(JSCALL_SIG_ORDER) }}};
var jsCallNumSigs = Object.keys(jsCallSigOrder).length;
var functionPointers = new Array(jsCallNumSigs * {{{ RESERVED_FUNCTION_POINTERS }}});
#else // WASM_BACKEND_WITH_RESERVED_FUNCTION_POINTERS == 0
var jsCallStartIndex = 1;
var functionPointers = new Array({{{ RESERVED_FUNCTION_POINTERS }}});
#endif // WASM_BACKEND_WITH_RESERVED_FUNCTION_POINTERS

// 'sig' parameter is only used on LLVM wasm backend
function addFunction(func, sig) {
#if WASM_BACKEND
  assert(typeof sig !== 'undefined',
         'Second argument of addFunction should be a wasm function signature ' +
         'string');
#endif // WASM_BACKEND
#if ASSERTIONS
  if (typeof sig === 'undefined') {
    err('warning: addFunction(): You should provide a wasm function signature string as a second argument. This is not necessary for asm.js and asm2wasm, but is required for the LLVM wasm backend, so it is recommended for full portability.');
  }
#endif // ASSERTIONS
#if EMULATED_FUNCTION_POINTERS == 0
#if WASM_BACKEND_WITH_RESERVED_FUNCTION_POINTERS
  var base = jsCallSigOrder[sig] * {{{ RESERVED_FUNCTION_POINTERS }}};
#else // WASM_BACKEND_WITH_RESERVED_FUNCTION_POINTERS == 0
  var base = 0;
#endif // WASM_BACKEND_WITH_RESERVED_FUNCTION_POINTERS
  for (var i = base; i < base + {{{ RESERVED_FUNCTION_POINTERS }}}; i++) {
    if (!functionPointers[i]) {
      functionPointers[i] = func;
      return jsCallStartIndex + i;
    }
  }
  throw 'Finished up all reserved function pointers. Use a higher value for RESERVED_FUNCTION_POINTERS.';
#else
#if WASM
  // we can simply append to the wasm table
  var table = Module['wasmTable'];
  var ret = table.length;
  table.grow(1);
  table.set(ret, func);
  return ret;
#else
  alignFunctionTables(); // XXX we should rely on this being an invariant
  var tables = getFunctionTables();
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
}

function removeFunction(index) {
#if EMULATED_FUNCTION_POINTERS == 0
  functionPointers[index-jsCallStartIndex] = null;
#else
  alignFunctionTables(); // XXX we should rely on this being an invariant
  var tables = getFunctionTables();
  for (var sig in tables) {
    tables[sig][index] = null;
  }
#endif
}

var funcWrappers = {};

function getFuncWrapper(func, sig) {
  if (!func) return; // on null pointer, return undefined
  assert(sig);
  if (!funcWrappers[sig]) {
    funcWrappers[sig] = {};
  }
  var sigCache = funcWrappers[sig];
  if (!sigCache[func]) {
    // optimize away arguments usage in common cases
    if (sig.length === 1) {
      sigCache[func] = function dynCall_wrapper() {
        return dynCall(sig, func);
      };
    } else if (sig.length === 2) {
      sigCache[func] = function dynCall_wrapper(arg) {
        return dynCall(sig, func, [arg]);
      };
    } else {
      // general case
      sigCache[func] = function dynCall_wrapper() {
        return dynCall(sig, func, Array.prototype.slice.call(arguments));
      };
    }
  }
  return sigCache[func];
}

#if RUNTIME_DEBUG
var runtimeDebug = true; // Switch to false at runtime to disable logging at the right times

var printObjectList = [];

function prettyPrint(arg) {
  if (typeof arg == 'undefined') return '!UNDEFINED!';
  if (typeof arg == 'boolean') arg = arg + 0;
  if (!arg) return arg;
  var index = printObjectList.indexOf(arg);
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
    printObjectList.push(arg);
    return '<' + arg + '|' + (printObjectList.length-1) + '>';
  }
  if (typeof arg == 'number') {
    if (arg > 0) return '0x' + arg.toString(16) + ' (' + arg + ')';
  }
  return arg;
}
#endif

function makeBigInt(low, high, unsigned) {
  return unsigned ? ((+((low>>>0)))+((+((high>>>0)))*4294967296.0)) : ((+((low>>>0)))+((+((high|0)))*4294967296.0));
}

function dynCall(sig, ptr, args) {
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
}

var tempRet0 = 0;

var setTempRet0 = function(value) {
  tempRet0 = value;
}

var getTempRet0 = function() {
  return tempRet0;
}

#if RETAIN_COMPILER_SETTINGS
var compilerSettings = {{{ JSON.stringify(makeRetainedCompilerSettings()) }}} ;

function getCompilerSetting(name) {
  if (!(name in compilerSettings)) return 'invalid compiler setting: ' + name;
  return compilerSettings[name];
}
#else // RETAIN_COMPILER_SETTINGS
#if ASSERTIONS
function getCompilerSetting(name) {
  throw 'You must build with -s RETAIN_COMPILER_SETTINGS=1 for getCompilerSetting or emscripten_get_compiler_setting to work';
}
#endif // ASSERTIONS
#endif // RETAIN_COMPILER_SETTINGS

var Runtime = {
  // FIXME backwards compatibility layer for ports. Support some Runtime.*
  //       for now, fix it there, then remove it from here. That way we
  //       can minimize any period of breakage.
  dynCall: dynCall, // for SDL2 port
#if ASSERTIONS
  // helpful errors
  getTempRet0: function() { abort('getTempRet0() is now a top-level function, after removing the Runtime object. Remove "Runtime."') },
  staticAlloc: function() { abort('staticAlloc() is now a top-level function, after removing the Runtime object. Remove "Runtime."') },
  stackAlloc: function() { abort('stackAlloc() is now a top-level function, after removing the Runtime object. Remove "Runtime."') },
#endif
};

// The address globals begin at. Very low in memory, for code size and optimization opportunities.
// Above 0 is static memory, starting with globals.
// Then the stack.
// Then 'dynamic' memory for sbrk.
var GLOBAL_BASE = {{{ GLOBAL_BASE }}};

#if RELOCATABLE
GLOBAL_BASE = alignMemory(GLOBAL_BASE, {{{ MAX_GLOBAL_ALIGN || 1 }}});
#endif
