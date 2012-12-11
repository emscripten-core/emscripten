// === Auto-generated preamble library stuff ===

//========================================
// Runtime code shared with compiler
//========================================

{{RUNTIME}}

#if SAFE_HEAP
//========================================
// Debugging tools - Heap
//========================================
var HEAP_WATCHED = [];
var HEAP_HISTORY = [];
function SAFE_HEAP_CLEAR(dest) {
#if SAFE_HEAP_LOG
  Module.print('SAFE_HEAP clear: ' + dest);
#endif
  HEAP_HISTORY[dest] = undefined;
}
var SAFE_HEAP_ERRORS = 0;
var ACCEPTABLE_SAFE_HEAP_ERRORS = 0;

function SAFE_HEAP_ACCESS(dest, type, store, ignore) {
  //if (dest === A_NUMBER) Module.print ([dest, type, store] + ' ' + new Error().stack); // Something like this may be useful, in debugging

  assert(dest >= STACK_ROOT, 'segmentation fault: null pointer, or below normal memory');

#if USE_TYPED_ARRAYS
  // When using typed arrays, reads over the top of TOTAL_MEMORY will fail silently, so we must
  // correct that by growing TOTAL_MEMORY as needed. Without typed arrays, memory is a normal
  // JS array so it will work (potentially slowly, depending on the engine).
  assert(dest < STATICTOP);
  assert(STATICTOP <= TOTAL_MEMORY);
#endif

#if USE_TYPED_ARRAYS == 2
  return; // It is legitimate to violate the load-store assumption in this case
#endif
  if (type && type.charAt(type.length-1) == '*') type = 'i32'; // pointers are ints, for our purposes here
  // Note that this will pass even with unions: You can store X, load X, then store Y and load Y.
  // You cannot, however, do the nonportable act of store X and load Y!
  if (store) {
    HEAP_HISTORY[dest] = ignore ? null : type;
  } else {
#if USE_TYPED_ARRAYS == 0
    if (!HEAP[dest] && HEAP[dest] !== 0 && HEAP[dest] !== false && !ignore) { // false can be the result of a mathop comparator
      var error = true;
      try {
        if (HEAP[dest].toString() === 'NaN') error = false; // NaN is acceptable, as a double value
      } catch(e){}
      if (error) throw('Warning: Reading an invalid value at ' + dest + ' :: ' + new Error().stack + '\n');
    }
#endif
    if (type === null) return;
    var history = HEAP_HISTORY[dest];
    if (history === null) return;
    if (!ignore)
      assert(history, 'Must have a history for a safe heap load! ' + dest + ':' + type); // Warning - bit fields in C structs cause loads+stores for each store, so
                                                                                         //           they will show up here...
//    assert((history && history[0]) /* || HEAP[dest] === 0 */, "Loading from where there was no store! " + dest + ',' + HEAP[dest] + ',' + type + ', \n\n' + new Error().stack + '\n');
//    if (history[0].type !== type) {
    if (history !== type && !ignore) {
      Module.print('Load-store consistency assumption failure! ' + dest);
      Module.print('\n');
      Module.print(JSON.stringify(history));
      Module.print('\n');
      Module.print('LOAD: ' + type + ', ' + new Error().stack);
      Module.print('\n');
      SAFE_HEAP_ERRORS++;
      assert(SAFE_HEAP_ERRORS <= ACCEPTABLE_SAFE_HEAP_ERRORS, 'Load-store consistency assumption failure!');
    }
  }
}

function SAFE_HEAP_STORE(dest, value, type, ignore) {
#if SAFE_HEAP_LOG
  Module.print('SAFE_HEAP store: ' + [dest, type, value, ignore]);
#endif

  if (!ignore && !value && (value === null || value === undefined)) {
    throw('Warning: Writing an invalid value of ' + JSON.stringify(value) + ' at ' + dest + ' :: ' + new Error().stack + '\n');
  }
  //if (!ignore && (value === Infinity || value === -Infinity || isNaN(value))) throw [value, typeof value, new Error().stack];

  SAFE_HEAP_ACCESS(dest, type, true, ignore);
  if (dest in HEAP_WATCHED) {
    Module.print((new Error()).stack);
    throw "Bad store!" + dest;
  }

#if USE_TYPED_ARRAYS == 2
  // Check alignment
  switch(type) {
    case 'i16': assert(dest % 2 == 0); break;
    case 'i32': assert(dest % 4 == 0); break;
    case 'i64': assert(dest % 8 == 0); break;
    case 'float': assert(dest % 4 == 0); break;
#if DOUBLE_MODE == 1
    case 'double': assert(dest % 4 == 0); break;
#else
    case 'double': assert(dest % 4 == 0); break;
#endif
  }
#endif

  setValue(dest, value, type, 1);
}

function SAFE_HEAP_LOAD(dest, type, unsigned, ignore) {
  SAFE_HEAP_ACCESS(dest, type, false, ignore);

#if SAFE_HEAP_LOG
    Module.print('SAFE_HEAP load: ' + [dest, type, getValue(dest, type, 1), ignore]);
#endif

#if USE_TYPED_ARRAYS == 2
  // Check alignment
  switch(type) {
    case 'i16': assert(dest % 2 == 0); break;
    case 'i32': assert(dest % 4 == 0); break;
    case 'i64': assert(dest % 8 == 0); break;
    case 'float': assert(dest % 4 == 0); break;
#if DOUBLE_MODE == 1
    case 'double': assert(dest % 4 == 0); break;
#else
    case 'double': assert(dest % 4 == 0); break;
#endif
  }
#endif

  var ret = getValue(dest, type, 1);
  if (unsigned) ret = unSign(ret, parseInt(type.substr(1)), 1);
  return ret;
}

function SAFE_HEAP_COPY_HISTORY(dest, src) {
#if SAFE_HEAP_LOG
  Module.print('SAFE_HEAP copy: ' + [dest, src]);
#endif
  HEAP_HISTORY[dest] = HEAP_HISTORY[src];
  SAFE_HEAP_ACCESS(dest, HEAP_HISTORY[dest] || null, true, false);
}

//==========================================
#endif

var CorrectionsMonitor = {
#if PGO
  MAX_ALLOWED: Infinity,
#else
  MAX_ALLOWED: 0, // XXX
#endif
  corrections: 0,
  sigs: {},

  note: function(type, succeed, sig) {
    if (!succeed) {
      this.corrections++;
      if (this.corrections >= this.MAX_ALLOWED) abort('\n\nToo many corrections!');
    }
#if PGO
    if (!sig)
      sig = (new Error().stack).toString().split('\n')[2].split(':').slice(-1)[0]; // Spidermonkey-specific FIXME
    sig = type + '|' + sig;
    if (!this.sigs[sig]) {
      //Module.print('Correction: ' + sig);
      this.sigs[sig] = [0, 0]; // fail, succeed
    }
    this.sigs[sig][succeed ? 1 : 0]++;
#endif
  },

  print: function() {
#if PGO
    var items = [];
    for (var sig in this.sigs) {
      items.push({
        sig: sig,
        fails: this.sigs[sig][0],
        succeeds: this.sigs[sig][1],
        total: this.sigs[sig][0] + this.sigs[sig][1]
      });
    }
    items.sort(function(x, y) { return y.total - x.total; });
    for (var i = 0; i < items.length; i++) {
      var item = items[i];
      Module.print(item.sig + ' : ' + item.total + ' hits, %' + (Math.ceil(100*item.fails/item.total)) + ' failures');
    }
#endif
  }
};

#if CHECK_OVERFLOWS
//========================================
// Debugging tools - Mathop overflows
//========================================
function CHECK_OVERFLOW(value, bits, ignore, sig) {
  if (ignore) return value;
  var twopbits = Math.pow(2, bits);
  var twopbits1 = Math.pow(2, bits-1);
  // For signedness issue here, see settings.js, CHECK_SIGNED_OVERFLOWS
#if CHECK_SIGNED_OVERFLOWS
  if (value === Infinity || value === -Infinity || value >= twopbits1 || value < -twopbits1) {
    CorrectionsMonitor.note('SignedOverflow', 0, sig);
    if (value === Infinity || value === -Infinity || Math.abs(value) >= twopbits) CorrectionsMonitor.note('Overflow');
#else
  if (value === Infinity || value === -Infinity || Math.abs(value) >= twopbits) {
    CorrectionsMonitor.note('Overflow', 0, sig);
#endif
#if CORRECT_OVERFLOWS
    // Fail on >32 bits - we warned at compile time
    if (bits <= 32) {
      value = value & (twopbits - 1);
    }
#endif
  } else {
#if CHECK_SIGNED_OVERFLOWS
    CorrectionsMonitor.note('SignedOverflow', 1, sig);
#endif
    CorrectionsMonitor.note('Overflow', 1, sig);
  }
  return value;
}
#endif

#if LABEL_DEBUG
//========================================
// Debugging tools - Code flow progress
//========================================
var INDENT = '';
#endif

#if EXECUTION_TIMEOUT
//========================================
// Debugging tools - Execution timeout
//========================================
var START_TIME = Date.now();
#endif

#if PROFILE
var PROFILING = 0;
var PROFILING_ROOT = { time: 0, children: {}, calls: 0 };
var PROFILING_NODE;

function startProfiling() {
  PROFILING_NODE = PROFILING_ROOT;
  PROFILING = 1;
}
Module['startProfiling'] = startProfiling;

function stopProfiling() {
  PROFILING = 0;
  assert(PROFILING_NODE === PROFILING_ROOT, 'Must have popped all the profiling call stack');
}
Module['stopProfiling'] = stopProfiling;

function printProfiling() {
  function dumpData(name_, node, indent) {
    Module.print(indent + ('________' + node.time).substr(-8) + ': ' + name_ + ' (' + node.calls + ')');
    var children = [];
    for (var child in node.children) {
      children.push(node.children[child]);
      children[children.length-1].name_ = child;
    }
    children.sort(function(x, y) { return y.time - x.time });
    children.forEach(function(child) { dumpData(child.name_, child, indent + '  ') });
  }
  dumpData('root', PROFILING_ROOT, ' ');
}
Module['printProfiling'] = printProfiling;
#endif

//========================================
// Runtime essentials
//========================================

var __THREW__ = false; // Used in checking for thrown exceptions.
var setjmpId = 1; // Used in setjmp/longjmp
var setjmpLabels = {};

var ABORT = false;

var undef = 0;
// tempInt is used for 32-bit signed values or smaller. tempBigInt is used
// for 32-bit unsigned values or more than 32 bits. TODO: audit all uses of tempInt
var tempValue, tempInt, tempBigInt, tempInt2, tempBigInt2, tempPair, tempBigIntI, tempBigIntR, tempBigIntS, tempBigIntP, tempBigIntD;
#if USE_TYPED_ARRAYS == 2
var tempI64, tempI64b;
#endif

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
//       able to call them. Adding
//
//         __attribute__((used))
//
//       to the function definition will prevent that.
//
// Note: Closure optimizations will minify function names, making
//       functions no longer callable. If you run closure (on by default
//       in -O2 and above), you should export the functions you will call
//       by calling emcc with something like
//
//         -s EXPORTED_FUNCTIONS='["_func1","_func2"]'
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
#if SAFE_HEAP
  if (noSafe) {
    switch(type) {
      case 'i1': {{{ makeSetValue('ptr', '0', 'value', 'i1', undefined, undefined, undefined, '1') }}}; break;
      case 'i8': {{{ makeSetValue('ptr', '0', 'value', 'i8', undefined, undefined, undefined, '1') }}}; break;
      case 'i16': {{{ makeSetValue('ptr', '0', 'value', 'i16', undefined, undefined, undefined, '1') }}}; break;
      case 'i32': {{{ makeSetValue('ptr', '0', 'value', 'i32', undefined, undefined, undefined, '1') }}}; break;
      case 'i64': {{{ makeSetValue('ptr', '0', 'value', 'i64', undefined, undefined, undefined, '1') }}}; break;
      case 'float': {{{ makeSetValue('ptr', '0', 'value', 'float', undefined, undefined, undefined, '1') }}}; break;
      case 'double': {{{ makeSetValue('ptr', '0', 'value', 'double', undefined, undefined, undefined, '1') }}}; break;
      default: abort('invalid type for setValue: ' + type);
    }
  } else {
#endif
    switch(type) {
      case 'i1': {{{ makeSetValue('ptr', '0', 'value', 'i1') }}}; break;
      case 'i8': {{{ makeSetValue('ptr', '0', 'value', 'i8') }}}; break;
      case 'i16': {{{ makeSetValue('ptr', '0', 'value', 'i16') }}}; break;
      case 'i32': {{{ makeSetValue('ptr', '0', 'value', 'i32') }}}; break;
      case 'i64': {{{ makeSetValue('ptr', '0', 'value', 'i64') }}}; break;
      case 'float': {{{ makeSetValue('ptr', '0', 'value', 'float') }}}; break;
      case 'double': {{{ makeSetValue('ptr', '0', 'value', 'double') }}}; break;
      default: abort('invalid type for setValue: ' + type);
    }
#if SAFE_HEAP
  }
#endif
}
Module['setValue'] = setValue;

// Parallel to setValue.
function getValue(ptr, type, noSafe) {
  type = type || 'i8';
  if (type.charAt(type.length-1) === '*') type = 'i32'; // pointers are 32-bit
#if SAFE_HEAP
  if (noSafe) {
    switch(type) {
      case 'i1': return {{{ makeGetValue('ptr', '0', 'i1', undefined, undefined, undefined, undefined, '1') }}};
      case 'i8': return {{{ makeGetValue('ptr', '0', 'i8', undefined, undefined, undefined, undefined, '1') }}};
      case 'i16': return {{{ makeGetValue('ptr', '0', 'i16', undefined, undefined, undefined, undefined, '1') }}};
      case 'i32': return {{{ makeGetValue('ptr', '0', 'i32', undefined, undefined, undefined, undefined, '1') }}};
      case 'i64': return {{{ makeGetValue('ptr', '0', 'i64', undefined, undefined, undefined, undefined, '1') }}};
      case 'float': return {{{ makeGetValue('ptr', '0', 'float', undefined, undefined, undefined, undefined, '1') }}};
      case 'double': return {{{ makeGetValue('ptr', '0', 'double', undefined, undefined, undefined, undefined, '1') }}};
      default: abort('invalid type for setValue: ' + type);
    }
  } else {
#endif
    switch(type) {
      case 'i1': return {{{ makeGetValue('ptr', '0', 'i1') }}};
      case 'i8': return {{{ makeGetValue('ptr', '0', 'i8') }}};
      case 'i16': return {{{ makeGetValue('ptr', '0', 'i16') }}};
      case 'i32': return {{{ makeGetValue('ptr', '0', 'i32') }}};
      case 'i64': return {{{ makeGetValue('ptr', '0', 'i64') }}};
      case 'float': return {{{ makeGetValue('ptr', '0', 'float') }}};
      case 'double': return {{{ makeGetValue('ptr', '0', 'double') }}};
      default: abort('invalid type for setValue: ' + type);
    }
#if SAFE_HEAP
  }
#endif
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
#if ASSERTIONS
    assert(type, 'Must know what type to store in allocate!');
#endif

#if USE_TYPED_ARRAYS == 2
    if (type == 'i64') type = 'i32'; // special case: we have one i32 here, and one i32 later
#endif

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
#if ASSERTIONS
  assert(i < TOTAL_MEMORY);
#endif
    t = {{{ makeGetValue('ptr', 'i', 'i8', 0, 1) }}};
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

var FUNCTION_TABLE; // XXX: In theory the indexes here can be equal to pointers to stacked or malloced memory. Such comparisons should
                    //      be false, but can turn out true. We should probably set the top bit to prevent such issues.

var PAGE_SIZE = 4096;
function alignMemoryPage(x) {
  return ((x+4095)>>12)<<12;
}

var HEAP;
#if USE_TYPED_ARRAYS == 1
var IHEAP, IHEAPU;
#if USE_FHEAP
var FHEAP;
#endif
#endif
#if USE_TYPED_ARRAYS == 2
var HEAP8, HEAPU8, HEAP16, HEAPU16, HEAP32, HEAPU32, HEAPF32, HEAPF64;
#endif

var STACK_ROOT, STACKTOP, STACK_MAX;
var STATICTOP;
#if USE_TYPED_ARRAYS
function enlargeMemory() {
#if ALLOW_MEMORY_GROWTH == 0
  abort('Cannot enlarge memory arrays. Either (1) compile with -s TOTAL_MEMORY=X with X higher than the current value ( ' + TOTAL_MEMORY + '), (2) compile with ALLOW_MEMORY_GROWTH which adjusts the size at runtime but prevents some optimizations, or (3) set Module.TOTAL_MEMORY before the program runs.');
#else
  // TOTAL_MEMORY is the current size of the actual array, and STATICTOP is the new top.
#if ASSERTIONS
  Module.printErr('Warning: Enlarging memory arrays, this is not fast, and ALLOW_MEMORY_GROWTH is not fully tested with all optimizations on! ' + [STATICTOP, TOTAL_MEMORY]); // We perform safe elimination instead of elimination in this mode, but if you see this error, try to disable it and other optimizations entirely
  assert(STATICTOP >= TOTAL_MEMORY);
  assert(TOTAL_MEMORY > 4); // So the loop below will not be infinite
#endif
  while (TOTAL_MEMORY <= STATICTOP) { // Simple heuristic. Override enlargeMemory() if your program has something more optimal for it
    TOTAL_MEMORY = alignMemoryPage(2*TOTAL_MEMORY);
  }
#if USE_TYPED_ARRAYS == 1
  var oldIHEAP = IHEAP;
  Module['HEAP'] = Module['IHEAP'] = HEAP = IHEAP = new Int32Array(TOTAL_MEMORY);
  IHEAP.set(oldIHEAP);
  IHEAPU = new Uint32Array(IHEAP.buffer);
#if USE_FHEAP
  var oldFHEAP = FHEAP;
  Module['FHEAP'] = FHEAP = new Float64Array(TOTAL_MEMORY);
  FHEAP.set(oldFHEAP);
#endif
#endif
#if USE_TYPED_ARRAYS == 2
  var oldHEAP8 = HEAP8;
  var buffer = new ArrayBuffer(TOTAL_MEMORY);
  Module['HEAP8'] = HEAP8 = new Int8Array(buffer);
  Module['HEAP16'] = HEAP16 = new Int16Array(buffer);
  Module['HEAP32'] = HEAP32 = new Int32Array(buffer);
  Module['HEAPU8'] = HEAPU8 = new Uint8Array(buffer);
  Module['HEAPU16'] = HEAPU16 = new Uint16Array(buffer);
  Module['HEAPU32'] = HEAPU32 = new Uint32Array(buffer);
  Module['HEAPF32'] = HEAPF32 = new Float32Array(buffer);
  Module['HEAPF64'] = HEAPF64 = new Float64Array(buffer);
  HEAP8.set(oldHEAP8);
#endif
#endif
}
#endif

var TOTAL_STACK = Module['TOTAL_STACK'] || {{{ TOTAL_STACK }}};
var TOTAL_MEMORY = Module['TOTAL_MEMORY'] || {{{ TOTAL_MEMORY }}};
var FAST_MEMORY = Module['FAST_MEMORY'] || {{{ FAST_MEMORY }}};

// Initialize the runtime's memory
#if USE_TYPED_ARRAYS
// check for full engine support (use string 'subarray' to avoid closure compiler confusion)
  assert(!!Int32Array && !!Float64Array && !!(new Int32Array(1)['subarray']) && !!(new Int32Array(1)['set']),
         'Cannot fallback to non-typed array case: Code is too specialized');

#if USE_TYPED_ARRAYS == 1
  HEAP = IHEAP = new Int32Array(TOTAL_MEMORY);
  IHEAPU = new Uint32Array(IHEAP.buffer);
#if USE_FHEAP
  FHEAP = new Float64Array(TOTAL_MEMORY);
#endif
#endif
#if USE_TYPED_ARRAYS == 2
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
#endif
#else
  // Make sure that our HEAP is implemented as a flat array.
  HEAP = []; // Hinting at the size with |new Array(TOTAL_MEMORY)| should help in theory but makes v8 much slower
  for (var i = 0; i < FAST_MEMORY; i++) {
    HEAP[i] = 0; // XXX We do *not* use {{| makeSetValue(0, 'i', 0, 'null') |}} here, since this is done just to optimize runtime speed
  }
#endif

Module['HEAP'] = HEAP;
#if USE_TYPED_ARRAYS == 1
Module['IHEAP'] = IHEAP;
#if USE_FHEAP
Module['FHEAP'] = FHEAP;
#endif
#endif
#if USE_TYPED_ARRAYS == 2
Module['HEAP8'] = HEAP8;
Module['HEAP16'] = HEAP16;
Module['HEAP32'] = HEAP32;
Module['HEAPU8'] = HEAPU8;
Module['HEAPU16'] = HEAPU16;
Module['HEAPU32'] = HEAPU32;
Module['HEAPF32'] = HEAPF32;
Module['HEAPF64'] = HEAPF64;
#endif

STACK_ROOT = STACKTOP = Runtime.alignMemory(1);
STACK_MAX = STACK_ROOT + TOTAL_STACK;

#if USE_TYPED_ARRAYS == 2
var tempDoublePtr = Runtime.alignMemory(STACK_MAX, 8);
var tempDoubleI8  = HEAP8.subarray(tempDoublePtr);
var tempDoubleI32 = HEAP32.subarray(tempDoublePtr >> 2);
var tempDoubleF32 = HEAPF32.subarray(tempDoublePtr >> 2);
var tempDoubleF64 = HEAPF64.subarray(tempDoublePtr >> 3);
function copyTempFloat(ptr) { // functions, because inlining this code is increases code size too much
  tempDoubleI8[0] = HEAP8[ptr];
  tempDoubleI8[1] = HEAP8[ptr+1];
  tempDoubleI8[2] = HEAP8[ptr+2];
  tempDoubleI8[3] = HEAP8[ptr+3];
}
function copyTempDouble(ptr) {
  tempDoubleI8[0] = HEAP8[ptr];
  tempDoubleI8[1] = HEAP8[ptr+1];
  tempDoubleI8[2] = HEAP8[ptr+2];
  tempDoubleI8[3] = HEAP8[ptr+3];
  tempDoubleI8[4] = HEAP8[ptr+4];
  tempDoubleI8[5] = HEAP8[ptr+5];
  tempDoubleI8[6] = HEAP8[ptr+6];
  tempDoubleI8[7] = HEAP8[ptr+7];
}
STACK_MAX = tempDoublePtr + 8;
#endif

STATICTOP = alignMemoryPage(STACK_MAX);

assert(STATICTOP < TOTAL_MEMORY); // Stack must fit in TOTAL_MEMORY; allocations from here on may enlarge TOTAL_MEMORY

var nullString = allocate(intArrayFromString('(null)'), 'i8', ALLOC_STATIC);

function callRuntimeCallbacks(callbacks) {
  while(callbacks.length > 0) {
    var callback = callbacks.shift();
    var func = callback.func;
    if (typeof func === 'number') {
      func = FUNCTION_TABLE[func];
    }
    func(callback.arg === undefined ? null : callback.arg);
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
  while ({{{ makeGetValue('i++', '0', 'i8') }}}) { // Note: should be |!= 0|, technically. But this helps catch bugs with undefineds
#if ASSERTIONS
  assert(i < TOTAL_MEMORY);
#endif
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
#if ASSERTIONS
        assert(false, 'Character code ' + chr + ' (' + String.fromCharCode(chr) + ')  at offset ' + i + ' not in 0x00-0xFF.');
#endif
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
    {{{ makeSetValue('buffer', 'i', 'chr', 'i8') }}}
    i = i + 1;
  }
}
Module['writeStringToMemory'] = writeStringToMemory;

function writeArrayToMemory(array, buffer) {
  for (var i = 0; i < array.length; i++) {
    {{{ makeSetValue('buffer', 'i', 'array[i]', 'i8') }}};
  }
}
Module['writeArrayToMemory'] = writeArrayToMemory;

{{{ unSign }}}
{{{ reSign }}}

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
    if (!calledRun) run();
  }
}
Module['removeRunDependency'] = removeRunDependency;

Module["preloadedImages"] = {}; // maps url to image data
Module["preloadedAudios"] = {}; // maps url to audio data

// === Body ===

