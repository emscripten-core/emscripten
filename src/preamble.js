
// === Auto-generated preamble library stuff ===

function __globalConstructor__() {
}

var __THREW__ = false; // Used in checking for thrown exceptions.

var __ATEXIT__ = [];

var HEAP = [];
var HEAPTOP = 0;

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
    HEAP[ret + i] = slab[pos + i];
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

__Znwj = _malloc; // Mangled "new"
__Znaj = _malloc; // Mangled "new"

function _free(ptr) {
// XXX hardcoded ptr impl
  // XXX TODO - actual implementation! Currently we leak it all

  // Nothing needs to be done! But we mark the pointer
  // as invalid. Note that we should do it for all other
  // pointers of this slab too.
//  ptr.slab = null;
//  ptr[0] = null;
}

__ZdlPv = _free; // Mangled "delete"
__ZdaPv = _free; // Mangled "delete"

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
    // We only support numbers - use puts for strings!
    if (curr == '%'.charCodeAt(0) && ['d', 'f'].indexOf(String.fromCharCode(next)) != -1) {
      String(arguments[argIndex]).split('').forEach(function(chr) {
        ret.push(chr.charCodeAt(0));
      });
      argIndex += 1;
      textIndex += 2;
      continue;
    } else {
      ret.push(curr);
      textIndex ++;
    }
  }
  return Pointer_make(ret);
}

function _printf() {
  var text = __formatString.apply(null, arguments);
  print(Pointer_stringify(text));
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

function String_copy(p, addZero) {
  // XXX hardcoded ptr impl
  return HEAP.slice(p, p+_strlen(p)).concat(addZero ? [0] : []);
}

// stdlib.h

// Get a pointer, return int value of the string it points to
function _atoi(s) {
  return Math.floor(Number(Pointer_stringify(s)));
}

function _llvm_memcpy_i32(dest, src, num, idunno) {
// XXX hardcoded ptr impl
  for (var i = 0; i < num; i++) {
    HEAP[dest + i] = HEAP[src + i];
  }
//  dest = Pointer_niceify(dest);
//  src = Pointer_niceify(src);
//  dest.slab = src.slab.slice(src.pos);
}

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

