// {{PREAMBLE_ADDITIONS}}

var STACK_ALIGN = {{{ STACK_ALIGN }}};

#if ASSERTIONS
// stack management, and other functionality that is provided by the compiled code,
// should not be used before it is ready
stackSave = stackRestore = stackAlloc = setTempRet0 = getTempRet0 = function() {
  abort('cannot use the stack before compiled code is ready to run, and has provided stack access');
};
#endif

function staticAlloc(size) {
  assert(!staticSealed);
  var ret = STATICTOP;
  STATICTOP = (STATICTOP + size + 15) & -16;
  return ret;
}

function dynamicAlloc(size) {
  assert(DYNAMICTOP_PTR);
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

function alignMemory(size, factor) {
  if (!factor) factor = STACK_ALIGN; // stack alignment (16-byte) by default
  var ret = size = Math.ceil(size / factor) * factor;
  return ret;
}

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
        var bits = parseInt(type.substr(1));
        assert(bits % 8 === 0);
        return bits / 8;
      } else {
        return 0;
      }
    }
  }
}

function warnOnce(text) {
  if (!warnOnce.shown) warnOnce.shown = {};
  if (!warnOnce.shown[text]) {
    warnOnce.shown[text] = 1;
    Module.printErr(text);
  }
}

#if RELOCATABLE
var loadedDynamicLibraries = [];

function loadDynamicLibrary(lib) {
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
        Module.printErr("warning: trying to dynamically load symbol '" + sym + "' (from '" + lib + "') that already exists (duplicate symbol? or weak linking, which isn't supported yet?)"); // + [curr, ' vs ', next]);
      }
    }
#endif
  }
  loadedDynamicLibraries.push(libModule);
}

#if BINARYEN
// Loads a side module from binary data
function loadWebAssemblyModule(binary) {
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
  env['memoryBase'] = env['gb'] = alignMemory(getMemory(memorySize + STACK_ALIGN), STACK_ALIGN); // TODO: add to cleanups
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
    global: {
      'NaN': NaN,
      'Infinity': Infinity,
      'Math': Math
    },
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
}
#endif // BINARYEN
#endif // RELOCATABLE

#if EMULATED_FUNCTION_POINTERS
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
#endif

var functionPointers = new Array({{{ RESERVED_FUNCTION_POINTERS }}});

function addFunction(func) {
#if EMULATED_FUNCTION_POINTERS == 0
  for (var i = 0; i < functionPointers.length; i++) {
    if (!functionPointers[i]) {
      functionPointers[i] = func;
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
  functionPointers[(index-{{{ FUNCTION_POINTER_ALIGNMENT }}})/{{{ FUNCTION_POINTER_ALIGNMENT }}}] = null;
#else
  alignFunctionTables(); // XXX we should rely on this being an invariant
  var tables = getFunctionTables();
  for (var sig in tables) {
    tables[sig][index] = null;
  }
#endif
}

#if RUNTIME_DEBUG
var runtimeDebug = true; // Switch to false at runtime to disable logging at the right times

var printObjectList = [];

function prettyPrint(arg) {
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

// The address globals begin at. Very low in memory, for code size and optimization opportunities.
// Above 0 is static memory, starting with globals.
// Then the stack.
// Then 'dynamic' memory for sbrk.
var GLOBAL_BASE = {{{ GLOBAL_BASE }}};

#if RELOCATABLE
GLOBAL_BASE = alignMemory(GLOBAL_BASE, {{{ MAX_GLOBAL_ALIGN || 1 }}});
#endif

