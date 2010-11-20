// === Auto-generated preamble library stuff ===

{{RUNTIME}}

function __globalConstructor__() {
}

// Maps ints ==> functions. This lets us pass around ints, which are
// actually pointers to functions, and we convert at call()time
var FUNCTION_TABLE = [];

var __THREW__ = false; // Used in checking for thrown exceptions.

var __ATEXIT__ = [];

#if SAFE_HEAP
// Semi-manual memory corruption debugging
var HEAP_WATCHED = {};
function SAFE_HEAP_STORE(dest, value) {
  if (dest in HEAP_WATCHED) {
    print((new Error()).stack);
    throw "Bad store!" + dest;
  }
  HEAP[dest] = value;
}
function __Z16PROTECT_HEAPADDRPv(dest) {
  HEAP_WATCHED[dest] = true;
}
function __Z18UNPROTECT_HEAPADDRPv(dest) {
  delete HEAP_WATCHED[dest];
}
//==========================================
#endif

#if LABEL_DEBUG
var INDENT = '';
#endif

#if EXECUTION_TIMEOUT
var START_TIME = Date.now();
#endif

var ABORT = false;

var undef = 0;

function assert(condition, text) {
  if (!condition) {
    var text = "Assertion failed: " + text;
    print(text + ':\n' + (new Error).stack);
    ABORT = true;
    throw "Assertion: " + text;
  }
}

function Pointer_niceify(ptr) {
  return { slab: IHEAP, pos: ptr };
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
  var size = 0;

  // The slab may contain arrays, which we basically need to 'flatten' out
  // into one long slab. We do that by traversing it, and not by creating
  // a new slab, to save time and memory
  var stack = [[slab, 0]], top, curr;
  while(1) {
    top = stack[stack.length-1];
    if (top[1] >= top[0].length) {
      stack.pop();
      if (stack.length === 0) break;
      top = stack[stack.length-1];
      top[1]++;
      continue;
    }
    var curr = top[0][top[1]];
    if (curr === undefined)
      throw 'Invalid element in slab'; // This can be caught, and you can try again to allocate later, see globalFuncs in run()
    if (curr.length) {
      stack.push([curr,0]);
      continue;
    }
    size++;
    top[1]++;
  }

  // Finalize
  var ret = [_malloc, Runtime.stackAlloc, Runtime.staticAlloc][allocator ? allocator : ALLOC_STATIC](Math.max(size, 1));

  stack = [[slab, 0]];
  var i = 0;
  while(1) {
    top = stack[stack.length-1];
    if (top[1] >= top[0].length) {
      stack.pop();
      if (stack.length === 0) break;
      top = stack[stack.length-1];
      top[1]++;
      continue;
    }
    var curr = top[0][top[1]];
    if (curr.length) {
      stack.push([curr,0]);
      continue;
    }

    if (typeof curr === 'function') {
      curr = Runtime.getFunctionIndex(curr);
    }
#if SAFE_HEAP
    SAFE_HEAP_STORE(ret + i, curr);
#else
#if USE_TYPED_ARRAYS
    // TODO: Check - also in non-typedarray case - for functions, and if so add |.__index__|
    if (typeof curr === 'number' || typeof curr === 'boolean') {
      IHEAP[ret + i] = curr; // TODO: optimize. Can easily detect floats, but 1.0 might look like an int...
      FHEAP[ret + i] = curr;
    } else {
      HEAP[ret + i] = curr;
    }
#else
    HEAP[ret + i] = curr;
#endif
#endif

    top[1]++;
    i++;
  }

  return ret;
}

function Pointer_stringify(ptr) {
  ptr = Pointer_niceify(ptr);

  var ret = "";
  var i = 0;
  var t;
  while (1) {
//    if ((ptr.pos + i) >= ptr.slab.length) { return "<< Invalid read: " + (ptr.pos+i) + " : " + ptr.slab.length + " >>"; } else {}
    if ((ptr.pos+i) >= ptr.slab.length) { break; } else {}
    t = String.fromCharCode(ptr.slab[ptr.pos + i]);
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
var _malloc, _free, __Znwj, __Znaj, __Znam, __Znwm, __ZdlPv, __ZdaPv;

function __initializeRuntime__() {
  // If we don't have malloc/free implemented, use a simple implementation.
  Module['_malloc'] = _malloc = __Znwj = __Znaj = __Znam = __Znwm = Module['_malloc'] ? Module['_malloc'] : Runtime.staticAlloc;
  Module['_free']   = _free = __ZdlPv = __ZdaPv =                   Module['_free']   ? Module['_free']   : function() { };

  HEAP = intArrayFromString('(null)'); // So printing %s of NULL gives '(null)'
                                       // Also this ensures we leave 0 as an invalid address, 'NULL'
#if USE_TYPED_ARRAYS
  if (!this['TOTAL_MEMORY']) TOTAL_MEMORY = 50*1024*1024;
  if (this['Int32Array']) { // check for engine support
    IHEAP = new Int32Array(TOTAL_MEMORY);
    for (var i = 0; i < HEAP.length; i++) {
      IHEAP[i] = HEAP[i];
    }
  } else {
    IHEAP = HEAP; // fallback
  }
  if (this['Float64Array']) { // check for engine support
    FHEAP = new Float64Array(TOTAL_MEMORY);
  } else {
    FHEAP = HEAP; // fallback
  }
#else
  IHEAP = HEAP; // We use that name in our runtime code that processes strings etc., see library.js
#endif

  Module['HEAP'] = HEAP;
  Module['IHEAP'] = IHEAP;
  Module['FHEAP'] = FHEAP;

  STACK_ROOT = STACKTOP = alignMemoryPage(10);
  if (!this['TOTAL_STACK']) TOTAL_STACK = 1024*1024; // Reserved room for stack
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

// stdio.h

// C-style: we work on ints on the HEAP.
function __formatString() {
  var textIndex = arguments[0];
  var argIndex = 1;
  var ret = [];
  var curr = -1;
  while (curr) { // Note: should be curr != 0, technically. But this helps catch bugs with undefineds
#if USE_TYPED_ARRAYS
    curr = IHEAP[textIndex];
    next = IHEAP[textIndex+1];
#else
    curr = HEAP[textIndex];
    next = HEAP[textIndex+1];
#endif
    if (curr == '%'.charCodeAt(0) && ['d', 'u', 'f', '.'].indexOf(String.fromCharCode(next)) != -1) {
      var argText = String(+arguments[argIndex]); // +: boolean=>int
      // Handle very very simply formatting, namely only %.Xf
      if (next == '.'.charCodeAt(0)) {
#if USE_TYPED_ARRAYS
        var limit = parseInt(String.fromCharCode(IHEAP[textIndex+2]));
#else
        var limit = parseInt(String.fromCharCode(HEAP[textIndex+2]));
#endif
        var dotIndex = argText.indexOf('.');
        if (dotIndex == -1) {
          dotIndex = argText.length;
          argText += '.';
        }
        argText += '00000000000'; // padding
        argText = argText.substr(0, dotIndex+1+limit);
        textIndex += 2;
      } else if (next == 'u'.charCodeAt(0)) {
        argText = String(unSign(arguments[argIndex], 32));
      }
      argText.split('').forEach(function(chr) {
        ret.push(chr.charCodeAt(0));
      });
      argIndex += 1;
      textIndex += 2;
    } else if (curr == '%'.charCodeAt(0) && next == 's'.charCodeAt(0)) {
      ret = ret.concat(String_copy(arguments[argIndex]));
      argIndex += 1;
      textIndex += 2;
    } else if (curr == '%'.charCodeAt(0) && next == 'c'.charCodeAt(0)) {
      ret = ret.concat(arguments[argIndex]);
      argIndex += 1;
      textIndex += 2;
    } else {
      ret.push(curr);
      textIndex += 1;
    }
  }
  return Pointer_make(ret, 0, ALLOC_STACK); // NB: Stored on the stack
}

// Copies a list of num items on the HEAP into a
// a normal JavaScript array of numbers
function Array_copy(ptr, num) {
#if USE_TYPED_ARRAYS
  return Array.prototype.slice.call(IHEAP.slice(ptr, ptr+num)); // Make a normal array out of the typed one
#else
  return IHEAP.slice(ptr, ptr+num);
#endif
}

// Copies a C-style string, terminated by a zero, from the HEAP into
// a normal JavaScript array of numbers
function String_copy(ptr, addZero) {
  return Array_copy(ptr, _strlen(ptr)).concat(addZero ? [0] : []);
}

// stdlib.h

// Get a pointer, return int value of the string it points to
function _atoi(s) {
  return Math.floor(Number(Pointer_stringify(s)));
}

function _llvm_memcpy_i32(dest, src, num, idunno) {
// XXX hardcoded ptr impl
  for (var i = 0; i < num; i++) {
#if SAFE_HEAP
    SAFE_HEAP_STORE(dest + i, HEAP[src + i]);
#else
    HEAP[dest + i] = HEAP[src + i];
#if USE_TYPED_ARRAYS
    // TODO: optimize somehow - this is slower than without typed arrays
    IHEAP[dest + i] = IHEAP[src + i];
    FHEAP[dest + i] = FHEAP[src + i];
#endif
#endif
  }
//  dest = Pointer_niceify(dest);
//  src = Pointer_niceify(src);
//  dest.slab = src.slab.slice(src.pos);
}
_llvm_memcpy_i64 = _llvm_memcpy_i32;
_llvm_memcpy_p0i8_p0i8_i32 = _llvm_memcpy_i32;

function llvm_memset_i32(ptr, value, num) {
  for (var i = 0; i < num; i++) {
#if USE_TYPED_ARRAYS
    HEAP[ptr+i] = IHEAP[ptr+i] = FHEAP[ptr+i] = value;
#else
    HEAP[ptr+i] = value;
#endif
  }
}
_llvm_memset_p0i8_i64 = _llvm_memset_p0i8_i32 = llvm_memset_i32;

function _strlen(ptr) {
  var i = 0;
  while (IHEAP[ptr+i]) i++; // Note: should be IHEAP[ptr+i] != 0, technically. But this helps catch bugs with undefineds
  return i;
}

// Tools

PRINTBUFFER = '';
function __print__(text) {
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

// Converts a value we have as signed, into an unsigned value. For
// example, -1 in int32 would be a very large number as unsigned.
function unSign(value, bits) {
  if (value >= 0) return value;
  return 2*Math.abs(1 << (bits-1)) + value;
}

// === Body ===

