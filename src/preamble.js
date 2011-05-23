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
  HEAP_HISTORY[dest] = [];
}
var SAFE_HEAP_ERRORS = 0;
var ACCEPTABLE_SAFE_HEAP_ERRORS = 0;

function SAFE_HEAP_ACCESS(dest, type, store, ignore) {
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
function SAFE_HEAP_STORE(dest, value, type, ignore) {
#if SAFE_HEAP_LOG
  print('store: ' + dest + ' [' + type + '] |' + value + '|');
#endif
  if (!value && value !== 0 && value !== false) { // false can be the result of a mathop comparator
    throw('Warning: Writing an invalid value of ' + JSON.stringify(value) + ' at ' + dest + ' :: ' + new Error().stack + '\n');
  }
  SAFE_HEAP_ACCESS(dest, type, true, ignore);
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
function SAFE_HEAP_LOAD(dest, type, ignore) {
  SAFE_HEAP_ACCESS(dest, type, ignore);
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
function SAFE_HEAP_COPY_HISTORY(dest, src) {
  HEAP_HISTORY[dest] = HEAP_HISTORY[src];
  SAFE_HEAP_ACCESS(dest, HEAP_HISTORY[dest] || null, true, false);
#if SAFE_HEAP_LOG
  print('copy history: ' + dest + ' [' + HEAP_HISTORY[dest] + '] from ' + src);
#endif
}
function __Z16PROTECT_HEAPADDRPv(dest) {
  HEAP_WATCHED[dest] = true;
}
function __Z18UNPROTECT_HEAPADDRPv(dest) {
  delete HEAP_WATCHED[dest];
}
//==========================================
#endif

var CorrectionsMonitor = {
  MAX_ALLOWED: 0, // Infinity,
  corrections: 0,
  sigs: {},

  note: function(type) {
    var sig = type + '|' + new Error().stack;
    if (!this.sigs[sig]) {
      print('Correction: ' + sig);
      this.sigs[sig] = 0;
    }
    this.sigs[sig]++;
    this.corrections++;
    if (this.corrections >= this.MAX_ALLOWED) abort('\n\nToo many corrections!');
  }
};

#if CORRECT_ROUNDINGS
function cRound(x) {
  return x >= 0 ? Math.floor(x) : Math.ceil(x);
}
#endif

#if CHECK_OVERFLOWS
//========================================
// Debugging tools - Mathop overflows
//========================================
function CHECK_OVERFLOW(value, bits, ignore) {
  if (ignore) return value;
  if (value === Infinity || value === -Infinity || Math.abs(value) >= Math.pow(2, bits)) {
    CorrectionsMonitor.note('Overflow');
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

//========================================
// Runtime essentials
//========================================

function __globalConstructor__() {
}

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
#if ASSERTIONS
  for (i = 0; i < size; i++) {
    if (slab[i] === undefined) {
      throw 'Invalid element in slab at ' + new Error().stack; // This can be caught, and you can try again to allocate later, see globalFuncs in run()
    }
  }
#endif

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

var HAS_TYPED_ARRAYS = false;
var TOTAL_MEMORY = 50*1024*1024;

function __initializeRuntime__() {
#if USE_TYPED_ARRAYS
  // TODO: Remove one of the 3 heaps!
  HAS_TYPED_ARRAYS = false;
  try {
    HAS_TYPED_ARRAYS = !!Int32Array && !!Float64Array && !!(new Int32Array()['subarray']); // check for full engine support (use string 'subarray' to avoid closure compiler confusion)
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
      IHEAP[i] = 0; // We do *not* use {{{ makeSetValue(0, 'i', 0, 'null') }}} here, since this is done just to optimize runtime speed
    }
  }

  var base = intArrayFromString('(null)'); // So printing %s of NULL gives '(null)'
                                           // Also this ensures we leave 0 as an invalid address, 'NULL'
  for (var i = 0; i < base.length; i++) {
    {{{ makeSetValue(0, 'i', 'base[i]', 'i8') }}}
  }

  Module['HEAP'] = HEAP;
  Module['IHEAP'] = IHEAP;
  Module['FHEAP'] = FHEAP;

  STACK_ROOT = STACKTOP = alignMemoryPage(10);
  var TOTAL_STACK = 1024*1024; // XXX: Changing this value can lead to bad perf on v8!
  STACK_MAX = STACK_ROOT + TOTAL_STACK;

  STATICTOP = alignMemoryPage(STACK_MAX);
}

function __shutdownRuntime__() {
  while( __ATEXIT__.length > 0) {
    var atexit = __ATEXIT__.pop();
    var func = atexit.func;
    if (typeof func === 'number') {
      func = FUNCTION_TABLE[func];
    }
    func(atexit.arg);
  }
  //HEAP = IHEAP = FHEAP = null; // allow browser to GC?
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
  var len = String_len(ptr);
  if (addZero) len++;
  var ret = Array_copy(ptr, len);
  if (addZero) ret[len-1] = 0;
  return ret;
}

// Tools

var PRINTBUFFER = '';
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

// This processes a JS string into a C-line array of numbers, 0-terminated.
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

var unSign = {{{ unSign.toString() }}}
var reSign = {{{ reSign.toString() }}}

// Use console read if available, otherwise we are in a browser, use an XHR
try {
  read;
} catch(e) {
  this['read'] = function(url) {
    // TODO: use mozResponseArrayBuffer/responseStream/etc. if available
    var xhr = new XMLHttpRequest();
    xhr.open("GET", url, false);
    xhr.overrideMimeType('text/plain; charset=x-user-defined'); // ask for binary data
    xhr.send(null);
    if (xhr.status != 200 && xhr.status != 0) throw 'failed to open: ' + url;
    return xhr.responseText;
  }
}

function readBinary(filename) {
  var stringy = read(filename);
  var data = new Array(stringy.length+1);
  for (var i = 0; i < stringy.length; i++) {
    data[i] = stringy.charCodeAt(i) & 0xff;
  }
  data[stringy.length] = 0;
  return data;
}

// === Body ===

