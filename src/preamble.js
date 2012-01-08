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
  print('SAFE_HEAP clear: ' + dest);
#endif
  HEAP_HISTORY[dest] = [];
}
var SAFE_HEAP_ERRORS = 0;
var ACCEPTABLE_SAFE_HEAP_ERRORS = 0;

function SAFE_HEAP_ACCESS(dest, type, store, ignore) {
  //if (dest === A_NUMBER) print ([dest, type, store] + ' ' + new Error().stack); // Something like this may be useful, in debugging

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
  if (type && type[type.length-1] == '*') type = 'i32'; // pointers are ints, for our purposes here
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
      print('Load-store consistency assumption failure! ' + dest);
      print('\n');
      print(JSON.stringify(history));
      print('\n');
      print('LOAD: ' + type + ', ' + new Error().stack);
      print('\n');
      SAFE_HEAP_ERRORS++;
      assert(SAFE_HEAP_ERRORS <= ACCEPTABLE_SAFE_HEAP_ERRORS, 'Load-store consistency assumption failure!');
    }
  }
}
#if USE_TYPED_ARRAYS == 2
var warned64 = false;
function warn64() {
  if (!warned64) {
    __ATEXIT__.push({ func: function() {
      print('Warning: using a 64-bit type with USE_TYPED_ARRAYS == 2. Depending on I64_MODE this may be problematic.');
    } });
    warned64 = true;
  }
}
#endif

function SAFE_HEAP_STORE(dest, value, type, ignore) {
#if SAFE_HEAP_LOG
  print('SAFE_HEAP store: ' + [dest, type, value, ignore]);
#endif

  if (!ignore && !value && value !== 0 && value !== false && !isNaN(value)) { // false can be the result of a mathop comparator; NaN can be the result of a math function
    throw('Warning: Writing an invalid value of ' + JSON.stringify(value) + ' at ' + dest + ' :: ' + new Error().stack + '\n');
  }
  SAFE_HEAP_ACCESS(dest, type, true, ignore);
  if (dest in HEAP_WATCHED) {
    print((new Error()).stack);
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
    case 'double': assert(dest % 4 == 0); warn64(); break;
#endif
  }
#endif

  setValue(dest, value, type, 1);
}

function SAFE_HEAP_LOAD(dest, type, unsigned, ignore) {
  SAFE_HEAP_ACCESS(dest, type, ignore);

#if SAFE_HEAP_LOG
    print('SAFE_HEAP load: ' + [dest, type, getValue(dest, type, 1), ignore]);
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
    case 'double': assert(dest % 4 == 0); warn64(); break;
#endif
  }
#endif

  var ret = getValue(dest, type, 1);
  if (unsigned) ret = unSign(ret, parseInt(type.substr(1)), 1);
  return ret;
}

function SAFE_HEAP_COPY_HISTORY(dest, src) {
#if SAFE_HEAP_LOG
  print('SAFE_HEAP copy: ' + [dest, src]);
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
      //print('Correction: ' + sig);
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
      print(item.sig + ' : ' + item.total + ' hits, %' + (Math.ceil(100*item.fails/item.total)) + ' failures');
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
    print(indent + ('________' + node.time).substr(-8) + ': ' + name_ + ' (' + node.calls + ')');
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

function printXULProfiling() {
  function dumpData(name_, node, indent) {
    var children = [];
    for (var child in node.children) {
      children.push(node.children[child]);
      children[children.length-1].name_ = child;
    }
    print('<treeitem' + (children.length > 0 ? ' container="true"' : '') + '>');
    print('  <treerow>');
    print('    <treecell label="' + name_ + '"/>');
    print('    <treecell label="' + node.time + '"/>');
    print('    <treecell label="' + node.calls + '"/>');
    print('  </treerow>');

    if (children.length > 0) {
      print('  <treechildren>');
      children.sort(function(x, y) { return y.time - x.time });
      children.forEach(function(child) { dumpData(child.name_, child, indent + '  ') });
      print('  </treechildren>');
    }

    print('</treeitem>');
  }

  print('<?xml version="1.0"?>');
  print('<?xml-stylesheet href="chrome://global/skin/" type="text/css"?>  ');
  print('<?xml-stylesheet href="file://C:/main.css" type="text/css"?>        ');
  print('<window xmlns="http://www.mozilla.org/keymaster/gatekeeper/there.is.only.xul">  ');
  print('<tree id="myTree" flex="1" hidecolumnpicker="false" seltype="single" class="tree"');
  print('      rows="5">');
  print('  <treecols id="myTree2-treeCols">');
  print('    <treecol id="myTree2-treeCol0" primary="true" flex="2" label="Name"');
  print('             persist="width" ordinal="1"/>');
  print('    <splitter class="tree-splitter" ordinal="2"/>');
  print('    <treecol id="myTree2-treeCol1" flex="1" label="Milliseconds"');
  print('             persist="width" ordinal="3"/>');
  print('    <treecol id="myTree2-treeCol2" flex="1" label="Calls"');
  print('             persist="width" ordinal="4"/>');
  print('  </treecols>');
  print('  <treechildren>');

  dumpData('root', PROFILING_ROOT, ' ');

  print('  </treechildren>');
  print('</tree>');
  print('</window>');

  // This requires    dom.allow_XUL_XBL_for_file
}
Module['printXULProfiling'] = printXULProfiling;
#endif

//========================================
// Runtime essentials
//========================================

var __globalConstructor__ = function globalConstructor() {
};

var __THREW__ = false; // Used in checking for thrown exceptions.

var __ATEXIT__ = [];

var ABORT = false;

var undef = 0;
// tempInt is used for 32-bit signed values or smaller. tempBigInt is used
// for 32-bit unsigned values or more than 32 bits. TODO: audit all uses of tempInt
var tempValue, tempInt, tempBigInt, tempInt2, tempBigInt2, tempPair, tempBigIntI, tempBigIntR, tempBigIntS, tempBigIntP, tempBigIntD;
#if I64_MODE == 1
var tempI64, tempI64b;
#endif
#if DOUBLE_MODE == 1
#if USE_TYPED_ARRAYS == 2
var tempDoubleBuffer = new ArrayBuffer(8);
var tempDoubleI32 = new Int32Array(tempDoubleBuffer);
var tempDoubleF64 = new Float64Array(tempDoubleBuffer);
#endif
#endif

function abort(text) {
  print(text + ':\n' + (new Error).stack);
  ABORT = true;
  throw "Assertion: " + text;
}

function assert(condition, text) {
  if (!condition) {
    abort('Assertion failed: ' + text);
  }
}

// Sets a value in memory in a dynamic way at run-time. Uses the
// type data. This is the same as makeSetValue, except that
// makeSetValue is done at compile-time and generates the needed
// code then, whereas this function picks the right code at
// run-time.

function setValue(ptr, value, type, noSafe) {
  type = type || 'i8';
  if (type[type.length-1] === '*') type = 'i32'; // pointers are 32-bit
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
  if (type[type.length-1] === '*') type = 'i32'; // pointers are 32-bit
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

// Allocates memory for some data and initializes it properly.

var ALLOC_NORMAL = 0; // Tries to use _malloc()
var ALLOC_STACK = 1; // Lives for the duration of the current function call
var ALLOC_STATIC = 2; // Cannot be freed
Module['ALLOC_NORMAL'] = ALLOC_NORMAL;
Module['ALLOC_STACK'] = ALLOC_STACK;
Module['ALLOC_STATIC'] = ALLOC_STATIC;

function allocate(slab, types, allocator) {
  var zeroinit, size;
  if (typeof slab === 'number') {
    zeroinit = true;
    size = slab;
  } else {
    zeroinit = false;
    size = slab.length;
  }

  var singleType = typeof types === 'string' ? types : null;

  var ret = [_malloc, Runtime.stackAlloc, Runtime.staticAlloc][allocator === undefined ? ALLOC_STATIC : allocator](Math.max(size, singleType ? 1 : types.length));

  var i = 0, type;
  while (i < size) {
    var curr = zeroinit ? 0 : slab[i];

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

#if I64_MODE == 1
    if (type == 'i64') type = 'i32'; // special case: we have one i32 here, and one i32 later
#endif

    setValue(ret+i, curr, type);
    i += Runtime.getNativeTypeSize(type);
  }

  return ret;
}
Module['allocate'] = allocate;

function Pointer_stringify(ptr) {
  var ret = "";
  var i = 0;
  var t;
  var nullByte = String.fromCharCode(0);
  while (1) {
    t = String.fromCharCode({{{ makeGetValue('ptr', 'i', 'i8', 0, 1) }}});
    if (t == nullByte) { break; } else {}
    ret += t;
    i += 1;
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
  return Math.ceil(x/PAGE_SIZE)*PAGE_SIZE;
}

var HEAP;
#if USE_TYPED_ARRAYS == 1
var IHEAP, IHEAPU;
#if USE_FHEAP
var FHEAP;
#endif
#endif
#if USE_TYPED_ARRAYS == 2
var HEAP8, HEAPU8, HEAP16, HEAPU16, HEAP32, HEAPU32, HEAPF32;
#endif

var STACK_ROOT, STACKTOP, STACK_MAX;
var STATICTOP;
#if USE_TYPED_ARRAYS
function enlargeMemory() {
  // TOTAL_MEMORY is the current size of the actual array, and STATICTOP is the new top.
#if ASSERTIONS
  printErr('Warning: Enlarging memory arrays, this is not fast! ' + [STATICTOP, TOTAL_MEMORY]);
  assert(STATICTOP >= TOTAL_MEMORY);
  assert(TOTAL_MEMORY > 4); // So the loop below will not be infinite
#endif
  while (TOTAL_MEMORY <= STATICTOP) { // Simple heuristic. Override enlargeMemory() if your program has something more optimal for it
    TOTAL_MEMORY = alignMemoryPage(2*TOTAL_MEMORY);
  }
#if USE_TYPED_ARRAYS == 1
  var oldIHEAP = IHEAP;
  HEAP = IHEAP = new Int32Array(TOTAL_MEMORY);
  IHEAP.set(oldIHEAP);
  IHEAPU = new Uint32Array(IHEAP.buffer);
#if USE_FHEAP
  var oldFHEAP = FHEAP;
  FHEAP = new Float64Array(TOTAL_MEMORY);
  FHEAP.set(oldFHEAP);
#endif
#endif
#if USE_TYPED_ARRAYS == 2
  var oldHEAP8 = HEAP8;
  var buffer = new ArrayBuffer(TOTAL_MEMORY);
  HEAP8 = new Int8Array(buffer);
  HEAP16 = new Int16Array(buffer);
  HEAP32 = new Int32Array(buffer);
  HEAPU8 = new Uint8Array(buffer);
  HEAPU16 = new Uint16Array(buffer);
  HEAPU32 = new Uint32Array(buffer);
  HEAPF32 = new Float32Array(buffer);
  HEAP8.set(oldHEAP8);
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

  // Endianness check (note: assumes compiler arch was little-endian)
  HEAP32[0] = 255;
  assert(HEAPU8[0] === 255 && HEAPU8[3] === 0, 'Typed arrays 2 must be run on a little-endian system');
#endif
#else
  // Make sure that our HEAP is implemented as a flat array.
  HEAP = new Array(TOTAL_MEMORY);
  for (var i = 0; i < FAST_MEMORY; i++) {
    HEAP[i] = 0; // XXX We do *not* use {{| makeSetValue(0, 'i', 0, 'null') |}} here, since this is done just to optimize runtime speed
  }
#endif

var base = intArrayFromString('(null)'); // So printing %s of NULL gives '(null)'
                                         // Also this ensures we leave 0 as an invalid address, 'NULL'
STATICTOP = base.length;
for (var i = 0; i < base.length; i++) {
  {{{ makeSetValue(0, 'i', 'base[i]', 'i8') }}}
}

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
#endif

STACK_ROOT = STACKTOP = alignMemoryPage(10);
STACK_MAX = STACK_ROOT + TOTAL_STACK;

STATICTOP = alignMemoryPage(STACK_MAX);

function __shutdownRuntime__() {
  while(__ATEXIT__.length > 0) {
    var atexit = __ATEXIT__.pop();
    var func = atexit.func;
    if (typeof func === 'number') {
      func = FUNCTION_TABLE[func];
    }
    func(atexit.arg === undefined ? null : atexit.arg);
  }

  // allow browser to GC, set heaps to null?

  // Print summary of correction activity
  CorrectionsMonitor.print();
}


// Copies a list of num items on the HEAP into a
// a normal JavaScript array of numbers
function Array_copy(ptr, num) {
#if USE_TYPED_ARRAYS == 1
  // TODO: In the SAFE_HEAP case, do some reading here, for debugging purposes - currently this is an 'unnoticed read'.
  return Array.prototype.slice.call(IHEAP.subarray(ptr, ptr+num)); // Make a normal array out of the typed 'view'
                                                                   // Consider making a typed array here, for speed?
#endif
#if USE_TYPED_ARRAYS == 2
  return Array.prototype.slice.call(HEAP8.subarray(ptr, ptr+num)); // Make a normal array out of the typed 'view'
                                                                   // Consider making a typed array here, for speed?
#endif
  return HEAP.slice(ptr, ptr+num);
}
Module['Array_copy'] = Array_copy;

function String_len(ptr) {
  var i = 0;
  while ({{{ makeGetValue('ptr', 'i', 'i8') }}}) i++; // Note: should be |!= 0|, technically. But this helps catch bugs with undefineds
  return i;
}
Module['String_len'] = String_len;

// Copies a C-style string, terminated by a zero, from the HEAP into
// a normal JavaScript array of numbers
function String_copy(ptr, addZero) {
  var len = String_len(ptr);
  if (addZero) len++;
  var ret = Array_copy(ptr, len);
  if (addZero) ret[len-1] = 0;
  return ret;
}
Module['String_copy'] = String_copy;

// Tools

// This processes a JS string into a C-line array of numbers, 0-terminated.
// For LLVM-originating strings, see parser.js:parseLLVMString function
function intArrayFromString(stringy, dontAddNull) {
  var ret = [];
  var t;
  var i = 0;
  while (i < stringy.length) {
    var chr = stringy.charCodeAt(i);
    if (chr > 0xFF) {
#if ASSERTIONS
        assert(false, 'Character code ' + chr + ' (' + stringy[i] + ')  at offset ' + i + ' not in 0x00-0xFF.');
#endif
      chr &= 0xFF;
    }
    ret.push(chr);
    i = i + 1;
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

{{{ unSign }}}
{{{ reSign }}}

// === Body ===

