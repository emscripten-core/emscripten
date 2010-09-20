
// === Auto-generated preamble library stuff ===

function __globalConstructor__() {
}

var __THREW__ = false; // Used in checking for thrown exceptions.

var __ATEXIT__ = [];

var HEAP = intArrayFromString('(null)'); // So printing %s of NULL gives '(null)'
var HEAPTOP = HEAP.length+1; // Leave 0 as an invalid address, 'NULL'

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

function abort(text) {
  text = "ABORT: " + text;
  print(text + "\n");
//  print((new Error).stack); // for stack traces
  print("\n");
  throw text;
}

function Pointer_niceify(ptr) {
// XXX hardcoded ptr impl
  return { slab: HEAP, pos: ptr };
//  if (!ptr.slab)
//    return { slab: ptr[0], pos: ptr[1] };
//  else
//    return ptr;
}

function Pointer_make(slab, pos) {
  pos = pos ? pos : 0;
// XXX hardcoded ptr impl
  if (slab === HEAP) return pos;
  // Flatten out - needed for global consts/vars
  function flatten(slab) {
    if (!slab || slab.length === undefined || typeof slab === 'function') return [slab];
    return slab.map(flatten).reduce(function(a,b) { return a.concat(b) }, []);
  }
  var slab = flatten(slab);
  // Finalize
  var ret = _malloc(Math.max(slab.length - pos, 1));
  for (var i = 0; i < slab.length - pos; i++) {
#if SAFE_HEAP
    SAFE_HEAP_STORE(ret + i, slab[pos + i]);
#else
    HEAP[ret + i] = slab[pos + i];
#endif
  }
  return ret;
//  return { slab: slab, pos: pos ? pos : 0 };
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
    ret = ret + t;
    i = i + 1;
  }
  return ret;
}

function _malloc(size) {
// XXX hardcoded ptr impl
  var ret = HEAPTOP;
  HEAPTOP += size;
  return ret;
  // We don't actually do new Array(size) - memory is uninitialized anyhow
//  return Pointer_make([]);
}

// Mangled "new"s... need a heuristic for autogeneration...
__Znwj = _malloc; // gcc
__Znaj = _malloc; // gcc
__Znam = _malloc; // clang
__Znwm = _malloc; // clang

function _free(ptr) {
// XXX hardcoded ptr impl
  // XXX TODO - actual implementation! Currently we leak it all

  // Nothing needs to be done! But we mark the pointer
  // as invalid. Note that we should do it for all other
  // pointers of this slab too.
//  ptr.slab = null;
//  ptr[0] = null;
}

// Mangled "delete"s... need a heuristic for autogeneration...
__ZdlPv = _free; // gcc
__ZdaPv = _free; // gcc

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
    if (curr == '%'.charCodeAt(0) && ['d', 'f'].indexOf(String.fromCharCode(next)) != -1) {
      String(arguments[argIndex]).split('').forEach(function(chr) {
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
      textIndex ++;
    }
  }
  return Pointer_make(ret);
}

function _printf() {
  var text = Pointer_stringify(__formatString.apply(null, arguments));
  // Our print() will print a \n anyhow... remove dupes
  if (text[text.length-1] == '\n') {
    text = text.substr(0, text.length-1);
  }
  print(text);
}

function _puts(p) {
  _printf(p);
//  print("\n"); // XXX print already always adds one
}

function _putchar(p) {
  print(String.fromCharCode(p));
}

function _strlen(p) {
  // XXX hardcoded ptr impl
  var q = p;
  while (HEAP[q] != 0) q++;
  return q - p;
//  p = Pointer_niceify(p);
//  return p.slab.length; // XXX might want to find the null terminator...
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
// println((new Error).stack); // for stack traces

function println(text) {
  print(text);// + "\n"); // XXX print already always adds one
}

function jrint(label, obj) {
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

// === Body ===

