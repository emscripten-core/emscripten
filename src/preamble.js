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
  if (type && type[type.length-1] == '*') type = 'i32'; // pointers are ints, for our purposes here
  // Note that this will pass even with unions: You can store X, load X, then store Y and load Y.
  // You cannot, however, do the nonportable act of store X and load Y!
  if (store) {
    HEAP_HISTORY[dest] = ignore ? null : type;
  } else {
#if USE_TYPED_ARRAYS == 0
    if (!HEAP[dest] && HEAP[dest] !== 0 && HEAP[dest] !== false) { // false can be the result of a mathop comparator
      throw('Warning: Reading an invalid value at ' + dest + ' :: ' + new Error().stack + '\n');
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
      print('Warning: using a 64-bit type with USE_TYPED_ARRAYS == 2. This is emulated as a 32-bit value, and will likely fail horribly.');
    } });
    warned64 = true;
  }
}
#endif

function SAFE_HEAP_STORE(dest, value, type, ignore) {
#if SAFE_HEAP_LOG
  print('SAFE_HEAP store: ' + [dest, type, value, ignore]);
#endif

  if (!ignore && !value && value !== 0 && value !== false) { // false can be the result of a mathop comparator
    throw('Warning: Writing an invalid value of ' + JSON.stringify(value) + ' at ' + dest + ' :: ' + new Error().stack + '\n');
  }
  SAFE_HEAP_ACCESS(dest, type, true, ignore);
  if (dest in HEAP_WATCHED) {
    print((new Error()).stack);
    throw "Bad store!" + dest;
  }
#if USE_TYPED_ARRAYS == 1
  if (type === null) {
    IHEAP[dest] = value;
    FHEAP[dest] = value;
  } else if (type in Runtime.FLOAT_TYPES) {
    FHEAP[dest] = value;
  } else {
    IHEAP[dest] = value;
  }
#else
#if USE_TYPED_ARRAYS == 2
  assert(type != 'null', 'typed arrays 2 with null type!');
  if (type[type.length-1] === '*') type = 'i32'; // hardcoded pointers as 32-bit
  switch(type) {
    case 'i1': case 'i8': HEAP8[dest] = value; break;
    case 'i16': assert(dest % 2 === 0, type + ' loads must be aligned'); HEAP16[dest>>1] = value; break;
    case 'i32': assert(dest % 4 === 0, type + ' loads must be aligned'); HEAP32[dest>>2] = value; break;
    case 'i64': assert(dest % 4 === 0, type + ' loads must be aligned'); warn64(); HEAP32[dest>>2] = value; break; // XXX store int64 as int32
    case 'float': assert(dest % 4 === 0, type + ' loads must be aligned'); HEAPF32[dest>>2] = value; break;
    case 'double': assert(dest % 4 === 0, type + ' loads must be aligned'); warn64(); HEAPF32[dest>>2] = value; break; // XXX store doubles as floats
    default: throw 'weird type for typed array II: ' + type + new Error().stack;
  }
#else
  HEAP[dest] = value;
#endif
#endif
}

function SAFE_HEAP_LOAD(dest, type, unsigned, ignore) {
  SAFE_HEAP_ACCESS(dest, type, ignore);

#if USE_TYPED_ARRAYS == 1
  if (type in Runtime.FLOAT_TYPES) {
#if SAFE_HEAP_LOG
    print('SAFE_HEAP load: ' + [dest, type, FHEAP[dest], ignore]);
#endif
    return FHEAP[dest];
  } else {
#if SAFE_HEAP_LOG
    print('SAFE_HEAP load: ' + [dest, type, IHEAP[dest], ignore]);
#endif
    return IHEAP[dest];
  }
#else
#if USE_TYPED_ARRAYS == 2
#if SAFE_HEAP_LOG
  var originalType = type;
#endif
  var ret;
  if (type[type.length-1] === '*') type = 'i32'; // hardcoded pointers as 32-bit
  switch(type) {
    case 'i1': case 'i8': {
      ret = (unsigned ? HEAPU8 : HEAP8)[dest];
      break;
    }
    case 'i16': {
      assert(dest % 2 === 0, type + ' loads must be aligned');
      ret = (unsigned ? HEAPU16 : HEAP16)[dest>>1];
      break;
    }
    case 'i32': case 'i64': { // XXX store int64 as int32
      assert(dest % 4 === 0, type + ' loads must be aligned');
      if (type === 'i64') warn64();
      ret = (unsigned ? HEAPU32 : HEAP32)[dest>>2];
      break;
    }
    case 'float': case 'double': { // XXX store doubles as floats
      assert(dest % 4 === 0, type + ' loads must be aligned');
      if (type === 'double') warn64();
      ret = HEAPF32[dest>>2];
      break;
    }
    default: throw 'weird type for typed array II: ' + type;
  }
#if SAFE_HEAP_LOG
  print('SAFE_HEAP load: ' + [dest, originalType, ret, unsigned, ignore]);
#endif
  return ret;
#else
#if SAFE_HEAP_LOG
  print('SAFE_HEAP load: ' + [dest, type, HEAP[dest], ignore]);
#endif
  return HEAP[dest];
#endif
#endif
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
#if AUTO_OPTIMIZE
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
#if AUTO_OPTIMIZE
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
      print(item.sig + ' : ' + item.total + ' hits, %' + (Math.floor(100*item.fails/item.total)) + ' failures');
    }
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

function Pointer_make(slab, pos, allocator, types) {
  pos = pos ? pos : 0;
  assert(pos === 0); // TODO: remove 'pos'
  if (slab === HEAP) return pos;
  var zeroinit, size;
  if (typeof slab === 'number') {
    zeroinit = true;
    size = slab;
  } else {
    zeroinit = false;
    size = slab.length;
  }

  // Finalize
  var ret = [_malloc, Runtime.stackAlloc, Runtime.staticAlloc][allocator ? allocator : ALLOC_STATIC](Math.max(size, 1));

  var singleType = typeof types === 'string' ? types : null;

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
    assert(type, 'Must know what type to store in Pointer_make!');
#endif

    if (type === 'i1') {
      {{{ makeSetValue(0, 'ret+i', 'curr', 'i1') }}}
      i += {{{ getNativeFieldSize('i1', true) }}};
    } else if (type === 'i8') {
      {{{ makeSetValue(0, 'ret+i', 'curr', 'i8') }}}
      i += {{{ getNativeFieldSize('i8', true) }}};
    } else if (type === 'i16') {
      {{{ makeSetValue(0, 'ret+i', 'curr', 'i16') }}}
      i += {{{ getNativeFieldSize('i16', true) }}};
    } else if (type === 'i32' || type[type.length-1] === '*') { // hardcoded pointers as 32-bit
      {{{ makeSetValue(0, 'ret+i', 'curr', 'i32') }}}
      i += {{{ getNativeFieldSize('i32', true) }}};
    } else if (type === 'float') {
      {{{ makeSetValue(0, 'ret+i', 'curr', 'float') }}}
      i += {{{ getNativeFieldSize('float', true) }}};
    } else if (type === 'i64') {
      {{{ makeSetValue(0, 'ret+i', 'curr', 'i64') }}}
      i += {{{ getNativeFieldSize('i64', true) }}};
    } else if (type === 'double') {
      {{{ makeSetValue(0, 'ret+i', 'curr', 'double') }}}
      i += {{{ getNativeFieldSize('double', true) }}};
    } else {
      abort('invalid type for Pointer_make: ' + type);
    }
  }

  return ret;
}
Module['Pointer_make'] = Pointer_make;

function Pointer_stringify(ptr) {
  var ret = "";
  var i = 0;
  var t;
  while (1) {
    t = String.fromCharCode({{{ makeGetValue('ptr', 'i', 'i8', 0, 1) }}});
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

var HEAP;
#if USE_TYPED_ARRAYS == 1
var IHEAP, FHEAP;
#endif
#if USE_TYPED_ARRAYS == 2
var HEAP8, HEAPU8, HEAP16, HEAPU16, HEAP32, HEAPU32, HEAPF32;
#endif

var STACK_ROOT, STACKTOP, STACK_MAX;
var STATICTOP;

var HAS_TYPED_ARRAYS = false;
var TOTAL_MEMORY = 50*1024*1024;

// Initialize the runtime's memory
#if USE_TYPED_ARRAYS
HAS_TYPED_ARRAYS = false;
try {
  HAS_TYPED_ARRAYS = !!Int32Array && !!Float64Array && !!(new Int32Array(1)['subarray']); // check for full engine support (use string 'subarray' to avoid closure compiler confusion)
} catch(e) {}

if (HAS_TYPED_ARRAYS) {
#if USE_TYPED_ARRAYS == 1
  HEAP = IHEAP = new Int32Array(TOTAL_MEMORY);
  FHEAP = new Float64Array(TOTAL_MEMORY);
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
#endif
} else
#endif
{
  // Without this optimization, Chrome is slow. Sadly, the constant here needs to be tweaked depending on the code being run...
  var FAST_MEMORY = TOTAL_MEMORY/32;
  HEAP = new Array(FAST_MEMORY);
  for (var i = 0; i < FAST_MEMORY; i++) {
    HEAP[i] = 0; // XXX We do *not* use {{| makeSetValue(0, 'i', 0, 'null') |}} here, since this is done just to optimize runtime speed
  }
#if USE_TYPED_ARRAYS == 1
  IHEAP = FHEAP = HEAP;
#endif
#if USE_TYPED_ARRAYS == 2
  abort('Cannot fallback to non-typed array case in USE_TYPED_ARRAYS == 2: Code is too specialized');
#endif
}

var base = intArrayFromString('(null)'); // So printing %s of NULL gives '(null)'
                                         // Also this ensures we leave 0 as an invalid address, 'NULL'
for (var i = 0; i < base.length; i++) {
  {{{ makeSetValue(0, 'i', 'base[i]', 'i8') }}}
}

Module['HEAP'] = HEAP;

STACK_ROOT = STACKTOP = alignMemoryPage(10);
var TOTAL_STACK = 1024*1024; // XXX: Changing this value can lead to bad perf on v8!
STACK_MAX = STACK_ROOT + TOTAL_STACK;

STATICTOP = alignMemoryPage(STACK_MAX);

function __shutdownRuntime__() {
  while(__ATEXIT__.length > 0) {
    var atexit = __ATEXIT__.pop();
    var func = atexit.func;
    if (typeof func === 'number') {
      func = FUNCTION_TABLE[func];
    }
    func(atexit.arg);
  }

  // allow browser to GC, set heaps to null?

  // Print summary of correction activity
  CorrectionsMonitor.print();
}


// Copies a list of num items on the HEAP into a
// a normal JavaScript array of numbers
function Array_copy(ptr, num) {
  // TODO: In the SAFE_HEAP case, do some reading here, for debugging purposes - currently this is an 'unnoticed read'.
#if USE_TYPED_ARRAYS == 1
  if (HAS_TYPED_ARRAYS) {
    return Array.prototype.slice.call(IHEAP.subarray(ptr, ptr+num)); // Make a normal array out of the typed 'view'
                                                                     // Consider making a typed array here, for speed?
  } else {
    return IHEAP.slice(ptr, ptr+num);
  }
#endif
#if USE_TYPED_ARRAYS == 2
  if (HAS_TYPED_ARRAYS) {
    return Array.prototype.slice.call(HEAP8.subarray(ptr, ptr+num)); // Make a normal array out of the typed 'view'
                                                                     // Consider making a typed array here, for speed?
  } else {
    return HEAP8.slice(ptr, ptr+num);
  }
#endif
  return HEAP.slice(ptr, ptr+num);
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
function intArrayFromString(stringy, dontAddNull) {
  var ret = [];
  var t;
  var i = 0;
  while (i < stringy.length) {
    ret.push(stringy.charCodeAt(i));
    i = i + 1;
  }
  if (!dontAddNull) {
    ret.push(0);
  }
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

{{{ unSign }}}
{{{ reSign }}}

// Use console read if available, otherwise we are in a browser, use an XHR
if (!this['read']) {
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
  var data = new Array(stringy.length);
  for (var i = 0; i < stringy.length; i++) {
    data[i] = stringy.charCodeAt(i) & 0xff;
  }
  return data;
}

// === Body ===

