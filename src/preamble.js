// === Auto-generated preamble library stuff ===

//========================================
// Runtime code shared with compiler
//========================================

{{RUNTIME}}

#if SAFE_HEAP
//========================================
// Debugging tools - Heap
//========================================
var HEAP_WATCHED = {};
var HEAP_HISTORY = {};
function SAFE_HEAP_CLEAR(dest) {
  HEAP_HISTORY[dest] = [];
}
var SAFE_HEAP_ERRORS = 0;
var ACCEPTABLE_SAFE_HEAP_ERRORS = 0;

function SAFE_HEAP_COPY_HISTORY(dest, src) {
  HEAP_HISTORY[dest] = HEAP_HISTORY[src];
#if SAFE_HEAP_LOG
  print('copy history: ' + dest + ' [' + HEAP_HISTORY[dest] + '] from ' + src);
#endif
}

function SAFE_HEAP_ACCESS(dest, type, store) {
#if SAFE_HEAP_LOG
  //if (dest === A_NUMBER) print ([dest, type, store] + ' ' + new Error().stack); // Something like this may be useful, in debugging
#endif
  if (type && type[type.length-1] == '*') type = 'i32'; // pointers are ints, for our purposes here
  // Note that this will pass even with unions: You can store X, load X, then store Y and load Y.
  // You cannot, however, do the nonportable act of store X and load Y!
  if (store) {
    HEAP_HISTORY[dest] = type; // [{ type: type, stack: new Error().stack }]; // |stack| is useful for debugging. Also uncomment the lines later down
  } else {
    if (!HEAP[dest] && HEAP[dest] !== 0 && HEAP[dest] !== false) { // false can be the result of a mathop comparator
      throw('Warning: Reading an invalid value at ' + dest + ' :: ' + new Error().stack + '\n');
    }
    if (type === null) return;
    var history = HEAP_HISTORY[dest];
    if (history === null) return;
    assert(history, 'Must have a history for a safe heap load! ' + dest + ':' + type); // Warning - bit fields in C structs cause loads+stores for each store, so
                                                                                       //           they will show up here...
//    assert((history && history[0]) /* || HEAP[dest] === 0 */, "Loading from where there was no store! " + dest + ',' + HEAP[dest] + ',' + type + ', \n\n' + new Error().stack + '\n');
//    if (history[0].type !== type) {
    if (history !== type) {
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
function SAFE_HEAP_STORE(dest, value, type) {
#if SAFE_HEAP_LOG
  print('store: ' + dest + ' [' + type + '] |' + value + '|');
#endif
  if (!value && value !== 0 && value !== false) { // false can be the result of a mathop comparator
    throw('Warning: Writing an invalid value of ' + JSON.stringify(value) + ' at ' + dest + ' :: ' + new Error().stack + '\n');
  }
  SAFE_HEAP_ACCESS(dest, type, true);
  if (dest in HEAP_WATCHED) {
    print((new Error()).stack);
    throw "Bad store!" + dest;
  }
  if (type === null) {
    IHEAP[dest] = value;
    FHEAP[dest] = value;
  } else if (type in Runtime.FLOAT_TYPES) {
    FHEAP[dest] = value;
  } else {
    IHEAP[dest] = value;
  }
}
function SAFE_HEAP_LOAD(dest, type) {
  SAFE_HEAP_ACCESS(dest, type);
  if (type in Runtime.FLOAT_TYPES) {
#if SAFE_HEAP_LOG
  print('load : ' + dest + ' [' + type + '] |' + FHEAP[dest] + '|');
#endif
    return FHEAP[dest];
  } else {
#if SAFE_HEAP_LOG
  print('load : ' + dest + ' [' + type + '] |' + IHEAP[dest] + '|');
#endif
    return IHEAP[dest];
  }
}
function __Z16PROTECT_HEAPADDRPv(dest) {
  HEAP_WATCHED[dest] = true;
}
function __Z18UNPROTECT_HEAPADDRPv(dest) {
  delete HEAP_WATCHED[dest];
}
//==========================================
#endif

#if CHECK_OVERFLOWS
//========================================
// Debugging tools - Mathop overflows
//========================================
function CHECK_OVERFLOW(value, bits) {
  assert(value !== Infinity && value !== -Infinity, 'Infinity!');
  assert(Math.abs(value) < Math.pow(2, bits), 'Overflow!');
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

//========================================
// Runtime essentials
//========================================

function __globalConstructor__() {
}

// Maps ints ==> functions. This lets us pass around ints, which are
// actually pointers to functions, and we convert at call()time
var FUNCTION_TABLE = [];

var __THREW__ = false; // Used in checking for thrown exceptions.

var __ATEXIT__ = [];

var ABORT = false;

var undef = 0;

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

// Creates a pointer for a certain slab and a certain address in that slab.
// If just a slab is given, will allocate room for it and copy it there. In
// other words, do whatever is necessary in order to return a pointer, that
// points to the slab (and possibly position) we are given.

var ALLOC_NORMAL = 0; // Tries to use _malloc()
var ALLOC_STACK = 1; // Lives for the duration of the current function call
var ALLOC_STATIC = 2; // Cannot be freed

function Pointer_make(slab, pos, allocator) {
  pos = pos ? pos : 0;
  assert(pos === 0); // TODO: remove 'pos'
  if (slab === HEAP) return pos;
  var size = slab.length;

  var i;
  for (i = 0; i < size; i++) {
    if (slab[i] === undefined) {
      throw 'Invalid element in slab at ' + new Error().stack; // This can be caught, and you can try again to allocate later, see globalFuncs in run()
    }
  }

  // Finalize
  var ret = [_malloc, Runtime.stackAlloc, Runtime.staticAlloc][allocator ? allocator : ALLOC_STATIC](Math.max(size, 1));

  for (i = 0; i < size; i++) {
    var curr = slab[i];

    if (typeof curr === 'function') {
      curr = Runtime.getFunctionIndex(curr);
    }

    {{{ makeSetValue(0, 'ret+i', 'curr', 'null') }}}
  }

  return ret;
}
Module['Pointer_make'] = Pointer_make;

function Pointer_stringify(ptr) {
  var ret = "";
  var i = 0;
  var t;
  while (1) {
    t = String.fromCharCode({{{ makeGetValue('ptr', 'i', 'i8') }}});
    if (t == "\0") { break; } else {}
    ret += t;
    i += 1;
  }
  return ret;
}

// Memory management

var PAGE_SIZE = 4096;
function alignMemoryPage(x) {
  return Math.ceil(x/PAGE_SIZE)*PAGE_SIZE;
}

var HEAP, IHEAP, FHEAP;
var STACK_ROOT, STACKTOP, STACK_MAX;
var STATICTOP;

// Mangled |new| and |free| (various manglings, for int, long params; new and new[], etc.
var _malloc, _calloc, _free, __Znwj, __Znaj, __Znam, __Znwm, __ZdlPv, __ZdaPv;

var HAS_TYPED_ARRAYS = false;
var TOTAL_MEMORY = 50*1024*1024;

function __initializeRuntime__() {
  // If we don't have malloc/free implemented, use a simple implementation.
  Module['_malloc'] = _malloc = __Znwj = __Znaj = __Znam = __Znwm = Module['_malloc'] ? Module['_malloc'] : Runtime.staticAlloc;
  Module['_calloc'] = _calloc                                     = Module['_calloc'] ? Module['_calloc'] : function(n, s) {
    var ret = _malloc(n*s);
    _memset(ret, 0, n*s);
    return ret;
  };
  Module['_free']   = _free = __ZdlPv = __ZdaPv                   = Module['_free']   ? Module['_free']   : function() { };

#if USE_TYPED_ARRAYS
  // TODO: Remove one of the 3 heaps!
  HAS_TYPED_ARRAYS = false;
  try {
    HAS_TYPED_ARRAYS = !!Int32Array && !!Float64Array && !!(new Int32Array().subarray); // check for full engine support
  } catch(e) {}

  if (HAS_TYPED_ARRAYS) {
    HEAP = IHEAP = new Int32Array(TOTAL_MEMORY);
    FHEAP = new Float64Array(TOTAL_MEMORY);
  } else
#endif
  {
    // Without this optimization, Chrome is slow. Sadly, the constant here needs to be tweaked depending on the code being run...
    var FAST_MEMORY = TOTAL_MEMORY/32;
    IHEAP = FHEAP = HEAP = new Array(FAST_MEMORY);
    for (var i = 0; i < FAST_MEMORY; i++) {
      IHEAP[i] = FHEAP[i] = 0; // We do *not* use {{{ makeSetValue(0, 'i', 0, 'null') }}} here, since this is done just to optimize runtime speed
    }
  }

  var base = intArrayFromString('(null)').concat(0); // So printing %s of NULL gives '(null)'
                                                     // Also this ensures we leave 0 as an invalid address, 'NULL'
  for (var i = 0; i < base.length; i++) {
    {{{ makeSetValue(0, 'i', 'base[i]', 'i8') }}}
  }

  Module['HEAP'] = HEAP;
  Module['IHEAP'] = IHEAP;
  Module['FHEAP'] = FHEAP;

  STACK_ROOT = STACKTOP = alignMemoryPage(10);
  try {
    var x = TOTAL_STACK;
  } catch(e) {
    TOTAL_STACK = 1024*1024; // Reserved room for stack XXX: Changing this value can lead to bad perf on v8!
  }
  STACK_MAX = STACK_ROOT + TOTAL_STACK;

  STATICTOP = alignMemoryPage(STACK_MAX);
}

function __shutdownRuntime__() {
  while( __ATEXIT__.length > 0) {
    var func = __ATEXIT__.pop();
    if (typeof func === 'number') {
      func = FUNCTION_TABLE[func];
    }
    func();
  }
}


// Copies a list of num items on the HEAP into a
// a normal JavaScript array of numbers
function Array_copy(ptr, num) {
  // TODO: In the SAFE_HEAP case, do some reading here, for debugging purposes - currently this is an 'unnoticed read'.
#if USE_TYPED_ARRAYS
  if (HAS_TYPED_ARRAYS) {
    return Array.prototype.slice.call(IHEAP.subarray(ptr, ptr+num)); // Make a normal array out of the typed 'view'
                                                                     // Consider making a typed array here, for speed?
  } else
#endif
  {
    return IHEAP.slice(ptr, ptr+num);
  }
}

function String_len(ptr) {
  var i = 0;
  while ({{{ makeGetValue('ptr', 'i', 'i8') }}}) i++; // Note: should be |!= 0|, technically. But this helps catch bugs with undefineds
  return i;
}

// Copies a C-style string, terminated by a zero, from the HEAP into
// a normal JavaScript array of numbers
function String_copy(ptr, addZero) {
  return Array_copy(ptr, String_len(ptr)).concat(addZero ? [0] : []);
}

// Tools

PRINTBUFFER = '';
function __print__(text) {
  if (text === null) {
    // Flush
    print(PRINTBUFFER);
    PRINTBUFFER = '';
    return;
  }
  // We print only when we see a '\n', as console JS engines always add
  // one anyhow.
  PRINTBUFFER = PRINTBUFFER + text;
  var endIndex;
  while ((endIndex = PRINTBUFFER.indexOf('\n')) != -1) {
    print(PRINTBUFFER.substr(0, endIndex));
    PRINTBUFFER = PRINTBUFFER.substr(endIndex + 1);
  }
}

function jrint(label, obj) { // XXX manual debugging
  if (!obj) {
    obj = label;
    label = '';
  } else
    label = label + ' : ';
  print(label + JSON.stringify(obj));
}

// This processes a 'normal' string into a C-line array of numbers.
// For LLVM-originating strings, see parser.js:parseLLVMString function
function intArrayFromString(stringy) {
  var ret = [];
  var t;
  var i = 0;
  while (i < stringy.length) {
    ret.push(stringy.charCodeAt(i));
    i = i + 1;
  }
  ret.push(0);
  return ret;
}
Module['intArrayFromString'] = intArrayFromString;

function intArrayToString(array) {
  var ret = '';
  for (var i = 0; i < array.length; i++) {
    ret += String.fromCharCode(array[i]);
  }
  return ret;
}

// Converts a value we have as signed, into an unsigned value. For
// example, -1 in int32 would be a very large number as unsigned.
function unSign(value, bits) {
  if (value >= 0) return value;
#if CHECK_SIGNS
  print('WARNING: unSign needed, ' + [value, bits] + ' at ' + new Error().stack);
#endif
  return bits <= 32 ? 2*Math.abs(1 << (bits-1)) + value // Need some trickery, since if bits == 32, we are right at the limit of the bits JS uses in bitshifts
                    : Math.pow(2, bits)         + value;
  // TODO: clean up previous line
}

// Converts a value we have as unsigned, into a signed value. For
// example, 200 in a uint8 would be a negative number.
function reSign(value, bits) {
  if (value <= 0) return value;
  var half = bits <= 32 ? Math.abs(1 << (bits-1)) // abs is needed if bits == 32
                        : Math.pow(2, bits-1);
  if (value >= half) {
#if CHECK_SIGNS
  print('WARNING: reSign needed, ' + [value, bits] + ' at ' + new Error().stack);
#endif
    value = -2*half + value; // Cannot bitshift half, as it may be at the limit of the bits JS uses in bitshifts
  }
  return value;
}

// === Body ===

