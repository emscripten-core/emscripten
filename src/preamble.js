
// === Auto-generated preamble library stuff ===

function __globalConstructor__() {
}

var __THREW__ = false; // Used in checking for thrown exceptions.

var __ATEXIT__ = [];

#if SAFE_HEAP
// Semi-manual memory corruption debugging
HEAP_WATCHED = {};
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
INDENT = '';
#endif

#if EXECUTION_TIMEOUT
START_TIME = Date.now();
#endif

function assert(condition, text) {
  if (!condition) {
    throw 'Assertion failed: ' + text + ':\n' + (new Error).stack;
  }
}

function Pointer_niceify(ptr) {
  return { slab: HEAP, pos: ptr };
}

// Creates a pointer for a certain slab and a certain address in that slab.
// If just a slab is given, will allocate room for it and copy it there. In
// other words, do whatever is necessary in order to return a pointer, that
// points to the slab (and possibly position) we are given.
function Pointer_make(slab, pos, allocator) {
  pos = pos ? pos : 0;
  if (slab === HEAP) return pos;
  // Flatten out - needed for global consts/vars
  function flatten(slab) {
    if (!slab || slab.length === undefined || typeof slab === 'function') return [slab];
    return slab.map(flatten).reduce(function(a,b) { return a.concat(b) }, []);
  }
  var slab = flatten(slab);
  // Finalize
  var ret = [_malloc, stackAlloc, unfreeableMalloc][allocator ? allocator : ALLOC_UNFREEABLE](Math.max(slab.length - pos, 1));
  for (var i = 0; i < slab.length - pos; i++) {
#if SAFE_HEAP
    SAFE_HEAP_STORE(ret + i, slab[pos + i]);
#else
    HEAP[ret + i] = slab[pos + i];
#endif
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

ALLOC_NORMAL = 0;
ALLOC_STACK = 1;
ALLOC_UNFREEABLE = 2;

// Stack allocation
function stackEnter() {
  STACK_STACK.push(STACKTOP);
}
function stackExit() {
  STACKTOP = STACK_STACK.pop();
}
function stackAlloc(size) {
  size = Math.ceil(size/QUANTUM_SIZE)*QUANTUM_SIZE; // Allocate blocks of proper minimum size
                                                    // Also keeps STACKTOP aligned
  assert(STACKTOP + size < TOTAL_STACK);
  var ret = STACKTOP;
  STACKTOP += size;
  return ret;
}

function unfreeableMalloc(size) {
  size = Math.ceil(size/1)*1; // Allocate blocks of proper minimum size
                              // Also keeps HEAPTOP aligned
  var ret = HEAPTOP;
  HEAPTOP += size;
  return ret;
}

function unfreeableFree(ptr) {
}

// If we don't have malloc/free implemented, use a simple implementation. This
// allows compiled C/C++ to implement its own malloc/free
if (!this._malloc) {
  _malloc = unfreeableMalloc;
  _free = unfreeableFree;
}

// Mangled "new"s... need a heuristic for autogeneration...
__Znwj = _malloc; // llvm-gcc
__Znaj = _malloc; // llvm-gcc
__Znam = _malloc; // clang
__Znwm = _malloc; // clang
// Mangled "delete"s... need a heuristic for autogeneration...
__ZdlPv = _free; // llvm-gcc
__ZdaPv = _free; // llvm-gcc

function __initializeRuntime__() {
  HEAP = intArrayFromString('(null)'); // So printing %s of NULL gives '(null)'
                                       // Also this ensures we leave 0 as an invalid address, 'NULL'
  TOTAL_STACK = 64*1024; // Reserved room for stack
  STACK_STACK = [];
  STACKTOP = HEAP.length;
  HEAPTOP = STACKTOP + TOTAL_STACK;
}

// stdio.h

// C-style: we work on ints on the HEAP.
function __formatString() {
  var textIndex = arguments[0];
  var argIndex = 1;
  var ret = [];
  var curr = -1;
  while (curr != 0) {
    curr = HEAP[textIndex];
    next = HEAP[textIndex+1];
    if (curr == '%'.charCodeAt(0) && ['d', 'u', 'f', '.'].indexOf(String.fromCharCode(next)) != -1) {
      var argText = String(arguments[argIndex]);
      // Handle very very simply formatting, namely only %.Xf
      if (next == '.'.charCodeAt(0)) {
        var limit = parseInt(String.fromCharCode(HEAP[textIndex+2]));
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
    } else {
      ret.push(curr);
      textIndex += 1;
    }
  }
  return Pointer_make(ret); // Leak!
}

// Copies a list of num items on the HEAP into a
// a normal JavaScript array of numbers
function Array_copy(ptr, num) {
  // XXX hardcoded ptr impl
  return HEAP.slice(ptr, ptr+num);
}

// Copies a C-style string, terminated by a zero, from the HEAP into
// a normal JavaScript array of numbers
function String_copy(ptr, addZero) {
  // XXX hardcoded ptr impl
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
#endif
  }
//  dest = Pointer_niceify(dest);
//  src = Pointer_niceify(src);
//  dest.slab = src.slab.slice(src.pos);
}
_llvm_memcpy_i64 = _llvm_memcpy_i32;

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

