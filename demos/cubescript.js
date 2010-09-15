// License: The original CubeScript code is zlib licensed.
//          The Emscripten code is MIT licensed.
//          So, all of this is permissively licensed.

// Forward requested command to cubescript engine
function executeCS(cmd) {
  __Z7executePKc(Pointer_make(intArrayFromString(cmd)));
}

arguments = []; // Needed since the Emscriptened code expects it


// === Auto-generated preamble library stuff ===

function __globalConstructor__() {
}

var __THREW__ = false; // Used in checking for thrown exceptions.

var __ATEXIT__ = [];

var HEAP = intArrayFromString('(null)'); // So printing %s of NULL gives '(null)'
var HEAPTOP = HEAP.length+1; // Leave 0 as an invalid address, 'NULL'



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


var _0___FLATTENER = [0,1];
var _struct___class_type_info_pseudo___FLATTENER = [0];
var _struct___type_info_pseudo___FLATTENER = [0,1];
var _struct_cline___FLATTENER = [0,1,2];
var __struct_databuf_char_____FLATTENER = [0,1,2,3];
var __struct_hashset_hashtableentry_const_char___ident_______FLATTENER = [0,1,2,3,4];
var __struct_hashset_hashtableentry_const_char___ident_____chain____FLATTENER = [0,12];
var __struct_hashset_hashtableentry_const_char___ident_____chainchunk____FLATTENER = [0,832];
var __struct_hashtable_const_char__ident_____FLATTENER = [0];
var __struct_hashtableentry_const_char__ident_____FLATTENER = [0,1];
var _struct_ident___FLATTENER = [0,1,2,3,4,5,6,7,8,9,10];
var _struct_identstack___FLATTENER = [0,1];
var _struct_stringformatter___FLATTENER = [0];
var __struct_vector_char______FLATTENER = [0,1,2];
var __struct_vector_char_____FLATTENER = [0,1,2];
var __struct_vector_cline_____FLATTENER = [0,1,2];
var __struct_vector_ident______FLATTENER = [0,1,2];
var __struct_vector_vector_char_______FLATTENER = [0,1,2];
var _union__0__40___FLATTENER = [0];
var __union_ident____34____FLATTENER = [0];
var __union_ident____35____FLATTENER = [0];
var __union_ident____36____FLATTENER = [0];
var __union_ident____37____FLATTENER = [0];
var __union_ident____38____FLATTENER = [0];
var _union_identval___FLATTENER = [0];
var _union_identvalptr___FLATTENER = [0];
this.__defineGetter__("__ZTV5ident", function() { delete __ZTV5ident; __ZTV5ident = Pointer_make([ 0, __ZTI5ident, __ZN5identD1Ev, __ZN5identD0Ev, __ZN5ident7changedEv ], 0); return __ZTV5ident });
this.__defineGetter__("__ZTI5ident", function() { delete __ZTI5ident; __ZTI5ident = Pointer_make([ [(__ZTVN10__cxxabiv117__class_type_infoE + 8), __ZTS5ident+0*1] ], 0); return __ZTI5ident });
var __ZTVN10__cxxabiv117__class_type_infoE = 0; /* external value? */
this.__defineGetter__("__ZTS5ident", function() { delete __ZTS5ident; __ZTS5ident = Pointer_make([53,105,100,101,110,116,0] /* 5ident\00*/, 0); return __ZTS5ident });
this.__defineGetter__("__ZL5state", function() { delete __ZL5state; __ZL5state = Pointer_make([0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0], 0); return __ZL5state });
this.__defineGetter__("__ZL4left", function() { delete __ZL4left; __ZL4left = Pointer_make([-1], 0); return __ZL4left });
this.__defineGetter__("_conlines", function() { delete _conlines; _conlines = Pointer_make([0,0,0], 0); return _conlines });
this.__defineGetter__("__ZL8wordbufs", function() { delete __ZL8wordbufs; __ZL8wordbufs = Pointer_make([0,0,0], 0); return __ZL8wordbufs });
this.__defineGetter__("_maxcon", function() { delete _maxcon; _maxcon = Pointer_make([0], 0); return _maxcon });
this.__defineGetter__("__ZZ10parsemacroRPKciR6vectorIcEE5ident", function() { delete __ZZ10parsemacroRPKciR6vectorIcEE5ident; __ZZ10parsemacroRPKciR6vectorIcEE5ident = Pointer_make([0,0,0], 0); return __ZZ10parsemacroRPKciR6vectorIcEE5ident });
this.__defineGetter__("__ZZ10executeretPKcE6argids", function() { delete __ZZ10executeretPKcE6argids; __ZZ10executeretPKcE6argids = Pointer_make([0,0,0], 0); return __ZZ10executeretPKcE6argids });
this.__defineGetter__("__ZZ5fatalPKczE6errors", function() { delete __ZZ5fatalPKczE6errors; __ZZ5fatalPKczE6errors = Pointer_make([0], 0); return __ZZ5fatalPKczE6errors });
this.__defineGetter__("__ZL6retidx", function() { delete __ZL6retidx; __ZL6retidx = Pointer_make([0], 0); return __ZL6retidx });
this.__defineGetter__("__str", function() { delete __str; __str = Pointer_make([37,46,49,102,0] /* %.1f\00*/, 0); return __str });
this.__defineGetter__("__str1", function() { delete __str1; __str1 = Pointer_make([37,46,55,103,0] /* %.7g\00*/, 0); return __str1 });
this.__defineGetter__("__ZL6retbuf", function() { delete __ZL6retbuf; __ZL6retbuf = Pointer_make([0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0], 0); return __ZL6retbuf });
this.__defineGetter__("__str2", function() { delete __str2; __str2 = Pointer_make([37,100,0] /* %d\00*/, 0); return __str2 });
this.__defineGetter__("__ZL4next", function() { delete __ZL4next; __ZL4next = Pointer_make([0], 0); return __ZL4next });
this.__defineGetter__("__ZZN6vectorI5clineE6insertEiRKS0_E5C_237", function() { delete __ZZN6vectorI5clineE6insertEiRKS0_E5C_237; __ZZN6vectorI5clineE6insertEiRKS0_E5C_237 = Pointer_make([0,0,0], 0); return __ZZN6vectorI5clineE6insertEiRKS0_E5C_237 });
this.__defineGetter__("__str3", function() { delete __str3; __str3 = Pointer_make([0], 0); return __str3 });
this.__defineGetter__("_totalmillis", function() { delete _totalmillis; _totalmillis = Pointer_make([-1], 0); return _totalmillis });
this.__defineGetter__("__ZZ8conoutfviPKcPcE3buf", function() { delete __ZZ8conoutfviPKcPcE3buf; __ZZ8conoutfviPKcPcE3buf = Pointer_make([0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0], 0); return __ZZ8conoutfviPKcPcE3buf });
this.__defineGetter__("__str4", function() { delete __str4; __str4 = Pointer_make([118,97,114,105,97,98,108,101,32,37,115,32,105,115,32,114,101,97,100,45,111,110,108,121,0] /* variable %s is read-only\00*/, 0); return __str4 });
this.__defineGetter__("_overrideidents", function() { delete _overrideidents; _overrideidents = Pointer_make([0], 0); return _overrideidents });
this.__defineGetter__("__str5", function() { delete __str5; __str5 = Pointer_make([99,97,110,110,111,116,32,111,118,101,114,114,105,100,101,32,112,101,114,115,105,115,116,101,110,116,32,118,97,114,105,97,98,108,101,32,37,115,0] /* cannot override persistent variable %s\00*/, 0); return __str5 });
this.__defineGetter__("__str6", function() { delete __str6; __str6 = Pointer_make([118,97,108,105,100,32,114,97,110,103,101,32,102,111,114,32,37,115,32,105,115,32,37,115,46,46,37,115,0] /* valid range for %s is %s..%s\00*/, 0); return __str6 });
this.__defineGetter__("__str7", function() { delete __str7; __str7 = Pointer_make([118,97,108,105,100,32,114,97,110,103,101,32,102,111,114,32,37,115,32,105,115,32,37,100,46,46,48,120,37,88,0] /* valid range for %s is %d..0x%X\00*/, 0); return __str7 });
this.__defineGetter__("__str8", function() { delete __str8; __str8 = Pointer_make([118,97,108,105,100,32,114,97,110,103,101,32,102,111,114,32,37,115,32,105,115,32,48,120,37,88,46,46,48,120,37,88,0] /* valid range for %s is 0x%X..0x%X\00*/, 0); return __str8 });
this.__defineGetter__("__str9", function() { delete __str9; __str9 = Pointer_make([118,97,108,105,100,32,114,97,110,103,101,32,102,111,114,32,37,115,32,105,115,32,37,100,46,46,37,100,0] /* valid range for %s is %d..%d\00*/, 0); return __str9 });
this.__defineGetter__("__str10", function() { delete __str10; __str10 = Pointer_make([37,115,0] /* %s\00*/, 0); return __str10 });
this.__defineGetter__("_commandret", function() { delete _commandret; _commandret = Pointer_make([0], 0); return _commandret });
this.__defineGetter__("_idents", function() { delete _idents; _idents = Pointer_make([0], 0); return _idents });
this.__defineGetter__("__str11", function() { delete __str11; __str11 = Pointer_make([117,110,107,110,111,119,110,32,97,108,105,97,115,32,108,111,111,107,117,112,58,32,37,115,0] /* unknown alias lookup: %s\00*/, 0); return __str11 });
this.__defineGetter__("__str12", function() { delete __str12; __str12 = Pointer_make([10,9,32,0] /* \0A\09 \00*/, 0); return __str12 });
this.__defineGetter__("__str13", function() { delete __str13; __str13 = Pointer_make([34,10,0,0] /* \22\0A\00\00*/, 0); return __str13 });
this.__defineGetter__("__str14", function() { delete __str14; __str14 = Pointer_make([10,9,32,0,0] /* \0A\09 \00\00*/, 0); return __str14 });
this.__defineGetter__("__str15", function() { delete __str15; __str15 = Pointer_make([32,0] /*  \00*/, 0); return __str15 });
this.__defineGetter__("_persistidents", function() { delete _persistidents; _persistidents = Pointer_make([1], 0); return _persistidents });
this.__defineGetter__("__str16", function() { delete __str16; __str16 = Pointer_make([99,97,110,110,111,116,32,114,101,100,101,102,105,110,101,32,98,117,105,108,116,105,110,32,37,115,32,119,105,116,104,32,97,110,32,97,108,105,97,115,0] /* cannot redefine builtin %s with an alias\00*/, 0); return __str16 });
this.__defineGetter__("__str17", function() { delete __str17; __str17 = Pointer_make([112,117,115,104,0] /* push\00*/, 0); return __str17 });
this.__defineGetter__("__str18", function() { delete __str18; __str18 = Pointer_make([115,115,0] /* ss\00*/, 0); return __str18 });
this.__defineGetter__("__ZL12__dummy_push", function() { delete __ZL12__dummy_push; __ZL12__dummy_push = Pointer_make([0], 0); return __ZL12__dummy_push });
this.__defineGetter__("__str19", function() { delete __str19; __str19 = Pointer_make([112,111,112,0] /* pop\00*/, 0); return __str19 });
this.__defineGetter__("__str20", function() { delete __str20; __str20 = Pointer_make([115,0] /* s\00*/, 0); return __str20 });
this.__defineGetter__("__ZL11__dummy_pop", function() { delete __ZL11__dummy_pop; __ZL11__dummy_pop = Pointer_make([0], 0); return __ZL11__dummy_pop });
this.__defineGetter__("__str21", function() { delete __str21; __str21 = Pointer_make([114,101,115,101,116,118,97,114,0] /* resetvar\00*/, 0); return __str21 });
this.__defineGetter__("__ZL16__dummy_resetvar", function() { delete __ZL16__dummy_resetvar; __ZL16__dummy_resetvar = Pointer_make([0], 0); return __ZL16__dummy_resetvar });
this.__defineGetter__("__str22", function() { delete __str22; __str22 = Pointer_make([97,108,105,97,115,0] /* alias\00*/, 0); return __str22 });
this.__defineGetter__("__ZL13__dummy_alias", function() { delete __ZL13__dummy_alias; __ZL13__dummy_alias = Pointer_make([0], 0); return __ZL13__dummy_alias });
var ___dso_handle = 0; /* external value? */
this.__defineGetter__("__str23", function() { delete __str23; __str23 = Pointer_make([110,117,109,97,114,103,115,0] /* numargs\00*/, 0); return __str23 });
this.__defineGetter__("__numargs", function() { delete __numargs; __numargs = Pointer_make([0], 0); return __numargs });
this.__defineGetter__("__str24", function() { delete __str24; __str24 = Pointer_make([105,102,0] /* if\00*/, 0); return __str24 });
this.__defineGetter__("__str25", function() { delete __str25; __str25 = Pointer_make([115,115,115,0] /* sss\00*/, 0); return __str25 });
this.__defineGetter__("__ZN7_stdcmdILi846EE4initE", function() { delete __ZN7_stdcmdILi846EE4initE; __ZN7_stdcmdILi846EE4initE = Pointer_make([0], 0); return __ZN7_stdcmdILi846EE4initE });
this.__defineGetter__("__str26", function() { delete __str26; __str26 = Pointer_make([63,0] /* ?\00*/, 0); return __str26 });
this.__defineGetter__("__ZN7_stdcmdILi847EE4initE", function() { delete __ZN7_stdcmdILi847EE4initE; __ZN7_stdcmdILi847EE4initE = Pointer_make([0], 0); return __ZN7_stdcmdILi847EE4initE });
this.__defineGetter__("__str27", function() { delete __str27; __str27 = Pointer_make([108,111,111,112,0] /* loop\00*/, 0); return __str27 });
this.__defineGetter__("__str28", function() { delete __str28; __str28 = Pointer_make([115,105,115,0] /* sis\00*/, 0); return __str28 });
this.__defineGetter__("__ZN7_stdcmdILi860EE4initE", function() { delete __ZN7_stdcmdILi860EE4initE; __ZN7_stdcmdILi860EE4initE = Pointer_make([0], 0); return __ZN7_stdcmdILi860EE4initE });
this.__defineGetter__("__str29", function() { delete __str29; __str29 = Pointer_make([108,111,111,112,119,104,105,108,101,0] /* loopwhile\00*/, 0); return __str29 });
this.__defineGetter__("__str30", function() { delete __str30; __str30 = Pointer_make([115,105,115,115,0] /* siss\00*/, 0); return __str30 });
this.__defineGetter__("__ZN7_stdcmdILi874EE4initE", function() { delete __ZN7_stdcmdILi874EE4initE; __ZN7_stdcmdILi874EE4initE = Pointer_make([0], 0); return __ZN7_stdcmdILi874EE4initE });
this.__defineGetter__("__str31", function() { delete __str31; __str31 = Pointer_make([119,104,105,108,101,0] /* while\00*/, 0); return __str31 });
this.__defineGetter__("__ZN7_stdcmdILi875EE4initE", function() { delete __ZN7_stdcmdILi875EE4initE; __ZN7_stdcmdILi875EE4initE = Pointer_make([0], 0); return __ZN7_stdcmdILi875EE4initE });
this.__defineGetter__("__str32", function() { delete __str32; __str32 = Pointer_make([99,111,110,99,97,116,0] /* concat\00*/, 0); return __str32 });
this.__defineGetter__("__str33", function() { delete __str33; __str33 = Pointer_make([67,0] /* C\00*/, 0); return __str33 });
this.__defineGetter__("__ZL14__dummy_concat", function() { delete __ZL14__dummy_concat; __ZL14__dummy_concat = Pointer_make([0], 0); return __ZL14__dummy_concat });
this.__defineGetter__("__str34", function() { delete __str34; __str34 = Pointer_make([114,101,115,117,108,116,0] /* result\00*/, 0); return __str34 });
this.__defineGetter__("__ZL14__dummy_result", function() { delete __ZL14__dummy_result; __ZL14__dummy_result = Pointer_make([0], 0); return __ZL14__dummy_result });
this.__defineGetter__("__str35", function() { delete __str35; __str35 = Pointer_make([99,111,110,99,97,116,119,111,114,100,0] /* concatword\00*/, 0); return __str35 });
this.__defineGetter__("__str36", function() { delete __str36; __str36 = Pointer_make([86,0] /* V\00*/, 0); return __str36 });
this.__defineGetter__("__ZL18__dummy_concatword", function() { delete __ZL18__dummy_concatword; __ZL18__dummy_concatword = Pointer_make([0], 0); return __ZL18__dummy_concatword });
this.__defineGetter__("__str37", function() { delete __str37; __str37 = Pointer_make([102,111,114,109,97,116,0] /* format\00*/, 0); return __str37 });
this.__defineGetter__("__ZL14__dummy_format", function() { delete __ZL14__dummy_format; __ZL14__dummy_format = Pointer_make([0], 0); return __ZL14__dummy_format });
this.__defineGetter__("__str38", function() { delete __str38; __str38 = Pointer_make([97,116,0] /* at\00*/, 0); return __str38 });
this.__defineGetter__("__str39", function() { delete __str39; __str39 = Pointer_make([115,105,0] /* si\00*/, 0); return __str39 });
this.__defineGetter__("__ZL10__dummy_at", function() { delete __ZL10__dummy_at; __ZL10__dummy_at = Pointer_make([0], 0); return __ZL10__dummy_at });
this.__defineGetter__("__str40", function() { delete __str40; __str40 = Pointer_make([115,117,98,115,116,114,0] /* substr\00*/, 0); return __str40 });
this.__defineGetter__("__ZL14__dummy_substr", function() { delete __ZL14__dummy_substr; __ZL14__dummy_substr = Pointer_make([0], 0); return __ZL14__dummy_substr });
this.__defineGetter__("__str41", function() { delete __str41; __str41 = Pointer_make([108,105,115,116,108,101,110,0] /* listlen\00*/, 0); return __str41 });
this.__defineGetter__("__ZN7_stdcmdILi973EE4initE", function() { delete __ZN7_stdcmdILi973EE4initE; __ZN7_stdcmdILi973EE4initE = Pointer_make([0], 0); return __ZN7_stdcmdILi973EE4initE });
this.__defineGetter__("__str42", function() { delete __str42; __str42 = Pointer_make([103,101,116,97,108,105,97,115,0] /* getalias\00*/, 0); return __str42 });
this.__defineGetter__("__ZL17__dummy_getalias_", function() { delete __ZL17__dummy_getalias_; __ZL17__dummy_getalias_ = Pointer_make([0], 0); return __ZL17__dummy_getalias_ });
this.__defineGetter__("__str43", function() { delete __str43; __str43 = Pointer_make([112,114,101,116,116,121,108,105,115,116,0] /* prettylist\00*/, 0); return __str43 });
this.__defineGetter__("__ZL18__dummy_prettylist", function() { delete __ZL18__dummy_prettylist; __ZL18__dummy_prettylist = Pointer_make([0], 0); return __ZL18__dummy_prettylist });
this.__defineGetter__("__str44", function() { delete __str44; __str44 = Pointer_make([108,105,115,116,100,101,108,0] /* listdel\00*/, 0); return __str44 });
this.__defineGetter__("__ZN7_stdcmdILi1070EE4initE", function() { delete __ZN7_stdcmdILi1070EE4initE; __ZN7_stdcmdILi1070EE4initE = Pointer_make([0], 0); return __ZN7_stdcmdILi1070EE4initE });
this.__defineGetter__("__str45", function() { delete __str45; __str45 = Pointer_make([105,110,100,101,120,111,102,0] /* indexof\00*/, 0); return __str45 });
this.__defineGetter__("__ZN7_stdcmdILi1071EE4initE", function() { delete __ZN7_stdcmdILi1071EE4initE; __ZN7_stdcmdILi1071EE4initE = Pointer_make([0], 0); return __ZN7_stdcmdILi1071EE4initE });
this.__defineGetter__("__str46", function() { delete __str46; __str46 = Pointer_make([108,105,115,116,102,105,110,100,0] /* listfind\00*/, 0); return __str46 });
this.__defineGetter__("__ZN7_stdcmdILi1072EE4initE", function() { delete __ZN7_stdcmdILi1072EE4initE; __ZN7_stdcmdILi1072EE4initE = Pointer_make([0], 0); return __ZN7_stdcmdILi1072EE4initE });
this.__defineGetter__("__str47", function() { delete __str47; __str47 = Pointer_make([108,111,111,112,108,105,115,116,0] /* looplist\00*/, 0); return __str47 });
this.__defineGetter__("__ZN7_stdcmdILi1073EE4initE", function() { delete __ZN7_stdcmdILi1073EE4initE; __ZN7_stdcmdILi1073EE4initE = Pointer_make([0], 0); return __ZN7_stdcmdILi1073EE4initE });
this.__defineGetter__("__str48", function() { delete __str48; __str48 = Pointer_make([43,0] /* +\00*/, 0); return __str48 });
this.__defineGetter__("__str49", function() { delete __str49; __str49 = Pointer_make([105,105,0] /* ii\00*/, 0); return __str49 });
this.__defineGetter__("__ZN7_stdcmdILi1075EE4initE", function() { delete __ZN7_stdcmdILi1075EE4initE; __ZN7_stdcmdILi1075EE4initE = Pointer_make([0], 0); return __ZN7_stdcmdILi1075EE4initE });
this.__defineGetter__("__str50", function() { delete __str50; __str50 = Pointer_make([42,0] /* *\00*/, 0); return __str50 });
this.__defineGetter__("__ZN7_stdcmdILi1076EE4initE", function() { delete __ZN7_stdcmdILi1076EE4initE; __ZN7_stdcmdILi1076EE4initE = Pointer_make([0], 0); return __ZN7_stdcmdILi1076EE4initE });
this.__defineGetter__("__str51", function() { delete __str51; __str51 = Pointer_make([45,0] /* -\00*/, 0); return __str51 });
this.__defineGetter__("__ZN7_stdcmdILi1077EE4initE", function() { delete __ZN7_stdcmdILi1077EE4initE; __ZN7_stdcmdILi1077EE4initE = Pointer_make([0], 0); return __ZN7_stdcmdILi1077EE4initE });
this.__defineGetter__("__str52", function() { delete __str52; __str52 = Pointer_make([43,102,0] /* +f\00*/, 0); return __str52 });
this.__defineGetter__("__str53", function() { delete __str53; __str53 = Pointer_make([102,102,0] /* ff\00*/, 0); return __str53 });
this.__defineGetter__("__ZN7_stdcmdILi1078EE4initE", function() { delete __ZN7_stdcmdILi1078EE4initE; __ZN7_stdcmdILi1078EE4initE = Pointer_make([0], 0); return __ZN7_stdcmdILi1078EE4initE });
this.__defineGetter__("__str54", function() { delete __str54; __str54 = Pointer_make([42,102,0] /* *f\00*/, 0); return __str54 });
this.__defineGetter__("__ZN7_stdcmdILi1079EE4initE", function() { delete __ZN7_stdcmdILi1079EE4initE; __ZN7_stdcmdILi1079EE4initE = Pointer_make([0], 0); return __ZN7_stdcmdILi1079EE4initE });
this.__defineGetter__("__str55", function() { delete __str55; __str55 = Pointer_make([45,102,0] /* -f\00*/, 0); return __str55 });
this.__defineGetter__("__ZN7_stdcmdILi1080EE4initE", function() { delete __ZN7_stdcmdILi1080EE4initE; __ZN7_stdcmdILi1080EE4initE = Pointer_make([0], 0); return __ZN7_stdcmdILi1080EE4initE });
this.__defineGetter__("__str56", function() { delete __str56; __str56 = Pointer_make([61,0] /* =\00*/, 0); return __str56 });
this.__defineGetter__("__ZN7_stdcmdILi1081EE4initE", function() { delete __ZN7_stdcmdILi1081EE4initE; __ZN7_stdcmdILi1081EE4initE = Pointer_make([0], 0); return __ZN7_stdcmdILi1081EE4initE });
this.__defineGetter__("__str57", function() { delete __str57; __str57 = Pointer_make([33,61,0] /* !=\00*/, 0); return __str57 });
this.__defineGetter__("__ZN7_stdcmdILi1082EE4initE", function() { delete __ZN7_stdcmdILi1082EE4initE; __ZN7_stdcmdILi1082EE4initE = Pointer_make([0], 0); return __ZN7_stdcmdILi1082EE4initE });
this.__defineGetter__("__str58", function() { delete __str58; __str58 = Pointer_make([60,0] /* <\00*/, 0); return __str58 });
this.__defineGetter__("__ZN7_stdcmdILi1083EE4initE", function() { delete __ZN7_stdcmdILi1083EE4initE; __ZN7_stdcmdILi1083EE4initE = Pointer_make([0], 0); return __ZN7_stdcmdILi1083EE4initE });
this.__defineGetter__("__str59", function() { delete __str59; __str59 = Pointer_make([62,0] /* >\00*/, 0); return __str59 });
this.__defineGetter__("__ZN7_stdcmdILi1084EE4initE", function() { delete __ZN7_stdcmdILi1084EE4initE; __ZN7_stdcmdILi1084EE4initE = Pointer_make([0], 0); return __ZN7_stdcmdILi1084EE4initE });
this.__defineGetter__("__str60", function() { delete __str60; __str60 = Pointer_make([60,61,0] /* <=\00*/, 0); return __str60 });
this.__defineGetter__("__ZN7_stdcmdILi1085EE4initE", function() { delete __ZN7_stdcmdILi1085EE4initE; __ZN7_stdcmdILi1085EE4initE = Pointer_make([0], 0); return __ZN7_stdcmdILi1085EE4initE });
this.__defineGetter__("__str61", function() { delete __str61; __str61 = Pointer_make([62,61,0] /* >=\00*/, 0); return __str61 });
this.__defineGetter__("__ZN7_stdcmdILi1086EE4initE", function() { delete __ZN7_stdcmdILi1086EE4initE; __ZN7_stdcmdILi1086EE4initE = Pointer_make([0], 0); return __ZN7_stdcmdILi1086EE4initE });
this.__defineGetter__("__str62", function() { delete __str62; __str62 = Pointer_make([61,102,0] /* =f\00*/, 0); return __str62 });
this.__defineGetter__("__ZN7_stdcmdILi1087EE4initE", function() { delete __ZN7_stdcmdILi1087EE4initE; __ZN7_stdcmdILi1087EE4initE = Pointer_make([0], 0); return __ZN7_stdcmdILi1087EE4initE });
this.__defineGetter__("__str63", function() { delete __str63; __str63 = Pointer_make([33,61,102,0] /* !=f\00*/, 0); return __str63 });
this.__defineGetter__("__ZN7_stdcmdILi1088EE4initE", function() { delete __ZN7_stdcmdILi1088EE4initE; __ZN7_stdcmdILi1088EE4initE = Pointer_make([0], 0); return __ZN7_stdcmdILi1088EE4initE });
this.__defineGetter__("__str64", function() { delete __str64; __str64 = Pointer_make([60,102,0] /* <f\00*/, 0); return __str64 });
this.__defineGetter__("__ZN7_stdcmdILi1089EE4initE", function() { delete __ZN7_stdcmdILi1089EE4initE; __ZN7_stdcmdILi1089EE4initE = Pointer_make([0], 0); return __ZN7_stdcmdILi1089EE4initE });
this.__defineGetter__("__str65", function() { delete __str65; __str65 = Pointer_make([62,102,0] /* >f\00*/, 0); return __str65 });
this.__defineGetter__("__ZN7_stdcmdILi1090EE4initE", function() { delete __ZN7_stdcmdILi1090EE4initE; __ZN7_stdcmdILi1090EE4initE = Pointer_make([0], 0); return __ZN7_stdcmdILi1090EE4initE });
this.__defineGetter__("__str66", function() { delete __str66; __str66 = Pointer_make([60,61,102,0] /* <=f\00*/, 0); return __str66 });
this.__defineGetter__("__ZN7_stdcmdILi1091EE4initE", function() { delete __ZN7_stdcmdILi1091EE4initE; __ZN7_stdcmdILi1091EE4initE = Pointer_make([0], 0); return __ZN7_stdcmdILi1091EE4initE });
this.__defineGetter__("__str67", function() { delete __str67; __str67 = Pointer_make([62,61,102,0] /* >=f\00*/, 0); return __str67 });
this.__defineGetter__("__ZN7_stdcmdILi1092EE4initE", function() { delete __ZN7_stdcmdILi1092EE4initE; __ZN7_stdcmdILi1092EE4initE = Pointer_make([0], 0); return __ZN7_stdcmdILi1092EE4initE });
this.__defineGetter__("__str68", function() { delete __str68; __str68 = Pointer_make([94,0] /* ^\00*/, 0); return __str68 });
this.__defineGetter__("__ZN7_stdcmdILi1093EE4initE", function() { delete __ZN7_stdcmdILi1093EE4initE; __ZN7_stdcmdILi1093EE4initE = Pointer_make([0], 0); return __ZN7_stdcmdILi1093EE4initE });
this.__defineGetter__("__str69", function() { delete __str69; __str69 = Pointer_make([33,0] /* !\00*/, 0); return __str69 });
this.__defineGetter__("__str70", function() { delete __str70; __str70 = Pointer_make([105,0] /* i\00*/, 0); return __str70 });
this.__defineGetter__("__ZN7_stdcmdILi1094EE4initE", function() { delete __ZN7_stdcmdILi1094EE4initE; __ZN7_stdcmdILi1094EE4initE = Pointer_make([0], 0); return __ZN7_stdcmdILi1094EE4initE });
this.__defineGetter__("__str71", function() { delete __str71; __str71 = Pointer_make([38,0] /* &\00*/, 0); return __str71 });
this.__defineGetter__("__ZN7_stdcmdILi1095EE4initE", function() { delete __ZN7_stdcmdILi1095EE4initE; __ZN7_stdcmdILi1095EE4initE = Pointer_make([0], 0); return __ZN7_stdcmdILi1095EE4initE });
this.__defineGetter__("__str72", function() { delete __str72; __str72 = Pointer_make([124,0] /* |\00*/, 0); return __str72 });
this.__defineGetter__("__ZN7_stdcmdILi1096EE4initE", function() { delete __ZN7_stdcmdILi1096EE4initE; __ZN7_stdcmdILi1096EE4initE = Pointer_make([0], 0); return __ZN7_stdcmdILi1096EE4initE });
this.__defineGetter__("__str73", function() { delete __str73; __str73 = Pointer_make([126,0] /* ~\00*/, 0); return __str73 });
this.__defineGetter__("__ZN7_stdcmdILi1097EE4initE", function() { delete __ZN7_stdcmdILi1097EE4initE; __ZN7_stdcmdILi1097EE4initE = Pointer_make([0], 0); return __ZN7_stdcmdILi1097EE4initE });
this.__defineGetter__("__str74", function() { delete __str74; __str74 = Pointer_make([94,126,0] /* ^~\00*/, 0); return __str74 });
this.__defineGetter__("__ZN7_stdcmdILi1098EE4initE", function() { delete __ZN7_stdcmdILi1098EE4initE; __ZN7_stdcmdILi1098EE4initE = Pointer_make([0], 0); return __ZN7_stdcmdILi1098EE4initE });
this.__defineGetter__("__str75", function() { delete __str75; __str75 = Pointer_make([38,126,0] /* &~\00*/, 0); return __str75 });
this.__defineGetter__("__ZN7_stdcmdILi1099EE4initE", function() { delete __ZN7_stdcmdILi1099EE4initE; __ZN7_stdcmdILi1099EE4initE = Pointer_make([0], 0); return __ZN7_stdcmdILi1099EE4initE });
this.__defineGetter__("__str76", function() { delete __str76; __str76 = Pointer_make([124,126,0] /* |~\00*/, 0); return __str76 });
this.__defineGetter__("__ZN7_stdcmdILi1100EE4initE", function() { delete __ZN7_stdcmdILi1100EE4initE; __ZN7_stdcmdILi1100EE4initE = Pointer_make([0], 0); return __ZN7_stdcmdILi1100EE4initE });
this.__defineGetter__("__str77", function() { delete __str77; __str77 = Pointer_make([60,60,0] /* <<\00*/, 0); return __str77 });
this.__defineGetter__("__ZN7_stdcmdILi1101EE4initE", function() { delete __ZN7_stdcmdILi1101EE4initE; __ZN7_stdcmdILi1101EE4initE = Pointer_make([0], 0); return __ZN7_stdcmdILi1101EE4initE });
this.__defineGetter__("__str78", function() { delete __str78; __str78 = Pointer_make([62,62,0] /* >>\00*/, 0); return __str78 });
this.__defineGetter__("__ZN7_stdcmdILi1102EE4initE", function() { delete __ZN7_stdcmdILi1102EE4initE; __ZN7_stdcmdILi1102EE4initE = Pointer_make([0], 0); return __ZN7_stdcmdILi1102EE4initE });
this.__defineGetter__("__str79", function() { delete __str79; __str79 = Pointer_make([38,38,0] /* &&\00*/, 0); return __str79 });
this.__defineGetter__("__ZN7_stdcmdILi1108EE4initE", function() { delete __ZN7_stdcmdILi1108EE4initE; __ZN7_stdcmdILi1108EE4initE = Pointer_make([0], 0); return __ZN7_stdcmdILi1108EE4initE });
this.__defineGetter__("__str80", function() { delete __str80; __str80 = Pointer_make([124,124,0] /* ||\00*/, 0); return __str80 });
this.__defineGetter__("__ZN7_stdcmdILi1114EE4initE", function() { delete __ZN7_stdcmdILi1114EE4initE; __ZN7_stdcmdILi1114EE4initE = Pointer_make([0], 0); return __ZN7_stdcmdILi1114EE4initE });
this.__defineGetter__("__str81", function() { delete __str81; __str81 = Pointer_make([100,105,118,0] /* div\00*/, 0); return __str81 });
this.__defineGetter__("__ZN7_stdcmdILi1116EE4initE", function() { delete __ZN7_stdcmdILi1116EE4initE; __ZN7_stdcmdILi1116EE4initE = Pointer_make([0], 0); return __ZN7_stdcmdILi1116EE4initE });
this.__defineGetter__("__str82", function() { delete __str82; __str82 = Pointer_make([109,111,100,0] /* mod\00*/, 0); return __str82 });
this.__defineGetter__("__ZN7_stdcmdILi1117EE4initE", function() { delete __ZN7_stdcmdILi1117EE4initE; __ZN7_stdcmdILi1117EE4initE = Pointer_make([0], 0); return __ZN7_stdcmdILi1117EE4initE });
this.__defineGetter__("__str83", function() { delete __str83; __str83 = Pointer_make([100,105,118,102,0] /* divf\00*/, 0); return __str83 });
this.__defineGetter__("__ZN7_stdcmdILi1118EE4initE", function() { delete __ZN7_stdcmdILi1118EE4initE; __ZN7_stdcmdILi1118EE4initE = Pointer_make([0], 0); return __ZN7_stdcmdILi1118EE4initE });
this.__defineGetter__("__str84", function() { delete __str84; __str84 = Pointer_make([109,111,100,102,0] /* modf\00*/, 0); return __str84 });
this.__defineGetter__("__ZN7_stdcmdILi1119EE4initE", function() { delete __ZN7_stdcmdILi1119EE4initE; __ZN7_stdcmdILi1119EE4initE = Pointer_make([0], 0); return __ZN7_stdcmdILi1119EE4initE });
this.__defineGetter__("__str85", function() { delete __str85; __str85 = Pointer_make([115,105,110,0] /* sin\00*/, 0); return __str85 });
this.__defineGetter__("__str86", function() { delete __str86; __str86 = Pointer_make([102,0] /* f\00*/, 0); return __str86 });
this.__defineGetter__("__ZN7_stdcmdILi1120EE4initE", function() { delete __ZN7_stdcmdILi1120EE4initE; __ZN7_stdcmdILi1120EE4initE = Pointer_make([0], 0); return __ZN7_stdcmdILi1120EE4initE });
this.__defineGetter__("__str87", function() { delete __str87; __str87 = Pointer_make([99,111,115,0] /* cos\00*/, 0); return __str87 });
this.__defineGetter__("__ZN7_stdcmdILi1121EE4initE", function() { delete __ZN7_stdcmdILi1121EE4initE; __ZN7_stdcmdILi1121EE4initE = Pointer_make([0], 0); return __ZN7_stdcmdILi1121EE4initE });
this.__defineGetter__("__str88", function() { delete __str88; __str88 = Pointer_make([116,97,110,0] /* tan\00*/, 0); return __str88 });
this.__defineGetter__("__ZN7_stdcmdILi1122EE4initE", function() { delete __ZN7_stdcmdILi1122EE4initE; __ZN7_stdcmdILi1122EE4initE = Pointer_make([0], 0); return __ZN7_stdcmdILi1122EE4initE });
this.__defineGetter__("__str89", function() { delete __str89; __str89 = Pointer_make([97,115,105,110,0] /* asin\00*/, 0); return __str89 });
this.__defineGetter__("__ZN7_stdcmdILi1123EE4initE", function() { delete __ZN7_stdcmdILi1123EE4initE; __ZN7_stdcmdILi1123EE4initE = Pointer_make([0], 0); return __ZN7_stdcmdILi1123EE4initE });
this.__defineGetter__("__str90", function() { delete __str90; __str90 = Pointer_make([97,99,111,115,0] /* acos\00*/, 0); return __str90 });
this.__defineGetter__("__ZN7_stdcmdILi1124EE4initE", function() { delete __ZN7_stdcmdILi1124EE4initE; __ZN7_stdcmdILi1124EE4initE = Pointer_make([0], 0); return __ZN7_stdcmdILi1124EE4initE });
this.__defineGetter__("__str91", function() { delete __str91; __str91 = Pointer_make([97,116,97,110,0] /* atan\00*/, 0); return __str91 });
this.__defineGetter__("__ZN7_stdcmdILi1125EE4initE", function() { delete __ZN7_stdcmdILi1125EE4initE; __ZN7_stdcmdILi1125EE4initE = Pointer_make([0], 0); return __ZN7_stdcmdILi1125EE4initE });
this.__defineGetter__("__str92", function() { delete __str92; __str92 = Pointer_make([115,113,114,116,0] /* sqrt\00*/, 0); return __str92 });
this.__defineGetter__("__ZN7_stdcmdILi1126EE4initE", function() { delete __ZN7_stdcmdILi1126EE4initE; __ZN7_stdcmdILi1126EE4initE = Pointer_make([0], 0); return __ZN7_stdcmdILi1126EE4initE });
this.__defineGetter__("__str93", function() { delete __str93; __str93 = Pointer_make([112,111,119,0] /* pow\00*/, 0); return __str93 });
this.__defineGetter__("__ZN7_stdcmdILi1127EE4initE", function() { delete __ZN7_stdcmdILi1127EE4initE; __ZN7_stdcmdILi1127EE4initE = Pointer_make([0], 0); return __ZN7_stdcmdILi1127EE4initE });
this.__defineGetter__("__str94", function() { delete __str94; __str94 = Pointer_make([108,111,103,101,0] /* loge\00*/, 0); return __str94 });
this.__defineGetter__("__ZN7_stdcmdILi1128EE4initE", function() { delete __ZN7_stdcmdILi1128EE4initE; __ZN7_stdcmdILi1128EE4initE = Pointer_make([0], 0); return __ZN7_stdcmdILi1128EE4initE });
this.__defineGetter__("__str95", function() { delete __str95; __str95 = Pointer_make([108,111,103,50,0] /* log2\00*/, 0); return __str95 });
this.__defineGetter__("__ZN7_stdcmdILi1129EE4initE", function() { delete __ZN7_stdcmdILi1129EE4initE; __ZN7_stdcmdILi1129EE4initE = Pointer_make([0], 0); return __ZN7_stdcmdILi1129EE4initE });
this.__defineGetter__("__str96", function() { delete __str96; __str96 = Pointer_make([108,111,103,49,48,0] /* log10\00*/, 0); return __str96 });
this.__defineGetter__("__ZN7_stdcmdILi1130EE4initE", function() { delete __ZN7_stdcmdILi1130EE4initE; __ZN7_stdcmdILi1130EE4initE = Pointer_make([0], 0); return __ZN7_stdcmdILi1130EE4initE });
this.__defineGetter__("__str97", function() { delete __str97; __str97 = Pointer_make([101,120,112,0] /* exp\00*/, 0); return __str97 });
this.__defineGetter__("__ZN7_stdcmdILi1131EE4initE", function() { delete __ZN7_stdcmdILi1131EE4initE; __ZN7_stdcmdILi1131EE4initE = Pointer_make([0], 0); return __ZN7_stdcmdILi1131EE4initE });
this.__defineGetter__("__str98", function() { delete __str98; __str98 = Pointer_make([109,105,110,0] /* min\00*/, 0); return __str98 });
this.__defineGetter__("__ZN7_stdcmdILi1137EE4initE", function() { delete __ZN7_stdcmdILi1137EE4initE; __ZN7_stdcmdILi1137EE4initE = Pointer_make([0], 0); return __ZN7_stdcmdILi1137EE4initE });
this.__defineGetter__("__str99", function() { delete __str99; __str99 = Pointer_make([109,97,120,0] /* max\00*/, 0); return __str99 });
this.__defineGetter__("__ZN7_stdcmdILi1143EE4initE", function() { delete __ZN7_stdcmdILi1143EE4initE; __ZN7_stdcmdILi1143EE4initE = Pointer_make([0], 0); return __ZN7_stdcmdILi1143EE4initE });
this.__defineGetter__("__str100", function() { delete __str100; __str100 = Pointer_make([109,105,110,102,0] /* minf\00*/, 0); return __str100 });
this.__defineGetter__("__ZN7_stdcmdILi1149EE4initE", function() { delete __ZN7_stdcmdILi1149EE4initE; __ZN7_stdcmdILi1149EE4initE = Pointer_make([0], 0); return __ZN7_stdcmdILi1149EE4initE });
this.__defineGetter__("__str101", function() { delete __str101; __str101 = Pointer_make([109,97,120,102,0] /* maxf\00*/, 0); return __str101 });
this.__defineGetter__("__ZN7_stdcmdILi1155EE4initE", function() { delete __ZN7_stdcmdILi1155EE4initE; __ZN7_stdcmdILi1155EE4initE = Pointer_make([0], 0); return __ZN7_stdcmdILi1155EE4initE });
this.__defineGetter__("__str102", function() { delete __str102; __str102 = Pointer_make([99,111,110,100,0] /* cond\00*/, 0); return __str102 });
this.__defineGetter__("__ZN7_stdcmdILi1167EE4initE", function() { delete __ZN7_stdcmdILi1167EE4initE; __ZN7_stdcmdILi1167EE4initE = Pointer_make([0], 0); return __ZN7_stdcmdILi1167EE4initE });
this.__defineGetter__("__str103", function() { delete __str103; __str103 = Pointer_make([99,97,115,101,0] /* case\00*/, 0); return __str103 });
this.__defineGetter__("__str104", function() { delete __str104; __str104 = Pointer_make([105,86,0] /* iV\00*/, 0); return __str104 });
this.__defineGetter__("__ZN7_stdcmdILi1182EE4initE", function() { delete __ZN7_stdcmdILi1182EE4initE; __ZN7_stdcmdILi1182EE4initE = Pointer_make([0], 0); return __ZN7_stdcmdILi1182EE4initE });
this.__defineGetter__("__str105", function() { delete __str105; __str105 = Pointer_make([99,97,115,101,102,0] /* casef\00*/, 0); return __str105 });
this.__defineGetter__("__str106", function() { delete __str106; __str106 = Pointer_make([102,86,0] /* fV\00*/, 0); return __str106 });
this.__defineGetter__("__ZN7_stdcmdILi1183EE4initE", function() { delete __ZN7_stdcmdILi1183EE4initE; __ZN7_stdcmdILi1183EE4initE = Pointer_make([0], 0); return __ZN7_stdcmdILi1183EE4initE });
this.__defineGetter__("__str107", function() { delete __str107; __str107 = Pointer_make([99,97,115,101,115,0] /* cases\00*/, 0); return __str107 });
this.__defineGetter__("__str108", function() { delete __str108; __str108 = Pointer_make([115,86,0] /* sV\00*/, 0); return __str108 });
this.__defineGetter__("__ZN7_stdcmdILi1184EE4initE", function() { delete __ZN7_stdcmdILi1184EE4initE; __ZN7_stdcmdILi1184EE4initE = Pointer_make([0], 0); return __ZN7_stdcmdILi1184EE4initE });
this.__defineGetter__("__str109", function() { delete __str109; __str109 = Pointer_make([114,110,100,0] /* rnd\00*/, 0); return __str109 });
this.__defineGetter__("__ZN7_stdcmdILi1186EE4initE", function() { delete __ZN7_stdcmdILi1186EE4initE; __ZN7_stdcmdILi1186EE4initE = Pointer_make([0], 0); return __ZN7_stdcmdILi1186EE4initE });
this.__defineGetter__("__str110", function() { delete __str110; __str110 = Pointer_make([115,116,114,99,109,112,0] /* strcmp\00*/, 0); return __str110 });
this.__defineGetter__("__ZN7_stdcmdILi1187EE4initE", function() { delete __ZN7_stdcmdILi1187EE4initE; __ZN7_stdcmdILi1187EE4initE = Pointer_make([0], 0); return __ZN7_stdcmdILi1187EE4initE });
this.__defineGetter__("__str111", function() { delete __str111; __str111 = Pointer_make([61,115,0] /* =s\00*/, 0); return __str111 });
this.__defineGetter__("__ZN7_stdcmdILi1188EE4initE", function() { delete __ZN7_stdcmdILi1188EE4initE; __ZN7_stdcmdILi1188EE4initE = Pointer_make([0], 0); return __ZN7_stdcmdILi1188EE4initE });
this.__defineGetter__("__str112", function() { delete __str112; __str112 = Pointer_make([33,61,115,0] /* !=s\00*/, 0); return __str112 });
this.__defineGetter__("__ZN7_stdcmdILi1189EE4initE", function() { delete __ZN7_stdcmdILi1189EE4initE; __ZN7_stdcmdILi1189EE4initE = Pointer_make([0], 0); return __ZN7_stdcmdILi1189EE4initE });
this.__defineGetter__("__str113", function() { delete __str113; __str113 = Pointer_make([60,115,0] /* <s\00*/, 0); return __str113 });
this.__defineGetter__("__ZN7_stdcmdILi1190EE4initE", function() { delete __ZN7_stdcmdILi1190EE4initE; __ZN7_stdcmdILi1190EE4initE = Pointer_make([0], 0); return __ZN7_stdcmdILi1190EE4initE });
this.__defineGetter__("__str114", function() { delete __str114; __str114 = Pointer_make([62,115,0] /* >s\00*/, 0); return __str114 });
this.__defineGetter__("__ZN7_stdcmdILi1191EE4initE", function() { delete __ZN7_stdcmdILi1191EE4initE; __ZN7_stdcmdILi1191EE4initE = Pointer_make([0], 0); return __ZN7_stdcmdILi1191EE4initE });
this.__defineGetter__("__str115", function() { delete __str115; __str115 = Pointer_make([60,61,115,0] /* <=s\00*/, 0); return __str115 });
this.__defineGetter__("__ZN7_stdcmdILi1192EE4initE", function() { delete __ZN7_stdcmdILi1192EE4initE; __ZN7_stdcmdILi1192EE4initE = Pointer_make([0], 0); return __ZN7_stdcmdILi1192EE4initE });
this.__defineGetter__("__str116", function() { delete __str116; __str116 = Pointer_make([62,61,115,0] /* >=s\00*/, 0); return __str116 });
this.__defineGetter__("__ZN7_stdcmdILi1193EE4initE", function() { delete __ZN7_stdcmdILi1193EE4initE; __ZN7_stdcmdILi1193EE4initE = Pointer_make([0], 0); return __ZN7_stdcmdILi1193EE4initE });
this.__defineGetter__("__str117", function() { delete __str117; __str117 = Pointer_make([101,99,104,111,0] /* echo\00*/, 0); return __str117 });
this.__defineGetter__("__ZN7_stdcmdILi1194EE4initE", function() { delete __ZN7_stdcmdILi1194EE4initE; __ZN7_stdcmdILi1194EE4initE = Pointer_make([0], 0); return __ZN7_stdcmdILi1194EE4initE });
this.__defineGetter__("__str118", function() { delete __str118; __str118 = Pointer_make([101,114,114,111,114,0] /* error\00*/, 0); return __str118 });
this.__defineGetter__("__ZN7_stdcmdILi1195EE4initE", function() { delete __ZN7_stdcmdILi1195EE4initE; __ZN7_stdcmdILi1195EE4initE = Pointer_make([0], 0); return __ZN7_stdcmdILi1195EE4initE });
this.__defineGetter__("__str119", function() { delete __str119; __str119 = Pointer_make([115,116,114,115,116,114,0] /* strstr\00*/, 0); return __str119 });
this.__defineGetter__("__ZN7_stdcmdILi1196EE4initE", function() { delete __ZN7_stdcmdILi1196EE4initE; __ZN7_stdcmdILi1196EE4initE = Pointer_make([0], 0); return __ZN7_stdcmdILi1196EE4initE });
this.__defineGetter__("__str120", function() { delete __str120; __str120 = Pointer_make([115,116,114,108,101,110,0] /* strlen\00*/, 0); return __str120 });
this.__defineGetter__("__ZN7_stdcmdILi1197EE4initE", function() { delete __ZN7_stdcmdILi1197EE4initE; __ZN7_stdcmdILi1197EE4initE = Pointer_make([0], 0); return __ZN7_stdcmdILi1197EE4initE });
this.__defineGetter__("__str121", function() { delete __str121; __str121 = Pointer_make([115,116,114,114,101,112,108,97,99,101,0] /* strreplace\00*/, 0); return __str121 });
this.__defineGetter__("__ZN7_stdcmdILi1223EE4initE", function() { delete __ZN7_stdcmdILi1223EE4initE; __ZN7_stdcmdILi1223EE4initE = Pointer_make([0], 0); return __ZN7_stdcmdILi1223EE4initE });
this.__defineGetter__("__str122", function() { delete __str122; __str122 = Pointer_make([109,97,120,99,111,110,0] /* maxcon\00*/, 0); return __str122 });
this.__defineGetter__("__str123", function() { delete __str123; __str123 = Pointer_make([115,111,109,101,118,97,114,0] /* somevar\00*/, 0); return __str123 });
this.__defineGetter__("_somevar", function() { delete _somevar; _somevar = Pointer_make([0], 0); return _somevar });
this.__defineGetter__("__str124", function() { delete __str124; __str124 = Pointer_make([59,10,0,0] /* ;\0A\00\00*/, 0); return __str124 });
this.__defineGetter__("__str125", function() { delete __str125; __str125 = Pointer_make([117,110,107,110,111,119,110,32,99,111,109,109,97,110,100,58,32,37,115,0] /* unknown command: %s\00*/, 0); return __str125 });
this.__defineGetter__("__str126", function() { delete __str126; __str126 = Pointer_make([98,117,105,108,116,105,110,32,100,101,99,108,97,114,101,100,32,119,105,116,104,32,105,108,108,101,103,97,108,32,116,121,112,101,0] /* builtin declared with illegal type\00*/, 0); return __str126 });
this.__defineGetter__("__str127", function() { delete __str127; __str127 = Pointer_make([98,117,105,108,116,105,110,32,100,101,99,108,97,114,101,100,32,119,105,116,104,32,116,111,111,32,109,97,110,121,32,97,114,103,115,32,40,117,115,101,32,86,63,41,0] /* builtin declared with too many args (use V?)\00*/, 0); return __str127 });
this.__defineGetter__("__str128", function() { delete __str128; __str128 = Pointer_make([37,115,32,61,32,48,120,37,46,54,88,32,40,37,100,44,32,37,100,44,32,37,100,41,0] /* %s = 0x%.6X (%d, %d, %d)\00*/, 0); return __str128 });
this.__defineGetter__("__str129", function() { delete __str129; __str129 = Pointer_make([37,115,32,61,32,48,120,37,88,0] /* %s = 0x%X\00*/, 0); return __str129 });
this.__defineGetter__("__str130", function() { delete __str130; __str130 = Pointer_make([37,115,32,61,32,37,100,0] /* %s = %d\00*/, 0); return __str130 });
this.__defineGetter__("__str131", function() { delete __str131; __str131 = Pointer_make([37,115,32,61,32,37,115,0] /* %s = %s\00*/, 0); return __str131 });
this.__defineGetter__("__str132", function() { delete __str132; __str132 = Pointer_make([37,115,32,61,32,91,37,115,93,0] /* %s = [%s]\00*/, 0); return __str132 });
this.__defineGetter__("__str133", function() { delete __str133; __str133 = Pointer_make([37,115,32,61,32,34,37,115,34,0] /* %s = \22%s\22\00*/, 0); return __str133 });
this.__defineGetter__("__ZGVZ10executeretPKcE6argids", function() { delete __ZGVZ10executeretPKcE6argids; __ZGVZ10executeretPKcE6argids = Pointer_make([0], 0); return __ZGVZ10executeretPKcE6argids });
this.__defineGetter__("__str134", function() { delete __str134; __str134 = Pointer_make([97,114,103,37,100,0] /* arg%d\00*/, 0); return __str134 });
this.__defineGetter__("__str135", function() { delete __str135; __str135 = Pointer_make([115,111,109,101,118,97,114,32,57,0] /* somevar 9\00*/, 0); return __str135 });
this.__defineGetter__("__str136", function() { delete __str136; __str136 = Pointer_make([116,101,109,112,32,61,32,40,43,32,50,50,32,36,115,111,109,101,118,97,114,41,0] /* temp = (+ 22 $somevar)\00*/, 0); return __str136 });
this.__defineGetter__("__str137", function() { delete __str137; __str137 = Pointer_make([105,102,32,40,62,32,36,116,101,109,112,32,51,48,41,32,91,32,116,101,109,112,32,61,32,40,43,32,36,116,101,109,112,32,49,41,32,93,32,91,32,116,101,109,112,32,61,32,40,42,32,36,116,101,109,112,32,50,41,32,93,0] /* if (> $temp 30) [ temp = (+ $temp 1) ] [ temp = (* $temp 2) ]\00*/, 0); return __str137 });
this.__defineGetter__("__str138", function() { delete __str138; __str138 = Pointer_make([105,102,32,40,60,32,36,116,101,109,112,32,51,48,41,32,91,32,116,101,109,112,32,61,32,48,32,93,32,91,32,116,101,109,112,32,61,32,40,43,32,36,116,101,109,112,32,49,41,32,93,0] /* if (< $temp 30) [ temp = 0 ] [ temp = (+ $temp 1) ]\00*/, 0); return __str138 });
this.__defineGetter__("__str139", function() { delete __str139; __str139 = Pointer_make([101,99,104,111,32,91,84,101,109,112,32,105,115,93,32,36,116,101,109,112,0] /* echo [Temp is] $temp\00*/, 0); return __str139 });
this.__defineGetter__("__str140", function() { delete __str140; __str140 = Pointer_make([37,100,10,0] /* %d\0A\00*/, 0); return __str140 });
this.__defineGetter__("__str141", function() { delete __str141; __str141 = Pointer_make([120,32,61,32,50,0] /* x = 2\00*/, 0); return __str141 });
this.__defineGetter__("__str142", function() { delete __str142; __str142 = Pointer_make([112,117,115,104,32,120,32,53,0] /* push x 5\00*/, 0); return __str142 });
this.__defineGetter__("__str143", function() { delete __str143; __str143 = Pointer_make([112,117,115,104,32,120,32,49,49,0] /* push x 11\00*/, 0); return __str143 });
this.__defineGetter__("__str144", function() { delete __str144; __str144 = Pointer_make([112,111,112,32,120,0] /* pop x\00*/, 0); return __str144 });
this.__defineGetter__("__str145", function() { delete __str145; __str145 = Pointer_make([101,99,104,111,32,36,120,0] /* echo $x\00*/, 0); return __str145 });
this.__defineGetter__("__str146", function() { delete __str146; __str146 = Pointer_make([103,114,101,101,116,32,61,32,91,32,101,99,104,111,32,104,101,108,108,111,44,32,36,97,114,103,49,32,93,0] /* greet = [ echo hello, $arg1 ]\00*/, 0); return __str146 });
this.__defineGetter__("__str147", function() { delete __str147; __str147 = Pointer_make([103,114,101,101,116,32,101,118,101,114,121,111,110,101,0] /* greet everyone\00*/, 0); return __str147 });
this.__defineGetter__("__str148", function() { delete __str148; __str148 = Pointer_make([48,0] /* 0\00*/, 0); return __str148 });
this.__defineGetter__("__ZL7bufnest", function() { delete __ZL7bufnest; __ZL7bufnest = Pointer_make([0], 0); return __ZL7bufnest });
this.__defineGetter__("__str149", function() { delete __str149; __str149 = Pointer_make([13,64,34,47,40,41,91,93,0] /* \0D@\22/()[]\00*/, 0); return __str149 });
this.__defineGetter__("__str150", function() { delete __str150; __str150 = Pointer_make([10,0,0] /* \0A\00\00*/, 0); return __str150 });
this.__defineGetter__("__str151", function() { delete __str151; __str151 = Pointer_make([109,105,115,115,105,110,103,32,34,37,99,34,0] /* missing \22%c\22\00*/, 0); return __str151 });
this.__defineGetter__("__str152", function() { delete __str152; __str152 = Pointer_make([32,9,13,0] /*  \09\0D\00*/, 0); return __str152 });
this.__defineGetter__("__str153", function() { delete __str153; __str153 = Pointer_make([47,59,32,9,13,10,0,0] /* /; \09\0D\0A\00\00*/, 0); return __str153 });
this.__defineGetter__("__ZGVZ10parsemacroRPKciR6vectorIcEE5ident", function() { delete __ZGVZ10parsemacroRPKciR6vectorIcEE5ident; __ZGVZ10parsemacroRPKciR6vectorIcEE5ident = Pointer_make([0], 0); return __ZGVZ10parsemacroRPKciR6vectorIcEE5ident });
this.__defineGetter__("_commandmillis", function() { delete _commandmillis; _commandmillis = Pointer_make([-1], 0); return _commandmillis });
this.__defineGetter__("_commandbuf", function() { delete _commandbuf; _commandbuf = Pointer_make([0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0], 0); return _commandbuf });
this.__defineGetter__("_commandaction", function() { delete _commandaction; _commandaction = Pointer_make([0], 0); return _commandaction });
this.__defineGetter__("_commandprompt", function() { delete _commandprompt; _commandprompt = Pointer_make([0], 0); return _commandprompt });
this.__defineGetter__("_commandpos", function() { delete _commandpos; _commandpos = Pointer_make([-1], 0); return _commandpos });

__globalConstructor__ = function() {
  __GLOBAL__I__Z11exchangestrPcPKc();
}


// stub for __ZdaPv
_vsnprintf = function (dst, num, src, ptr) {
    var args = Array_copy(ptr+1, HEAP[ptr]); // # of args in in first place
    var text = __formatString.apply(null, [src].concat(args));
    for (var i = 0; i < num; i++) {
      HEAP[dst+i] = HEAP[text+i];
      if (HEAP[dst+i] == 0) break;
    }
  }
// stub for _llvm_va_start
// stub for _llvm_va_end
// stub for _puts
// stub for _exit
// stub for _time
// stub for __Znaj
// stub for _llvm_memcpy_i32
_strncpy = function (pdest, psrc, num) {
    var padding = false;
    for (var i = 0; i < num; i++) {
      HEAP[pdest+i] = padding ? 0 : HEAP[psrc+i];
      padding = padding || HEAP[psrc+i] == 0;
    }
  }
_strlen = function (ptr) {
    var i = 0;
    while (HEAP[ptr+i] != 0) i++;
    return i;
  }
// stub for _strstr
// stub for _llvm_eh_exception
// stub for _llvm_eh_selector
// stub for _llvm_eh_typeid_for
// stub for __ZSt9terminatev
// stub for ___gxx_personality_v0
// stub for __Unwind_Resume_or_Rethrow
_strcmp = function (px, py) {
    var i = 0;
    while (true) {
      var x = HEAP[px+i];
      var y = HEAP[py+i];
      if (x == y && x == 0) return 0;
      if (x == 0) return -1;
      if (y == 0) return 1;
      if (x == y) {
        i ++;
        continue;
      } else {
        return x > y ? 1 : -1;
      }
    }
  }
_strtol = function (ptr) {
    // XXX: We ignore the other two params!
    return parseInt(Pointer_stringify(ptr));
  }
// stub for _strtod
// stub for _llvm_exp_f64
// stub for _llvm_log10_f64
// stub for _llvm_log_f64
// stub for _llvm_pow_f64
// stub for _sqrt
// stub for _atan
// stub for _acos
// stub for _asin
// stub for _tan
// stub for _cos
// stub for _sin
// stub for _fmod
_strspn = function (pstr, pset) {
    var str = String_copy(pstr, true);
    var set = String_copy(pset);
    var i = 0;
    while (set.indexOf(str[i]) != -1) i++; // Must halt, as 0 is in str but not set
    return i;
  }
_strcspn = function (pstr, pset) {
    var str = String_copy(pstr, true);
    var set = String_copy(pset, true);
    var i = 0;
    while (set.indexOf(str[i]) == -1) i++; // Must halt, as 0 is in both
    return i;
  }
// stub for _strncmp
_strcat = function (pdest, psrc) {
    var len = Pointer_stringify(pdest).length; // TODO: use strlen, but need dependencies system
    var i = 0;
    do {
      HEAP[pdest+len+i] = HEAP[psrc+i];
      i ++;
    } while (HEAP[psrc+i-1] != 0);
  }
// stub for __Znwj
// stub for __ZdlPv
___cxa_atexit = function (func) {
    __ATEXIT__.push(func);
  }
// stub for _strchr
___cxa_guard_acquire = function () {
    return 0;
  }
___cxa_guard_release = function () {
    return 0;
  }
// stub for _printf
// stub for _sprintf
// stub for _isalnum


function __GLOBAL__I__Z11exchangestrPcPKc() {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      __Z41__static_initialization_and_destruction_0ii(1, 65535);
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __Z11exchangestrPcPKc(_o, _n) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _o_addr = Pointer_make([0], 0);
      var _n_addr = Pointer_make([0], 0);
      var _retval = Pointer_make([0], 0);
      var _0 = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_o_addr] = _o;
      HEAP[_n_addr] = _n;
      var _1 = HEAP[_o_addr];
      var _2 = 0+(_1 != 0);
      if (_2) { __label__ = 0; break; } else { __label__ = 1; break; }
    case 0: // _bb
      var _3 = HEAP[_o_addr];
      __ZdaPv(_3);
      __label__ = 1; break;
    case 1: // _bb1
      var _4 = HEAP[_n_addr];
      var _5 = __Z9newstringPKc(_4);
      HEAP[_0] = _5;
      var _6 = HEAP[_0];
      HEAP[_retval] = _6;
      __label__ = 2; break;
    case 2: // _return
      var _retval2 = HEAP[_retval];
      return _retval2;
  }
}


function __ZnwjPv(_unnamed_arg, ___p) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _unnamed_arg_addr;
      var ___p_addr = Pointer_make([0], 0);
      var _retval = Pointer_make([0], 0);
      var _0 = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      _unnamed_arg_addr = _unnamed_arg;
      HEAP[___p_addr] = ___p;
      var _1 = HEAP[___p_addr];
      HEAP[_0] = _1;
      var _2 = HEAP[_0];
      HEAP[_0] = _2;
      var _3 = HEAP[_0];
      HEAP[_retval] = _3;
      __label__ = 2; break;
    case 2: // _return
      var _retval1 = HEAP[_retval];
      return _retval1;
  }
}


function __ZN15stringformatterC1EPc(_this, _buf) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _this_addr = Pointer_make([0], 0);
      var _buf_addr = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_this_addr] = _this;
      HEAP[_buf_addr] = _buf;
      var _0 = HEAP[_this_addr];
      var _1 = _0+0*1;
      var _2 = HEAP[_buf_addr];
      HEAP[_1] = _2;
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __ZL6hthashPKc(_key) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _key_addr = Pointer_make([0], 0);
      var _retval;
      var _0;
      var _retval_5;
      var _h;
      var _i;
      var _k;
      var __alloca_point_ = 0;
      HEAP[_key_addr] = _key;
      _h = 5381;
      _i = 0;
      __label__ = 1; break;
    case 0: // _bb
      var _2 = _h << 5;
      var _4 = _2 + _h;
      _h = _4 ^ _k;
      _i = _i + 1;
      __label__ = 1; break;
    case 1: // _bb1
      var _9 = HEAP[_key_addr];
      var _12 = HEAP[0 + _9+_i];
      _k = _12;
      var _15 = 0+(_k != 0);
      _retval_5 = _15;
      var _toBool = 0+(_retval_5 != 0);
      if (_toBool) { __label__ = 0; break; } else { __label__ = 3; break; }
    case 3: // _bb2
      _0 = _h;
      _retval = _0;
      __label__ = 2; break;
    case 2: // _return
      var _retval3 = _retval;
      return _retval3;
  }
}


function __ZN5identC1Ev(_this) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _this_addr = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_this_addr] = _this;
      var _0 = HEAP[_this_addr];
      var _1 = _0+0*1;
      HEAP[_1] = __ZTV5ident+2*1;
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __ZN5identD1Ev(_this) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _this_addr = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_this_addr] = _this;
      var _0 = HEAP[_this_addr];
      var _1 = _0+0*1;
      HEAP[_1] = __ZTV5ident+2*1;
      __label__ = 0; break;
    case 0: // _bb
      var _2 = 0;
      var _toBool = 0+(_2 != 0);
      if (_toBool) { __label__ = 1; break; } else { __label__ = 3; break; }
    case 1: // _bb1
      var _3 = HEAP[_this_addr];
      __ZdlPv(_3);
      __label__ = 3; break;
    case 3: // _bb2
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __ZN5identD0Ev(_this) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _this_addr = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_this_addr] = _this;
      var _0 = HEAP[_this_addr];
      var _1 = _0+0*1;
      HEAP[_1] = __ZTV5ident+2*1;
      __label__ = 0; break;
    case 0: // _bb
      var _2 = 1;
      var _toBool = 0+(_2 != 0);
      if (_toBool) { __label__ = 1; break; } else { __label__ = 3; break; }
    case 1: // _bb1
      var _3 = HEAP[_this_addr];
      __ZdlPv(_3);
      __label__ = 3; break;
    case 3: // _bb2
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __ZN5ident7changedEv(_this) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _this_addr = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_this_addr] = _this;
      var _0 = HEAP[_this_addr];
      var _1 = _0+6*1;
      var _3 = HEAP[0 + _1];
      var _4 = 0+(_3 != 0);
      if (_4) { __label__ = 0; break; } else { __label__ = 1; break; }
    case 0: // _bb
      var _5 = HEAP[_this_addr];
      var _6 = _5+6*1;
      var _8 = HEAP[0 + _6];
      _8();
      __label__ = 1; break;
    case 1: // _bb1
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __ZN5identC1EiPKciiiPiPvi(_this, _t, _n, _m, _c, _x, _s, _f, _flags) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _this_addr = Pointer_make([0], 0);
      var _t_addr;
      var _n_addr = Pointer_make([0], 0);
      var _m_addr;
      var _c_addr;
      var _x_addr;
      var _s_addr = Pointer_make([0], 0);
      var _f_addr = Pointer_make([0], 0);
      var _flags_addr;
      var _iftmp_17;
      var __alloca_point_ = 0;
      HEAP[_this_addr] = _this;
      _t_addr = _t;
      HEAP[_n_addr] = _n;
      _m_addr = _m;
      _c_addr = _c;
      _x_addr = _x;
      HEAP[_s_addr] = _s;
      HEAP[_f_addr] = _f;
      _flags_addr = _flags;
      var _0 = HEAP[_this_addr];
      var _1 = _0+0*1;
      HEAP[_1] = __ZTV5ident+2*1;
      var _2 = HEAP[_this_addr];
      var _3 = _2+1*1;
      HEAP[_3] = _t_addr;
      var _5 = HEAP[_this_addr];
      var _6 = _5+2*1;
      var _7 = HEAP[_n_addr];
      HEAP[_6] = _7;
      var _8 = HEAP[_this_addr];
      var _9 = _8+3*1;
      var _10 = _9;
      HEAP[_10] = _m_addr;
      var _12 = HEAP[_this_addr];
      var _13 = _12+4*1;
      var _14 = _13;
      HEAP[_14] = _x_addr;
      var _16 = HEAP[_this_addr];
      HEAP[0 + _16+5*1] = 2147483647;
      var _18 = HEAP[_f_addr];
      var _19 = _18;
      var _20 = HEAP[_this_addr];
      var _21 = _20+6*1;
      HEAP[0 + _21] = _19;
      var _25 = 0+(_m_addr > _x_addr);
      if (_25) { __label__ = 0; break; } else { __label__ = 1; break; }
    case 0: // _bb
      _iftmp_17 = 8;
      __label__ = 3; break;
    case 1: // _bb1
      _iftmp_17 = 0;
      __label__ = 3; break;
    case 3: // _bb2
      var _28 = _iftmp_17 | _flags_addr;
      var _29 = HEAP[_this_addr];
      HEAP[0 + _29+10*1] = _28;
      var _31 = HEAP[_this_addr];
      var _32 = _31+7*1;
      var _33 = _32+0*1;
      var _35 = _33;
      HEAP[_35] = _c_addr;
      var _37 = HEAP[_this_addr];
      var _38 = _37+9*1;
      var _39 = _38;
      var _40 = HEAP[_s_addr];
      HEAP[_39] = _40;
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __ZN5identC1EiPKcfffPfPvi(_this, _t, _n, _m, _c, _x, _s, _f, _flags) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _this_addr = Pointer_make([0], 0);
      var _t_addr;
      var _n_addr = Pointer_make([0], 0);
      var _m_addr;
      var _c_addr;
      var _x_addr;
      var _s_addr = Pointer_make([0], 0);
      var _f_addr = Pointer_make([0], 0);
      var _flags_addr;
      var _iftmp_19;
      var __alloca_point_ = 0;
      HEAP[_this_addr] = _this;
      _t_addr = _t;
      HEAP[_n_addr] = _n;
      _m_addr = _m;
      _c_addr = _c;
      _x_addr = _x;
      HEAP[_s_addr] = _s;
      HEAP[_f_addr] = _f;
      _flags_addr = _flags;
      var _0 = HEAP[_this_addr];
      var _1 = _0+0*1;
      HEAP[_1] = __ZTV5ident+2*1;
      var _2 = HEAP[_this_addr];
      var _3 = _2+1*1;
      HEAP[_3] = _t_addr;
      var _5 = HEAP[_this_addr];
      var _6 = _5+2*1;
      var _7 = HEAP[_n_addr];
      HEAP[_6] = _7;
      var _8 = HEAP[_this_addr];
      var _9 = _8+3*1;
      var _10 = _9;
      var _11 = _10;
      HEAP[_11] = _m_addr;
      var _13 = HEAP[_this_addr];
      var _14 = _13+4*1;
      var _15 = _14;
      var _16 = _15;
      HEAP[_16] = _x_addr;
      var _18 = HEAP[_this_addr];
      HEAP[0 + _18+5*1] = 2147483647;
      var _20 = HEAP[_f_addr];
      var _21 = _20;
      var _22 = HEAP[_this_addr];
      var _23 = _22+6*1;
      HEAP[0 + _23] = _21;
      var _27 = 0+(_m_addr > _x_addr);
      if (_27) { __label__ = 0; break; } else { __label__ = 1; break; }
    case 0: // _bb
      _iftmp_19 = 8;
      __label__ = 3; break;
    case 1: // _bb1
      _iftmp_19 = 0;
      __label__ = 3; break;
    case 3: // _bb2
      var _30 = _iftmp_19 | _flags_addr;
      var _31 = HEAP[_this_addr];
      HEAP[0 + _31+10*1] = _30;
      var _33 = HEAP[_this_addr];
      var _34 = _33+7*1;
      var _35 = _34+0*1;
      var _37 = _35;
      var _38 = _37;
      HEAP[_38] = _c_addr;
      var _40 = HEAP[_this_addr];
      var _41 = _40+9*1;
      var _42 = _41;
      var _43 = _42;
      var _44 = HEAP[_s_addr];
      HEAP[_43] = _44;
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __ZN5identC1EiPKcPcPS2_Pvi(_this, _t, _n, _c, _s, _f, _flags) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _this_addr = Pointer_make([0], 0);
      var _t_addr;
      var _n_addr = Pointer_make([0], 0);
      var _c_addr = Pointer_make([0], 0);
      var _s_addr = Pointer_make([0], 0);
      var _f_addr = Pointer_make([0], 0);
      var _flags_addr;
      var __alloca_point_ = 0;
      HEAP[_this_addr] = _this;
      _t_addr = _t;
      HEAP[_n_addr] = _n;
      HEAP[_c_addr] = _c;
      HEAP[_s_addr] = _s;
      HEAP[_f_addr] = _f;
      _flags_addr = _flags;
      var _0 = HEAP[_this_addr];
      var _1 = _0+0*1;
      HEAP[_1] = __ZTV5ident+2*1;
      var _2 = HEAP[_this_addr];
      var _3 = _2+1*1;
      HEAP[_3] = _t_addr;
      var _5 = HEAP[_this_addr];
      var _6 = _5+2*1;
      var _7 = HEAP[_n_addr];
      HEAP[_6] = _7;
      var _8 = HEAP[_this_addr];
      HEAP[0 + _8+5*1] = 2147483647;
      var _10 = HEAP[_f_addr];
      var _11 = _10;
      var _12 = HEAP[_this_addr];
      var _13 = _12+6*1;
      HEAP[0 + _13] = _11;
      var _15 = HEAP[_this_addr];
      var _16 = _15+10*1;
      HEAP[_16] = _flags_addr;
      var _18 = HEAP[_this_addr];
      var _19 = _18+7*1;
      var _20 = _19+0*1;
      var _22 = _20;
      var _23 = _22;
      var _24 = HEAP[_c_addr];
      HEAP[_23] = _24;
      var _25 = HEAP[_this_addr];
      var _26 = _25+9*1;
      var _27 = _26;
      var _28 = _27;
      var _29 = HEAP[_s_addr];
      HEAP[_28] = _29;
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __ZN5identC1EiPKcPci(_this, _t, _n, _a, _flags) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _this_addr = Pointer_make([0], 0);
      var _t_addr;
      var _n_addr = Pointer_make([0], 0);
      var _a_addr = Pointer_make([0], 0);
      var _flags_addr;
      var __alloca_point_ = 0;
      HEAP[_this_addr] = _this;
      _t_addr = _t;
      HEAP[_n_addr] = _n;
      HEAP[_a_addr] = _a;
      _flags_addr = _flags;
      var _0 = HEAP[_this_addr];
      var _1 = _0+0*1;
      HEAP[_1] = __ZTV5ident+2*1;
      var _2 = HEAP[_this_addr];
      var _3 = _2+1*1;
      HEAP[_3] = _t_addr;
      var _5 = HEAP[_this_addr];
      var _6 = _5+2*1;
      var _7 = HEAP[_n_addr];
      HEAP[_6] = _7;
      var _8 = HEAP[_this_addr];
      HEAP[0 + _8+5*1] = 2147483647;
      var _10 = HEAP[_this_addr];
      var _11 = _10+6*1;
      var _12 = _11;
      HEAP[_12] = 0;
      var _14 = HEAP[_this_addr];
      var _15 = _14+7*1;
      var _16 = _15+0*1;
      var _17 = HEAP[_a_addr];
      HEAP[_16] = _17;
      var _18 = HEAP[_this_addr];
      var _19 = _18+8*1;
      HEAP[0 + _19+0*1] = 0;
      var _21 = HEAP[_this_addr];
      var _22 = _21+10*1;
      HEAP[_22] = _flags_addr;
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __ZN5identC1EiPKcS1_PvS2_i(_this, _t, _n, _narg, _f, _s, _flags) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _this_addr = Pointer_make([0], 0);
      var _t_addr;
      var _n_addr = Pointer_make([0], 0);
      var _narg_addr = Pointer_make([0], 0);
      var _f_addr = Pointer_make([0], 0);
      var _s_addr = Pointer_make([0], 0);
      var _flags_addr;
      var __alloca_point_ = 0;
      HEAP[_this_addr] = _this;
      _t_addr = _t;
      HEAP[_n_addr] = _n;
      HEAP[_narg_addr] = _narg;
      HEAP[_f_addr] = _f;
      HEAP[_s_addr] = _s;
      _flags_addr = _flags;
      var _0 = HEAP[_this_addr];
      var _1 = _0+0*1;
      HEAP[_1] = __ZTV5ident+2*1;
      var _2 = HEAP[_this_addr];
      var _3 = _2+1*1;
      HEAP[_3] = _t_addr;
      var _5 = HEAP[_this_addr];
      var _6 = _5+2*1;
      var _7 = HEAP[_n_addr];
      HEAP[_6] = _7;
      var _8 = HEAP[_this_addr];
      HEAP[0 + _8+5*1] = 2147483647;
      var _10 = HEAP[_f_addr];
      var _11 = _10;
      var _12 = HEAP[_this_addr];
      var _13 = _12+6*1;
      HEAP[0 + _13] = _11;
      var _15 = HEAP[_this_addr];
      var _16 = _15+7*1;
      var _17 = _16+0*1;
      var _18 = HEAP[_narg_addr];
      HEAP[_17] = _18;
      var _19 = HEAP[_this_addr];
      var _20 = _19+8*1;
      var _21 = _20+0*1;
      var _22 = HEAP[_s_addr];
      HEAP[_21] = _22;
      var _23 = HEAP[_this_addr];
      var _24 = _23+10*1;
      HEAP[_24] = _flags_addr;
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __Z11parsestringPKc(_p) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _p_addr = Pointer_make([0], 0);
      var _retval = Pointer_make([0], 0);
      var _retval_89;
      var _0 = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_p_addr] = _p;
      __label__ = 4; break;
    case 0: // _bb
      var _1 = HEAP[_p_addr];
      var _2 = HEAP[_1];
      if (_2 == 10) {
  __label__ = 1; break;
}
else if (_2 == 13) {
  __label__ = 1; break;
}
else if (_2 == 34) {
  __label__ = 1; break;
}
else if (_2 == 94) {
  __label__ = 3; break;
}
else {
__label__ = 6; break;
}

    case 1: // _bb1
      var _3 = HEAP[_p_addr];
      HEAP[_0] = _3;
      __label__ = 5; break;
    case 3: // _bb2
      var _4 = HEAP[_p_addr];
      var _5 = _4+1;
      HEAP[_p_addr] = _5;
      var _6 = HEAP[_p_addr];
      var _7 = HEAP[_6];
      var _8 = 0+(_7 != 0);
      _retval_89 = _8;
      var _toBool = 0+(_retval_89 != 0);
      if (_toBool) { __label__ = 6; break; } else { __label__ = 7; break; }
    case 7: // _bb3
      var _11 = HEAP[_p_addr];
      HEAP[_0] = _11;
      __label__ = 5; break;
    case 6: // _bb4
      var _12 = HEAP[_p_addr];
      var _13 = _12+1;
      HEAP[_p_addr] = _13;
      __label__ = 4; break;
    case 4: // _bb5
      var _14 = HEAP[_p_addr];
      var _15 = HEAP[_14];
      var _16 = 0+(_15 != 0);
      if (_16) { __label__ = 0; break; } else { __label__ = 8; break; }
    case 8: // _bb6
      var _17 = HEAP[_p_addr];
      HEAP[_0] = _17;
      __label__ = 5; break;
    case 5: // _bb7
      var _18 = HEAP[_0];
      HEAP[_retval] = _18;
      __label__ = 2; break;
    case 2: // _return
      var _retval8 = HEAP[_retval];
      return _retval8;
  }
}


function __Z12escapestringPcPKcS1_(_dst, _src, _end) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _dst_addr = Pointer_make([0], 0);
      var _src_addr = Pointer_make([0], 0);
      var _end_addr = Pointer_make([0], 0);
      var _retval;
      var _0;
      var _start = Pointer_make([0], 0);
      var _c;
      var _e;
      var __alloca_point_ = 0;
      HEAP[_dst_addr] = _dst;
      HEAP[_src_addr] = _src;
      HEAP[_end_addr] = _end;
      var _1 = HEAP[_dst_addr];
      HEAP[_start] = _1;
      __label__ = 9; break;
    case 0: // _bb
      var _2 = HEAP[_src_addr];
      var _3 = HEAP[_2];
      _c = _3;
      var _6 = _2+1;
      HEAP[_src_addr] = _6;
      var _8 = 0+(_c == 94);
      if (_8) { __label__ = 1; break; } else { __label__ = 10; break; }
    case 1: // _bb1
      var _9 = HEAP[_src_addr];
      var _10 = HEAP[_end_addr];
      var _11 = 0+(_9 >= _10);
      if (_11) { __label__ = 11; break; } else { __label__ = 3; break; }
    case 3: // _bb2
      var _12 = HEAP[_src_addr];
      var _13 = HEAP[_12];
      _e = _13;
      var _16 = _12+1;
      HEAP[_src_addr] = _16;
      if (_e == 102) {
  __label__ = 4; break;
}
else if (_e == 110) {
  __label__ = 7; break;
}
else if (_e == 116) {
  __label__ = 6; break;
}
else {
__label__ = 8; break;
}

    case 7: // _bb3
      var _18 = HEAP[_dst_addr];
      HEAP[_18] = 10;
      var _19 = HEAP[_dst_addr];
      var _20 = _19+1;
      HEAP[_dst_addr] = _20;
      __label__ = 5; break;
    case 6: // _bb4
      var _21 = HEAP[_dst_addr];
      HEAP[_21] = 9;
      var _22 = HEAP[_dst_addr];
      var _23 = _22+1;
      HEAP[_dst_addr] = _23;
      __label__ = 5; break;
    case 4: // _bb5
      var _24 = HEAP[_dst_addr];
      HEAP[_24] = 12;
      var _25 = HEAP[_dst_addr];
      var _26 = _25+1;
      HEAP[_dst_addr] = _26;
      __label__ = 5; break;
    case 8: // _bb6
      var _28 = _e;
      var _29 = HEAP[_dst_addr];
      HEAP[_29] = _28;
      var _30 = HEAP[_dst_addr];
      var _31 = _30+1;
      HEAP[_dst_addr] = _31;
      __label__ = 5; break;
    case 5: // _bb7
      __label__ = 9; break;
    case 10: // _bb8
      var _33 = _c;
      var _34 = HEAP[_dst_addr];
      HEAP[_34] = _33;
      var _35 = HEAP[_dst_addr];
      var _36 = _35+1;
      HEAP[_dst_addr] = _36;
      __label__ = 9; break;
    case 9: // _bb9
      var _37 = HEAP[_src_addr];
      var _38 = HEAP[_end_addr];
      var _39 = 0+(_37 < _38);
      if (_39) { __label__ = 0; break; } else { __label__ = 11; break; }
    case 11: // _bb10
      var _40 = HEAP[_dst_addr];
      var _41 = _40;
      var _42 = HEAP[_start];
      var _43 = _42;
      _0 = _41 - _43;
      _retval = _0;
      __label__ = 2; break;
    case 2: // _return
      var _retval11 = _retval;
      return _retval11;
  }
}


function __ZL9isintegerPc(_c) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _c_addr = Pointer_make([0], 0);
      var _retval;
      var _iftmp_106;
      var _0;
      var __alloca_point_ = 0;
      HEAP[_c_addr] = _c;
      var _1 = HEAP[_c_addr];
      var _3 = HEAP[0 + _1];
      var _4 = _3;
      var _5 = _4 - 48;
      var _6 = 0+(_5 <= 9);
      if (_6) { __label__ = 6; break; } else { __label__ = 0; break; }
    case 0: // _bb
      var _7 = HEAP[_c_addr];
      var _9 = HEAP[0 + _7];
      var _10 = 0+(_9 == 43);
      if (_10) { __label__ = 7; break; } else { __label__ = 1; break; }
    case 1: // _bb1
      var _11 = HEAP[_c_addr];
      var _13 = HEAP[0 + _11];
      var _14 = 0+(_13 == 45);
      if (_14) { __label__ = 7; break; } else { __label__ = 3; break; }
    case 3: // _bb2
      var _15 = HEAP[_c_addr];
      var _17 = HEAP[0 + _15];
      var _18 = 0+(_17 == 46);
      if (_18) { __label__ = 7; break; } else { __label__ = 4; break; }
    case 7: // _bb3
      var _19 = HEAP[_c_addr];
      var _21 = HEAP[0 + _19+1];
      var _22 = _21;
      var _23 = _22 - 48;
      var _24 = 0+(_23 <= 9);
      if (_24) { __label__ = 6; break; } else { __label__ = 4; break; }
    case 6: // _bb4
      _iftmp_106 = 1;
      __label__ = 8; break;
    case 4: // _bb5
      _iftmp_106 = 0;
      __label__ = 8; break;
    case 8: // _bb6
      _0 = _iftmp_106;
      _retval = _0;
      __label__ = 2; break;
    case 2: // _return
      var _retval78 = _retval;
      return _retval78;
  }
}


function __Z6seedMTj(_seed) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _seed_addr;
      var _retval_209;
      var _x;
      var _s = Pointer_make([0], 0);
      var _j;
      var __alloca_point_ = 0;
      _seed_addr = _seed;
      _x = _seed_addr | 1;
      HEAP[_s] = __ZL5state+0*1;
      HEAP[__ZL4left] = 0;
      var _2 = HEAP[_s];
      HEAP[_2] = _x;
      var _4 = HEAP[_s];
      var _5 = _4+1;
      HEAP[_s] = _5;
      _j = 624;
      __label__ = 1; break;
    case 0: // _bb
      _x = _x * 69069;
      var _8 = HEAP[_s];
      HEAP[_8] = _x;
      var _10 = HEAP[_s];
      var _11 = _10+1;
      HEAP[_s] = _11;
      __label__ = 1; break;
    case 1: // _bb1
      _j = _j - 1;
      var _15 = 0+(_j != 0);
      _retval_209 = _15;
      var _toBool = 0+(_retval_209 != 0);
      if (_toBool) { __label__ = 0; break; } else { __label__ = 3; break; }
    case 3: // _bb2
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __Z3maxIiET_S0_S0_(_a, _b) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _a_addr;
      var _b_addr;
      var _retval;
      var _iftmp_223;
      var _0;
      var __alloca_point_ = 0;
      _a_addr = _a;
      _b_addr = _b;
      var _3 = 0+(_a_addr > _b_addr);
      if (_3) { __label__ = 0; break; } else { __label__ = 1; break; }
    case 0: // _bb
      _iftmp_223 = _a_addr;
      __label__ = 3; break;
    case 1: // _bb1
      _iftmp_223 = _b_addr;
      __label__ = 3; break;
    case 3: // _bb2
      _0 = _iftmp_223;
      _retval = _0;
      __label__ = 2; break;
    case 2: // _return
      var _retval3 = _retval;
      return _retval3;
  }
}


function __Z9es_sizeofI5identEiPT_(_x) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _x_addr = Pointer_make([0], 0);
      var _retval;
      var _0;
      var __alloca_point_ = 0;
      HEAP[_x_addr] = _x;
      var _1 = HEAP[_x_addr];
      var _2 = _1+11*1;
      _0 = _2;
      _retval = _0;
      __label__ = 2; break;
    case 2: // _return
      var _retval1 = _retval;
      return _retval1;
  }
}


function __ZN9hashtableIPKc5identE7getdataEPv(_i) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _i_addr = Pointer_make([0], 0);
      var _retval = Pointer_make([0], 0);
      var _0 = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_i_addr] = _i;
      var _1 = HEAP[_i_addr];
      var _3 = _1+__struct_hashset_hashtableentry_const_char___ident_____chain____FLATTENER[0];
      var _4 = _3+__struct_hashtableentry_const_char__ident_____FLATTENER[1];
      HEAP[_0] = _4;
      var _5 = HEAP[_0];
      HEAP[_retval] = _5;
      __label__ = 2; break;
    case 2: // _return
      var _retval1 = HEAP[_retval];
      return _retval1;
  }
}


function __ZN9hashtableIPKc5identE7getnextEPv(_i) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _i_addr = Pointer_make([0], 0);
      var _retval = Pointer_make([0], 0);
      var _0 = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_i_addr] = _i;
      var _1 = HEAP[_i_addr];
      var _4 = HEAP[0 + _1+__struct_hashset_hashtableentry_const_char___ident_____chain____FLATTENER[1]];
      HEAP[_0] = _4;
      var _5 = HEAP[_0];
      HEAP[_retval] = _5;
      __label__ = 2; break;
    case 2: // _return
      var _retval1 = HEAP[_retval];
      return _retval1;
  }
}


function __Z3minIiET_S0_S0_(_a, _b) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _a_addr;
      var _b_addr;
      var _retval;
      var _iftmp_230;
      var _0;
      var __alloca_point_ = 0;
      _a_addr = _a;
      _b_addr = _b;
      var _3 = 0+(_a_addr < _b_addr);
      if (_3) { __label__ = 0; break; } else { __label__ = 1; break; }
    case 0: // _bb
      _iftmp_230 = _a_addr;
      __label__ = 3; break;
    case 1: // _bb1
      _iftmp_230 = _b_addr;
      __label__ = 3; break;
    case 3: // _bb2
      _0 = _iftmp_230;
      _retval = _0;
      __label__ = 2; break;
    case 2: // _return
      var _retval3 = _retval;
      return _retval3;
  }
}


function __Z3minIfET_S0_S0_(_a, _b) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _a_addr;
      var _b_addr;
      var _retval;
      var _iftmp_231;
      var _0;
      var __alloca_point_ = 0;
      _a_addr = _a;
      _b_addr = _b;
      var _3 = 0+(_a_addr < _b_addr);
      if (_3) { __label__ = 0; break; } else { __label__ = 1; break; }
    case 0: // _bb
      _iftmp_231 = _a_addr;
      __label__ = 3; break;
    case 1: // _bb1
      _iftmp_231 = _b_addr;
      __label__ = 3; break;
    case 3: // _bb2
      _0 = _iftmp_231;
      _retval = _0;
      __label__ = 2; break;
    case 2: // _return
      var _retval3 = _retval;
      return _retval3;
  }
}


function __Z3maxIfET_S0_S0_(_a, _b) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _a_addr;
      var _b_addr;
      var _retval;
      var _iftmp_232;
      var _0;
      var __alloca_point_ = 0;
      _a_addr = _a;
      _b_addr = _b;
      var _3 = 0+(_a_addr > _b_addr);
      if (_3) { __label__ = 0; break; } else { __label__ = 1; break; }
    case 0: // _bb
      _iftmp_232 = _a_addr;
      __label__ = 3; break;
    case 1: // _bb1
      _iftmp_232 = _b_addr;
      __label__ = 3; break;
    case 3: // _bb2
      _0 = _iftmp_232;
      _retval = _0;
      __label__ = 2; break;
    case 2: // _return
      var _retval3 = _retval;
      return _retval3;
  }
}


function __ZN6vectorIPS_IcEEC1Ev(_this) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _this_addr = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_this_addr] = _this;
      var _0 = HEAP[_this_addr];
      HEAP[0 + _0+0*1] = 0;
      var _2 = HEAP[_this_addr];
      HEAP[0 + _2+1*1] = 0;
      var _4 = HEAP[_this_addr];
      HEAP[0 + _4+2*1] = 0;
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __ZN6vectorIcEC1Ev(_this) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _this_addr = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_this_addr] = _this;
      var _0 = HEAP[_this_addr];
      HEAP[0 + _0+0*1] = 0;
      var _2 = HEAP[_this_addr];
      HEAP[0 + _2+1*1] = 0;
      var _4 = HEAP[_this_addr];
      HEAP[0 + _4+2*1] = 0;
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __ZN6vectorIcE7setsizeEi(_this, _i) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _this_addr = Pointer_make([0], 0);
      var _i_addr;
      var __alloca_point_ = 0;
      HEAP[_this_addr] = _this;
      _i_addr = _i;
      var _0 = HEAP[_this_addr];
      var _2 = HEAP[0 + _0+2*1];
      var _4 = _0+2*1;
      HEAP[_4] = _i_addr;
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __ZN6vectorIcE6getbufEv(_this) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _this_addr = Pointer_make([0], 0);
      var _retval = Pointer_make([0], 0);
      var _0 = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_this_addr] = _this;
      var _1 = HEAP[_this_addr];
      var _3 = HEAP[0 + _1+0*1];
      HEAP[_0] = _3;
      var _4 = HEAP[_0];
      HEAP[_retval] = _4;
      __label__ = 2; break;
    case 2: // _return
      var _retval1 = HEAP[_retval];
      return _retval1;
  }
}


function __ZNK6vectorIPS_IcEE6lengthEv(_this) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _this_addr = Pointer_make([0], 0);
      var _retval;
      var _0;
      var __alloca_point_ = 0;
      HEAP[_this_addr] = _this;
      var _1 = HEAP[_this_addr];
      var _0 = HEAP[0 + _1+2*1];
      _retval = _0;
      __label__ = 2; break;
    case 2: // _return
      var _retval1 = _retval;
      return _retval1;
  }
}


function __ZN6vectorIPS_IcEEixEi(_this, _i) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _this_addr = Pointer_make([0], 0);
      var _i_addr;
      var _retval = Pointer_make([0], 0);
      var _0 = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_this_addr] = _this;
      _i_addr = _i;
      var _2 = 0+(_i_addr >= 0);
      if (_2) { __label__ = 0; break; } else { __label__ = 1; break; }
    case 0: // _bb
      var _3 = HEAP[_this_addr];
      var _5 = HEAP[0 + _3+2*1];
      __label__ = 1; break;
    case 1: // _bb1
      var _6 = HEAP[_this_addr];
      var _8 = HEAP[0 + _6+0*1];
      var _10 = _8+_i_addr;
      HEAP[_0] = _10;
      var _11 = HEAP[_0];
      HEAP[_retval] = _11;
      __label__ = 2; break;
    case 2: // _return
      var _retval2 = HEAP[_retval];
      return _retval2;
  }
}


function __ZN6vectorIcE3popEv(_this) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _this_addr = Pointer_make([0], 0);
      var _retval = Pointer_make([0], 0);
      var _0 = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_this_addr] = _this;
      var _1 = HEAP[_this_addr];
      var _3 = HEAP[0 + _1+0*1];
      var _6 = HEAP[0 + _1+2*1];
      var _7 = _6 - 1;
      var _8 = HEAP[_this_addr];
      HEAP[0 + _8+2*1] = _7;
      var _10 = HEAP[_this_addr];
      var _12 = HEAP[0 + _10+2*1];
      var _13 = _3+_12;
      HEAP[_0] = _13;
      var _14 = HEAP[_0];
      HEAP[_retval] = _14;
      __label__ = 2; break;
    case 2: // _return
      var _retval1 = HEAP[_retval];
      return _retval1;
  }
}


function __ZNK6vectorIcE6lengthEv(_this) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _this_addr = Pointer_make([0], 0);
      var _retval;
      var _0;
      var __alloca_point_ = 0;
      HEAP[_this_addr] = _this;
      var _1 = HEAP[_this_addr];
      var _0 = HEAP[0 + _1+2*1];
      _retval = _0;
      __label__ = 2; break;
    case 2: // _return
      var _retval1 = _retval;
      return _retval1;
  }
}


function __ZN6vectorIP5identEC1Ev(_this) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _this_addr = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_this_addr] = _this;
      var _0 = HEAP[_this_addr];
      HEAP[0 + _0+0*1] = 0;
      var _2 = HEAP[_this_addr];
      HEAP[0 + _2+1*1] = 0;
      var _4 = HEAP[_this_addr];
      HEAP[0 + _4+2*1] = 0;
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __ZNK6vectorIP5identE6lengthEv(_this) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _this_addr = Pointer_make([0], 0);
      var _retval;
      var _0;
      var __alloca_point_ = 0;
      HEAP[_this_addr] = _this;
      var _1 = HEAP[_this_addr];
      var _0 = HEAP[0 + _1+2*1];
      _retval = _0;
      __label__ = 2; break;
    case 2: // _return
      var _retval1 = _retval;
      return _retval1;
  }
}


function __ZN6vectorIP5identEixEi(_this, _i) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _this_addr = Pointer_make([0], 0);
      var _i_addr;
      var _retval = Pointer_make([0], 0);
      var _0 = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_this_addr] = _this;
      _i_addr = _i;
      var _2 = 0+(_i_addr >= 0);
      if (_2) { __label__ = 0; break; } else { __label__ = 1; break; }
    case 0: // _bb
      var _3 = HEAP[_this_addr];
      var _5 = HEAP[0 + _3+2*1];
      __label__ = 1; break;
    case 1: // _bb1
      var _6 = HEAP[_this_addr];
      var _8 = HEAP[0 + _6+0*1];
      var _10 = _8+_i_addr;
      HEAP[_0] = _10;
      var _11 = HEAP[_0];
      HEAP[_retval] = _11;
      __label__ = 2; break;
    case 2: // _return
      var _retval2 = HEAP[_retval];
      return _retval2;
  }
}


function __ZNK6vectorIcE5emptyEv(_this) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _this_addr = Pointer_make([0], 0);
      var _retval;
      var _0;
      var __alloca_point_ = 0;
      HEAP[_this_addr] = _this;
      var _1 = HEAP[_this_addr];
      var _3 = HEAP[0 + _1+2*1];
      var _4 = 0+(_3 == 0);
      _0 = _4;
      _retval = _0;
      __label__ = 2; break;
    case 2: // _return
      var _retval12 = _retval;
      return _retval12;
  }
}


function __ZN6vectorI5clineEC1Ev(_this) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _this_addr = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_this_addr] = _this;
      var _0 = HEAP[_this_addr];
      HEAP[0 + _0+0*1] = 0;
      var _2 = HEAP[_this_addr];
      HEAP[0 + _2+1*1] = 0;
      var _4 = HEAP[_this_addr];
      HEAP[0 + _4+2*1] = 0;
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __ZNK6vectorI5clineE6lengthEv(_this) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _this_addr = Pointer_make([0], 0);
      var _retval;
      var _0;
      var __alloca_point_ = 0;
      HEAP[_this_addr] = _this;
      var _1 = HEAP[_this_addr];
      var _0 = HEAP[0 + _1+2*1];
      _retval = _0;
      __label__ = 2; break;
    case 2: // _return
      var _retval1 = _retval;
      return _retval1;
  }
}


function __ZN6vectorI5clineE3popEv(_this) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _this_addr = Pointer_make([0], 0);
      var _retval = Pointer_make([0], 0);
      var _0 = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_this_addr] = _this;
      var _1 = HEAP[_this_addr];
      var _3 = HEAP[0 + _1+0*1];
      var _6 = HEAP[0 + _1+2*1];
      var _7 = _6 - 1;
      var _8 = HEAP[_this_addr];
      HEAP[0 + _8+2*1] = _7;
      var _10 = HEAP[_this_addr];
      var _12 = HEAP[0 + _10+2*1];
      var _13 = _3+3*_12;
      HEAP[_0] = _13;
      var _14 = HEAP[_0];
      HEAP[_retval] = _14;
      __label__ = 2; break;
    case 2: // _return
      var _retval1 = HEAP[_retval];
      return _retval1;
  }
}


function __Z9es_sizeofIcEiPT_(_x) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _x_addr = Pointer_make([0], 0);
      var _retval;
      var _0;
      var __alloca_point_ = 0;
      HEAP[_x_addr] = _x;
      var _1 = HEAP[_x_addr];
      var _2 = _1+1;
      _0 = _2;
      _retval = _0;
      __label__ = 2; break;
    case 2: // _return
      var _retval1 = _retval;
      return _retval1;
  }
}


function __ZN6vectorIcE4dropEv(_this) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _this_addr = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_this_addr] = _this;
      var _0 = HEAP[_this_addr];
      var _2 = HEAP[0 + _0+2*1];
      var _3 = _2 - 1;
      HEAP[0 + _0+2*1] = _3;
      var _6 = HEAP[_this_addr];
      var _8 = HEAP[0 + _6+0*1];
      var _11 = HEAP[0 + _6+2*1];
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __ZN6vectorIcE6shrinkEi(_this, _i) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _this_addr = Pointer_make([0], 0);
      var _i_addr;
      var __alloca_point_ = 0;
      HEAP[_this_addr] = _this;
      _i_addr = _i;
      var _0 = HEAP[_this_addr];
      var _2 = HEAP[0 + _0+2*1];
      __label__ = 1; break;
    case 0: // _bb
      var _3 = HEAP[_this_addr];
      __ZN6vectorIcE4dropEv(_3);
      __label__ = 1; break;
    case 1: // _bb1
      var _4 = HEAP[_this_addr];
      var _6 = HEAP[0 + _4+2*1];
      var _8 = 0+(_6 > _i_addr);
      if (_8) { __label__ = 0; break; } else { __label__ = 3; break; }
    case 3: // _bb2
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __Z9es_sizeofIP6vectorIcEEiPT_(_x) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _x_addr = Pointer_make([0], 0);
      var _retval;
      var _0;
      var __alloca_point_ = 0;
      HEAP[_x_addr] = _x;
      var _1 = HEAP[_x_addr];
      var _2 = _1+1;
      _0 = _2;
      _retval = _0;
      __label__ = 2; break;
    case 2: // _return
      var _retval1 = _retval;
      return _retval1;
  }
}


function __ZN7databufIcEC1IiEEPcT_(_this, _buf, _maxlen) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _this_addr = Pointer_make([0], 0);
      var _buf_addr = Pointer_make([0], 0);
      var _maxlen_addr;
      var __alloca_point_ = 0;
      HEAP[_this_addr] = _this;
      HEAP[_buf_addr] = _buf;
      _maxlen_addr = _maxlen;
      var _0 = HEAP[_this_addr];
      var _1 = _0+0*1;
      var _2 = HEAP[_buf_addr];
      HEAP[_1] = _2;
      var _3 = HEAP[_this_addr];
      HEAP[0 + _3+1*1] = 0;
      var _5 = HEAP[_this_addr];
      var _6 = _5+2*1;
      HEAP[_6] = _maxlen_addr;
      var _8 = HEAP[_this_addr];
      HEAP[0 + _8+3*1] = 0;
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __ZN6vectorIcE7advanceEi(_this, _sz) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _this_addr = Pointer_make([0], 0);
      var _sz_addr;
      var __alloca_point_ = 0;
      HEAP[_this_addr] = _this;
      _sz_addr = _sz;
      var _0 = HEAP[_this_addr];
      var _2 = HEAP[0 + _0+2*1];
      var _4 = _2 + _sz_addr;
      HEAP[0 + _0+2*1] = _4;
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __ZNK7databufIcE6lengthEv(_this) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _this_addr = Pointer_make([0], 0);
      var _retval;
      var _0;
      var __alloca_point_ = 0;
      HEAP[_this_addr] = _this;
      var _1 = HEAP[_this_addr];
      var _0 = HEAP[0 + _1+1*1];
      _retval = _0;
      __label__ = 2; break;
    case 2: // _return
      var _retval1 = _retval;
      return _retval1;
  }
}


function __ZN6vectorIcE6addbufERK7databufIcE(_this, _p) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _this_addr = Pointer_make([0], 0);
      var _p_addr = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_this_addr] = _this;
      HEAP[_p_addr] = _p;
      var _0 = HEAP[_p_addr];
      var _1 = __ZNK7databufIcE6lengthEv(_0);
      var _2 = HEAP[_this_addr];
      __ZN6vectorIcE7advanceEi(_2, _1);
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __ZN6vectorIP5identE4dropEv(_this) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _this_addr = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_this_addr] = _this;
      var _0 = HEAP[_this_addr];
      var _2 = HEAP[0 + _0+2*1];
      var _3 = _2 - 1;
      HEAP[0 + _0+2*1] = _3;
      var _6 = HEAP[_this_addr];
      var _8 = HEAP[0 + _6+0*1];
      var _11 = HEAP[0 + _6+2*1];
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __ZN6vectorIP5identE6shrinkEi(_this, _i) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _this_addr = Pointer_make([0], 0);
      var _i_addr;
      var __alloca_point_ = 0;
      HEAP[_this_addr] = _this;
      _i_addr = _i;
      var _0 = HEAP[_this_addr];
      var _2 = HEAP[0 + _0+2*1];
      __label__ = 1; break;
    case 0: // _bb
      var _3 = HEAP[_this_addr];
      __ZN6vectorIP5identE4dropEv(_3);
      __label__ = 1; break;
    case 1: // _bb1
      var _4 = HEAP[_this_addr];
      var _6 = HEAP[0 + _4+2*1];
      var _8 = 0+(_6 > _i_addr);
      if (_8) { __label__ = 0; break; } else { __label__ = 3; break; }
    case 3: // _bb2
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __Z9es_sizeofIP5identEiPT_(_x) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _x_addr = Pointer_make([0], 0);
      var _retval;
      var _0;
      var __alloca_point_ = 0;
      HEAP[_x_addr] = _x;
      var _1 = HEAP[_x_addr];
      var _2 = _1+1;
      _0 = _2;
      _retval = _0;
      __label__ = 2; break;
    case 2: // _return
      var _retval1 = _retval;
      return _retval1;
  }
}


function __Z9es_sizeofIPcEiPT_(_x) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _x_addr = Pointer_make([0], 0);
      var _retval;
      var _0;
      var __alloca_point_ = 0;
      HEAP[_x_addr] = _x;
      var _1 = HEAP[_x_addr];
      var _2 = _1+1;
      _0 = _2;
      _retval = _0;
      __label__ = 2; break;
    case 2: // _return
      var _retval1 = _retval;
      return _retval1;
  }
}


function __ZN14hashtableentryIPKc5identEC1Ev(_this) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _this_addr = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_this_addr] = _this;
      var _0 = HEAP[_this_addr];
      var _1 = _0+__struct_hashtableentry_const_char__ident_____FLATTENER[1];
      __ZN5identC1Ev(_1);
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __ZN7hashsetI14hashtableentryIPKc5identEE5chainC1Ev(_this) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _this_addr = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_this_addr] = _this;
      var _0 = HEAP[_this_addr];
      var _1 = _0+__struct_hashset_hashtableentry_const_char___ident_____chain____FLATTENER[0];
      __ZN14hashtableentryIPKc5identEC1Ev(_1);
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __Z9es_sizeofI5clineEiPT_(_x) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _x_addr = Pointer_make([0], 0);
      var _retval;
      var _0;
      var __alloca_point_ = 0;
      HEAP[_x_addr] = _x;
      var _1 = HEAP[_x_addr];
      var _2 = _1+3*1;
      _0 = _2;
      _retval = _0;
      __label__ = 2; break;
    case 2: // _return
      var _retval1 = _retval;
      return _retval1;
  }
}


function __ZN6vectorIPS_IcEE4dropEv(_this) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _this_addr = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_this_addr] = _this;
      var _0 = HEAP[_this_addr];
      var _2 = HEAP[0 + _0+2*1];
      var _3 = _2 - 1;
      HEAP[0 + _0+2*1] = _3;
      var _6 = HEAP[_this_addr];
      var _8 = HEAP[0 + _6+0*1];
      var _11 = HEAP[0 + _6+2*1];
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __ZN6vectorIPS_IcEE6shrinkEi(_this, _i) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _this_addr = Pointer_make([0], 0);
      var _i_addr;
      var __alloca_point_ = 0;
      HEAP[_this_addr] = _this;
      _i_addr = _i;
      var _0 = HEAP[_this_addr];
      var _2 = HEAP[0 + _0+2*1];
      __label__ = 1; break;
    case 0: // _bb
      var _3 = HEAP[_this_addr];
      __ZN6vectorIPS_IcEE4dropEv(_3);
      __label__ = 1; break;
    case 1: // _bb1
      var _4 = HEAP[_this_addr];
      var _6 = HEAP[0 + _4+2*1];
      var _8 = 0+(_6 > _i_addr);
      if (_8) { __label__ = 0; break; } else { __label__ = 3; break; }
    case 3: // _bb2
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __ZN5clineD1Ev(_this) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _this_addr = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_this_addr] = _this;
      __label__ = 0; break;
    case 0: // _bb
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __ZN6vectorI5clineE4dropEv(_this) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _this_addr = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_this_addr] = _this;
      var _0 = HEAP[_this_addr];
      var _2 = HEAP[0 + _0+2*1];
      var _3 = _2 - 1;
      HEAP[0 + _0+2*1] = _3;
      var _6 = HEAP[_this_addr];
      var _8 = HEAP[0 + _6+0*1];
      var _11 = HEAP[0 + _6+2*1];
      var _12 = _8+3*_11;
      __ZN5clineD1Ev(_12);
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __ZN6vectorI5clineE6shrinkEi(_this, _i) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _this_addr = Pointer_make([0], 0);
      var _i_addr;
      var __alloca_point_ = 0;
      HEAP[_this_addr] = _this;
      _i_addr = _i;
      var _0 = HEAP[_this_addr];
      var _2 = HEAP[0 + _0+2*1];
      __label__ = 1; break;
    case 0: // _bb
      var _3 = HEAP[_this_addr];
      __ZN6vectorI5clineE4dropEv(_3);
      __label__ = 1; break;
    case 1: // _bb1
      var _4 = HEAP[_this_addr];
      var _6 = HEAP[0 + _4+2*1];
      var _8 = 0+(_6 > _i_addr);
      if (_8) { __label__ = 0; break; } else { __label__ = 3; break; }
    case 3: // _bb2
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __ZN6vectorI5clineED1Ev(_this) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _this_addr = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_this_addr] = _this;
      var _0 = HEAP[_this_addr];
      __ZN6vectorI5clineE6shrinkEi(_0, 0);
      var _1 = HEAP[_this_addr];
      var _3 = HEAP[0 + _1+0*1];
      var _4 = 0+(_3 != 0);
      if (_4) { __label__ = 0; break; } else { __label__ = 3; break; }
    case 0: // _bb
      var _5 = HEAP[_this_addr];
      var _7 = HEAP[0 + _5+0*1];
      var _9 = 0+(_7 != 0);
      if (_9) { __label__ = 1; break; } else { __label__ = 3; break; }
    case 1: // _bb1
      var _10 = HEAP[_this_addr];
      var _12 = HEAP[0 + _10+0*1];
      __ZdaPv(_12);
      __label__ = 3; break;
    case 3: // _bb2
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function ___tcf_3(_unnamed_arg) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _unnamed_arg_addr = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_unnamed_arg_addr] = _unnamed_arg;
      __ZN6vectorI5clineED1Ev(_conlines);
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __ZN6vectorIPS_IcEED1Ev(_this) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _this_addr = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_this_addr] = _this;
      var _0 = HEAP[_this_addr];
      __ZN6vectorIPS_IcEE6shrinkEi(_0, 0);
      var _1 = HEAP[_this_addr];
      var _3 = HEAP[0 + _1+0*1];
      var _4 = 0+(_3 != 0);
      if (_4) { __label__ = 0; break; } else { __label__ = 3; break; }
    case 0: // _bb
      var _5 = HEAP[_this_addr];
      var _7 = HEAP[0 + _5+0*1];
      var _9 = 0+(_7 != 0);
      if (_9) { __label__ = 1; break; } else { __label__ = 3; break; }
    case 1: // _bb1
      var _10 = HEAP[_this_addr];
      var _12 = HEAP[0 + _10+0*1];
      __ZdaPv(_12);
      __label__ = 3; break;
    case 3: // _bb2
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function ___tcf_2(_unnamed_arg) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _unnamed_arg_addr = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_unnamed_arg_addr] = _unnamed_arg;
      __ZN6vectorIPS_IcEED1Ev(__ZL8wordbufs);
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __Z10var_maxconv() {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _0 = Pointer_make([0], 0);
      var _retval_202;
      var __alloca_point_ = 0;
      __label__ = 3; break;
    case 0: // _bb
      var _1 = __ZN6vectorI5clineE3popEv(_conlines);
      var _3 = HEAP[0 + _1+0*1];
      HEAP[_0] = _3;
      var _4 = HEAP[_0];
      var _5 = 0+(_4 != 0);
      if (_5) { __label__ = 1; break; } else { __label__ = 3; break; }
    case 1: // _bb1
      var _6 = HEAP[_0];
      __ZdaPv(_6);
      __label__ = 3; break;
    case 3: // _bb2
      var _7 = __ZNK6vectorI5clineE6lengthEv(_conlines);
      var _8 = HEAP[_maxcon];
      var _9 = 0+(_7 > _8);
      _retval_202 = _9;
      var _toBool = 0+(_retval_202 != 0);
      if (_toBool) { __label__ = 0; break; } else { __label__ = 7; break; }
    case 7: // _bb3
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __ZN6vectorIcED1Ev(_this) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _this_addr = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_this_addr] = _this;
      var _0 = HEAP[_this_addr];
      __ZN6vectorIcE6shrinkEi(_0, 0);
      var _1 = HEAP[_this_addr];
      var _3 = HEAP[0 + _1+0*1];
      var _4 = 0+(_3 != 0);
      if (_4) { __label__ = 0; break; } else { __label__ = 3; break; }
    case 0: // _bb
      var _5 = HEAP[_this_addr];
      var _7 = HEAP[0 + _5+0*1];
      var _8 = 0+(_7 != 0);
      if (_8) { __label__ = 1; break; } else { __label__ = 3; break; }
    case 1: // _bb1
      var _9 = HEAP[_this_addr];
      var _11 = HEAP[0 + _9+0*1];
      __ZdaPv(_11);
      __label__ = 3; break;
    case 3: // _bb2
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function ___tcf_0(_unnamed_arg) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _unnamed_arg_addr = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_unnamed_arg_addr] = _unnamed_arg;
      __ZN6vectorIcED1Ev(__ZZ10parsemacroRPKciR6vectorIcEE5ident);
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __ZN6vectorIP5identED1Ev(_this) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _this_addr = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_this_addr] = _this;
      var _0 = HEAP[_this_addr];
      __ZN6vectorIP5identE6shrinkEi(_0, 0);
      var _1 = HEAP[_this_addr];
      var _3 = HEAP[0 + _1+0*1];
      var _4 = 0+(_3 != 0);
      if (_4) { __label__ = 0; break; } else { __label__ = 3; break; }
    case 0: // _bb
      var _5 = HEAP[_this_addr];
      var _7 = HEAP[0 + _5+0*1];
      var _9 = 0+(_7 != 0);
      if (_9) { __label__ = 1; break; } else { __label__ = 3; break; }
    case 1: // _bb1
      var _10 = HEAP[_this_addr];
      var _12 = HEAP[0 + _10+0*1];
      __ZdaPv(_12);
      __label__ = 3; break;
    case 3: // _bb2
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function ___tcf_1(_unnamed_arg) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _unnamed_arg_addr = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_unnamed_arg_addr] = _unnamed_arg;
      __ZN6vectorIP5identED1Ev(__ZZ10executeretPKcE6argids);
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __Z13vformatstringPcPKcS_i(_d, _fmt, _v, _len) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _d_addr = Pointer_make([0], 0);
      var _fmt_addr = Pointer_make([0], 0);
      var _v_addr = Pointer_make([0], 0);
      var _len_addr;
      var __alloca_point_ = 0;
      HEAP[_d_addr] = _d;
      HEAP[_fmt_addr] = _fmt;
      HEAP[_v_addr] = _v;
      _len_addr = _len;
      var _0 = _len_addr;
      var _1 = HEAP[_d_addr];
      var _2 = HEAP[_fmt_addr];
      var _3 = HEAP[_v_addr];
      var _4 = _vsnprintf(_1, _0, _2, _3);
      var _6 = _len_addr - 1;
      var _7 = HEAP[_d_addr];
      HEAP[0 + _7+_6] = 0;
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __Z5fatalPKcz(_s) {
  __numArgs__ = 1;
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _s_addr = Pointer_make([0], 0);
      var _msg = Pointer_make([0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0], 0);
      var _ap = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_s_addr] = _s;
      var _0 = HEAP[__ZZ5fatalPKczE6errors];
      var _1 = _0 + 1;
      HEAP[__ZZ5fatalPKczE6errors] = _1;
      var _2 = HEAP[__ZZ5fatalPKczE6errors];
      var _3 = 0+(_2 <= 2);
      if (_3) { __label__ = 0; break; } else { __label__ = 7; break; }
    case 0: // _bb
      HEAP[_ap] = Pointer_make([Array.prototype.slice.call(arguments, __numArgs__).length].concat(Array.prototype.slice.call(arguments, __numArgs__)), 0);
      var _4 = HEAP[_ap];
      var _5 = _msg+0*1;
      var _6 = HEAP[_s_addr];
      __Z13vformatstringPcPKcS_i(_5, _6, _4, 260);
      ;;
      var _7 = _msg+0*1;
      var _8 = _puts(_7);
      __label__ = 7; break;
    case 7: // _bb3
      _exit(1);
      // unreachable
    case 2: // _return
      return;
  }
}


function __ZN15stringformatterclEPKcz(_this, _fmt) {
  __numArgs__ = 2;
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _this_addr = Pointer_make([0], 0);
      var _fmt_addr = Pointer_make([0], 0);
      var _v = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_this_addr] = _this;
      HEAP[_fmt_addr] = _fmt;
      HEAP[_v] = Pointer_make([Array.prototype.slice.call(arguments, __numArgs__).length].concat(Array.prototype.slice.call(arguments, __numArgs__)), 0);
      var _0 = HEAP[_v];
      var _1 = HEAP[_this_addr];
      var _3 = HEAP[0 + _1+0*1];
      var _4 = HEAP[_fmt_addr];
      __Z13vformatstringPcPKcS_i(_3, _4, _0, 260);
      ;;
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __Z8floatstrf(_v) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _v_addr;
      var _retval = Pointer_make([0], 0);
      var _0 = Pointer_make([0], 0);
      var _1 = Pointer_make([0], 0);
      var _iftmp_150 = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      _v_addr = _v;
      var _2 = HEAP[__ZL6retidx];
      var _3 = _2 + 1;
      var _4 = Math.floor(_3 % 3);
      HEAP[__ZL6retidx] = _4;
      var _6 = _v_addr;
      var _8 = Math.floor(_v_addr);
      var _9 = _8;
      var _11 = 0+(_9 == _v_addr);
      if (_11) { __label__ = 0; break; } else { __label__ = 1; break; }
    case 0: // _bb
      HEAP[_iftmp_150] = __str;
      __label__ = 3; break;
    case 1: // _bb1
      HEAP[_iftmp_150] = __str1;
      __label__ = 3; break;
    case 3: // _bb2
      var _12 = HEAP[_iftmp_150];
      var _13 = _12;
      var _14 = HEAP[__ZL6retidx];
      var _15 = __ZL6retbuf+_14*260;
      var _16 = _15+0*1;
      __ZN15stringformatterC1EPc(_1, _16);
      __ZN15stringformatterclEPKcz(_1, _13, _6);
      var _17 = HEAP[__ZL6retidx];
      var _18 = __ZL6retbuf+_17*260;
      var _19 = _18+0*1;
      HEAP[_0] = _19;
      var _20 = HEAP[_0];
      HEAP[_retval] = _20;
      __label__ = 2; break;
    case 2: // _return
      var _retval3 = HEAP[_retval];
      return _retval3;
  }
}


function __Z6intstri(_v) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _v_addr;
      var _retval = Pointer_make([0], 0);
      var _0 = Pointer_make([0], 0);
      var _1 = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      _v_addr = _v;
      var _2 = HEAP[__ZL6retidx];
      var _3 = _2 + 1;
      var _4 = Math.floor(_3 % 3);
      HEAP[__ZL6retidx] = _4;
      var _5 = HEAP[__ZL6retidx];
      var _6 = __ZL6retbuf+_5*260;
      var _7 = _6+0*1;
      __ZN15stringformatterC1EPc(_1, _7);
      __ZN15stringformatterclEPKcz(_1, __str2+0*1, _v_addr);
      var _9 = HEAP[__ZL6retidx];
      var _10 = __ZL6retbuf+_9*260;
      var _11 = _10+0*1;
      HEAP[_0] = _11;
      var _12 = HEAP[_0];
      HEAP[_retval] = _12;
      __label__ = 2; break;
    case 2: // _return
      var _retval1 = HEAP[_retval];
      return _retval1;
  }
}


function __Z8reloadMTv() {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _retval;
      var _0;
      var _iftmp_215;
      var _iftmp_214;
      var _retval_213;
      var _iftmp_212;
      var _retval_211;
      var _p0 = Pointer_make([0], 0);
      var _p2 = Pointer_make([0], 0);
      var _pM = Pointer_make([0], 0);
      var _s0;
      var _s1;
      var _j;
      var __alloca_point_ = 0;
      HEAP[_p0] = __ZL5state+0*1;
      HEAP[_p2] = __ZL5state+2*1;
      HEAP[_pM] = __ZL5state+397*1;
      var _1 = HEAP[__ZL4left];
      var _2 = 0+(_1 < -1);
      if (_2) { __label__ = 0; break; } else { __label__ = 1; break; }
    case 0: // _bb
      var _3 = _time(0);
      __Z6seedMTj(_3);
      __label__ = 1; break;
    case 1: // _bb1
      HEAP[__ZL4left] = 623;
      HEAP[__ZL4next] = __ZL5state+1*1;
      var _4 = HEAP[0 + __ZL5state+0*1];
      _s0 = _4;
      var _5 = HEAP[0 + __ZL5state+1*1];
      _s1 = _5;
      _j = 228;
      __label__ = 8; break;
    case 3: // _bb2
      var _6 = HEAP[_pM];
      var _7 = HEAP[_6];
      var _9 = _s0 & -2147483648;
      var _11 = _s1 & 2147483647;
      var _12 = _9 | _11;
      var _13 = _12 >> 1;
      var _14 = _7 ^ _13;
      var _16 = _s1 & 1;
      var _17 = _16;
      var _toBool = 0+(_17 != 0);
      if (_toBool) { __label__ = 7; break; } else { __label__ = 6; break; }
    case 7: // _bb3
      _iftmp_212 = -1727483681;
      __label__ = 4; break;
    case 6: // _bb4
      _iftmp_212 = 0;
      __label__ = 4; break;
    case 4: // _bb5
      var _19 = _14 ^ _iftmp_212;
      var _20 = HEAP[_p0];
      HEAP[_20] = _19;
      var _21 = HEAP[_p0];
      var _22 = _21+1;
      HEAP[_p0] = _22;
      var _23 = HEAP[_pM];
      var _24 = _23+1;
      HEAP[_pM] = _24;
      _s0 = _s1;
      var _26 = HEAP[_p2];
      _s1 = HEAP[_26];
      var _29 = _26+1;
      HEAP[_p2] = _29;
      __label__ = 8; break;
    case 8: // _bb6
      _j = _j - 1;
      var _33 = 0+(_j != 0);
      _retval_211 = _33;
      var _toBool7 = 0+(_retval_211 != 0);
      if (_toBool7) { __label__ = 3; break; } else { __label__ = 10; break; }
    case 10: // _bb8
      HEAP[_pM] = __ZL5state+0*1;
      _j = 397;
      __label__ = 12; break;
    case 9: // _bb9
      var _36 = HEAP[_pM];
      var _37 = HEAP[_36];
      var _39 = _s0 & -2147483648;
      var _41 = _s1 & 2147483647;
      var _42 = _39 | _41;
      var _43 = _42 >> 1;
      var _44 = _37 ^ _43;
      var _46 = _s1 & 1;
      var _47 = _46;
      var _toBool10 = 0+(_47 != 0);
      if (_toBool10) { __label__ = 13; break; } else { __label__ = 14; break; }
    case 13: // _bb11
      _iftmp_214 = -1727483681;
      __label__ = 15; break;
    case 14: // _bb12
      _iftmp_214 = 0;
      __label__ = 15; break;
    case 15: // _bb13
      var _49 = _44 ^ _iftmp_214;
      var _50 = HEAP[_p0];
      HEAP[_50] = _49;
      var _51 = HEAP[_p0];
      var _52 = _51+1;
      HEAP[_p0] = _52;
      var _53 = HEAP[_pM];
      var _54 = _53+1;
      HEAP[_pM] = _54;
      _s0 = _s1;
      var _56 = HEAP[_p2];
      _s1 = HEAP[_56];
      var _59 = _56+1;
      HEAP[_p2] = _59;
      __label__ = 12; break;
    case 12: // _bb14
      _j = _j - 1;
      var _63 = 0+(_j != 0);
      _retval_213 = _63;
      var _toBool15 = 0+(_retval_213 != 0);
      if (_toBool15) { __label__ = 9; break; } else { __label__ = 16; break; }
    case 16: // _bb16
      var _66 = HEAP[0 + __ZL5state+0*1];
      _s1 = _66;
      var _67 = HEAP[_pM];
      var _68 = HEAP[_67];
      var _70 = _s0 & -2147483648;
      var _72 = _s1 & 2147483647;
      var _73 = _70 | _72;
      var _74 = _73 >> 1;
      var _75 = _68 ^ _74;
      var _77 = _s1 & 1;
      var _78 = _77;
      var _toBool17 = 0+(_78 != 0);
      if (_toBool17) { __label__ = 17; break; } else { __label__ = 18; break; }
    case 17: // _bb18
      _iftmp_215 = -1727483681;
      __label__ = 19; break;
    case 18: // _bb19
      _iftmp_215 = 0;
      __label__ = 19; break;
    case 19: // _bb20
      var _80 = _75 ^ _iftmp_215;
      var _81 = HEAP[_p0];
      HEAP[_81] = _80;
      var _83 = _s1 >> 11;
      _s1 = _83 ^ _s1;
      var _87 = _s1 << 7;
      var _88 = _87 & -1658038656;
      _s1 = _88 ^ _s1;
      var _92 = _s1 << 15;
      var _93 = _92 & -272236544;
      _s1 = _93 ^ _s1;
      var _97 = _s1 >> 18;
      _0 = _97 ^ _s1;
      _retval = _0;
      __label__ = 2; break;
    case 2: // _return
      var _retval21 = _retval;
      return _retval21;
  }
}


function __Z8randomMTv() {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _retval;
      var _0;
      var _retval_216;
      var _y;
      var __alloca_point_ = 0;
      var _1 = HEAP[__ZL4left];
      var _2 = _1 - 1;
      HEAP[__ZL4left] = _2;
      var _3 = HEAP[__ZL4left];
      var _4 = 0+(_3 < 0);
      _retval_216 = _4;
      var _toBool = 0+(_retval_216 != 0);
      if (_toBool) { __label__ = 0; break; } else { __label__ = 1; break; }
    case 0: // _bb
      _0 = __Z8reloadMTv();
      __label__ = 3; break;
    case 1: // _bb1
      var _8 = HEAP[__ZL4next];
      _y = HEAP[_8];
      var _10 = _8+1;
      HEAP[__ZL4next] = _10;
      var _12 = _y >> 11;
      _y = _12 ^ _y;
      var _16 = _y << 7;
      var _17 = _16 & -1658038656;
      _y = _17 ^ _y;
      var _21 = _y << 15;
      var _22 = _21 & -272236544;
      _y = _22 ^ _y;
      var _26 = _y >> 18;
      _0 = _26 ^ _y;
      __label__ = 3; break;
    case 3: // _bb2
      _retval = _0;
      __label__ = 2; break;
    case 2: // _return
      var _retval3 = _retval;
      return _retval3;
  }
}


function __Z9newstringj(_l) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _l_addr;
      var _retval = Pointer_make([0], 0);
      var _0;
      var _1 = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      _l_addr = _l;
      _0 = _l_addr + 1;
      var _5 = __Znaj(_0);
      HEAP[_1] = _5;
      var _6 = HEAP[_1];
      HEAP[_retval] = _6;
      __label__ = 2; break;
    case 2: // _return
      var _retval1 = HEAP[_retval];
      return _retval1;
  }
}


function __ZN7hashsetI14hashtableentryIPKc5identEEC2Ei(_this, _size) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _this_addr = Pointer_make([0], 0);
      var _size_addr;
      var _size_239;
      var _i;
      var __alloca_point_ = 0;
      HEAP[_this_addr] = _this;
      _size_addr = _size;
      var _0 = HEAP[_this_addr];
      var _1 = _0+0*1;
      HEAP[_1] = _size_addr;
      var _3 = HEAP[_this_addr];
      HEAP[0 + _3+1*1] = 0;
      var _5 = HEAP[_this_addr];
      HEAP[0 + _5+3*1] = 0;
      var _7 = HEAP[_this_addr];
      HEAP[0 + _7+4*1] = 0;
      _size_239 = _size_addr;
      var _11 = _size_239 * 4;
      var _12 = __Znaj(_11);
      var _13 = _12;
      var _14 = HEAP[_this_addr];
      HEAP[0 + _14+2*1] = _13;
      _i = 0;
      __label__ = 1; break;
    case 0: // _bb
      var _16 = HEAP[_this_addr];
      var _18 = HEAP[0 + _16+2*1];
      HEAP[0 + _18+_i] = 0;
      _i = _i + 1;
      __label__ = 1; break;
    case 1: // _bb1
      var _25 = 0+(_i < _size_addr);
      if (_25) { __label__ = 0; break; } else { __label__ = 3; break; }
    case 3: // _bb2
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __ZN9hashtableIPKc5identEC1Ei(_this, _size) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _this_addr = Pointer_make([0], 0);
      var _size_addr;
      var __alloca_point_ = 0;
      HEAP[_this_addr] = _this;
      _size_addr = _size;
      var _0 = HEAP[_this_addr];
      var _1 = _0+0*5;
      __ZN7hashsetI14hashtableentryIPKc5identEEC2Ei(_1, _size_addr);
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __ZN6vectorI5clineE7growbufEi(_this, _sz) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _this_addr = Pointer_make([0], 0);
      var _sz_addr;
      var _0;
      var _olen;
      var _newbuf = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_this_addr] = _this;
      _sz_addr = _sz;
      var _1 = HEAP[_this_addr];
      var _olen = HEAP[0 + _1+1*1];
      var _6 = HEAP[0 + _1+1*1];
      var _7 = 0+(_6 == 0);
      if (_7) { __label__ = 0; break; } else { __label__ = 1; break; }
    case 0: // _bb
      var _9 = __Z3maxIiET_S0_S0_(8, _sz_addr);
      var _10 = HEAP[_this_addr];
      HEAP[0 + _10+1*1] = _9;
      __label__ = 6; break;
    case 1: // _bb1
      __label__ = 7; break;
    case 3: // _bb2
      var _12 = HEAP[_this_addr];
      var _14 = HEAP[0 + _12+1*1];
      var _15 = _14 * 2;
      HEAP[0 + _12+1*1] = _15;
      __label__ = 7; break;
    case 7: // _bb3
      var _18 = HEAP[_this_addr];
      var _20 = HEAP[0 + _18+1*1];
      var _22 = 0+(_20 < _sz_addr);
      if (_22) { __label__ = 3; break; } else { __label__ = 6; break; }
    case 6: // _bb4
      var _23 = HEAP[_this_addr];
      var _25 = HEAP[0 + _23+1*1];
      var _27 = 0+(_25 <= _olen);
      if (_27) { __label__ = 9; break; } else { __label__ = 4; break; }
    case 4: // _bb5
      var _28 = HEAP[_this_addr];
      var _30 = HEAP[0 + _28+1*1];
      var _31 = __Z9es_sizeofI5clineEiPT_(0);
      _0 = _30 * _31;
      var _34 = __Znaj(_0);
      HEAP[_newbuf] = _34;
      var _36 = 0+(_olen > 0);
      if (_36) { __label__ = 8; break; } else { __label__ = 10; break; }
    case 8: // _bb6
      var _37 = __Z9es_sizeofI5clineEiPT_(0);
      var _39 = _37 * _olen;
      var _40 = HEAP[_this_addr];
      var _42 = HEAP[0 + _40+0*1];
      var _43 = HEAP[_newbuf];
      _llvm_memcpy_i32(_43, _42, _39, 1);
      var _45 = HEAP[_this_addr];
      var _47 = HEAP[0 + _45+0*1];
      var _49 = 0+(_47 != 0);
      if (_49) { __label__ = 5; break; } else { __label__ = 10; break; }
    case 5: // _bb7
      var _50 = HEAP[_this_addr];
      var _52 = HEAP[0 + _50+0*1];
      __ZdaPv(_52);
      __label__ = 10; break;
    case 10: // _bb8
      var _54 = HEAP[_newbuf];
      var _55 = _54;
      var _56 = HEAP[_this_addr];
      HEAP[0 + _56+0*1] = _55;
      __label__ = 9; break;
    case 9: // _bb9
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __ZN6vectorI5clineE3addERKS0_(_this, _x) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _this_addr = Pointer_make([0], 0);
      var _x_addr = Pointer_make([0], 0);
      var _retval = Pointer_make([0], 0);
      var _0 = Pointer_make([0], 0);
      var _iftmp_244 = Pointer_make([0], 0);
      var _1 = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_this_addr] = _this;
      HEAP[_x_addr] = _x;
      var _2 = HEAP[_this_addr];
      var _4 = HEAP[0 + _2+2*1];
      var _7 = HEAP[0 + _2+1*1];
      var _8 = 0+(_4 == _7);
      if (_8) { __label__ = 0; break; } else { __label__ = 1; break; }
    case 0: // _bb
      var _9 = HEAP[_this_addr];
      var _11 = HEAP[0 + _9+2*1];
      var _12 = _11 + 1;
      __ZN6vectorI5clineE7growbufEi(_9, _12);
      __label__ = 1; break;
    case 1: // _bb1
      var _14 = HEAP[_this_addr];
      var _16 = HEAP[0 + _14+0*1];
      var _19 = HEAP[0 + _14+2*1];
      var _20 = _16+3*_19;
      var _22 = __ZnwjPv(12, _20);
      HEAP[_1] = _22;
      var _24 = HEAP[_1];
      var _25 = 0+(_24 != 0);
      if (_25) { __label__ = 3; break; } else { __label__ = 7; break; }
    case 3: // _bb2
      var _26 = HEAP[_1];
      var _27 = HEAP[_x_addr];
      var _28 = _26+0*1;
      var _30 = HEAP[0 + _27+0*1];
      HEAP[_28] = _30;
      var _31 = _26+1*1;
      var _33 = HEAP[0 + _27+1*1];
      HEAP[_31] = _33;
      var _34 = _26+2*1;
      var _36 = HEAP[0 + _27+2*1];
      HEAP[_34] = _36;
      var _37 = HEAP[_1];
      HEAP[_iftmp_244] = _37;
      __label__ = 6; break;
    case 7: // _bb3
      var _38 = HEAP[_1];
      HEAP[_iftmp_244] = _38;
      __label__ = 6; break;
    case 6: // _bb4
      var _39 = HEAP[_this_addr];
      var _41 = HEAP[0 + _39+0*1];
      var _44 = HEAP[0 + _39+2*1];
      var _45 = _41+3*_44;
      HEAP[_0] = _45;
      var _46 = _44 + 1;
      var _47 = HEAP[_this_addr];
      HEAP[0 + _47+2*1] = _46;
      var _49 = HEAP[_0];
      HEAP[_retval] = _49;
      __label__ = 2; break;
    case 2: // _return
      var _retval5 = HEAP[_retval];
      return _retval5;
  }
}


function __ZN6vectorI5clineE6insertEiRKS0_(_this, _i, _e) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _this_addr = Pointer_make([0], 0);
      var _i_addr;
      var _e_addr = Pointer_make([0], 0);
      var _retval = Pointer_make([0], 0);
      var _0 = Pointer_make([0], 0);
      var _1 = Pointer_make([0,0,0], 0);
      var _p;
      var __alloca_point_ = 0;
      HEAP[_this_addr] = _this;
      _i_addr = _i;
      HEAP[_e_addr] = _e;
      var _2 = _1+0*1;
      var _3 = HEAP[0 + __ZZN6vectorI5clineE6insertEiRKS0_E5C_237+0*1];
      HEAP[_2] = _3;
      var _4 = _1+1*1;
      var _5 = HEAP[0 + __ZZN6vectorI5clineE6insertEiRKS0_E5C_237+1*1];
      HEAP[_4] = _5;
      var _6 = _1+2*1;
      var _7 = HEAP[0 + __ZZN6vectorI5clineE6insertEiRKS0_E5C_237+2*1];
      HEAP[_6] = _7;
      var _8 = HEAP[_this_addr];
      var _9 = __ZN6vectorI5clineE3addERKS0_(_8, _1);
      var _10 = HEAP[_this_addr];
      var _12 = HEAP[0 + _10+2*1];
      _p = _12 - 1;
      __label__ = 1; break;
    case 0: // _bb
      var _14 = HEAP[_this_addr];
      var _16 = HEAP[0 + _14+0*1];
      var _19 = HEAP[0 + _14+0*1];
      var _21 = _p - 1;
      var _23 = _16+3*_p;
      var _24 = _19+3*_21;
      var _25 = _23+0*1;
      var _27 = HEAP[0 + _24+0*1];
      HEAP[_25] = _27;
      var _28 = _23+1*1;
      var _30 = HEAP[0 + _24+1*1];
      HEAP[_28] = _30;
      var _31 = _23+2*1;
      var _33 = HEAP[0 + _24+2*1];
      HEAP[_31] = _33;
      _p = _p - 1;
      __label__ = 1; break;
    case 1: // _bb1
      var _38 = 0+(_p > _i_addr);
      if (_38) { __label__ = 0; break; } else { __label__ = 3; break; }
    case 3: // _bb2
      var _39 = HEAP[_this_addr];
      var _41 = HEAP[0 + _39+0*1];
      var _43 = _41+3*_i_addr;
      var _44 = HEAP[_e_addr];
      var _45 = _43+0*1;
      var _47 = HEAP[0 + _44+0*1];
      HEAP[_45] = _47;
      var _48 = _43+1*1;
      var _50 = HEAP[0 + _44+1*1];
      HEAP[_48] = _50;
      var _51 = _43+2*1;
      var _53 = HEAP[0 + _44+2*1];
      HEAP[_51] = _53;
      var _54 = HEAP[_this_addr];
      var _56 = HEAP[0 + _54+0*1];
      var _58 = _56+3*_i_addr;
      HEAP[_0] = _58;
      var _59 = HEAP[_0];
      HEAP[_retval] = _59;
      __label__ = 2; break;
    case 2: // _return
      var _retval3 = HEAP[_retval];
      return _retval3;
  }
}


function __ZN6vectorIcE7growbufEi(_this, _sz) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _this_addr = Pointer_make([0], 0);
      var _sz_addr;
      var _0;
      var _olen;
      var _newbuf = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_this_addr] = _this;
      _sz_addr = _sz;
      var _1 = HEAP[_this_addr];
      var _olen = HEAP[0 + _1+1*1];
      var _6 = HEAP[0 + _1+1*1];
      var _7 = 0+(_6 == 0);
      if (_7) { __label__ = 0; break; } else { __label__ = 1; break; }
    case 0: // _bb
      var _9 = __Z3maxIiET_S0_S0_(8, _sz_addr);
      var _10 = HEAP[_this_addr];
      HEAP[0 + _10+1*1] = _9;
      __label__ = 6; break;
    case 1: // _bb1
      __label__ = 7; break;
    case 3: // _bb2
      var _12 = HEAP[_this_addr];
      var _14 = HEAP[0 + _12+1*1];
      var _15 = _14 * 2;
      HEAP[0 + _12+1*1] = _15;
      __label__ = 7; break;
    case 7: // _bb3
      var _18 = HEAP[_this_addr];
      var _20 = HEAP[0 + _18+1*1];
      var _22 = 0+(_20 < _sz_addr);
      if (_22) { __label__ = 3; break; } else { __label__ = 6; break; }
    case 6: // _bb4
      var _23 = HEAP[_this_addr];
      var _25 = HEAP[0 + _23+1*1];
      var _27 = 0+(_25 <= _olen);
      if (_27) { __label__ = 9; break; } else { __label__ = 4; break; }
    case 4: // _bb5
      var _28 = HEAP[_this_addr];
      var _30 = HEAP[0 + _28+1*1];
      var _31 = __Z9es_sizeofIcEiPT_(0);
      _0 = _30 * _31;
      var _34 = __Znaj(_0);
      HEAP[_newbuf] = _34;
      var _36 = 0+(_olen > 0);
      if (_36) { __label__ = 8; break; } else { __label__ = 10; break; }
    case 8: // _bb6
      var _37 = __Z9es_sizeofIcEiPT_(0);
      var _39 = _37 * _olen;
      var _40 = HEAP[_this_addr];
      var _42 = HEAP[0 + _40+0*1];
      var _43 = HEAP[_newbuf];
      _llvm_memcpy_i32(_43, _42, _39, 1);
      var _44 = HEAP[_this_addr];
      var _46 = HEAP[0 + _44+0*1];
      var _47 = 0+(_46 != 0);
      if (_47) { __label__ = 5; break; } else { __label__ = 10; break; }
    case 5: // _bb7
      var _48 = HEAP[_this_addr];
      var _50 = HEAP[0 + _48+0*1];
      __ZdaPv(_50);
      __label__ = 10; break;
    case 10: // _bb8
      var _51 = HEAP[_newbuf];
      var _52 = HEAP[_this_addr];
      HEAP[0 + _52+0*1] = _51;
      __label__ = 9; break;
    case 9: // _bb9
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __ZN6vectorIcE3addERKc(_this, _x) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _this_addr = Pointer_make([0], 0);
      var _x_addr = Pointer_make([0], 0);
      var _retval = Pointer_make([0], 0);
      var _0 = Pointer_make([0], 0);
      var _iftmp_233 = Pointer_make([0], 0);
      var _1 = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_this_addr] = _this;
      HEAP[_x_addr] = _x;
      var _2 = HEAP[_this_addr];
      var _4 = HEAP[0 + _2+2*1];
      var _7 = HEAP[0 + _2+1*1];
      var _8 = 0+(_4 == _7);
      if (_8) { __label__ = 0; break; } else { __label__ = 1; break; }
    case 0: // _bb
      var _9 = HEAP[_this_addr];
      var _11 = HEAP[0 + _9+2*1];
      var _12 = _11 + 1;
      __ZN6vectorIcE7growbufEi(_9, _12);
      __label__ = 1; break;
    case 1: // _bb1
      var _14 = HEAP[_this_addr];
      var _16 = HEAP[0 + _14+0*1];
      var _19 = HEAP[0 + _14+2*1];
      var _20 = _16+_19;
      var _21 = __ZnwjPv(1, _20);
      HEAP[_1] = _21;
      var _22 = HEAP[_1];
      var _23 = 0+(_22 != 0);
      if (_23) { __label__ = 3; break; } else { __label__ = 7; break; }
    case 3: // _bb2
      var _24 = HEAP[_x_addr];
      var _25 = HEAP[_24];
      var _26 = HEAP[_1];
      HEAP[_26] = _25;
      var _27 = HEAP[_1];
      HEAP[_iftmp_233] = _27;
      __label__ = 6; break;
    case 7: // _bb3
      var _28 = HEAP[_1];
      HEAP[_iftmp_233] = _28;
      __label__ = 6; break;
    case 6: // _bb4
      var _29 = HEAP[_this_addr];
      var _31 = HEAP[0 + _29+0*1];
      var _34 = HEAP[0 + _29+2*1];
      var _35 = _31+_34;
      HEAP[_0] = _35;
      var _36 = _34 + 1;
      var _37 = HEAP[_this_addr];
      HEAP[0 + _37+2*1] = _36;
      var _39 = HEAP[_0];
      HEAP[_retval] = _39;
      __label__ = 2; break;
    case 2: // _return
      var _retval5 = HEAP[_retval];
      return _retval5;
  }
}


function __ZN6vectorIcE7reserveEi(_agg_result, _this, _sz) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _this_addr = Pointer_make([0], 0);
      var _sz_addr;
      var __alloca_point_ = 0;
      HEAP[_this_addr] = _this;
      _sz_addr = _sz;
      var _0 = HEAP[_this_addr];
      var _2 = HEAP[0 + _0+2*1];
      var _4 = _2 + _sz_addr;
      var _7 = HEAP[0 + _0+1*1];
      var _8 = 0+(_4 > _7);
      if (_8) { __label__ = 0; break; } else { __label__ = 1; break; }
    case 0: // _bb
      var _9 = HEAP[_this_addr];
      var _11 = HEAP[0 + _9+2*1];
      var _13 = _11 + _sz_addr;
      __ZN6vectorIcE7growbufEi(_9, _13);
      __label__ = 1; break;
    case 1: // _bb1
      var _15 = HEAP[_this_addr];
      var _17 = HEAP[0 + _15+0*1];
      var _20 = HEAP[0 + _15+2*1];
      var _21 = _17+_20;
      __ZN7databufIcEC1IiEEPcT_(_agg_result, _21, _sz_addr);
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __ZN7databufIcE3putEPKci(_this, _vals, _numvals) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _this_addr = Pointer_make([0], 0);
      var _vals_addr = Pointer_make([0], 0);
      var _numvals_addr;
      var __alloca_point_ = 0;
      HEAP[_this_addr] = _this;
      HEAP[_vals_addr] = _vals;
      _numvals_addr = _numvals;
      var _0 = HEAP[_this_addr];
      var _2 = HEAP[0 + _0+2*1];
      var _5 = HEAP[0 + _0+1*1];
      var _6 = _2 - _5;
      var _8 = 0+(_6 < _numvals_addr);
      if (_8) { __label__ = 0; break; } else { __label__ = 1; break; }
    case 0: // _bb
      var _9 = HEAP[_this_addr];
      var _11 = HEAP[0 + _9+3*1];
      var _12 = _11 | 2;
      HEAP[0 + _9+3*1] = _12;
      __label__ = 1; break;
    case 1: // _bb1
      var _15 = HEAP[_this_addr];
      var _17 = HEAP[0 + _15+2*1];
      var _20 = HEAP[0 + _15+1*1];
      var _21 = _17 - _20;
      var _23 = __Z3minIiET_S0_S0_(_21, _numvals_addr);
      var _24 = __Z9es_sizeofIcEiPT_(0);
      var _25 = _23 * _24;
      var _26 = HEAP[_this_addr];
      var _28 = HEAP[0 + _26+0*1];
      var _31 = HEAP[0 + _26+1*1];
      var _32 = _28+_31;
      var _33 = HEAP[_vals_addr];
      _llvm_memcpy_i32(_32, _33, _25, 1);
      var _34 = HEAP[_this_addr];
      var _36 = HEAP[0 + _34+1*1];
      var _39 = HEAP[0 + _34+2*1];
      var _40 = HEAP[_this_addr];
      var _42 = HEAP[0 + _40+1*1];
      var _43 = _39 - _42;
      var _45 = __Z3minIiET_S0_S0_(_43, _numvals_addr);
      var _46 = _36 + _45;
      var _47 = HEAP[_this_addr];
      HEAP[0 + _47+1*1] = _46;
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __ZN6vectorIcE3putEPKci(_this, _v, _n) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _this_addr = Pointer_make([0], 0);
      var _v_addr = Pointer_make([0], 0);
      var _n_addr;
      var _buf = Pointer_make([0,0,0,0], 0);
      var __alloca_point_ = 0;
      HEAP[_this_addr] = _this;
      HEAP[_v_addr] = _v;
      _n_addr = _n;
      var _0 = HEAP[_this_addr];
      __ZN6vectorIcE7reserveEi(_buf, _0, _n_addr);
      var _2 = HEAP[_v_addr];
      __ZN7databufIcE3putEPKci(_buf, _2, _n_addr);
      var _4 = HEAP[_this_addr];
      __ZN6vectorIcE6addbufERK7databufIcE(_4, _buf);
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __ZN6vectorIPcE7growbufEi(_this, _sz) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _this_addr = Pointer_make([0], 0);
      var _sz_addr;
      var _0;
      var _olen;
      var _newbuf = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_this_addr] = _this;
      _sz_addr = _sz;
      var _1 = HEAP[_this_addr];
      var _olen = HEAP[0 + _1+1*1];
      var _6 = HEAP[0 + _1+1*1];
      var _7 = 0+(_6 == 0);
      if (_7) { __label__ = 0; break; } else { __label__ = 1; break; }
    case 0: // _bb
      var _9 = __Z3maxIiET_S0_S0_(8, _sz_addr);
      var _10 = HEAP[_this_addr];
      HEAP[0 + _10+1*1] = _9;
      __label__ = 6; break;
    case 1: // _bb1
      __label__ = 7; break;
    case 3: // _bb2
      var _12 = HEAP[_this_addr];
      var _14 = HEAP[0 + _12+1*1];
      var _15 = _14 * 2;
      HEAP[0 + _12+1*1] = _15;
      __label__ = 7; break;
    case 7: // _bb3
      var _18 = HEAP[_this_addr];
      var _20 = HEAP[0 + _18+1*1];
      var _22 = 0+(_20 < _sz_addr);
      if (_22) { __label__ = 3; break; } else { __label__ = 6; break; }
    case 6: // _bb4
      var _23 = HEAP[_this_addr];
      var _25 = HEAP[0 + _23+1*1];
      var _27 = 0+(_25 <= _olen);
      if (_27) { __label__ = 9; break; } else { __label__ = 4; break; }
    case 4: // _bb5
      var _28 = HEAP[_this_addr];
      var _30 = HEAP[0 + _28+1*1];
      var _31 = __Z9es_sizeofIPcEiPT_(0);
      _0 = _30 * _31;
      var _34 = __Znaj(_0);
      HEAP[_newbuf] = _34;
      var _36 = 0+(_olen > 0);
      if (_36) { __label__ = 8; break; } else { __label__ = 10; break; }
    case 8: // _bb6
      var _37 = __Z9es_sizeofIPcEiPT_(0);
      var _39 = _37 * _olen;
      var _40 = HEAP[_this_addr];
      var _42 = HEAP[0 + _40+0*1];
      var _43 = HEAP[_newbuf];
      _llvm_memcpy_i32(_43, _42, _39, 1);
      var _45 = HEAP[_this_addr];
      var _47 = HEAP[0 + _45+0*1];
      var _49 = 0+(_47 != 0);
      if (_49) { __label__ = 5; break; } else { __label__ = 10; break; }
    case 5: // _bb7
      var _50 = HEAP[_this_addr];
      var _52 = HEAP[0 + _50+0*1];
      __ZdaPv(_52);
      __label__ = 10; break;
    case 10: // _bb8
      var _54 = HEAP[_newbuf];
      var _55 = _54;
      var _56 = HEAP[_this_addr];
      HEAP[0 + _56+0*1] = _55;
      __label__ = 9; break;
    case 9: // _bb9
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __ZN6vectorIPcE3addERKS0_(_this, _x) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _this_addr = Pointer_make([0], 0);
      var _x_addr = Pointer_make([0], 0);
      var _retval = Pointer_make([0], 0);
      var _0 = Pointer_make([0], 0);
      var _iftmp_236 = Pointer_make([0], 0);
      var _1 = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_this_addr] = _this;
      HEAP[_x_addr] = _x;
      var _2 = HEAP[_this_addr];
      var _4 = HEAP[0 + _2+2*1];
      var _7 = HEAP[0 + _2+1*1];
      var _8 = 0+(_4 == _7);
      if (_8) { __label__ = 0; break; } else { __label__ = 1; break; }
    case 0: // _bb
      var _9 = HEAP[_this_addr];
      var _11 = HEAP[0 + _9+2*1];
      var _12 = _11 + 1;
      __ZN6vectorIPcE7growbufEi(_9, _12);
      __label__ = 1; break;
    case 1: // _bb1
      var _14 = HEAP[_this_addr];
      var _16 = HEAP[0 + _14+0*1];
      var _19 = HEAP[0 + _14+2*1];
      var _20 = _16+_19;
      var _22 = __ZnwjPv(4, _20);
      HEAP[_1] = _22;
      var _24 = HEAP[_1];
      var _25 = 0+(_24 != 0);
      if (_25) { __label__ = 3; break; } else { __label__ = 7; break; }
    case 3: // _bb2
      var _26 = HEAP[_x_addr];
      var _27 = HEAP[_26];
      var _28 = HEAP[_1];
      HEAP[_28] = _27;
      var _29 = HEAP[_1];
      HEAP[_iftmp_236] = _29;
      __label__ = 6; break;
    case 7: // _bb3
      var _30 = HEAP[_1];
      HEAP[_iftmp_236] = _30;
      __label__ = 6; break;
    case 6: // _bb4
      var _31 = HEAP[_this_addr];
      var _33 = HEAP[0 + _31+0*1];
      var _36 = HEAP[0 + _31+2*1];
      var _37 = _33+_36;
      HEAP[_0] = _37;
      var _38 = _36 + 1;
      var _39 = HEAP[_this_addr];
      HEAP[0 + _39+2*1] = _38;
      var _41 = HEAP[_0];
      HEAP[_retval] = _41;
      __label__ = 2; break;
    case 2: // _return
      var _retval5 = HEAP[_retval];
      return _retval5;
  }
}


function __ZN6vectorIP5identE7growbufEi(_this, _sz) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _this_addr = Pointer_make([0], 0);
      var _sz_addr;
      var _0;
      var _olen;
      var _newbuf = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_this_addr] = _this;
      _sz_addr = _sz;
      var _1 = HEAP[_this_addr];
      var _olen = HEAP[0 + _1+1*1];
      var _6 = HEAP[0 + _1+1*1];
      var _7 = 0+(_6 == 0);
      if (_7) { __label__ = 0; break; } else { __label__ = 1; break; }
    case 0: // _bb
      var _9 = __Z3maxIiET_S0_S0_(8, _sz_addr);
      var _10 = HEAP[_this_addr];
      HEAP[0 + _10+1*1] = _9;
      __label__ = 6; break;
    case 1: // _bb1
      __label__ = 7; break;
    case 3: // _bb2
      var _12 = HEAP[_this_addr];
      var _14 = HEAP[0 + _12+1*1];
      var _15 = _14 * 2;
      HEAP[0 + _12+1*1] = _15;
      __label__ = 7; break;
    case 7: // _bb3
      var _18 = HEAP[_this_addr];
      var _20 = HEAP[0 + _18+1*1];
      var _22 = 0+(_20 < _sz_addr);
      if (_22) { __label__ = 3; break; } else { __label__ = 6; break; }
    case 6: // _bb4
      var _23 = HEAP[_this_addr];
      var _25 = HEAP[0 + _23+1*1];
      var _27 = 0+(_25 <= _olen);
      if (_27) { __label__ = 9; break; } else { __label__ = 4; break; }
    case 4: // _bb5
      var _28 = HEAP[_this_addr];
      var _30 = HEAP[0 + _28+1*1];
      var _31 = __Z9es_sizeofIP5identEiPT_(0);
      _0 = _30 * _31;
      var _34 = __Znaj(_0);
      HEAP[_newbuf] = _34;
      var _36 = 0+(_olen > 0);
      if (_36) { __label__ = 8; break; } else { __label__ = 10; break; }
    case 8: // _bb6
      var _37 = __Z9es_sizeofIP5identEiPT_(0);
      var _39 = _37 * _olen;
      var _40 = HEAP[_this_addr];
      var _42 = HEAP[0 + _40+0*1];
      var _43 = HEAP[_newbuf];
      _llvm_memcpy_i32(_43, _42, _39, 1);
      var _45 = HEAP[_this_addr];
      var _47 = HEAP[0 + _45+0*1];
      var _49 = 0+(_47 != 0);
      if (_49) { __label__ = 5; break; } else { __label__ = 10; break; }
    case 5: // _bb7
      var _50 = HEAP[_this_addr];
      var _52 = HEAP[0 + _50+0*1];
      __ZdaPv(_52);
      __label__ = 10; break;
    case 10: // _bb8
      var _54 = HEAP[_newbuf];
      var _55 = _54;
      var _56 = HEAP[_this_addr];
      HEAP[0 + _56+0*1] = _55;
      __label__ = 9; break;
    case 9: // _bb9
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __ZN6vectorIP5identE3addERKS1_(_this, _x) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _this_addr = Pointer_make([0], 0);
      var _x_addr = Pointer_make([0], 0);
      var _retval = Pointer_make([0], 0);
      var _0 = Pointer_make([0], 0);
      var _iftmp_235 = Pointer_make([0], 0);
      var _1 = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_this_addr] = _this;
      HEAP[_x_addr] = _x;
      var _2 = HEAP[_this_addr];
      var _4 = HEAP[0 + _2+2*1];
      var _7 = HEAP[0 + _2+1*1];
      var _8 = 0+(_4 == _7);
      if (_8) { __label__ = 0; break; } else { __label__ = 1; break; }
    case 0: // _bb
      var _9 = HEAP[_this_addr];
      var _11 = HEAP[0 + _9+2*1];
      var _12 = _11 + 1;
      __ZN6vectorIP5identE7growbufEi(_9, _12);
      __label__ = 1; break;
    case 1: // _bb1
      var _14 = HEAP[_this_addr];
      var _16 = HEAP[0 + _14+0*1];
      var _19 = HEAP[0 + _14+2*1];
      var _20 = _16+_19;
      var _22 = __ZnwjPv(4, _20);
      HEAP[_1] = _22;
      var _24 = HEAP[_1];
      var _25 = 0+(_24 != 0);
      if (_25) { __label__ = 3; break; } else { __label__ = 7; break; }
    case 3: // _bb2
      var _26 = HEAP[_x_addr];
      var _27 = HEAP[_26];
      var _28 = HEAP[_1];
      HEAP[_28] = _27;
      var _29 = HEAP[_1];
      HEAP[_iftmp_235] = _29;
      __label__ = 6; break;
    case 7: // _bb3
      var _30 = HEAP[_1];
      HEAP[_iftmp_235] = _30;
      __label__ = 6; break;
    case 6: // _bb4
      var _31 = HEAP[_this_addr];
      var _33 = HEAP[0 + _31+0*1];
      var _36 = HEAP[0 + _31+2*1];
      var _37 = _33+_36;
      HEAP[_0] = _37;
      var _38 = _36 + 1;
      var _39 = HEAP[_this_addr];
      HEAP[0 + _39+2*1] = _38;
      var _41 = HEAP[_0];
      HEAP[_retval] = _41;
      __label__ = 2; break;
    case 2: // _return
      var _retval5 = HEAP[_retval];
      return _retval5;
  }
}


function __ZN6vectorIPS_IcEE7growbufEi(_this, _sz) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _this_addr = Pointer_make([0], 0);
      var _sz_addr;
      var _0;
      var _olen;
      var _newbuf = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_this_addr] = _this;
      _sz_addr = _sz;
      var _1 = HEAP[_this_addr];
      var _olen = HEAP[0 + _1+1*1];
      var _6 = HEAP[0 + _1+1*1];
      var _7 = 0+(_6 == 0);
      if (_7) { __label__ = 0; break; } else { __label__ = 1; break; }
    case 0: // _bb
      var _9 = __Z3maxIiET_S0_S0_(8, _sz_addr);
      var _10 = HEAP[_this_addr];
      HEAP[0 + _10+1*1] = _9;
      __label__ = 6; break;
    case 1: // _bb1
      __label__ = 7; break;
    case 3: // _bb2
      var _12 = HEAP[_this_addr];
      var _14 = HEAP[0 + _12+1*1];
      var _15 = _14 * 2;
      HEAP[0 + _12+1*1] = _15;
      __label__ = 7; break;
    case 7: // _bb3
      var _18 = HEAP[_this_addr];
      var _20 = HEAP[0 + _18+1*1];
      var _22 = 0+(_20 < _sz_addr);
      if (_22) { __label__ = 3; break; } else { __label__ = 6; break; }
    case 6: // _bb4
      var _23 = HEAP[_this_addr];
      var _25 = HEAP[0 + _23+1*1];
      var _27 = 0+(_25 <= _olen);
      if (_27) { __label__ = 9; break; } else { __label__ = 4; break; }
    case 4: // _bb5
      var _28 = HEAP[_this_addr];
      var _30 = HEAP[0 + _28+1*1];
      var _31 = __Z9es_sizeofIP6vectorIcEEiPT_(0);
      _0 = _30 * _31;
      var _34 = __Znaj(_0);
      HEAP[_newbuf] = _34;
      var _36 = 0+(_olen > 0);
      if (_36) { __label__ = 8; break; } else { __label__ = 10; break; }
    case 8: // _bb6
      var _37 = __Z9es_sizeofIP6vectorIcEEiPT_(0);
      var _39 = _37 * _olen;
      var _40 = HEAP[_this_addr];
      var _42 = HEAP[0 + _40+0*1];
      var _43 = HEAP[_newbuf];
      _llvm_memcpy_i32(_43, _42, _39, 1);
      var _45 = HEAP[_this_addr];
      var _47 = HEAP[0 + _45+0*1];
      var _49 = 0+(_47 != 0);
      if (_49) { __label__ = 5; break; } else { __label__ = 10; break; }
    case 5: // _bb7
      var _50 = HEAP[_this_addr];
      var _52 = HEAP[0 + _50+0*1];
      __ZdaPv(_52);
      __label__ = 10; break;
    case 10: // _bb8
      var _54 = HEAP[_newbuf];
      var _55 = _54;
      var _56 = HEAP[_this_addr];
      HEAP[0 + _56+0*1] = _55;
      __label__ = 9; break;
    case 9: // _bb9
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __ZN6vectorIPS_IcEE3addERKS1_(_this, _x) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _this_addr = Pointer_make([0], 0);
      var _x_addr = Pointer_make([0], 0);
      var _retval = Pointer_make([0], 0);
      var _0 = Pointer_make([0], 0);
      var _iftmp_234 = Pointer_make([0], 0);
      var _1 = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_this_addr] = _this;
      HEAP[_x_addr] = _x;
      var _2 = HEAP[_this_addr];
      var _4 = HEAP[0 + _2+2*1];
      var _7 = HEAP[0 + _2+1*1];
      var _8 = 0+(_4 == _7);
      if (_8) { __label__ = 0; break; } else { __label__ = 1; break; }
    case 0: // _bb
      var _9 = HEAP[_this_addr];
      var _11 = HEAP[0 + _9+2*1];
      var _12 = _11 + 1;
      __ZN6vectorIPS_IcEE7growbufEi(_9, _12);
      __label__ = 1; break;
    case 1: // _bb1
      var _14 = HEAP[_this_addr];
      var _16 = HEAP[0 + _14+0*1];
      var _19 = HEAP[0 + _14+2*1];
      var _20 = _16+_19;
      var _22 = __ZnwjPv(4, _20);
      HEAP[_1] = _22;
      var _24 = HEAP[_1];
      var _25 = 0+(_24 != 0);
      if (_25) { __label__ = 3; break; } else { __label__ = 7; break; }
    case 3: // _bb2
      var _26 = HEAP[_x_addr];
      var _27 = HEAP[_26];
      var _28 = HEAP[_1];
      HEAP[_28] = _27;
      var _29 = HEAP[_1];
      HEAP[_iftmp_234] = _29;
      __label__ = 6; break;
    case 7: // _bb3
      var _30 = HEAP[_1];
      HEAP[_iftmp_234] = _30;
      __label__ = 6; break;
    case 6: // _bb4
      var _31 = HEAP[_this_addr];
      var _33 = HEAP[0 + _31+0*1];
      var _36 = HEAP[0 + _31+2*1];
      var _37 = _33+_36;
      HEAP[_0] = _37;
      var _38 = _36 + 1;
      var _39 = HEAP[_this_addr];
      HEAP[0 + _39+2*1] = _38;
      var _41 = HEAP[_0];
      HEAP[_retval] = _41;
      __label__ = 2; break;
    case 2: // _return
      var _retval5 = HEAP[_retval];
      return _retval5;
  }
}


function __ZN5identaSERKS_(_this, _o) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _this_addr = Pointer_make([0], 0);
      var _o_addr = Pointer_make([0], 0);
      var _retval = Pointer_make([0], 0);
      var _0 = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_this_addr] = _this;
      HEAP[_o_addr] = _o;
      var _1 = __Z9es_sizeofI5identEiPT_(0);
      var _2 = HEAP[_this_addr];
      var _3 = HEAP[_o_addr];
      var _4 = _2;
      _llvm_memcpy_i32(_4, _3, _1, 1);
      var _6 = HEAP[_this_addr];
      HEAP[_0] = _6;
      var _7 = HEAP[_0];
      HEAP[_retval] = _7;
      __label__ = 2; break;
    case 2: // _return
      var _retval1 = HEAP[_retval];
      return _retval1;
  }
}


function __Z10copystringPcPKcj(_d, _s, _len) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _d_addr = Pointer_make([0], 0);
      var _s_addr = Pointer_make([0], 0);
      var _len_addr;
      var _retval = Pointer_make([0], 0);
      var _0 = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_d_addr] = _d;
      HEAP[_s_addr] = _s;
      _len_addr = _len;
      var _1 = HEAP[_d_addr];
      var _2 = HEAP[_s_addr];
      var _4 = _strncpy(_1, _2, _len_addr);
      var _6 = _len_addr - 1;
      var _7 = HEAP[_d_addr];
      HEAP[0 + _7+_6] = 0;
      var _9 = HEAP[_d_addr];
      HEAP[_0] = _9;
      var _10 = HEAP[_0];
      HEAP[_retval] = _10;
      __label__ = 2; break;
    case 2: // _return
      var _retval1 = HEAP[_retval];
      return _retval1;
  }
}


function __Z9newstringPKcj(_s, _l) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _s_addr = Pointer_make([0], 0);
      var _l_addr;
      var _retval = Pointer_make([0], 0);
      var _0 = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_s_addr] = _s;
      _l_addr = _l;
      var _2 = _l_addr + 1;
      var _4 = __Z9newstringj(_l_addr);
      var _5 = HEAP[_s_addr];
      var _6 = __Z10copystringPcPKcj(_4, _5, _2);
      HEAP[_0] = _6;
      var _7 = HEAP[_0];
      HEAP[_retval] = _7;
      __label__ = 2; break;
    case 2: // _return
      var _retval1 = HEAP[_retval];
      return _retval1;
  }
}


function __Z7conlineiPKc(_type, _sf) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _type_addr;
      var _sf_addr = Pointer_make([0], 0);
      var _iftmp_204 = Pointer_make([0], 0);
      var _cl = Pointer_make([0,0,0], 0);
      var __alloca_point_ = 0;
      _type_addr = _type;
      HEAP[_sf_addr] = _sf;
      var _0 = __ZNK6vectorI5clineE6lengthEv(_conlines);
      var _1 = HEAP[_maxcon];
      var _2 = 0+(_0 > _1);
      if (_2) { __label__ = 0; break; } else { __label__ = 1; break; }
    case 0: // _bb
      var _3 = __ZN6vectorI5clineE3popEv(_conlines);
      var _5 = HEAP[0 + _3+0*1];
      HEAP[_iftmp_204] = _5;
      __label__ = 3; break;
    case 1: // _bb1
      var _6 = __Z9newstringPKcj(__str3+0*1, 511);
      HEAP[_iftmp_204] = _6;
      __label__ = 3; break;
    case 3: // _bb2
      var _7 = _cl+0*1;
      var _8 = HEAP[_iftmp_204];
      HEAP[_7] = _8;
      var _9 = _cl+1*1;
      HEAP[_9] = _type_addr;
      var _11 = HEAP[_totalmillis];
      HEAP[0 + _cl+2*1] = _11;
      var _13 = __ZN6vectorI5clineE6insertEiRKS0_(_conlines, 0, _cl);
      var _15 = HEAP[0 + _cl+0*1];
      var _16 = HEAP[_sf_addr];
      var _17 = __Z10copystringPcPKcj(_15, _16, 512);
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __Z8conoutfviPKcPc(_type, _fmt, _args) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _type_addr;
      var _fmt_addr = Pointer_make([0], 0);
      var _args_addr = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      _type_addr = _type;
      HEAP[_fmt_addr] = _fmt;
      HEAP[_args_addr] = _args;
      var _0 = __Z9es_sizeofIcEiPT_(0);
      var _1 = _0 * 512;
      var _2 = HEAP[_fmt_addr];
      var _3 = HEAP[_args_addr];
      __Z13vformatstringPcPKcS_i(__ZZ8conoutfviPKcPcE3buf+0*1, _2, _3, _1);
      __Z7conlineiPKc(_type_addr, __ZZ8conoutfviPKcPcE3buf+0*1);
      var _5 = _puts(__ZZ8conoutfviPKcPcE3buf+0*1);
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __Z7conoutfiPKcz(_type, _fmt) {
  __numArgs__ = 2;
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _type_addr;
      var _fmt_addr = Pointer_make([0], 0);
      var _args = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      _type_addr = _type;
      HEAP[_fmt_addr] = _fmt;
      HEAP[_args] = Pointer_make([Array.prototype.slice.call(arguments, __numArgs__).length].concat(Array.prototype.slice.call(arguments, __numArgs__)), 0);
      var _0 = HEAP[_args];
      var _1 = _type_addr;
      var _2 = HEAP[_fmt_addr];
      __Z8conoutfviPKcPc(_1, _2, _0);
      ;;
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __ZN7_stdcmdILi1195EE3runEPc(_s) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _s_addr = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_s_addr] = _s;
      var _0 = HEAP[_s_addr];
      __Z7conoutfiPKcz(4, _0);
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __Z14setfvarcheckedP5identf(_id, _val) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _id_addr = Pointer_make([0], 0);
      var _val_addr;
      var _iftmp_77;
      var __alloca_point_ = 0;
      HEAP[_id_addr] = _id;
      _val_addr = _val;
      var _0 = HEAP[_id_addr];
      var _2 = HEAP[0 + _0+10*1];
      var _3 = _2 & 8;
      var _4 = 0+(_3 != 0);
      if (_4) { __label__ = 0; break; } else { __label__ = 1; break; }
    case 0: // _bb
      var _5 = HEAP[_id_addr];
      var _7 = HEAP[0 + _5+2*1];
      __Z7conoutfiPKcz(4, __str4+0*1, _7);
      __label__ = 17; break;
    case 1: // _bb1
      var _8 = HEAP[_overrideidents];
      var _toBool = 0+(_8 != 0);
      if (_toBool) { __label__ = 7; break; } else { __label__ = 3; break; }
    case 3: // _bb2
      var _9 = HEAP[_id_addr];
      var _11 = HEAP[0 + _9+10*1];
      var _12 = _11 & 2;
      var _13 = 0+(_12 != 0);
      if (_13) { __label__ = 7; break; } else { __label__ = 9; break; }
    case 7: // _bb3
      var _14 = HEAP[_id_addr];
      var _16 = HEAP[0 + _14+10*1];
      var _17 = _16 & 1;
      var _18 = _17;
      var _toBool4 = 0+(_18 != 0);
      if (_toBool4) { __label__ = 4; break; } else { __label__ = 8; break; }
    case 4: // _bb5
      var _19 = HEAP[_id_addr];
      var _21 = HEAP[0 + _19+2*1];
      __Z7conoutfiPKcz(4, __str5+0*1, _21);
      __label__ = 17; break;
    case 8: // _bb6
      var _22 = HEAP[_id_addr];
      var _24 = HEAP[0 + _22+5*1];
      var _25 = 0+(_24 == 2147483647);
      if (_25) { __label__ = 5; break; } else { __label__ = 10; break; }
    case 5: // _bb7
      var _26 = HEAP[_id_addr];
      var _27 = _26+9*1;
      var _28 = _27;
      var _30 = HEAP[_28];
      var _31 = HEAP[_30];
      var _33 = _26+8*1;
      var _34 = _33+0*1;
      var _36 = _34;
      HEAP[_36] = _31;
      var _38 = HEAP[_id_addr];
      HEAP[0 + _38+5*1] = 0;
      __label__ = 10; break;
    case 10: // _bb8
      __label__ = 13; break;
    case 9: // _bb9
      var _40 = HEAP[_id_addr];
      var _42 = HEAP[0 + _40+5*1];
      var _43 = 0+(_42 != 2147483647);
      if (_43) { __label__ = 11; break; } else { __label__ = 13; break; }
    case 11: // _bb10
      var _44 = HEAP[_id_addr];
      HEAP[0 + _44+5*1] = 2147483647;
      __label__ = 13; break;
    case 13: // _bb11
      var _46 = HEAP[_id_addr];
      var _47 = _46+3*1;
      var _48 = _47;
      var _50 = HEAP[_48];
      var _52 = 0+(_50 > _val_addr);
      if (_52) { __label__ = 15; break; } else { __label__ = 14; break; }
    case 14: // _bb12
      var _53 = HEAP[_id_addr];
      var _54 = _53+4*1;
      var _55 = _54;
      var _57 = HEAP[_55];
      var _59 = 0+(_57 < _val_addr);
      if (_59) { __label__ = 15; break; } else { __label__ = 20; break; }
    case 15: // _bb13
      var _60 = HEAP[_id_addr];
      var _61 = _60+3*1;
      var _62 = _61;
      var _64 = HEAP[_62];
      var _66 = 0+(_64 > _val_addr);
      if (_66) { __label__ = 12; break; } else { __label__ = 21; break; }
    case 12: // _bb14
      var _67 = HEAP[_id_addr];
      var _68 = _67+3*1;
      var _69 = _68;
      _iftmp_77 = HEAP[_69];
      __label__ = 16; break;
    case 21: // _bb15
      var _72 = HEAP[_id_addr];
      var _73 = _72+4*1;
      var _74 = _73;
      _iftmp_77 = HEAP[_74];
      __label__ = 16; break;
    case 16: // _bb16
      _val_addr = _iftmp_77;
      var _78 = HEAP[_id_addr];
      var _79 = _78+4*1;
      var _80 = _79;
      var _82 = HEAP[_80];
      var _83 = __Z8floatstrf(_82);
      var _84 = HEAP[_id_addr];
      var _85 = _84+3*1;
      var _86 = _85;
      var _88 = HEAP[_86];
      var _89 = __Z8floatstrf(_88);
      var _90 = HEAP[_id_addr];
      var _92 = HEAP[0 + _90+2*1];
      __Z7conoutfiPKcz(4, __str6+0*1, _92, _89, _83);
      __label__ = 20; break;
    case 20: // _bb17
      var _93 = HEAP[_id_addr];
      var _94 = _93+9*1;
      var _95 = _94;
      var _97 = HEAP[_95];
      HEAP[_97] = _val_addr;
      var _99 = HEAP[_id_addr];
      var _101 = HEAP[0 + _99+0*1];
      var _103 = HEAP[0 + _101+2];
      var _104 = _103;
      _104(_99);
      __label__ = 17; break;
    case 17: // _bb18
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __Z13setvarcheckedP5identi(_id, _val) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _id_addr = Pointer_make([0], 0);
      var _val_addr;
      var _iftmp_75 = Pointer_make([0], 0);
      var _iftmp_74 = Pointer_make([0], 0);
      var _iftmp_73;
      var __alloca_point_ = 0;
      HEAP[_id_addr] = _id;
      _val_addr = _val;
      var _0 = HEAP[_id_addr];
      var _2 = HEAP[0 + _0+10*1];
      var _3 = _2 & 8;
      var _4 = 0+(_3 != 0);
      if (_4) { __label__ = 0; break; } else { __label__ = 1; break; }
    case 0: // _bb
      var _5 = HEAP[_id_addr];
      var _7 = HEAP[0 + _5+2*1];
      __Z7conoutfiPKcz(4, __str4+0*1, _7);
      __label__ = 22; break;
    case 1: // _bb1
      var _8 = HEAP[_overrideidents];
      var _toBool = 0+(_8 != 0);
      if (_toBool) { __label__ = 7; break; } else { __label__ = 3; break; }
    case 3: // _bb2
      var _9 = HEAP[_id_addr];
      var _11 = HEAP[0 + _9+10*1];
      var _12 = _11 & 2;
      var _13 = 0+(_12 != 0);
      if (_13) { __label__ = 7; break; } else { __label__ = 9; break; }
    case 7: // _bb3
      var _14 = HEAP[_id_addr];
      var _16 = HEAP[0 + _14+10*1];
      var _17 = _16 & 1;
      var _18 = _17;
      var _toBool4 = 0+(_18 != 0);
      if (_toBool4) { __label__ = 4; break; } else { __label__ = 8; break; }
    case 4: // _bb5
      var _19 = HEAP[_id_addr];
      var _21 = HEAP[0 + _19+2*1];
      __Z7conoutfiPKcz(4, __str5+0*1, _21);
      __label__ = 22; break;
    case 8: // _bb6
      var _22 = HEAP[_id_addr];
      var _24 = HEAP[0 + _22+5*1];
      var _25 = 0+(_24 == 2147483647);
      if (_25) { __label__ = 5; break; } else { __label__ = 10; break; }
    case 5: // _bb7
      var _26 = HEAP[_id_addr];
      var _27 = _26+9*1;
      var _29 = HEAP[0 + _27];
      var _30 = HEAP[_29];
      var _32 = _26+8*1;
      var _33 = _32+0*1;
      HEAP[0 + _33] = _30;
      var _36 = HEAP[_id_addr];
      HEAP[0 + _36+5*1] = 0;
      __label__ = 10; break;
    case 10: // _bb8
      __label__ = 13; break;
    case 9: // _bb9
      var _38 = HEAP[_id_addr];
      var _40 = HEAP[0 + _38+5*1];
      var _41 = 0+(_40 != 2147483647);
      if (_41) { __label__ = 11; break; } else { __label__ = 13; break; }
    case 11: // _bb10
      var _42 = HEAP[_id_addr];
      HEAP[0 + _42+5*1] = 2147483647;
      __label__ = 13; break;
    case 13: // _bb11
      var _44 = HEAP[_id_addr];
      var _45 = _44+3*1;
      var _47 = HEAP[0 + _45];
      var _49 = 0+(_47 > _val_addr);
      if (_49) { __label__ = 15; break; } else { __label__ = 14; break; }
    case 14: // _bb12
      var _50 = HEAP[_id_addr];
      var _51 = _50+4*1;
      var _53 = HEAP[0 + _51];
      var _55 = 0+(_53 < _val_addr);
      if (_55) { __label__ = 15; break; } else { __label__ = 23; break; }
    case 15: // _bb13
      var _56 = HEAP[_id_addr];
      var _57 = _56+3*1;
      var _59 = HEAP[0 + _57];
      var _61 = 0+(_59 > _val_addr);
      if (_61) { __label__ = 12; break; } else { __label__ = 21; break; }
    case 12: // _bb14
      var _62 = HEAP[_id_addr];
      var _63 = _62+3*1;
      var _iftmp_73 = HEAP[0 + _63];
      __label__ = 16; break;
    case 21: // _bb15
      var _66 = HEAP[_id_addr];
      var _67 = _66+4*1;
      var _iftmp_73 = HEAP[0 + _67];
      __label__ = 16; break;
    case 16: // _bb16
      _val_addr = _iftmp_73;
      var _71 = HEAP[_id_addr];
      var _72 = _71+4*1;
      var _74 = HEAP[0 + _72];
      var _76 = _71+3*1;
      var _78 = HEAP[0 + _76];
      var _79 = HEAP[_id_addr];
      var _81 = HEAP[0 + _79+2*1];
      var _84 = HEAP[0 + _79+10*1];
      var _85 = _84 & 4;
      var _86 = 0+(_85 != 0);
      if (_86) { __label__ = 20; break; } else { __label__ = 24; break; }
    case 20: // _bb17
      var _87 = HEAP[_id_addr];
      var _88 = _87+3*1;
      var _90 = HEAP[0 + _88];
      var _91 = 0+(_90 <= 255);
      if (_91) { __label__ = 17; break; } else { __label__ = 18; break; }
    case 17: // _bb18
      HEAP[_iftmp_75] = __str7+0*1;
      __label__ = 19; break;
    case 18: // _bb19
      HEAP[_iftmp_75] = __str8+0*1;
      __label__ = 19; break;
    case 19: // _bb20
      var _92 = HEAP[_iftmp_75];
      HEAP[_iftmp_74] = _92;
      __label__ = 25; break;
    case 24: // _bb21
      HEAP[_iftmp_74] = __str9+0*1;
      __label__ = 25; break;
    case 25: // _bb22
      var _93 = HEAP[_iftmp_74];
      __Z7conoutfiPKcz(4, _93, _81, _78, _74);
      __label__ = 23; break;
    case 23: // _bb23
      var _94 = HEAP[_id_addr];
      var _95 = _94+9*1;
      var _97 = HEAP[0 + _95];
      HEAP[_97] = _val_addr;
      var _99 = HEAP[_id_addr];
      var _101 = HEAP[0 + _99+0*1];
      var _103 = HEAP[0 + _101+2];
      var _104 = _103;
      _104(_99);
      __label__ = 22; break;
    case 22: // _bb24
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __Z7conoutfPKcz(_fmt) {
  __numArgs__ = 1;
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _fmt_addr = Pointer_make([0], 0);
      var _args = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_fmt_addr] = _fmt;
      HEAP[_args] = Pointer_make([Array.prototype.slice.call(arguments, __numArgs__).length].concat(Array.prototype.slice.call(arguments, __numArgs__)), 0);
      var _0 = HEAP[_args];
      var _1 = HEAP[_fmt_addr];
      __Z8conoutfviPKcPc(1, _1, _0);
      ;;
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __ZN7_stdcmdILi1194EE3runEPc(_s) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _s_addr = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_s_addr] = _s;
      var _0 = HEAP[_s_addr];
      __Z7conoutfPKcz(__str10+0*1, _0);
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __Z9newstringPKc(_s) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _s_addr = Pointer_make([0], 0);
      var _retval = Pointer_make([0], 0);
      var _0 = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_s_addr] = _s;
      var _1 = HEAP[_s_addr];
      var _2 = _strlen(_1);
      var _3 = HEAP[_s_addr];
      var _4 = __Z9newstringPKcj(_3, _2);
      HEAP[_0] = _4;
      var _5 = HEAP[_0];
      HEAP[_retval] = _5;
      __label__ = 2; break;
    case 2: // _return
      var _retval1 = HEAP[_retval];
      return _retval1;
  }
}


function __Z6resultPKc(_s) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _s_addr = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_s_addr] = _s;
      var _0 = HEAP[_s_addr];
      var _1 = __Z9newstringPKc(_0);
      HEAP[_commandret] = _1;
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __Z6concatPKc(_s) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _s_addr = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_s_addr] = _s;
      var _0 = HEAP[_s_addr];
      var _1 = __Z9newstringPKc(_0);
      HEAP[_commandret] = _1;
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __Z8floatretf(_v) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _v_addr;
      var __alloca_point_ = 0;
      _v_addr = _v;
      var _1 = __Z8floatstrf(_v_addr);
      var _2 = __Z9newstringPKc(_1);
      HEAP[_commandret] = _2;
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __ZN7_stdcmdILi1118EE3runEPfS1_(_a, _b) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _a_addr = Pointer_make([0], 0);
      var _b_addr = Pointer_make([0], 0);
      var _iftmp_187;
      var __alloca_point_ = 0;
      HEAP[_a_addr] = _a;
      HEAP[_b_addr] = _b;
      var _0 = HEAP[_b_addr];
      var _1 = HEAP[_0];
      var _2 = 0+(_1 != 0.000000e+00);
      if (_2) { __label__ = 0; break; } else { __label__ = 1; break; }
    case 0: // _bb
      var _3 = HEAP[_a_addr];
      var _4 = HEAP[_3];
      var _5 = HEAP[_b_addr];
      var _6 = HEAP[_5];
      _iftmp_187 = _4 / _6;
      __label__ = 3; break;
    case 1: // _bb1
      _iftmp_187 = 0.000000e+00;
      __label__ = 3; break;
    case 3: // _bb2
      __Z8floatretf(_iftmp_187);
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __ZN7_stdcmdILi1080EE3runEPfS1_(_a, _b) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _a_addr = Pointer_make([0], 0);
      var _b_addr = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_a_addr] = _a;
      HEAP[_b_addr] = _b;
      var _0 = HEAP[_a_addr];
      var _1 = HEAP[_0];
      var _2 = HEAP[_b_addr];
      var _3 = HEAP[_2];
      var _4 = _1 - _3;
      __Z8floatretf(_4);
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __ZN7_stdcmdILi1079EE3runEPfS1_(_a, _b) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _a_addr = Pointer_make([0], 0);
      var _b_addr = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_a_addr] = _a;
      HEAP[_b_addr] = _b;
      var _0 = HEAP[_a_addr];
      var _1 = HEAP[_0];
      var _2 = HEAP[_b_addr];
      var _3 = HEAP[_2];
      var _4 = _1 * _3;
      __Z8floatretf(_4);
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __ZN7_stdcmdILi1078EE3runEPfS1_(_a, _b) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _a_addr = Pointer_make([0], 0);
      var _b_addr = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_a_addr] = _a;
      HEAP[_b_addr] = _b;
      var _0 = HEAP[_a_addr];
      var _1 = HEAP[_0];
      var _2 = HEAP[_b_addr];
      var _3 = HEAP[_2];
      var _4 = _1 + _3;
      __Z8floatretf(_4);
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __Z6intreti(_v) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _v_addr;
      var __alloca_point_ = 0;
      _v_addr = _v;
      var _1 = __Z6intstri(_v_addr);
      var _2 = __Z9newstringPKc(_1);
      HEAP[_commandret] = _2;
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __ZN7_stdcmdILi1186EE3runEPiS1_(_a, _b) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _a_addr = Pointer_make([0], 0);
      var _b_addr = Pointer_make([0], 0);
      var _iftmp_196;
      var __alloca_point_ = 0;
      HEAP[_a_addr] = _a;
      HEAP[_b_addr] = _b;
      var _0 = HEAP[_a_addr];
      var _1 = HEAP[_0];
      var _2 = HEAP[_b_addr];
      var _3 = HEAP[_2];
      var _4 = _1 - _3;
      var _5 = 0+(_4 > 0);
      if (_5) { __label__ = 0; break; } else { __label__ = 1; break; }
    case 0: // _bb
      var _6 = __Z8randomMTv();
      var _7 = _6 & 16777215;
      var _8 = HEAP[_a_addr];
      var _9 = HEAP[_8];
      var _10 = HEAP[_b_addr];
      var _11 = HEAP[_10];
      var _12 = _9 - _11;
      var _13 = Math.floor(_7 % _12);
      var _15 = HEAP[_10];
      _iftmp_196 = _13 + _15;
      __label__ = 3; break;
    case 1: // _bb1
      var _17 = HEAP[_b_addr];
      _iftmp_196 = HEAP[_17];
      __label__ = 3; break;
    case 3: // _bb2
      __Z6intreti(_iftmp_196);
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __ZN7_stdcmdILi1117EE3runEPiS1_(_a, _b) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _a_addr = Pointer_make([0], 0);
      var _b_addr = Pointer_make([0], 0);
      var _iftmp_186;
      var __alloca_point_ = 0;
      HEAP[_a_addr] = _a;
      HEAP[_b_addr] = _b;
      var _0 = HEAP[_b_addr];
      var _1 = HEAP[_0];
      var _2 = 0+(_1 != 0);
      if (_2) { __label__ = 0; break; } else { __label__ = 1; break; }
    case 0: // _bb
      var _3 = HEAP[_a_addr];
      var _4 = HEAP[_3];
      var _5 = HEAP[_b_addr];
      var _6 = HEAP[_5];
      _iftmp_186 = Math.floor(_4 % _6);
      __label__ = 3; break;
    case 1: // _bb1
      _iftmp_186 = 0;
      __label__ = 3; break;
    case 3: // _bb2
      __Z6intreti(_iftmp_186);
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __ZN7_stdcmdILi1116EE3runEPiS1_(_a, _b) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _a_addr = Pointer_make([0], 0);
      var _b_addr = Pointer_make([0], 0);
      var _iftmp_185;
      var __alloca_point_ = 0;
      HEAP[_a_addr] = _a;
      HEAP[_b_addr] = _b;
      var _0 = HEAP[_b_addr];
      var _1 = HEAP[_0];
      var _2 = 0+(_1 != 0);
      if (_2) { __label__ = 0; break; } else { __label__ = 1; break; }
    case 0: // _bb
      var _3 = HEAP[_a_addr];
      var _4 = HEAP[_3];
      var _5 = HEAP[_b_addr];
      var _6 = HEAP[_5];
      _iftmp_185 = Math.floor(_4 / _6);
      __label__ = 3; break;
    case 1: // _bb1
      _iftmp_185 = 0;
      __label__ = 3; break;
    case 3: // _bb2
      __Z6intreti(_iftmp_185);
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __ZN7_stdcmdILi1102EE3runEPiS1_(_a, _b) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _a_addr = Pointer_make([0], 0);
      var _b_addr = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_a_addr] = _a;
      HEAP[_b_addr] = _b;
      var _0 = HEAP[_a_addr];
      var _1 = HEAP[_0];
      var _2 = HEAP[_b_addr];
      var _3 = HEAP[_2];
      var _4 = _1 >> _3;
      __Z6intreti(_4);
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __ZN7_stdcmdILi1101EE3runEPiS1_(_a, _b) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _a_addr = Pointer_make([0], 0);
      var _b_addr = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_a_addr] = _a;
      HEAP[_b_addr] = _b;
      var _0 = HEAP[_a_addr];
      var _1 = HEAP[_0];
      var _2 = HEAP[_b_addr];
      var _3 = HEAP[_2];
      var _4 = _1 << _3;
      __Z6intreti(_4);
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __ZN7_stdcmdILi1100EE3runEPiS1_(_a, _b) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _a_addr = Pointer_make([0], 0);
      var _b_addr = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_a_addr] = _a;
      HEAP[_b_addr] = _b;
      var _0 = HEAP[_a_addr];
      var _1 = HEAP[_0];
      var _2 = HEAP[_b_addr];
      var _3 = HEAP[_2];
      var _not = _3 ^ -1;
      var _4 = _1 | _not;
      __Z6intreti(_4);
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __ZN7_stdcmdILi1099EE3runEPiS1_(_a, _b) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _a_addr = Pointer_make([0], 0);
      var _b_addr = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_a_addr] = _a;
      HEAP[_b_addr] = _b;
      var _0 = HEAP[_a_addr];
      var _1 = HEAP[_0];
      var _2 = HEAP[_b_addr];
      var _3 = HEAP[_2];
      var _not = _3 ^ -1;
      var _4 = _1 & _not;
      __Z6intreti(_4);
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __ZN7_stdcmdILi1098EE3runEPiS1_(_a, _b) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _a_addr = Pointer_make([0], 0);
      var _b_addr = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_a_addr] = _a;
      HEAP[_b_addr] = _b;
      var _0 = HEAP[_a_addr];
      var _1 = HEAP[_0];
      var _2 = HEAP[_b_addr];
      var _3 = HEAP[_2];
      var _not = _3 ^ -1;
      var _4 = _1 ^ _not;
      __Z6intreti(_4);
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __ZN7_stdcmdILi1097EE3runEPi(_a) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _a_addr = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_a_addr] = _a;
      var _0 = HEAP[_a_addr];
      var _1 = HEAP[_0];
      var _not = _1 ^ -1;
      __Z6intreti(_not);
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __ZN7_stdcmdILi1096EE3runEPiS1_(_a, _b) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _a_addr = Pointer_make([0], 0);
      var _b_addr = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_a_addr] = _a;
      HEAP[_b_addr] = _b;
      var _0 = HEAP[_a_addr];
      var _1 = HEAP[_0];
      var _2 = HEAP[_b_addr];
      var _3 = HEAP[_2];
      var _4 = _1 | _3;
      __Z6intreti(_4);
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __ZN7_stdcmdILi1095EE3runEPiS1_(_a, _b) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _a_addr = Pointer_make([0], 0);
      var _b_addr = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_a_addr] = _a;
      HEAP[_b_addr] = _b;
      var _0 = HEAP[_a_addr];
      var _1 = HEAP[_0];
      var _2 = HEAP[_b_addr];
      var _3 = HEAP[_2];
      var _4 = _1 & _3;
      __Z6intreti(_4);
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __ZN7_stdcmdILi1094EE3runEPi(_a) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _a_addr = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_a_addr] = _a;
      var _0 = HEAP[_a_addr];
      var _1 = HEAP[_0];
      var _2 = 0+(_1 == 0);
      var _3 = _2;
      __Z6intreti(_3);
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __ZN7_stdcmdILi1093EE3runEPiS1_(_a, _b) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _a_addr = Pointer_make([0], 0);
      var _b_addr = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_a_addr] = _a;
      HEAP[_b_addr] = _b;
      var _0 = HEAP[_a_addr];
      var _1 = HEAP[_0];
      var _2 = HEAP[_b_addr];
      var _3 = HEAP[_2];
      var _4 = _1 ^ _3;
      __Z6intreti(_4);
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __ZN7_stdcmdILi1092EE3runEPfS1_(_a, _b) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _a_addr = Pointer_make([0], 0);
      var _b_addr = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_a_addr] = _a;
      HEAP[_b_addr] = _b;
      var _0 = HEAP[_a_addr];
      var _1 = HEAP[_0];
      var _2 = HEAP[_b_addr];
      var _3 = HEAP[_2];
      var _4 = 0+(_1 >= _3);
      var _5 = _4;
      __Z6intreti(_5);
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __ZN7_stdcmdILi1091EE3runEPfS1_(_a, _b) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _a_addr = Pointer_make([0], 0);
      var _b_addr = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_a_addr] = _a;
      HEAP[_b_addr] = _b;
      var _0 = HEAP[_a_addr];
      var _1 = HEAP[_0];
      var _2 = HEAP[_b_addr];
      var _3 = HEAP[_2];
      var _4 = 0+(_1 <= _3);
      var _5 = _4;
      __Z6intreti(_5);
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __ZN7_stdcmdILi1090EE3runEPfS1_(_a, _b) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _a_addr = Pointer_make([0], 0);
      var _b_addr = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_a_addr] = _a;
      HEAP[_b_addr] = _b;
      var _0 = HEAP[_a_addr];
      var _1 = HEAP[_0];
      var _2 = HEAP[_b_addr];
      var _3 = HEAP[_2];
      var _4 = 0+(_1 > _3);
      var _5 = _4;
      __Z6intreti(_5);
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __ZN7_stdcmdILi1089EE3runEPfS1_(_a, _b) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _a_addr = Pointer_make([0], 0);
      var _b_addr = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_a_addr] = _a;
      HEAP[_b_addr] = _b;
      var _0 = HEAP[_a_addr];
      var _1 = HEAP[_0];
      var _2 = HEAP[_b_addr];
      var _3 = HEAP[_2];
      var _4 = 0+(_1 < _3);
      var _5 = _4;
      __Z6intreti(_5);
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __ZN7_stdcmdILi1088EE3runEPfS1_(_a, _b) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _a_addr = Pointer_make([0], 0);
      var _b_addr = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_a_addr] = _a;
      HEAP[_b_addr] = _b;
      var _0 = HEAP[_a_addr];
      var _1 = HEAP[_0];
      var _2 = HEAP[_b_addr];
      var _3 = HEAP[_2];
      var _4 = 0+(_1 != _3);
      var _5 = _4;
      __Z6intreti(_5);
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __ZN7_stdcmdILi1087EE3runEPfS1_(_a, _b) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _a_addr = Pointer_make([0], 0);
      var _b_addr = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_a_addr] = _a;
      HEAP[_b_addr] = _b;
      var _0 = HEAP[_a_addr];
      var _1 = HEAP[_0];
      var _2 = HEAP[_b_addr];
      var _3 = HEAP[_2];
      var _4 = 0+(_1 == _3);
      var _5 = _4;
      __Z6intreti(_5);
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __ZN7_stdcmdILi1086EE3runEPiS1_(_a, _b) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _a_addr = Pointer_make([0], 0);
      var _b_addr = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_a_addr] = _a;
      HEAP[_b_addr] = _b;
      var _0 = HEAP[_a_addr];
      var _1 = HEAP[_0];
      var _2 = HEAP[_b_addr];
      var _3 = HEAP[_2];
      var _4 = 0+(_1 >= _3);
      var _5 = _4;
      __Z6intreti(_5);
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __ZN7_stdcmdILi1085EE3runEPiS1_(_a, _b) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _a_addr = Pointer_make([0], 0);
      var _b_addr = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_a_addr] = _a;
      HEAP[_b_addr] = _b;
      var _0 = HEAP[_a_addr];
      var _1 = HEAP[_0];
      var _2 = HEAP[_b_addr];
      var _3 = HEAP[_2];
      var _4 = 0+(_1 <= _3);
      var _5 = _4;
      __Z6intreti(_5);
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __ZN7_stdcmdILi1084EE3runEPiS1_(_a, _b) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _a_addr = Pointer_make([0], 0);
      var _b_addr = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_a_addr] = _a;
      HEAP[_b_addr] = _b;
      var _0 = HEAP[_a_addr];
      var _1 = HEAP[_0];
      var _2 = HEAP[_b_addr];
      var _3 = HEAP[_2];
      var _4 = 0+(_1 > _3);
      var _5 = _4;
      __Z6intreti(_5);
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __ZN7_stdcmdILi1083EE3runEPiS1_(_a, _b) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _a_addr = Pointer_make([0], 0);
      var _b_addr = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_a_addr] = _a;
      HEAP[_b_addr] = _b;
      var _0 = HEAP[_a_addr];
      var _1 = HEAP[_0];
      var _2 = HEAP[_b_addr];
      var _3 = HEAP[_2];
      var _4 = 0+(_1 < _3);
      var _5 = _4;
      __Z6intreti(_5);
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __ZN7_stdcmdILi1082EE3runEPiS1_(_a, _b) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _a_addr = Pointer_make([0], 0);
      var _b_addr = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_a_addr] = _a;
      HEAP[_b_addr] = _b;
      var _0 = HEAP[_a_addr];
      var _1 = HEAP[_0];
      var _2 = HEAP[_b_addr];
      var _3 = HEAP[_2];
      var _4 = 0+(_1 != _3);
      var _5 = _4;
      __Z6intreti(_5);
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __ZN7_stdcmdILi1081EE3runEPiS1_(_a, _b) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _a_addr = Pointer_make([0], 0);
      var _b_addr = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_a_addr] = _a;
      HEAP[_b_addr] = _b;
      var _0 = HEAP[_a_addr];
      var _1 = HEAP[_0];
      var _2 = HEAP[_b_addr];
      var _3 = HEAP[_2];
      var _4 = 0+(_1 == _3);
      var _5 = _4;
      __Z6intreti(_5);
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __ZN7_stdcmdILi1077EE3runEPiS1_(_a, _b) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _a_addr = Pointer_make([0], 0);
      var _b_addr = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_a_addr] = _a;
      HEAP[_b_addr] = _b;
      var _0 = HEAP[_a_addr];
      var _1 = HEAP[_0];
      var _2 = HEAP[_b_addr];
      var _3 = HEAP[_2];
      var _4 = _1 - _3;
      __Z6intreti(_4);
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __ZN7_stdcmdILi1076EE3runEPiS1_(_a, _b) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _a_addr = Pointer_make([0], 0);
      var _b_addr = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_a_addr] = _a;
      HEAP[_b_addr] = _b;
      var _0 = HEAP[_a_addr];
      var _1 = HEAP[_0];
      var _2 = HEAP[_b_addr];
      var _3 = HEAP[_2];
      var _4 = _1 * _3;
      __Z6intreti(_4);
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __ZN7_stdcmdILi1075EE3runEPiS1_(_a, _b) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _a_addr = Pointer_make([0], 0);
      var _b_addr = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_a_addr] = _a;
      HEAP[_b_addr] = _b;
      var _0 = HEAP[_a_addr];
      var _1 = HEAP[_0];
      var _2 = HEAP[_b_addr];
      var _3 = HEAP[_2];
      var _4 = _1 + _3;
      __Z6intreti(_4);
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __Z14setsvarcheckedP5identPKc(_id, _val) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _id_addr = Pointer_make([0], 0);
      var _val_addr = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_id_addr] = _id;
      HEAP[_val_addr] = _val;
      var _0 = HEAP[_id_addr];
      var _2 = HEAP[0 + _0+10*1];
      var _3 = _2 & 8;
      var _4 = 0+(_3 != 0);
      if (_4) { __label__ = 0; break; } else { __label__ = 1; break; }
    case 0: // _bb
      var _5 = HEAP[_id_addr];
      var _7 = HEAP[0 + _5+2*1];
      __Z7conoutfiPKcz(4, __str4+0*1, _7);
      __label__ = 17; break;
    case 1: // _bb1
      var _8 = HEAP[_overrideidents];
      var _toBool = 0+(_8 != 0);
      if (_toBool) { __label__ = 7; break; } else { __label__ = 3; break; }
    case 3: // _bb2
      var _9 = HEAP[_id_addr];
      var _11 = HEAP[0 + _9+10*1];
      var _12 = _11 & 2;
      var _13 = 0+(_12 != 0);
      if (_13) { __label__ = 7; break; } else { __label__ = 13; break; }
    case 7: // _bb3
      var _14 = HEAP[_id_addr];
      var _16 = HEAP[0 + _14+10*1];
      var _17 = _16 & 1;
      var _18 = _17;
      var _toBool4 = 0+(_18 != 0);
      if (_toBool4) { __label__ = 4; break; } else { __label__ = 8; break; }
    case 4: // _bb5
      var _19 = HEAP[_id_addr];
      var _21 = HEAP[0 + _19+2*1];
      __Z7conoutfiPKcz(4, __str5+0*1, _21);
      __label__ = 17; break;
    case 8: // _bb6
      var _22 = HEAP[_id_addr];
      var _24 = HEAP[0 + _22+5*1];
      var _25 = 0+(_24 == 2147483647);
      if (_25) { __label__ = 5; break; } else { __label__ = 10; break; }
    case 5: // _bb7
      var _26 = HEAP[_id_addr];
      var _27 = _26+9*1;
      var _28 = _27;
      var _30 = HEAP[_28];
      var _31 = HEAP[_30];
      var _33 = _26+8*1;
      var _34 = _33+0*1;
      var _36 = _34;
      HEAP[_36] = _31;
      var _38 = HEAP[_id_addr];
      HEAP[0 + _38+5*1] = 0;
      __label__ = 11; break;
    case 10: // _bb8
      var _40 = HEAP[_id_addr];
      var _41 = _40+9*1;
      var _42 = _41;
      var _44 = HEAP[_42];
      var _45 = HEAP[_44];
      var _46 = 0+(_45 != 0);
      if (_46) { __label__ = 9; break; } else { __label__ = 11; break; }
    case 9: // _bb9
      var _47 = HEAP[_id_addr];
      var _48 = _47+9*1;
      var _49 = _48;
      var _51 = HEAP[_49];
      var _52 = HEAP[_51];
      __ZdaPv(_52);
      __label__ = 11; break;
    case 11: // _bb10
      __label__ = 20; break;
    case 13: // _bb11
      var _53 = HEAP[_id_addr];
      var _55 = HEAP[0 + _53+5*1];
      var _56 = 0+(_55 != 2147483647);
      if (_56) { __label__ = 14; break; } else { __label__ = 21; break; }
    case 14: // _bb12
      var _57 = HEAP[_id_addr];
      var _58 = _57+8*1;
      var _59 = _58+0*1;
      var _61 = _59;
      var _63 = HEAP[_61];
      var _64 = 0+(_63 != 0);
      if (_64) { __label__ = 15; break; } else { __label__ = 12; break; }
    case 15: // _bb13
      var _65 = HEAP[_id_addr];
      var _66 = _65+8*1;
      var _67 = _66+0*1;
      var _69 = _67;
      var _71 = HEAP[_69];
      __ZdaPv(_71);
      __label__ = 12; break;
    case 12: // _bb14
      var _72 = HEAP[_id_addr];
      HEAP[0 + _72+5*1] = 2147483647;
      __label__ = 21; break;
    case 21: // _bb15
      var _74 = HEAP[_id_addr];
      var _75 = _74+9*1;
      var _76 = _75;
      var _78 = HEAP[_76];
      var _79 = HEAP[_78];
      var _80 = 0+(_79 != 0);
      if (_80) { __label__ = 16; break; } else { __label__ = 20; break; }
    case 16: // _bb16
      var _81 = HEAP[_id_addr];
      var _82 = _81+9*1;
      var _83 = _82;
      var _85 = HEAP[_83];
      var _86 = HEAP[_85];
      __ZdaPv(_86);
      __label__ = 20; break;
    case 20: // _bb17
      var _87 = HEAP[_id_addr];
      var _88 = _87+9*1;
      var _89 = _88;
      var _91 = HEAP[_89];
      var _92 = HEAP[_val_addr];
      var _93 = __Z9newstringPKc(_92);
      HEAP[_91] = _93;
      var _94 = HEAP[_id_addr];
      var _96 = HEAP[0 + _94+0*1];
      var _98 = HEAP[0 + _96+2];
      var _99 = _98;
      _99(_94);
      __label__ = 17; break;
    case 17: // _bb18
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __Z13clearoverrideR5ident(_i) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _i_addr = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_i_addr] = _i;
      var _0 = HEAP[_i_addr];
      var _2 = HEAP[0 + _0+5*1];
      var _3 = 0+(_2 == 2147483647);
      if (_3) { __label__ = 15; break; } else { __label__ = 0; break; }
    case 0: // _bb
      var _4 = HEAP[_i_addr];
      var _6 = HEAP[0 + _4+1*1];
      if (_6 == 0) {
  __label__ = 5; break;
}
else if (_6 == 1) {
  __label__ = 10; break;
}
else if (_6 == 2) {
  __label__ = 9; break;
}
else if (_6 == 5) {
  __label__ = 1; break;
}
else {
__label__ = 14; break;
}

    case 1: // _bb1
      var _7 = HEAP[_i_addr];
      var _8 = _7+7*1;
      var _10 = HEAP[0 + _8+0*1];
      var _12 = HEAP[0 + _10];
      var _13 = 0+(_12 != 0);
      if (_13) { __label__ = 3; break; } else { __label__ = 8; break; }
    case 3: // _bb2
      var _14 = HEAP[_i_addr];
      var _15 = _14+7*1;
      var _17 = HEAP[0 + _15+0*1];
      var _19 = _14+8*1;
      var _21 = HEAP[0 + _19+0*1];
      var _22 = 0+(_17 != _21);
      if (_22) { __label__ = 7; break; } else { __label__ = 4; break; }
    case 7: // _bb3
      var _23 = HEAP[_i_addr];
      var _24 = _23+7*1;
      var _26 = HEAP[0 + _24+0*1];
      var _27 = 0+(_26 != 0);
      if (_27) { __label__ = 6; break; } else { __label__ = 4; break; }
    case 6: // _bb4
      var _28 = HEAP[_i_addr];
      var _29 = _28+7*1;
      var _31 = HEAP[0 + _29+0*1];
      __ZdaPv(_31);
      __label__ = 4; break;
    case 4: // _bb5
      var _32 = __Z9newstringPKc(__str3+0*1);
      var _33 = HEAP[_i_addr];
      var _34 = _33+7*1;
      HEAP[0 + _34+0*1] = _32;
      __label__ = 8; break;
    case 8: // _bb6
      __label__ = 14; break;
    case 5: // _bb7
      var _36 = HEAP[_i_addr];
      var _37 = _36+9*1;
      var _39 = HEAP[0 + _37];
      var _41 = _36+8*1;
      var _42 = _41+0*1;
      var _45 = HEAP[0 + _42];
      HEAP[_39] = _45;
      var _46 = HEAP[_i_addr];
      var _48 = HEAP[0 + _46+0*1];
      var _50 = HEAP[0 + _48+2];
      var _51 = _50;
      _51(_46);
      __label__ = 14; break;
    case 10: // _bb8
      var _53 = HEAP[_i_addr];
      var _54 = _53+9*1;
      var _55 = _54;
      var _57 = HEAP[_55];
      var _59 = _53+8*1;
      var _60 = _59+0*1;
      var _62 = _60;
      var _64 = HEAP[_62];
      HEAP[_57] = _64;
      var _65 = HEAP[_i_addr];
      var _67 = HEAP[0 + _65+0*1];
      var _69 = HEAP[0 + _67+2];
      var _70 = _69;
      _70(_65);
      __label__ = 14; break;
    case 9: // _bb9
      var _72 = HEAP[_i_addr];
      var _73 = _72+9*1;
      var _74 = _73;
      var _76 = HEAP[_74];
      var _77 = HEAP[_76];
      var _78 = 0+(_77 != 0);
      if (_78) { __label__ = 11; break; } else { __label__ = 13; break; }
    case 11: // _bb10
      var _79 = HEAP[_i_addr];
      var _80 = _79+9*1;
      var _81 = _80;
      var _83 = HEAP[_81];
      var _84 = HEAP[_83];
      __ZdaPv(_84);
      __label__ = 13; break;
    case 13: // _bb11
      var _85 = HEAP[_i_addr];
      var _86 = _85+9*1;
      var _87 = _86;
      var _89 = HEAP[_87];
      var _91 = _85+8*1;
      var _92 = _91+0*1;
      var _94 = _92;
      var _96 = HEAP[_94];
      HEAP[_89] = _96;
      var _97 = HEAP[_i_addr];
      var _99 = HEAP[0 + _97+0*1];
      var _101 = HEAP[0 + _99+2];
      var _102 = _101;
      _102(_97);
      __label__ = 14; break;
    case 14: // _bb12
      var _104 = HEAP[_i_addr];
      HEAP[0 + _104+5*1] = 2147483647;
      __label__ = 15; break;
    case 15: // _bb13
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __Z14clearoverridesv() {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _i;
      var _enumc = Pointer_make([0], 0);
      var _i2 = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      _i = 0;
      __label__ = 4; break;
    case 0: // _bb
      var _0 = HEAP[_idents];
      var _1 = _0+0*5;
      var _3 = HEAP[0 + _1+2*1];
      var _6 = HEAP[0 + _3+_i];
      HEAP[_enumc] = _6;
      __label__ = 7; break;
    case 1: // _bb1
      var _8 = HEAP[_enumc];
      var _9 = __ZN9hashtableIPKc5identE7getdataEPv(_8);
      HEAP[_i2] = _9;
      var _10 = HEAP[_enumc];
      var _11 = __ZN9hashtableIPKc5identE7getnextEPv(_10);
      HEAP[_enumc] = _11;
      var _13 = HEAP[_i2];
      __Z13clearoverrideR5ident(_13);
      __label__ = 7; break;
    case 7: // _bb3
      var _14 = HEAP[_enumc];
      var _15 = 0+(_14 != 0);
      if (_15) { __label__ = 1; break; } else { __label__ = 6; break; }
    case 6: // _bb4
      _i = _i + 1;
      __label__ = 4; break;
    case 4: // _bb5
      var _18 = HEAP[_idents];
      var _19 = _18+0*5;
      var _21 = HEAP[0 + _19+0*1];
      var _23 = 0+(_21 > _i);
      if (_23) { __label__ = 0; break; } else { __label__ = 8; break; }
    case 8: // _bb6
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __ZN7_stdcmdILi1197EE3runEPc(_s) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _s_addr = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_s_addr] = _s;
      var _0 = HEAP[_s_addr];
      var _1 = _strlen(_0);
      __Z6intreti(_1);
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __ZN7_stdcmdILi1196EE3runEPcS1_(_a, _b) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _a_addr = Pointer_make([0], 0);
      var _b_addr = Pointer_make([0], 0);
      var _iftmp_197;
      var _s = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_a_addr] = _a;
      HEAP[_b_addr] = _b;
      var _0 = HEAP[_a_addr];
      var _1 = HEAP[_b_addr];
      var _2 = _strstr(_0, _1);
      HEAP[_s] = _2;
      var _3 = HEAP[_s];
      var _4 = 0+(_3 != 0);
      if (_4) { __label__ = 0; break; } else { __label__ = 1; break; }
    case 0: // _bb
      var _5 = HEAP[_s];
      var _6 = _5;
      var _7 = HEAP[_a_addr];
      var _8 = _7;
      _iftmp_197 = _6 - _8;
      __label__ = 3; break;
    case 1: // _bb1
      _iftmp_197 = -1;
      __label__ = 3; break;
    case 3: // _bb2
      __Z6intreti(_iftmp_197);
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __Z10strreplacePKcS0_S0_(_s, _oldval, _newval) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _s_addr = Pointer_make([0], 0);
      var _oldval_addr = Pointer_make([0], 0);
      var _newval_addr = Pointer_make([0], 0);
      var _retval = Pointer_make([0], 0);
      var _save_filt_338;
      var _save_eptr_337 = Pointer_make([0], 0);
      var _0 = Pointer_make([0], 0);
      var _1 = Pointer_make([0], 0);
      var _buf = Pointer_make([0,0,0], 0);
      var _oldlen;
      var _found = Pointer_make([0], 0);
      var _n = Pointer_make([0], 0);
      var _eh_exception = Pointer_make([0], 0);
      var _eh_selector;
      var __alloca_point_ = 0;
      HEAP[_s_addr] = _s;
      HEAP[_oldval_addr] = _oldval;
      HEAP[_newval_addr] = _newval;
      __ZN6vectorIcEC1Ev(_buf);
      var _2 = HEAP[_oldval_addr];
      _oldlen = _strlen(_2);
      var _5 = 0+(_oldlen == 0);
      if (_5) { __label__ = 0; break; } else { __label__ = 1; break; }
    case 0: // _bb
      var _6 = HEAP[_s_addr];
      var _7 = (function() { try { return __Z9newstringPKc(_6); __THREW__ = false } catch(e) { __THREW__ = true; } })(); if (!__THREW__) { __label__ = 137; break; } else { __label__ = 134; break; };
    case 137: // _invcont
      HEAP[_1] = _7;
      __label__ = 24; break;
    case 1: // _bb1
      var _8 = HEAP[_s_addr];
      var _9 = HEAP[_oldval_addr];
      var _10 = _strstr(_8, _9);
      HEAP[_found] = _10;
      var _11 = HEAP[_found];
      var _12 = 0+(_11 != 0);
      if (_12) { __label__ = 3; break; } else { __label__ = 13; break; }
    case 3: // _bb2
      __label__ = 4; break;
    case 7: // _bb3
      var _13 = HEAP[_s_addr];
      var _15 = _13+1;
      HEAP[_s_addr] = _15;
      var _16 = (function() { try { return __ZN6vectorIcE3addERKc(_buf, _13); __THREW__ = false } catch(e) { __THREW__ = true; } })(); if (!__THREW__) { __label__ = 143; break; } else { __label__ = 134; break; };
    case 143: // _invcont4
      __label__ = 4; break;
    case 4: // _bb5
      var _17 = HEAP[_s_addr];
      var _18 = HEAP[_found];
      var _19 = 0+(_17 < _18);
      if (_19) { __label__ = 7; break; } else { __label__ = 8; break; }
    case 8: // _bb6
      var _20 = HEAP[_newval_addr];
      HEAP[_n] = _20;
      __label__ = 9; break;
    case 5: // _bb7
      var _21 = HEAP[_n];
      var _22 = (function() { try { return __ZN6vectorIcE3addERKc(_buf, _21); __THREW__ = false } catch(e) { __THREW__ = true; } })(); if (!__THREW__) { __label__ = 144; break; } else { __label__ = 134; break; };
    case 144: // _invcont8
      var _23 = HEAP[_n];
      var _24 = _23+1;
      HEAP[_n] = _24;
      __label__ = 9; break;
    case 9: // _bb9
      var _25 = HEAP[_n];
      var _26 = HEAP[_25];
      var _27 = 0+(_26 != 0);
      if (_27) { __label__ = 5; break; } else { __label__ = 11; break; }
    case 11: // _bb10
      var _28 = HEAP[_found];
      var _30 = _28+_oldlen;
      HEAP[_s_addr] = _30;
      __label__ = 1; break;
    case 13: // _bb11
      __label__ = 12; break;
    case 14: // _bb12
      var _31 = HEAP[_s_addr];
      var _33 = _31+1;
      HEAP[_s_addr] = _33;
      var _34 = (function() { try { return __ZN6vectorIcE3addERKc(_buf, _31); __THREW__ = false } catch(e) { __THREW__ = true; } })(); if (!__THREW__) { __label__ = 145; break; } else { __label__ = 134; break; };
    case 145: // _invcont13
      __label__ = 12; break;
    case 12: // _bb14
      var _35 = HEAP[_s_addr];
      var _36 = HEAP[_35];
      var _37 = 0+(_36 != 0);
      if (_37) { __label__ = 14; break; } else { __label__ = 21; break; }
    case 21: // _bb15
      HEAP[_0] = 0;
      var _38 = (function() { try { return __ZN6vectorIcE3addERKc(_buf, _0); __THREW__ = false } catch(e) { __THREW__ = true; } })(); if (!__THREW__) { __label__ = 146; break; } else { __label__ = 134; break; };
    case 146: // _invcont16
      var _39 = __ZNK6vectorIcE6lengthEv(_buf);
      var _40 = __ZN6vectorIcE6getbufEv(_buf);
      var _41 = (function() { try { return __Z9newstringPKcj(_40, _39); __THREW__ = false } catch(e) { __THREW__ = true; } })(); if (!__THREW__) { __label__ = 147; break; } else { __label__ = 134; break; };
    case 147: // _invcont17
      HEAP[_1] = _41;
      __label__ = 24; break;
    case 17: // _bb18
      _save_filt_338 = _eh_selector;
      var _eh_value = HEAP[_eh_exception];
      HEAP[_save_eptr_337] = _eh_value;
      (function() { try { return __ZN6vectorIcED1Ev(_buf); __THREW__ = false } catch(e) { __THREW__ = true; } })(); if (!__THREW__) { __label__ = 131; break; } else { __label__ = 132; break; }
    case 131: // _invcont19
      var _42 = HEAP[_save_eptr_337];
      HEAP[_eh_exception] = _42;
      _eh_selector = _save_filt_338;
      __label__ = 26; break;
    case 19: // _bb20
      __ZSt9terminatev();
      // unreachable
    case 24: // _bb21
      __ZN6vectorIcED1Ev(_buf);
      var _44 = HEAP[_1];
      HEAP[_retval] = _44;
      __label__ = 2; break;
    case 2: // _return
      var _retval22 = HEAP[_retval];
      return _retval22;
    case 134: // _lpad
      var _eh_ptr = _llvm_eh_exception();
      HEAP[_eh_exception] = _eh_ptr;
      var _eh_ptr23 = HEAP[_eh_exception];
      _eh_selector = _llvm_eh_selector(_eh_ptr23, ___gxx_personality_v0, 0);
      __label__ = 27; break;
    case 132: // _lpad25
      var _eh_ptr26 = _llvm_eh_exception();
      HEAP[_eh_exception] = _eh_ptr26;
      var _eh_ptr27 = HEAP[_eh_exception];
      _eh_selector = _llvm_eh_selector(_eh_ptr27, ___gxx_personality_v0, 1);
      __label__ = 28; break;
    case 27: // _ppad
      __label__ = 17; break;
    case 28: // _ppad29
      __label__ = 19; break;
    case 26: // _Unwind
      var _eh_ptr30 = HEAP[_eh_exception];
      __Unwind_Resume_or_Rethrow(_eh_ptr30);
      // unreachable
  }
}


function __ZN7_stdcmdILi1223EE3runEPcS1_S1_(_s, _o, _n) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _s_addr = Pointer_make([0], 0);
      var _o_addr = Pointer_make([0], 0);
      var _n_addr = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_s_addr] = _s;
      HEAP[_o_addr] = _o;
      HEAP[_n_addr] = _n;
      var _0 = HEAP[_s_addr];
      var _1 = HEAP[_o_addr];
      var _2 = HEAP[_n_addr];
      var _3 = __Z10strreplacePKcS0_S0_(_0, _1, _2);
      HEAP[_commandret] = _3;
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __Z6formatPPcPi(_args, _numargs) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _args_addr = Pointer_make([0], 0);
      var _numargs_addr = Pointer_make([0], 0);
      var _save_filt_344;
      var _save_eptr_343 = Pointer_make([0], 0);
      var _0 = Pointer_make([0], 0);
      var _1 = Pointer_make([0], 0);
      var _2 = Pointer_make([0], 0);
      var _iftmp_158 = Pointer_make([0], 0);
      var _s = Pointer_make([0,0,0], 0);
      var _f = Pointer_make([0], 0);
      var _c;
      var _i;
      var _sub = Pointer_make([0], 0);
      var _eh_exception = Pointer_make([0], 0);
      var _eh_selector;
      var __alloca_point_ = 0;
      HEAP[_args_addr] = _args;
      HEAP[_numargs_addr] = _numargs;
      __ZN6vectorIcEC1Ev(_s);
      var _3 = HEAP[_args_addr];
      var _5 = HEAP[0 + _3];
      HEAP[_f] = _5;
      __label__ = 21; break;
    case 0: // _bb
      var _6 = HEAP[_f];
      var _7 = HEAP[_6];
      _c = _7;
      var _10 = _6+1;
      HEAP[_f] = _10;
      var _12 = 0+(_c == 37);
      if (_12) { __label__ = 1; break; } else { __label__ = 15; break; }
    case 1: // _bb1
      var _13 = HEAP[_f];
      var _14 = HEAP[_13];
      _i = _14;
      var _17 = _13+1;
      HEAP[_f] = _17;
      var _19 = 0+(_i <= 48);
      if (_19) { __label__ = 11; break; } else { __label__ = 3; break; }
    case 3: // _bb2
      var _21 = 0+(_i > 57);
      if (_21) { __label__ = 11; break; } else { __label__ = 7; break; }
    case 7: // _bb3
      _i = _i - 48;
      var _24 = HEAP[_numargs_addr];
      var _25 = HEAP[_24];
      var _27 = 0+(_25 > _i);
      if (_27) { __label__ = 6; break; } else { __label__ = 4; break; }
    case 6: // _bb4
      var _28 = HEAP[_args_addr];
      var _31 = HEAP[0 + _28+_i];
      HEAP[_iftmp_158] = _31;
      __label__ = 8; break;
    case 4: // _bb5
      HEAP[_iftmp_158] = __str3+0*1;
      __label__ = 8; break;
    case 8: // _bb6
      var _32 = HEAP[_iftmp_158];
      HEAP[_sub] = _32;
      __label__ = 10; break;
    case 5: // _bb7
      var _33 = HEAP[_sub];
      var _35 = _33+1;
      HEAP[_sub] = _35;
      var _36 = (function() { try { return __ZN6vectorIcE3addERKc(_s, _33); __THREW__ = false } catch(e) { __THREW__ = true; } })(); if (!__THREW__) { __label__ = 137; break; } else { __label__ = 134; break; };
    case 137: // _invcont
      __label__ = 10; break;
    case 10: // _bb8
      var _37 = HEAP[_sub];
      var _38 = HEAP[_37];
      var _39 = 0+(_38 != 0);
      if (_39) { __label__ = 5; break; } else { __label__ = 9; break; }
    case 9: // _bb9
      __label__ = 14; break;
    case 11: // _bb10
      var _41 = _i;
      HEAP[_2] = _41;
      var _42 = (function() { try { return __ZN6vectorIcE3addERKc(_s, _2); __THREW__ = false } catch(e) { __THREW__ = true; } })(); if (!__THREW__) { __label__ = 148; break; } else { __label__ = 134; break; };
    case 148: // _invcont11
      __label__ = 14; break;
    case 14: // _bb12
      __label__ = 21; break;
    case 15: // _bb13
      var _44 = _c;
      HEAP[_1] = _44;
      var _45 = (function() { try { return __ZN6vectorIcE3addERKc(_s, _1); __THREW__ = false } catch(e) { __THREW__ = true; } })(); if (!__THREW__) { __label__ = 149; break; } else { __label__ = 134; break; };
    case 149: // _invcont14
      __label__ = 21; break;
    case 21: // _bb15
      var _46 = HEAP[_f];
      var _47 = HEAP[_46];
      var _48 = 0+(_47 != 0);
      if (_48) { __label__ = 0; break; } else { __label__ = 16; break; }
    case 16: // _bb16
      HEAP[_0] = 0;
      var _49 = (function() { try { return __ZN6vectorIcE3addERKc(_s, _0); __THREW__ = false } catch(e) { __THREW__ = true; } })(); if (!__THREW__) { __label__ = 147; break; } else { __label__ = 134; break; };
    case 147: // _invcont17
      var _50 = __ZN6vectorIcE6getbufEv(_s);
      (function() { try { return __Z6resultPKc(_50); __THREW__ = false } catch(e) { __THREW__ = true; } })(); if (!__THREW__) { __label__ = 133; break; } else { __label__ = 134; break; }
    case 133: // _invcont18
      __ZN6vectorIcED1Ev(_s);
      __label__ = 2; break;
    case 18: // _bb19
      _save_filt_344 = _eh_selector;
      var _eh_value = HEAP[_eh_exception];
      HEAP[_save_eptr_343] = _eh_value;
      (function() { try { return __ZN6vectorIcED1Ev(_s); __THREW__ = false } catch(e) { __THREW__ = true; } })(); if (!__THREW__) { __label__ = 135; break; } else { __label__ = 136; break; }
    case 135: // _invcont20
      var _51 = HEAP[_save_eptr_343];
      HEAP[_eh_exception] = _51;
      _eh_selector = _save_filt_344;
      __label__ = 26; break;
    case 24: // _bb21
      __ZSt9terminatev();
      // unreachable
    case 2: // _return
      return;
    case 134: // _lpad
      var _eh_ptr = _llvm_eh_exception();
      HEAP[_eh_exception] = _eh_ptr;
      var _eh_ptr22 = HEAP[_eh_exception];
      _eh_selector = _llvm_eh_selector(_eh_ptr22, ___gxx_personality_v0, 0);
      __label__ = 27; break;
    case 136: // _lpad24
      var _eh_ptr25 = _llvm_eh_exception();
      HEAP[_eh_exception] = _eh_ptr25;
      var _eh_ptr26 = HEAP[_eh_exception];
      _eh_selector = _llvm_eh_selector(_eh_ptr26, ___gxx_personality_v0, 1);
      __label__ = 29; break;
    case 27: // _ppad
      __label__ = 18; break;
    case 29: // _ppad28
      __label__ = 24; break;
    case 26: // _Unwind
      var _eh_ptr29 = HEAP[_eh_exception];
      __Unwind_Resume_or_Rethrow(_eh_ptr29);
      // unreachable
  }
}


function __ZN7_stdcmdILi1193EE3runEPcS1_(_a, _b) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _a_addr = Pointer_make([0], 0);
      var _b_addr = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_a_addr] = _a;
      HEAP[_b_addr] = _b;
      var _0 = HEAP[_a_addr];
      var _1 = HEAP[_b_addr];
      var _2 = _strcmp(_0, _1);
      var _3 = 0+(_2 >= 0);
      var _4 = _3;
      __Z6intreti(_4);
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __ZN7_stdcmdILi1192EE3runEPcS1_(_a, _b) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _a_addr = Pointer_make([0], 0);
      var _b_addr = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_a_addr] = _a;
      HEAP[_b_addr] = _b;
      var _0 = HEAP[_a_addr];
      var _1 = HEAP[_b_addr];
      var _2 = _strcmp(_0, _1);
      var _3 = 0+(_2 <= 0);
      var _4 = _3;
      __Z6intreti(_4);
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __ZN7_stdcmdILi1191EE3runEPcS1_(_a, _b) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _a_addr = Pointer_make([0], 0);
      var _b_addr = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_a_addr] = _a;
      HEAP[_b_addr] = _b;
      var _0 = HEAP[_a_addr];
      var _1 = HEAP[_b_addr];
      var _2 = _strcmp(_0, _1);
      var _3 = 0+(_2 > 0);
      var _4 = _3;
      __Z6intreti(_4);
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __ZN7_stdcmdILi1190EE3runEPcS1_(_a, _b) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _a_addr = Pointer_make([0], 0);
      var _b_addr = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_a_addr] = _a;
      HEAP[_b_addr] = _b;
      var _0 = HEAP[_a_addr];
      var _1 = HEAP[_b_addr];
      var _2 = _strcmp(_0, _1);
      var _3 = 0+(_2 < 0);
      var _4 = _3;
      __Z6intreti(_4);
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __ZN7_stdcmdILi1189EE3runEPcS1_(_a, _b) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _a_addr = Pointer_make([0], 0);
      var _b_addr = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_a_addr] = _a;
      HEAP[_b_addr] = _b;
      var _0 = HEAP[_a_addr];
      var _1 = HEAP[_b_addr];
      var _2 = _strcmp(_0, _1);
      var _3 = 0+(_2 != 0);
      var _4 = _3;
      __Z6intreti(_4);
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __ZN7_stdcmdILi1188EE3runEPcS1_(_a, _b) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _a_addr = Pointer_make([0], 0);
      var _b_addr = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_a_addr] = _a;
      HEAP[_b_addr] = _b;
      var _0 = HEAP[_a_addr];
      var _1 = HEAP[_b_addr];
      var _2 = _strcmp(_0, _1);
      var _3 = 0+(_2 == 0);
      var _4 = _3;
      __Z6intreti(_4);
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __ZN7_stdcmdILi1187EE3runEPcS1_(_a, _b) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _a_addr = Pointer_make([0], 0);
      var _b_addr = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_a_addr] = _a;
      HEAP[_b_addr] = _b;
      var _0 = HEAP[_a_addr];
      var _1 = HEAP[_b_addr];
      var _2 = _strcmp(_0, _1);
      var _3 = 0+(_2 == 0);
      var _4 = _3;
      __Z6intreti(_4);
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __ZL5htcmpPKcS0_(_x, _y) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _x_addr = Pointer_make([0], 0);
      var _y_addr = Pointer_make([0], 0);
      var _retval;
      var _0;
      var __alloca_point_ = 0;
      HEAP[_x_addr] = _x;
      HEAP[_y_addr] = _y;
      var _1 = HEAP[_x_addr];
      var _2 = HEAP[_y_addr];
      var _3 = _strcmp(_1, _2);
      var _4 = 0+(_3 == 0);
      _0 = _4;
      _retval = _0;
      __label__ = 2; break;
    case 2: // _return
      var _retval12 = _retval;
      return _retval12;
  }
}


function __Z5htcmpIcPKc5identEbPKT_RK14hashtableentryIT0_T1_E(_x, _y) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _x_addr = Pointer_make([0], 0);
      var _y_addr = Pointer_make([0], 0);
      var _retval;
      var _0;
      var __alloca_point_ = 0;
      HEAP[_x_addr] = _x;
      HEAP[_y_addr] = _y;
      var _1 = HEAP[_y_addr];
      var _3 = HEAP[0 + _1+__struct_hashtableentry_const_char__ident_____FLATTENER[0]];
      var _4 = HEAP[_x_addr];
      var _5 = __ZL5htcmpPKcS0_(_4, _3);
      _0 = _5;
      _retval = _0;
      __label__ = 2; break;
    case 2: // _return
      var _retval12 = _retval;
      return _retval12;
  }
}


function __ZN9hashtableIPKc5identE6accessERKS1_(_this, _key) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _this_addr = Pointer_make([0], 0);
      var _key_addr = Pointer_make([0], 0);
      var _retval = Pointer_make([0], 0);
      var _0 = Pointer_make([0], 0);
      var _retval_227;
      var _h;
      var _c = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_this_addr] = _this;
      HEAP[_key_addr] = _key;
      var _1 = HEAP[_key_addr];
      var _2 = HEAP[_1];
      var _3 = __ZL6hthashPKc(_2);
      var _4 = HEAP[_this_addr];
      var _5 = _4+0*5;
      var _7 = HEAP[0 + _5+0*1];
      var _8 = _7 - 1;
      _h = _3 & _8;
      var _11 = _4+0*5;
      var _13 = HEAP[0 + _11+2*1];
      var _16 = HEAP[0 + _13+_h];
      HEAP[_c] = _16;
      __label__ = 7; break;
    case 0: // _bb
      var _17 = HEAP[_c];
      var _18 = _17+__struct_hashset_hashtableentry_const_char___ident_____chain____FLATTENER[0];
      var _19 = HEAP[_key_addr];
      var _20 = HEAP[_19];
      _retval_227 = __Z5htcmpIcPKc5identEbPKT_RK14hashtableentryIT0_T1_E(_20, _18);
      var _toBool = 0+(_retval_227 != 0);
      if (_toBool) { __label__ = 1; break; } else { __label__ = 3; break; }
    case 1: // _bb1
      var _23 = HEAP[_c];
      var _24 = _23+__struct_hashset_hashtableentry_const_char___ident_____chain____FLATTENER[0];
      var _25 = _24+__struct_hashtableentry_const_char__ident_____FLATTENER[1];
      HEAP[_0] = _25;
      __label__ = 4; break;
    case 3: // _bb2
      var _26 = HEAP[_c];
      var _28 = HEAP[0 + _26+__struct_hashset_hashtableentry_const_char___ident_____chain____FLATTENER[1]];
      HEAP[_c] = _28;
      __label__ = 7; break;
    case 7: // _bb3
      var _29 = HEAP[_c];
      var _30 = 0+(_29 != 0);
      if (_30) { __label__ = 0; break; } else { __label__ = 6; break; }
    case 6: // _bb4
      HEAP[_0] = 0;
      __label__ = 4; break;
    case 4: // _bb5
      var _31 = HEAP[_0];
      HEAP[_retval] = _31;
      __label__ = 2; break;
    case 2: // _return
      var _retval6 = HEAP[_retval];
      return _retval6;
  }
}


function __Z6lookupPc(_n) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _n_addr = Pointer_make([0], 0);
      var _retval = Pointer_make([0], 0);
      var _0 = Pointer_make([0], 0);
      var _1 = Pointer_make([0], 0);
      var _id = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_n_addr] = _n;
      var _2 = HEAP[_n_addr];
      var _3 = _2+1;
      HEAP[_1] = _3;
      var _4 = HEAP[_idents];
      var _5 = __ZN9hashtableIPKc5identE6accessERKS1_(_4, _1);
      HEAP[_id] = _5;
      var _6 = HEAP[_id];
      var _7 = 0+(_6 != 0);
      if (_7) { __label__ = 0; break; } else { __label__ = 4; break; }
    case 0: // _bb
      var _8 = HEAP[_id];
      var _10 = HEAP[0 + _8+1*1];
      if (_10 == 0) {
  __label__ = 1; break;
}
else if (_10 == 1) {
  __label__ = 3; break;
}
else if (_10 == 2) {
  __label__ = 7; break;
}
else if (_10 == 5) {
  __label__ = 6; break;
}
else {
__label__ = 4; break;
}

    case 1: // _bb1
      var _11 = HEAP[_id];
      var _12 = _11+9*1;
      var _14 = HEAP[0 + _12];
      var _15 = HEAP[_14];
      var _16 = __Z6intstri(_15);
      var _17 = HEAP[_n_addr];
      var _18 = __Z11exchangestrPcPKc(_17, _16);
      HEAP[_0] = _18;
      __label__ = 8; break;
    case 3: // _bb2
      var _19 = HEAP[_id];
      var _20 = _19+9*1;
      var _21 = _20;
      var _23 = HEAP[_21];
      var _24 = HEAP[_23];
      var _25 = __Z8floatstrf(_24);
      var _26 = HEAP[_n_addr];
      var _27 = __Z11exchangestrPcPKc(_26, _25);
      HEAP[_0] = _27;
      __label__ = 8; break;
    case 7: // _bb3
      var _28 = HEAP[_id];
      var _29 = _28+9*1;
      var _30 = _29;
      var _32 = HEAP[_30];
      var _33 = HEAP[_32];
      var _34 = HEAP[_n_addr];
      var _35 = __Z11exchangestrPcPKc(_34, _33);
      HEAP[_0] = _35;
      __label__ = 8; break;
    case 6: // _bb4
      var _36 = HEAP[_id];
      var _37 = _36+7*1;
      var _39 = HEAP[0 + _37+0*1];
      var _40 = HEAP[_n_addr];
      var _41 = __Z11exchangestrPcPKc(_40, _39);
      HEAP[_0] = _41;
      __label__ = 8; break;
    case 4: // _bb5
      var _42 = HEAP[_n_addr];
      var _43 = _42+1;
      __Z7conoutfiPKcz(4, __str11+0*1, _43);
      var _44 = HEAP[_n_addr];
      HEAP[_0] = _44;
      __label__ = 8; break;
    case 8: // _bb6
      var _45 = HEAP[_0];
      HEAP[_retval] = _45;
      __label__ = 2; break;
    case 2: // _return
      var _retval7 = HEAP[_retval];
      return _retval7;
  }
}


function __Z8getaliasPKc(_name) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _name_addr = Pointer_make([0], 0);
      var _retval = Pointer_make([0], 0);
      var _iftmp_71 = Pointer_make([0], 0);
      var _0 = Pointer_make([0], 0);
      var _i = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_name_addr] = _name;
      var _1 = HEAP[_idents];
      var _2 = __ZN9hashtableIPKc5identE6accessERKS1_(_1, _name_addr);
      HEAP[_i] = _2;
      var _3 = HEAP[_i];
      var _4 = 0+(_3 == 0);
      if (_4) { __label__ = 3; break; } else { __label__ = 0; break; }
    case 0: // _bb
      var _5 = HEAP[_i];
      var _7 = HEAP[0 + _5+1*1];
      var _8 = 0+(_7 != 5);
      if (_8) { __label__ = 3; break; } else { __label__ = 1; break; }
    case 1: // _bb1
      var _9 = HEAP[_i];
      var _10 = _9+7*1;
      var _12 = HEAP[0 + _10+0*1];
      HEAP[_iftmp_71] = _12;
      __label__ = 7; break;
    case 3: // _bb2
      HEAP[_iftmp_71] = __str3+0*1;
      __label__ = 7; break;
    case 7: // _bb3
      var _13 = HEAP[_iftmp_71];
      HEAP[_0] = _13;
      var _14 = HEAP[_0];
      HEAP[_retval] = _14;
      __label__ = 2; break;
    case 2: // _return
      var _retval4 = HEAP[_retval];
      return _retval4;
  }
}


function __Z9getalias_Pc(_s) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _s_addr = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_s_addr] = _s;
      var _0 = HEAP[_s_addr];
      var _1 = __Z8getaliasPKc(_0);
      __Z6resultPKc(_1);
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __Z8touchvarPKc(_name) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _name_addr = Pointer_make([0], 0);
      var _id = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_name_addr] = _name;
      var _0 = HEAP[_idents];
      var _1 = __ZN9hashtableIPKc5identE6accessERKS1_(_0, _name_addr);
      HEAP[_id] = _1;
      var _2 = HEAP[_id];
      var _3 = 0+(_2 != 0);
      if (_3) { __label__ = 0; break; } else { __label__ = 3; break; }
    case 0: // _bb
      var _4 = HEAP[_id];
      var _6 = HEAP[0 + _4+1*1];
      if (_6 == 0) {
  __label__ = 1; break;
}
else if (_6 == 1) {
  __label__ = 1; break;
}
else if (_6 == 2) {
  __label__ = 1; break;
}
else {
__label__ = 3; break;
}

    case 1: // _bb1
      var _7 = HEAP[_id];
      var _9 = HEAP[0 + _7+0*1];
      var _11 = HEAP[0 + _9+2];
      var _12 = _11;
      _12(_7);
      __label__ = 3; break;
    case 3: // _bb2
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __Z8getidentPKc(_name) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _name_addr = Pointer_make([0], 0);
      var _retval = Pointer_make([0], 0);
      var _0 = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_name_addr] = _name;
      var _1 = HEAP[_idents];
      var _2 = __ZN9hashtableIPKc5identE6accessERKS1_(_1, _name_addr);
      HEAP[_0] = _2;
      var _3 = HEAP[_0];
      HEAP[_retval] = _3;
      __label__ = 2; break;
    case 2: // _return
      var _retval1 = HEAP[_retval];
      return _retval1;
  }
}


function __Z11identexistsPKc(_name) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _name_addr = Pointer_make([0], 0);
      var _retval;
      var _0;
      var __alloca_point_ = 0;
      HEAP[_name_addr] = _name;
      var _1 = HEAP[_idents];
      var _2 = __ZN9hashtableIPKc5identE6accessERKS1_(_1, _name_addr);
      var _3 = 0+(_2 != 0);
      _0 = _3;
      _retval = _0;
      __label__ = 2; break;
    case 2: // _return
      var _retval12 = _retval;
      return _retval12;
  }
}


function __Z9getvarmaxPKc(_name) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _name_addr = Pointer_make([0], 0);
      var _retval;
      var _0;
      var _id = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_name_addr] = _name;
      var _1 = HEAP[_idents];
      var _2 = __ZN9hashtableIPKc5identE6accessERKS1_(_1, _name_addr);
      HEAP[_id] = _2;
      var _3 = HEAP[_id];
      var _4 = 0+(_3 == 0);
      if (_4) { __label__ = 1; break; } else { __label__ = 0; break; }
    case 0: // _bb
      var _5 = HEAP[_id];
      var _7 = HEAP[0 + _5+1*1];
      var _8 = 0+(_7 != 0);
      if (_8) { __label__ = 1; break; } else { __label__ = 3; break; }
    case 1: // _bb1
      _0 = 0;
      __label__ = 7; break;
    case 3: // _bb2
      var _9 = HEAP[_id];
      var _10 = _9+4*1;
      var _0 = HEAP[0 + _10];
      __label__ = 7; break;
    case 7: // _bb3
      _retval = _0;
      __label__ = 2; break;
    case 2: // _return
      var _retval4 = _retval;
      return _retval4;
  }
}


function __Z9getvarminPKc(_name) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _name_addr = Pointer_make([0], 0);
      var _retval;
      var _0;
      var _id = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_name_addr] = _name;
      var _1 = HEAP[_idents];
      var _2 = __ZN9hashtableIPKc5identE6accessERKS1_(_1, _name_addr);
      HEAP[_id] = _2;
      var _3 = HEAP[_id];
      var _4 = 0+(_3 == 0);
      if (_4) { __label__ = 1; break; } else { __label__ = 0; break; }
    case 0: // _bb
      var _5 = HEAP[_id];
      var _7 = HEAP[0 + _5+1*1];
      var _8 = 0+(_7 != 0);
      if (_8) { __label__ = 1; break; } else { __label__ = 3; break; }
    case 1: // _bb1
      _0 = 0;
      __label__ = 7; break;
    case 3: // _bb2
      var _9 = HEAP[_id];
      var _10 = _9+3*1;
      var _0 = HEAP[0 + _10];
      __label__ = 7; break;
    case 7: // _bb3
      _retval = _0;
      __label__ = 2; break;
    case 2: // _return
      var _retval4 = _retval;
      return _retval4;
  }
}


function __Z6getvarPKc(_name) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _name_addr = Pointer_make([0], 0);
      var _retval;
      var _0;
      var _id = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_name_addr] = _name;
      var _1 = HEAP[_idents];
      var _2 = __ZN9hashtableIPKc5identE6accessERKS1_(_1, _name_addr);
      HEAP[_id] = _2;
      var _3 = HEAP[_id];
      var _4 = 0+(_3 == 0);
      if (_4) { __label__ = 1; break; } else { __label__ = 0; break; }
    case 0: // _bb
      var _5 = HEAP[_id];
      var _7 = HEAP[0 + _5+1*1];
      var _8 = 0+(_7 != 0);
      if (_8) { __label__ = 1; break; } else { __label__ = 3; break; }
    case 1: // _bb1
      _0 = 0;
      __label__ = 7; break;
    case 3: // _bb2
      var _9 = HEAP[_id];
      var _10 = _9+9*1;
      var _12 = HEAP[0 + _10];
      _0 = HEAP[_12];
      __label__ = 7; break;
    case 7: // _bb3
      _retval = _0;
      __label__ = 2; break;
    case 2: // _return
      var _retval4 = _retval;
      return _retval4;
  }
}


function __Z7setsvarPKcS0_b(_name, _str, _dofunc) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _name_addr = Pointer_make([0], 0);
      var _str_addr = Pointer_make([0], 0);
      var _dofunc_addr;
      var _id = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_name_addr] = _name;
      HEAP[_str_addr] = _str;
      _dofunc_addr = _dofunc;
      var _0 = HEAP[_idents];
      var _1 = __ZN9hashtableIPKc5identE6accessERKS1_(_0, _name_addr);
      HEAP[_id] = _1;
      var _2 = HEAP[_id];
      var _3 = 0+(_2 == 0);
      if (_3) { __label__ = 1; break; } else { __label__ = 0; break; }
    case 0: // _bb
      var _4 = HEAP[_id];
      var _6 = HEAP[0 + _4+1*1];
      var _7 = 0+(_6 != 2);
      if (_7) { __label__ = 1; break; } else { __label__ = 3; break; }
    case 1: // _bb1
      __label__ = 24; break;
    case 3: // _bb2
      var _8 = HEAP[_overrideidents];
      var _toBool = 0+(_8 != 0);
      if (_toBool) { __label__ = 6; break; } else { __label__ = 7; break; }
    case 7: // _bb3
      var _9 = HEAP[_id];
      var _11 = HEAP[0 + _9+10*1];
      var _12 = _11 & 2;
      var _13 = 0+(_12 != 0);
      if (_13) { __label__ = 6; break; } else { __label__ = 14; break; }
    case 6: // _bb4
      var _14 = HEAP[_id];
      var _16 = HEAP[0 + _14+10*1];
      var _17 = _16 & 1;
      var _18 = _17;
      var _toBool5 = 0+(_18 != 0);
      if (_toBool5) { __label__ = 8; break; } else { __label__ = 5; break; }
    case 8: // _bb6
      var _19 = HEAP[_id];
      var _21 = HEAP[0 + _19+2*1];
      __Z7conoutfiPKcz(4, __str5+0*1, _21);
      __label__ = 24; break;
    case 5: // _bb7
      var _22 = HEAP[_id];
      var _24 = HEAP[0 + _22+5*1];
      var _25 = 0+(_24 == 2147483647);
      if (_25) { __label__ = 10; break; } else { __label__ = 9; break; }
    case 10: // _bb8
      var _26 = HEAP[_id];
      var _27 = _26+9*1;
      var _28 = _27;
      var _30 = HEAP[_28];
      var _31 = HEAP[_30];
      var _33 = _26+8*1;
      var _34 = _33+0*1;
      var _36 = _34;
      HEAP[_36] = _31;
      var _38 = HEAP[_id];
      HEAP[0 + _38+5*1] = 0;
      __label__ = 13; break;
    case 9: // _bb9
      var _40 = HEAP[_id];
      var _41 = _40+9*1;
      var _42 = _41;
      var _44 = HEAP[_42];
      var _45 = HEAP[_44];
      var _46 = 0+(_45 != 0);
      if (_46) { __label__ = 11; break; } else { __label__ = 13; break; }
    case 11: // _bb10
      var _47 = HEAP[_id];
      var _48 = _47+9*1;
      var _49 = _48;
      var _51 = HEAP[_49];
      var _52 = HEAP[_51];
      __ZdaPv(_52);
      __label__ = 13; break;
    case 13: // _bb11
      __label__ = 17; break;
    case 14: // _bb12
      var _53 = HEAP[_id];
      var _55 = HEAP[0 + _53+5*1];
      var _56 = 0+(_55 != 2147483647);
      if (_56) { __label__ = 15; break; } else { __label__ = 16; break; }
    case 15: // _bb13
      var _57 = HEAP[_id];
      var _58 = _57+8*1;
      var _59 = _58+0*1;
      var _61 = _59;
      var _63 = HEAP[_61];
      var _64 = 0+(_63 != 0);
      if (_64) { __label__ = 12; break; } else { __label__ = 21; break; }
    case 12: // _bb14
      var _65 = HEAP[_id];
      var _66 = _65+8*1;
      var _67 = _66+0*1;
      var _69 = _67;
      var _71 = HEAP[_69];
      __ZdaPv(_71);
      __label__ = 21; break;
    case 21: // _bb15
      var _72 = HEAP[_id];
      HEAP[0 + _72+5*1] = 2147483647;
      __label__ = 16; break;
    case 16: // _bb16
      var _74 = HEAP[_id];
      var _75 = _74+9*1;
      var _76 = _75;
      var _78 = HEAP[_76];
      var _79 = HEAP[_78];
      var _80 = 0+(_79 != 0);
      if (_80) { __label__ = 20; break; } else { __label__ = 17; break; }
    case 20: // _bb17
      var _81 = HEAP[_id];
      var _82 = _81+9*1;
      var _83 = _82;
      var _85 = HEAP[_83];
      var _86 = HEAP[_85];
      __ZdaPv(_86);
      __label__ = 17; break;
    case 17: // _bb18
      var _87 = HEAP[_id];
      var _88 = _87+9*1;
      var _89 = _88;
      var _91 = HEAP[_89];
      var _92 = HEAP[_str_addr];
      var _93 = __Z9newstringPKc(_92);
      HEAP[_91] = _93;
      var _toBool19 = 0+(_dofunc_addr != 0);
      if (_toBool19) { __label__ = 19; break; } else { __label__ = 24; break; }
    case 19: // _bb20
      var _95 = HEAP[_id];
      var _97 = HEAP[0 + _95+0*1];
      var _99 = HEAP[0 + _97+2];
      var _100 = _99;
      _100(_95);
      __label__ = 24; break;
    case 24: // _bb21
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __Z7setfvarPKcfbb(_name, _f, _dofunc, _doclamp) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _name_addr = Pointer_make([0], 0);
      var _f_addr;
      var _dofunc_addr;
      var _doclamp_addr;
      var _id = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_name_addr] = _name;
      _f_addr = _f;
      _dofunc_addr = _dofunc;
      _doclamp_addr = _doclamp;
      var _0 = HEAP[_idents];
      var _1 = __ZN9hashtableIPKc5identE6accessERKS1_(_0, _name_addr);
      HEAP[_id] = _1;
      var _2 = HEAP[_id];
      var _3 = 0+(_2 == 0);
      if (_3) { __label__ = 1; break; } else { __label__ = 0; break; }
    case 0: // _bb
      var _4 = HEAP[_id];
      var _6 = HEAP[0 + _4+1*1];
      var _7 = 0+(_6 != 1);
      if (_7) { __label__ = 1; break; } else { __label__ = 3; break; }
    case 1: // _bb1
      __label__ = 18; break;
    case 3: // _bb2
      var _8 = HEAP[_overrideidents];
      var _toBool = 0+(_8 != 0);
      if (_toBool) { __label__ = 6; break; } else { __label__ = 7; break; }
    case 7: // _bb3
      var _9 = HEAP[_id];
      var _11 = HEAP[0 + _9+10*1];
      var _12 = _11 & 2;
      var _13 = 0+(_12 != 0);
      if (_13) { __label__ = 6; break; } else { __label__ = 11; break; }
    case 6: // _bb4
      var _14 = HEAP[_id];
      var _16 = HEAP[0 + _14+10*1];
      var _17 = _16 & 1;
      var _18 = _17;
      var _toBool5 = 0+(_18 != 0);
      if (_toBool5) { __label__ = 8; break; } else { __label__ = 5; break; }
    case 8: // _bb6
      var _19 = HEAP[_id];
      var _21 = HEAP[0 + _19+2*1];
      __Z7conoutfiPKcz(4, __str5+0*1, _21);
      __label__ = 18; break;
    case 5: // _bb7
      var _22 = HEAP[_id];
      var _24 = HEAP[0 + _22+5*1];
      var _25 = 0+(_24 == 2147483647);
      if (_25) { __label__ = 10; break; } else { __label__ = 9; break; }
    case 10: // _bb8
      var _26 = HEAP[_id];
      var _27 = _26+9*1;
      var _28 = _27;
      var _30 = HEAP[_28];
      var _31 = HEAP[_30];
      var _33 = _26+8*1;
      var _34 = _33+0*1;
      var _36 = _34;
      HEAP[_36] = _31;
      var _38 = HEAP[_id];
      HEAP[0 + _38+5*1] = 0;
      __label__ = 9; break;
    case 9: // _bb9
      __label__ = 14; break;
    case 11: // _bb10
      var _40 = HEAP[_id];
      var _42 = HEAP[0 + _40+5*1];
      var _43 = 0+(_42 != 2147483647);
      if (_43) { __label__ = 13; break; } else { __label__ = 14; break; }
    case 13: // _bb11
      var _44 = HEAP[_id];
      HEAP[0 + _44+5*1] = 2147483647;
      __label__ = 14; break;
    case 14: // _bb12
      var _toBool13 = 0+(_doclamp_addr != 0);
      if (_toBool13) { __label__ = 12; break; } else { __label__ = 21; break; }
    case 12: // _bb14
      var _47 = HEAP[_id];
      var _48 = _47+9*1;
      var _49 = _48;
      var _51 = HEAP[_49];
      var _53 = _47+4*1;
      var _54 = _53;
      var _56 = HEAP[_54];
      var _58 = __Z3minIfET_S0_S0_(_f_addr, _56);
      var _59 = HEAP[_id];
      var _60 = _59+3*1;
      var _61 = _60;
      var _63 = HEAP[_61];
      var _64 = __Z3maxIfET_S0_S0_(_63, _58);
      HEAP[_51] = _64;
      __label__ = 16; break;
    case 21: // _bb15
      var _65 = HEAP[_id];
      var _66 = _65+9*1;
      var _67 = _66;
      var _69 = HEAP[_67];
      HEAP[_69] = _f_addr;
      __label__ = 16; break;
    case 16: // _bb16
      var _toBool17 = 0+(_dofunc_addr != 0);
      if (_toBool17) { __label__ = 17; break; } else { __label__ = 18; break; }
    case 17: // _bb18
      var _72 = HEAP[_id];
      var _74 = HEAP[0 + _72+0*1];
      var _76 = HEAP[0 + _74+2];
      var _77 = _76;
      _77(_72);
      __label__ = 18; break;
    case 18: // _bb19
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __Z6setvarPKcibb(_name, _i, _dofunc, _doclamp) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _name_addr = Pointer_make([0], 0);
      var _i_addr;
      var _dofunc_addr;
      var _doclamp_addr;
      var _id = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_name_addr] = _name;
      _i_addr = _i;
      _dofunc_addr = _dofunc;
      _doclamp_addr = _doclamp;
      var _0 = HEAP[_idents];
      var _1 = __ZN9hashtableIPKc5identE6accessERKS1_(_0, _name_addr);
      HEAP[_id] = _1;
      var _2 = HEAP[_id];
      var _3 = 0+(_2 == 0);
      if (_3) { __label__ = 1; break; } else { __label__ = 0; break; }
    case 0: // _bb
      var _4 = HEAP[_id];
      var _6 = HEAP[0 + _4+1*1];
      var _7 = 0+(_6 != 0);
      if (_7) { __label__ = 1; break; } else { __label__ = 3; break; }
    case 1: // _bb1
      __label__ = 18; break;
    case 3: // _bb2
      var _8 = HEAP[_overrideidents];
      var _toBool = 0+(_8 != 0);
      if (_toBool) { __label__ = 6; break; } else { __label__ = 7; break; }
    case 7: // _bb3
      var _9 = HEAP[_id];
      var _11 = HEAP[0 + _9+10*1];
      var _12 = _11 & 2;
      var _13 = 0+(_12 != 0);
      if (_13) { __label__ = 6; break; } else { __label__ = 11; break; }
    case 6: // _bb4
      var _14 = HEAP[_id];
      var _16 = HEAP[0 + _14+10*1];
      var _17 = _16 & 1;
      var _18 = _17;
      var _toBool5 = 0+(_18 != 0);
      if (_toBool5) { __label__ = 8; break; } else { __label__ = 5; break; }
    case 8: // _bb6
      var _19 = HEAP[_id];
      var _21 = HEAP[0 + _19+2*1];
      __Z7conoutfiPKcz(4, __str5+0*1, _21);
      __label__ = 18; break;
    case 5: // _bb7
      var _22 = HEAP[_id];
      var _24 = HEAP[0 + _22+5*1];
      var _25 = 0+(_24 == 2147483647);
      if (_25) { __label__ = 10; break; } else { __label__ = 9; break; }
    case 10: // _bb8
      var _26 = HEAP[_id];
      var _27 = _26+9*1;
      var _29 = HEAP[0 + _27];
      var _30 = HEAP[_29];
      var _32 = _26+8*1;
      var _33 = _32+0*1;
      HEAP[0 + _33] = _30;
      var _36 = HEAP[_id];
      HEAP[0 + _36+5*1] = 0;
      __label__ = 9; break;
    case 9: // _bb9
      __label__ = 14; break;
    case 11: // _bb10
      var _38 = HEAP[_id];
      var _40 = HEAP[0 + _38+5*1];
      var _41 = 0+(_40 != 2147483647);
      if (_41) { __label__ = 13; break; } else { __label__ = 14; break; }
    case 13: // _bb11
      var _42 = HEAP[_id];
      HEAP[0 + _42+5*1] = 2147483647;
      __label__ = 14; break;
    case 14: // _bb12
      var _toBool13 = 0+(_doclamp_addr != 0);
      if (_toBool13) { __label__ = 12; break; } else { __label__ = 21; break; }
    case 12: // _bb14
      var _45 = HEAP[_id];
      var _46 = _45+9*1;
      var _48 = HEAP[0 + _46];
      var _50 = _45+4*1;
      var _52 = HEAP[0 + _50];
      var _54 = __Z3minIiET_S0_S0_(_i_addr, _52);
      var _55 = HEAP[_id];
      var _56 = _55+3*1;
      var _58 = HEAP[0 + _56];
      var _59 = __Z3maxIiET_S0_S0_(_58, _54);
      HEAP[_48] = _59;
      __label__ = 16; break;
    case 21: // _bb15
      var _60 = HEAP[_id];
      var _61 = _60+9*1;
      var _63 = HEAP[0 + _61];
      HEAP[_63] = _i_addr;
      __label__ = 16; break;
    case 16: // _bb16
      var _toBool17 = 0+(_dofunc_addr != 0);
      if (_toBool17) { __label__ = 17; break; } else { __label__ = 18; break; }
    case 17: // _bb18
      var _66 = HEAP[_id];
      var _68 = HEAP[0 + _66+0*1];
      var _70 = HEAP[0 + _68+2];
      var _71 = _70;
      _71(_66);
      __label__ = 18; break;
    case 18: // _bb19
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __Z8resetvarPc(_name) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _name_addr = Pointer_make([0], 0);
      var _0 = Pointer_make([0], 0);
      var _id = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_name_addr] = _name;
      var _1 = HEAP[_name_addr];
      HEAP[_0] = _1;
      var _2 = HEAP[_idents];
      var _3 = __ZN9hashtableIPKc5identE6accessERKS1_(_2, _0);
      HEAP[_id] = _3;
      var _4 = HEAP[_id];
      var _5 = 0+(_4 == 0);
      if (_5) { __label__ = 7; break; } else { __label__ = 0; break; }
    case 0: // _bb
      var _6 = HEAP[_id];
      var _8 = HEAP[0 + _6+10*1];
      var _9 = _8 & 8;
      var _10 = 0+(_9 != 0);
      if (_10) { __label__ = 1; break; } else { __label__ = 3; break; }
    case 1: // _bb1
      var _11 = HEAP[_id];
      var _13 = HEAP[0 + _11+2*1];
      __Z7conoutfiPKcz(4, __str4+0*1, _13);
      __label__ = 7; break;
    case 3: // _bb2
      var _14 = HEAP[_id];
      __Z13clearoverrideR5ident(_14);
      __label__ = 7; break;
    case 7: // _bb3
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __ZL8parseintPKc(_s) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _s_addr = Pointer_make([0], 0);
      var _retval;
      var _0;
      var __alloca_point_ = 0;
      HEAP[_s_addr] = _s;
      var _1 = HEAP[_s_addr];
      _0 = _strtol(_1, 0, 0);
      _retval = _0;
      __label__ = 2; break;
    case 2: // _return
      var _retval1 = _retval;
      return _retval1;
  }
}


function __ZL10parsefloatPKc(_s) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _s_addr = Pointer_make([0], 0);
      var _retval;
      var _iftmp_22;
      var _0;
      var _end = Pointer_make([0], 0);
      var _val;
      var __alloca_point_ = 0;
      HEAP[_s_addr] = _s;
      var _1 = HEAP[_s_addr];
      _val = _strtod(_1, _end);
      var _4 = 0+(_val != 0.000000e+00);
      if (_4) { __label__ = 7; break; } else { __label__ = 0; break; }
    case 0: // _bb
      var _5 = HEAP[_end];
      var _6 = HEAP[_s_addr];
      var _7 = 0+(_5 == _6);
      if (_7) { __label__ = 7; break; } else { __label__ = 1; break; }
    case 1: // _bb1
      var _8 = HEAP[_end];
      var _9 = HEAP[_8];
      var _10 = 0+(_9 == 120);
      if (_10) { __label__ = 6; break; } else { __label__ = 3; break; }
    case 3: // _bb2
      var _11 = HEAP[_end];
      var _12 = HEAP[_11];
      var _13 = 0+(_12 != 88);
      if (_13) { __label__ = 7; break; } else { __label__ = 6; break; }
    case 7: // _bb3
      _iftmp_22 = _val;
      __label__ = 4; break;
    case 6: // _bb4
      var _16 = HEAP[_s_addr];
      var _17 = __ZL8parseintPKc(_16);
      _iftmp_22 = _17;
      __label__ = 4; break;
    case 4: // _bb5
      _0 = _iftmp_22;
      _retval = _0;
      __label__ = 2; break;
    case 2: // _return
      var _retval6 = _retval;
      return _retval6;
  }
}


function __ZN7_stdcmdILi1155EE3runEPPcPi(_args, _numargs) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _args_addr = Pointer_make([0], 0);
      var _numargs_addr = Pointer_make([0], 0);
      var _iftmp_192;
      var _val;
      var _i;
      var __alloca_point_ = 0;
      HEAP[_args_addr] = _args;
      HEAP[_numargs_addr] = _numargs;
      var _0 = HEAP[_numargs_addr];
      var _1 = HEAP[_0];
      var _2 = 0+(_1 > 0);
      if (_2) { __label__ = 0; break; } else { __label__ = 1; break; }
    case 0: // _bb
      var _3 = HEAP[_numargs_addr];
      var _4 = HEAP[_3];
      var _5 = _4 - 1;
      var _6 = HEAP[_args_addr];
      var _8 = HEAP[0 + _6+_5];
      _iftmp_192 = __ZL10parsefloatPKc(_8);
      __label__ = 3; break;
    case 1: // _bb1
      _iftmp_192 = 0.000000e+00;
      __label__ = 3; break;
    case 3: // _bb2
      _val = _iftmp_192;
      _i = 0;
      __label__ = 6; break;
    case 7: // _bb3
      var _11 = HEAP[_args_addr];
      var _14 = HEAP[0 + _11+_i];
      var _15 = __ZL10parsefloatPKc(_14);
      _val = __Z3maxIfET_S0_S0_(_val, _15);
      _i = _i + 1;
      __label__ = 6; break;
    case 6: // _bb4
      var _20 = HEAP[_numargs_addr];
      var _21 = HEAP[_20];
      var _22 = _21 - 1;
      var _24 = 0+(_22 > _i);
      if (_24) { __label__ = 7; break; } else { __label__ = 4; break; }
    case 4: // _bb5
      __Z8floatretf(_val);
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __ZN7_stdcmdILi1149EE3runEPPcPi(_args, _numargs) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _args_addr = Pointer_make([0], 0);
      var _numargs_addr = Pointer_make([0], 0);
      var _iftmp_191;
      var _val;
      var _i;
      var __alloca_point_ = 0;
      HEAP[_args_addr] = _args;
      HEAP[_numargs_addr] = _numargs;
      var _0 = HEAP[_numargs_addr];
      var _1 = HEAP[_0];
      var _2 = 0+(_1 > 0);
      if (_2) { __label__ = 0; break; } else { __label__ = 1; break; }
    case 0: // _bb
      var _3 = HEAP[_numargs_addr];
      var _4 = HEAP[_3];
      var _5 = _4 - 1;
      var _6 = HEAP[_args_addr];
      var _8 = HEAP[0 + _6+_5];
      _iftmp_191 = __ZL10parsefloatPKc(_8);
      __label__ = 3; break;
    case 1: // _bb1
      _iftmp_191 = 0.000000e+00;
      __label__ = 3; break;
    case 3: // _bb2
      _val = _iftmp_191;
      _i = 0;
      __label__ = 6; break;
    case 7: // _bb3
      var _11 = HEAP[_args_addr];
      var _14 = HEAP[0 + _11+_i];
      var _15 = __ZL10parsefloatPKc(_14);
      _val = __Z3minIfET_S0_S0_(_val, _15);
      _i = _i + 1;
      __label__ = 6; break;
    case 6: // _bb4
      var _20 = HEAP[_numargs_addr];
      var _21 = HEAP[_20];
      var _22 = _21 - 1;
      var _24 = 0+(_22 > _i);
      if (_24) { __label__ = 7; break; } else { __label__ = 4; break; }
    case 4: // _bb5
      __Z8floatretf(_val);
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __ZN7_stdcmdILi1143EE3runEPPcPi(_args, _numargs) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _args_addr = Pointer_make([0], 0);
      var _numargs_addr = Pointer_make([0], 0);
      var _iftmp_190;
      var _val;
      var _i;
      var __alloca_point_ = 0;
      HEAP[_args_addr] = _args;
      HEAP[_numargs_addr] = _numargs;
      var _0 = HEAP[_numargs_addr];
      var _1 = HEAP[_0];
      var _2 = 0+(_1 > 0);
      if (_2) { __label__ = 0; break; } else { __label__ = 1; break; }
    case 0: // _bb
      var _3 = HEAP[_numargs_addr];
      var _4 = HEAP[_3];
      var _5 = _4 - 1;
      var _6 = HEAP[_args_addr];
      var _8 = HEAP[0 + _6+_5];
      _iftmp_190 = __ZL8parseintPKc(_8);
      __label__ = 3; break;
    case 1: // _bb1
      _iftmp_190 = 0;
      __label__ = 3; break;
    case 3: // _bb2
      _val = _iftmp_190;
      _i = 0;
      __label__ = 6; break;
    case 7: // _bb3
      var _11 = HEAP[_args_addr];
      var _14 = HEAP[0 + _11+_i];
      var _15 = __ZL8parseintPKc(_14);
      _val = __Z3maxIiET_S0_S0_(_val, _15);
      _i = _i + 1;
      __label__ = 6; break;
    case 6: // _bb4
      var _20 = HEAP[_numargs_addr];
      var _21 = HEAP[_20];
      var _22 = _21 - 1;
      var _24 = 0+(_22 > _i);
      if (_24) { __label__ = 7; break; } else { __label__ = 4; break; }
    case 4: // _bb5
      __Z6intreti(_val);
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __ZN7_stdcmdILi1137EE3runEPPcPi(_args, _numargs) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _args_addr = Pointer_make([0], 0);
      var _numargs_addr = Pointer_make([0], 0);
      var _iftmp_189;
      var _val;
      var _i;
      var __alloca_point_ = 0;
      HEAP[_args_addr] = _args;
      HEAP[_numargs_addr] = _numargs;
      var _0 = HEAP[_numargs_addr];
      var _1 = HEAP[_0];
      var _2 = 0+(_1 > 0);
      if (_2) { __label__ = 0; break; } else { __label__ = 1; break; }
    case 0: // _bb
      var _3 = HEAP[_numargs_addr];
      var _4 = HEAP[_3];
      var _5 = _4 - 1;
      var _6 = HEAP[_args_addr];
      var _8 = HEAP[0 + _6+_5];
      _iftmp_189 = __ZL8parseintPKc(_8);
      __label__ = 3; break;
    case 1: // _bb1
      _iftmp_189 = 0;
      __label__ = 3; break;
    case 3: // _bb2
      _val = _iftmp_189;
      _i = 0;
      __label__ = 6; break;
    case 7: // _bb3
      var _11 = HEAP[_args_addr];
      var _14 = HEAP[0 + _11+_i];
      var _15 = __ZL8parseintPKc(_14);
      _val = __Z3minIiET_S0_S0_(_val, _15);
      _i = _i + 1;
      __label__ = 6; break;
    case 6: // _bb4
      var _20 = HEAP[_numargs_addr];
      var _21 = HEAP[_20];
      var _22 = _21 - 1;
      var _24 = 0+(_22 > _i);
      if (_24) { __label__ = 7; break; } else { __label__ = 4; break; }
    case 4: // _bb5
      __Z6intreti(_val);
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __Z6substrPcPiS_(_s, _start, _count) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _s_addr = Pointer_make([0], 0);
      var _start_addr = Pointer_make([0], 0);
      var _count_addr = Pointer_make([0], 0);
      var _iftmp_168;
      var _len;
      var _offset;
      var __alloca_point_ = 0;
      HEAP[_s_addr] = _s;
      HEAP[_start_addr] = _start;
      HEAP[_count_addr] = _count;
      var _0 = HEAP[_s_addr];
      _len = _strlen(_0);
      var _2 = HEAP[_start_addr];
      var _3 = HEAP[_2];
      var _5 = __Z3minIiET_S0_S0_(_3, _len);
      _offset = __Z3maxIiET_S0_S0_(0, _5);
      var _7 = HEAP[_count_addr];
      var _9 = HEAP[0 + _7];
      var _10 = 0+(_9 != 0);
      if (_10) { __label__ = 0; break; } else { __label__ = 1; break; }
    case 0: // _bb
      var _13 = _len - _offset;
      var _14 = HEAP[_count_addr];
      var _15 = __ZL8parseintPKc(_14);
      var _16 = __Z3minIiET_S0_S0_(_15, _13);
      _iftmp_168 = __Z3maxIiET_S0_S0_(0, _16);
      __label__ = 3; break;
    case 1: // _bb1
      _iftmp_168 = _len - _offset;
      __label__ = 3; break;
    case 3: // _bb2
      var _21 = HEAP[_s_addr];
      var _23 = _21+_offset;
      var _25 = __Z9newstringPKcj(_23, _iftmp_168);
      HEAP[_commandret] = _25;
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __ZN7_stdcmdILi847EE3runEPcS1_S1_(_cond, _t, _f) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _cond_addr = Pointer_make([0], 0);
      var _t_addr = Pointer_make([0], 0);
      var _f_addr = Pointer_make([0], 0);
      var _iftmp_155 = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_cond_addr] = _cond;
      HEAP[_t_addr] = _t;
      HEAP[_f_addr] = _f;
      var _0 = HEAP[_cond_addr];
      var _2 = HEAP[0 + _0];
      var _3 = 0+(_2 == 0);
      if (_3) { __label__ = 8; break; } else { __label__ = 0; break; }
    case 0: // _bb
      var _4 = HEAP[_cond_addr];
      var _5 = __ZL9isintegerPc(_4);
      var _toBool = 0+(_5 != 0);
      var _toBoolnot = _toBool ^ true;
      var _toBoolnot1 = _toBoolnot;
      var _toBool3 = 0+(_toBoolnot1 != 0);
      if (_toBool3) { __label__ = 4; break; } else { __label__ = 6; break; }
    case 6: // _bb4
      var _6 = HEAP[_cond_addr];
      var _7 = __ZL8parseintPKc(_6);
      var _8 = 0+(_7 != 0);
      if (_8) { __label__ = 4; break; } else { __label__ = 8; break; }
    case 4: // _bb5
      var _9 = HEAP[_t_addr];
      HEAP[_iftmp_155] = _9;
      __label__ = 5; break;
    case 8: // _bb6
      var _10 = HEAP[_f_addr];
      HEAP[_iftmp_155] = _10;
      __label__ = 5; break;
    case 5: // _bb7
      var _11 = HEAP[_iftmp_155];
      __Z6resultPKc(_11);
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __ZN7_stdcmdILi1131EE3runEPf(_a) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _a_addr = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_a_addr] = _a;
      var _0 = HEAP[_a_addr];
      var _1 = HEAP[_0];
      var _2 = _1;
      var _3 = _llvm_exp_f64(_2);
      var _4 = _3;
      __Z8floatretf(_4);
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __ZN7_stdcmdILi1130EE3runEPf(_a) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _a_addr = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_a_addr] = _a;
      var _0 = HEAP[_a_addr];
      var _1 = HEAP[_0];
      var _2 = _1;
      var _3 = _llvm_log10_f64(_2);
      var _4 = _3;
      __Z8floatretf(_4);
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __ZN7_stdcmdILi1129EE3runEPf(_a) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _a_addr = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_a_addr] = _a;
      var _0 = HEAP[_a_addr];
      var _1 = HEAP[_0];
      var _2 = _1;
      var _3 = _llvm_log_f64(_2);
      var _4 = _3 / 0x3FE62E42FEFA39EF;
      var _5 = _4;
      __Z8floatretf(_5);
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __ZN7_stdcmdILi1128EE3runEPf(_a) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _a_addr = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_a_addr] = _a;
      var _0 = HEAP[_a_addr];
      var _1 = HEAP[_0];
      var _2 = _1;
      var _3 = _llvm_log_f64(_2);
      var _4 = _3;
      __Z8floatretf(_4);
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __ZN7_stdcmdILi1127EE3runEPfS1_(_a, _b) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _a_addr = Pointer_make([0], 0);
      var _b_addr = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_a_addr] = _a;
      HEAP[_b_addr] = _b;
      var _0 = HEAP[_b_addr];
      var _1 = HEAP[_0];
      var _2 = _1;
      var _3 = HEAP[_a_addr];
      var _4 = HEAP[_3];
      var _5 = _4;
      var _6 = _llvm_pow_f64(_5, _2);
      var _7 = _6;
      __Z8floatretf(_7);
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __ZN7_stdcmdILi1126EE3runEPf(_a) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _a_addr = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_a_addr] = _a;
      var _0 = HEAP[_a_addr];
      var _1 = HEAP[_0];
      var _2 = _1;
      var _3 = _sqrt(_2);
      var _4 = _3;
      __Z8floatretf(_4);
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __ZN7_stdcmdILi1125EE3runEPf(_a) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _a_addr = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_a_addr] = _a;
      var _0 = HEAP[_a_addr];
      var _1 = HEAP[_0];
      var _2 = _1;
      var _3 = _atan(_2);
      var _4 = _3 / 0x3F91DF46A0000000;
      var _5 = _4;
      __Z8floatretf(_5);
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __ZN7_stdcmdILi1124EE3runEPf(_a) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _a_addr = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_a_addr] = _a;
      var _0 = HEAP[_a_addr];
      var _1 = HEAP[_0];
      var _2 = _1;
      var _3 = _acos(_2);
      var _4 = _3 / 0x3F91DF46A0000000;
      var _5 = _4;
      __Z8floatretf(_5);
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __ZN7_stdcmdILi1123EE3runEPf(_a) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _a_addr = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_a_addr] = _a;
      var _0 = HEAP[_a_addr];
      var _1 = HEAP[_0];
      var _2 = _1;
      var _3 = _asin(_2);
      var _4 = _3 / 0x3F91DF46A0000000;
      var _5 = _4;
      __Z8floatretf(_5);
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __ZN7_stdcmdILi1122EE3runEPf(_a) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _a_addr = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_a_addr] = _a;
      var _0 = HEAP[_a_addr];
      var _1 = HEAP[_0];
      var _2 = _1 * 0x3F91DF46A0000000;
      var _3 = _2;
      var _4 = _tan(_3);
      var _5 = _4;
      __Z8floatretf(_5);
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __ZN7_stdcmdILi1121EE3runEPf(_a) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _a_addr = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_a_addr] = _a;
      var _0 = HEAP[_a_addr];
      var _1 = HEAP[_0];
      var _2 = _1 * 0x3F91DF46A0000000;
      var _3 = _2;
      var _4 = _cos(_3);
      var _5 = _4;
      __Z8floatretf(_5);
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __ZN7_stdcmdILi1120EE3runEPf(_a) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _a_addr = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_a_addr] = _a;
      var _0 = HEAP[_a_addr];
      var _1 = HEAP[_0];
      var _2 = _1 * 0x3F91DF46A0000000;
      var _3 = _2;
      var _4 = _sin(_3);
      var _5 = _4;
      __Z8floatretf(_5);
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __ZN7_stdcmdILi1119EE3runEPfS1_(_a, _b) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _a_addr = Pointer_make([0], 0);
      var _b_addr = Pointer_make([0], 0);
      var _iftmp_188;
      var __alloca_point_ = 0;
      HEAP[_a_addr] = _a;
      HEAP[_b_addr] = _b;
      var _0 = HEAP[_b_addr];
      var _1 = HEAP[_0];
      var _2 = 0+(_1 != 0.000000e+00);
      if (_2) { __label__ = 0; break; } else { __label__ = 1; break; }
    case 0: // _bb
      var _3 = HEAP[_b_addr];
      var _4 = HEAP[_3];
      var _5 = _4;
      var _6 = HEAP[_a_addr];
      var _7 = HEAP[_6];
      var _8 = _7;
      var _9 = _fmod(_8, _5);
      _iftmp_188 = _9;
      __label__ = 3; break;
    case 1: // _bb1
      _iftmp_188 = 0.000000e+00;
      __label__ = 3; break;
    case 3: // _bb2
      __Z8floatretf(_iftmp_188);
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __Z7listlenPKc(_s) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _s_addr = Pointer_make([0], 0);
      var _retval;
      var _0;
      var _n;
      var __alloca_point_ = 0;
      HEAP[_s_addr] = _s;
      _n = 0;
      var _1 = HEAP[_s_addr];
      var _2 = _strspn(_1, __str12+0*1);
      var _3 = HEAP[_s_addr];
      var _4 = _3+_2;
      HEAP[_s_addr] = _4;
      __label__ = 6; break;
    case 0: // _bb
      var _5 = HEAP[_s_addr];
      var _6 = HEAP[_5];
      var _7 = 0+(_6 == 34);
      if (_7) { __label__ = 1; break; } else { __label__ = 3; break; }
    case 1: // _bb1
      var _8 = HEAP[_s_addr];
      var _9 = _8+1;
      HEAP[_s_addr] = _9;
      var _10 = HEAP[_s_addr];
      var _11 = _strcspn(_10, __str13+0*1);
      var _12 = HEAP[_s_addr];
      var _13 = _12+_11;
      HEAP[_s_addr] = _13;
      var _14 = HEAP[_s_addr];
      var _15 = HEAP[_14];
      var _16 = 0+(_15 == 34);
      var _17 = _16;
      var _19 = _14+_17;
      HEAP[_s_addr] = _19;
      __label__ = 7; break;
    case 3: // _bb2
      var _20 = HEAP[_s_addr];
      var _21 = _strcspn(_20, __str14+0*1);
      var _22 = HEAP[_s_addr];
      var _23 = _22+_21;
      HEAP[_s_addr] = _23;
      __label__ = 7; break;
    case 7: // _bb3
      var _24 = HEAP[_s_addr];
      var _25 = _strspn(_24, __str12+0*1);
      var _26 = HEAP[_s_addr];
      var _27 = _26+_25;
      HEAP[_s_addr] = _27;
      _n = _n + 1;
      __label__ = 6; break;
    case 6: // _bb4
      var _30 = HEAP[_s_addr];
      var _31 = HEAP[_30];
      var _32 = 0+(_31 != 0);
      if (_32) { __label__ = 0; break; } else { __label__ = 4; break; }
    case 4: // _bb5
      _0 = _n;
      _retval = _0;
      __label__ = 2; break;
    case 2: // _return
      var _retval6 = _retval;
      return _retval6;
  }
}


function __Z10prettylistPKcS0_(_s, _conj) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _s_addr = Pointer_make([0], 0);
      var _conj_addr = Pointer_make([0], 0);
      var _save_filt_342;
      var _save_eptr_341 = Pointer_make([0], 0);
      var _0 = Pointer_make([0], 0);
      var _1 = Pointer_make([0], 0);
      var _2 = Pointer_make([0], 0);
      var _3 = Pointer_make([0], 0);
      var _p = Pointer_make([0,0,0], 0);
      var _len;
      var _n;
      var _elem = Pointer_make([0], 0);
      var _eh_exception = Pointer_make([0], 0);
      var _eh_selector;
      var __alloca_point_ = 0;
      HEAP[_s_addr] = _s;
      HEAP[_conj_addr] = _conj;
      __ZN6vectorIcEC1Ev(_p);
      var _4 = HEAP[_s_addr];
      var _5 = _strspn(_4, __str12+0*1);
      var _6 = HEAP[_s_addr];
      var _7 = _6+_5;
      HEAP[_s_addr] = _7;
      var _8 = HEAP[_s_addr];
      _len = __Z7listlenPKc(_8);
      _n = 0;
      __label__ = 16; break;
    case 0: // _bb
      var _10 = HEAP[_s_addr];
      HEAP[_elem] = _10;
      var _11 = HEAP[_s_addr];
      var _12 = HEAP[_11];
      var _13 = 0+(_12 == 34);
      if (_13) { __label__ = 1; break; } else { __label__ = 3; break; }
    case 1: // _bb1
      var _14 = HEAP[_s_addr];
      var _15 = _14+1;
      HEAP[_s_addr] = _15;
      var _16 = HEAP[_s_addr];
      var _17 = _strcspn(_16, __str13+0*1);
      var _18 = HEAP[_s_addr];
      var _19 = _18+_17;
      HEAP[_s_addr] = _19;
      var _20 = HEAP[_s_addr];
      var _21 = HEAP[_20];
      var _22 = 0+(_21 == 34);
      var _23 = _22;
      var _25 = _20+_23;
      HEAP[_s_addr] = _25;
      __label__ = 7; break;
    case 3: // _bb2
      var _26 = HEAP[_s_addr];
      var _27 = _strcspn(_26, __str14+0*1);
      var _28 = HEAP[_s_addr];
      var _29 = _28+_27;
      HEAP[_s_addr] = _29;
      __label__ = 7; break;
    case 7: // _bb3
      var _30 = HEAP[_s_addr];
      var _31 = _30;
      var _32 = HEAP[_elem];
      var _33 = _32;
      var _34 = _31 - _33;
      (function() { try { return __ZN6vectorIcE3putEPKci(_p, _32, _34); __THREW__ = false } catch(e) { __THREW__ = true; } })(); if (!__THREW__) { __label__ = 137; break; } else { __label__ = 134; break; }
    case 137: // _invcont
      var _37 = _n + 1;
      var _39 = 0+(_37 < _len);
      if (_39) { __label__ = 6; break; } else { __label__ = 21; break; }
    case 6: // _bb4
      var _41 = 0+(_len > 2);
      if (_41) { __label__ = 8; break; } else { __label__ = 4; break; }
    case 4: // _bb5
      var _42 = HEAP[_conj_addr];
      var _44 = HEAP[0 + _42];
      var _45 = 0+(_44 == 0);
      if (_45) { __label__ = 8; break; } else { __label__ = 10; break; }
    case 8: // _bb6
      HEAP[_3] = 44;
      var _46 = (function() { try { return __ZN6vectorIcE3addERKc(_p, _3); __THREW__ = false } catch(e) { __THREW__ = true; } })(); if (!__THREW__) { __label__ = 150; break; } else { __label__ = 134; break; };
    case 150: // _invcont7
      __label__ = 10; break;
    case 10: // _bb8
      var _48 = _n + 2;
      var _50 = 0+(_48 == _len);
      if (_50) { __label__ = 9; break; } else { __label__ = 15; break; }
    case 9: // _bb9
      var _51 = HEAP[_conj_addr];
      var _53 = HEAP[0 + _51];
      var _54 = 0+(_53 != 0);
      if (_54) { __label__ = 11; break; } else { __label__ = 15; break; }
    case 11: // _bb10
      HEAP[_2] = 32;
      var _55 = (function() { try { return __ZN6vectorIcE3addERKc(_p, _2); __THREW__ = false } catch(e) { __THREW__ = true; } })(); if (!__THREW__) { __label__ = 148; break; } else { __label__ = 134; break; };
    case 148: // _invcont11
      var _56 = HEAP[_conj_addr];
      var _57 = _strlen(_56);
      var _58 = HEAP[_conj_addr];
      (function() { try { return __ZN6vectorIcE3putEPKci(_p, _58, _57); __THREW__ = false } catch(e) { __THREW__ = true; } })(); if (!__THREW__) { __label__ = 138; break; } else { __label__ = 134; break; }
    case 138: // _invcont12
      __label__ = 15; break;
    case 15: // _bb13
      HEAP[_1] = 32;
      var _59 = (function() { try { return __ZN6vectorIcE3addERKc(_p, _1); __THREW__ = false } catch(e) { __THREW__ = true; } })(); if (!__THREW__) { __label__ = 149; break; } else { __label__ = 134; break; };
    case 149: // _invcont14
      __label__ = 21; break;
    case 21: // _bb15
      var _60 = HEAP[_s_addr];
      var _61 = _strspn(_60, __str12+0*1);
      var _62 = HEAP[_s_addr];
      var _63 = _62+_61;
      HEAP[_s_addr] = _63;
      _n = _n + 1;
      __label__ = 16; break;
    case 16: // _bb16
      var _66 = HEAP[_s_addr];
      var _67 = HEAP[_66];
      var _68 = 0+(_67 != 0);
      if (_68) { __label__ = 0; break; } else { __label__ = 20; break; }
    case 20: // _bb17
      HEAP[_0] = 0;
      var _69 = (function() { try { return __ZN6vectorIcE3addERKc(_p, _0); __THREW__ = false } catch(e) { __THREW__ = true; } })(); if (!__THREW__) { __label__ = 133; break; } else { __label__ = 134; break; };
    case 133: // _invcont18
      var _70 = __ZN6vectorIcE6getbufEv(_p);
      (function() { try { return __Z6resultPKc(_70); __THREW__ = false } catch(e) { __THREW__ = true; } })(); if (!__THREW__) { __label__ = 131; break; } else { __label__ = 134; break; }
    case 131: // _invcont19
      __ZN6vectorIcED1Ev(_p);
      __label__ = 2; break;
    case 19: // _bb20
      _save_filt_342 = _eh_selector;
      var _eh_value = HEAP[_eh_exception];
      HEAP[_save_eptr_341] = _eh_value;
      (function() { try { return __ZN6vectorIcED1Ev(_p); __THREW__ = false } catch(e) { __THREW__ = true; } })(); if (!__THREW__) { __label__ = 139; break; } else { __label__ = 132; break; }
    case 139: // _invcont21
      var _71 = HEAP[_save_eptr_341];
      HEAP[_eh_exception] = _71;
      _eh_selector = _save_filt_342;
      __label__ = 26; break;
    case 25: // _bb22
      __ZSt9terminatev();
      // unreachable
    case 2: // _return
      return;
    case 134: // _lpad
      var _eh_ptr = _llvm_eh_exception();
      HEAP[_eh_exception] = _eh_ptr;
      var _eh_ptr23 = HEAP[_eh_exception];
      _eh_selector = _llvm_eh_selector(_eh_ptr23, ___gxx_personality_v0, 0);
      __label__ = 27; break;
    case 132: // _lpad25
      var _eh_ptr26 = _llvm_eh_exception();
      HEAP[_eh_exception] = _eh_ptr26;
      var _eh_ptr27 = HEAP[_eh_exception];
      _eh_selector = _llvm_eh_selector(_eh_ptr27, ___gxx_personality_v0, 1);
      __label__ = 28; break;
    case 27: // _ppad
      __label__ = 19; break;
    case 28: // _ppad29
      __label__ = 25; break;
    case 26: // _Unwind
      var _eh_ptr30 = HEAP[_eh_exception];
      __Unwind_Resume_or_Rethrow(_eh_ptr30);
      // unreachable
  }
}


function __ZN7_stdcmdILi973EE3runEPc(_s) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _s_addr = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_s_addr] = _s;
      var _0 = HEAP[_s_addr];
      var _1 = __Z7listlenPKc(_0);
      __Z6intreti(_1);
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __Z9indexlistPKci(_s, _pos) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _s_addr = Pointer_make([0], 0);
      var _pos_addr;
      var _retval = Pointer_make([0], 0);
      var _0 = Pointer_make([0], 0);
      var _e = Pointer_make([0], 0);
      var _i;
      var __alloca_point_ = 0;
      HEAP[_s_addr] = _s;
      _pos_addr = _pos;
      var _1 = HEAP[_s_addr];
      var _2 = _strspn(_1, __str12+0*1);
      var _3 = HEAP[_s_addr];
      var _4 = _3+_2;
      HEAP[_s_addr] = _4;
      _i = 0;
      __label__ = 4; break;
    case 0: // _bb
      var _5 = HEAP[_s_addr];
      var _6 = HEAP[_5];
      var _7 = 0+(_6 == 34);
      if (_7) { __label__ = 1; break; } else { __label__ = 3; break; }
    case 1: // _bb1
      var _8 = HEAP[_s_addr];
      var _9 = _8+1;
      HEAP[_s_addr] = _9;
      var _10 = HEAP[_s_addr];
      var _11 = _strcspn(_10, __str13+0*1);
      var _12 = HEAP[_s_addr];
      var _13 = _12+_11;
      HEAP[_s_addr] = _13;
      var _14 = HEAP[_s_addr];
      var _15 = HEAP[_14];
      var _16 = 0+(_15 == 34);
      var _17 = _16;
      var _19 = _14+_17;
      HEAP[_s_addr] = _19;
      __label__ = 7; break;
    case 3: // _bb2
      var _20 = HEAP[_s_addr];
      var _21 = _strcspn(_20, __str14+0*1);
      var _22 = HEAP[_s_addr];
      var _23 = _22+_21;
      HEAP[_s_addr] = _23;
      __label__ = 7; break;
    case 7: // _bb3
      var _24 = HEAP[_s_addr];
      var _25 = _strspn(_24, __str12+0*1);
      var _26 = HEAP[_s_addr];
      var _27 = _26+_25;
      HEAP[_s_addr] = _27;
      var _28 = HEAP[_s_addr];
      var _29 = HEAP[_28];
      var _30 = 0+(_29 == 0);
      if (_30) { __label__ = 8; break; } else { __label__ = 6; break; }
    case 6: // _bb4
      _i = _i + 1;
      __label__ = 4; break;
    case 4: // _bb5
      var _35 = 0+(_i < _pos_addr);
      if (_35) { __label__ = 0; break; } else { __label__ = 8; break; }
    case 8: // _bb6
      var _36 = HEAP[_s_addr];
      HEAP[_e] = _36;
      var _37 = HEAP[_s_addr];
      var _38 = HEAP[_37];
      var _39 = 0+(_38 == 34);
      if (_39) { __label__ = 5; break; } else { __label__ = 10; break; }
    case 5: // _bb7
      var _40 = HEAP[_s_addr];
      var _41 = _40+1;
      HEAP[_s_addr] = _41;
      var _42 = HEAP[_s_addr];
      var _43 = _strcspn(_42, __str13+0*1);
      var _44 = HEAP[_s_addr];
      var _45 = _44+_43;
      HEAP[_s_addr] = _45;
      var _46 = HEAP[_s_addr];
      var _47 = HEAP[_46];
      var _48 = 0+(_47 == 34);
      var _49 = _48;
      var _51 = _46+_49;
      HEAP[_s_addr] = _51;
      __label__ = 9; break;
    case 10: // _bb8
      var _52 = HEAP[_s_addr];
      var _53 = _strcspn(_52, __str14+0*1);
      var _54 = HEAP[_s_addr];
      var _55 = _54+_53;
      HEAP[_s_addr] = _55;
      __label__ = 9; break;
    case 9: // _bb9
      var _56 = HEAP[_e];
      var _57 = HEAP[_56];
      var _58 = 0+(_57 == 34);
      if (_58) { __label__ = 11; break; } else { __label__ = 14; break; }
    case 11: // _bb10
      var _59 = HEAP[_e];
      var _60 = _59+1;
      HEAP[_e] = _60;
      var _61 = HEAP[_s_addr];
      var _63 = HEAP[0 + _61+-1];
      var _64 = 0+(_63 == 34);
      if (_64) { __label__ = 13; break; } else { __label__ = 14; break; }
    case 13: // _bb11
      var _65 = HEAP[_s_addr];
      var _66 = _65+-1;
      HEAP[_s_addr] = _66;
      __label__ = 14; break;
    case 14: // _bb12
      var _67 = HEAP[_s_addr];
      var _68 = _67;
      var _69 = HEAP[_e];
      var _70 = _69;
      var _71 = _68 - _70;
      var _73 = __Z9newstringPKcj(_69, _71);
      HEAP[_0] = _73;
      var _74 = HEAP[_0];
      HEAP[_retval] = _74;
      __label__ = 2; break;
    case 2: // _return
      var _retval13 = HEAP[_retval];
      return _retval13;
  }
}


function __Z2atPcPi(_s, _pos) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _s_addr = Pointer_make([0], 0);
      var _pos_addr = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_s_addr] = _s;
      HEAP[_pos_addr] = _pos;
      var _0 = HEAP[_pos_addr];
      var _1 = HEAP[_0];
      var _2 = HEAP[_s_addr];
      var _3 = __Z9indexlistPKci(_2, _1);
      HEAP[_commandret] = _3;
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __Z11explodelistPKcR6vectorIPcE(_s, _elems) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _s_addr = Pointer_make([0], 0);
      var _elems_addr = Pointer_make([0], 0);
      var _iftmp_163;
      var _iftmp_160 = Pointer_make([0], 0);
      var _0 = Pointer_make([0], 0);
      var _elem = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_s_addr] = _s;
      HEAP[_elems_addr] = _elems;
      var _1 = HEAP[_s_addr];
      var _2 = _strspn(_1, __str12+0*1);
      var _3 = HEAP[_s_addr];
      var _4 = _3+_2;
      HEAP[_s_addr] = _4;
      __label__ = 11; break;
    case 0: // _bb
      var _5 = HEAP[_s_addr];
      HEAP[_elem] = _5;
      var _6 = HEAP[_s_addr];
      var _7 = HEAP[_6];
      var _8 = 0+(_7 == 34);
      if (_8) { __label__ = 1; break; } else { __label__ = 3; break; }
    case 1: // _bb1
      var _9 = HEAP[_s_addr];
      var _10 = _9+1;
      HEAP[_s_addr] = _10;
      var _11 = HEAP[_s_addr];
      var _12 = _strcspn(_11, __str13+0*1);
      var _13 = HEAP[_s_addr];
      var _14 = _13+_12;
      HEAP[_s_addr] = _14;
      var _15 = HEAP[_s_addr];
      var _16 = HEAP[_15];
      var _17 = 0+(_16 == 34);
      var _18 = _17;
      var _20 = _15+_18;
      HEAP[_s_addr] = _20;
      __label__ = 7; break;
    case 3: // _bb2
      var _21 = HEAP[_s_addr];
      var _22 = _strcspn(_21, __str14+0*1);
      var _23 = HEAP[_s_addr];
      var _24 = _23+_22;
      HEAP[_s_addr] = _24;
      __label__ = 7; break;
    case 7: // _bb3
      var _25 = HEAP[_elem];
      var _26 = HEAP[_25];
      var _27 = 0+(_26 == 34);
      if (_27) { __label__ = 6; break; } else { __label__ = 10; break; }
    case 6: // _bb4
      var _28 = HEAP[_s_addr];
      var _29 = _28;
      var _30 = HEAP[_elem];
      var _31 = _30;
      var _32 = _29 - _31;
      var _35 = HEAP[0 + _28+-1];
      var _36 = 0+(_35 == 34);
      if (_36) { __label__ = 4; break; } else { __label__ = 8; break; }
    case 4: // _bb5
      _iftmp_163 = 2;
      __label__ = 5; break;
    case 8: // _bb6
      _iftmp_163 = 1;
      __label__ = 5; break;
    case 5: // _bb7
      var _38 = _32 - _iftmp_163;
      var _39 = HEAP[_elem];
      var _40 = _39+1;
      var _41 = __Z9newstringPKcj(_40, _38);
      HEAP[_iftmp_160] = _41;
      __label__ = 9; break;
    case 10: // _bb8
      var _42 = HEAP[_s_addr];
      var _43 = _42;
      var _44 = HEAP[_elem];
      var _45 = _44;
      var _46 = _43 - _45;
      var _48 = __Z9newstringPKcj(_44, _46);
      HEAP[_iftmp_160] = _48;
      __label__ = 9; break;
    case 9: // _bb9
      var _49 = HEAP[_iftmp_160];
      HEAP[_0] = _49;
      var _50 = HEAP[_elems_addr];
      var _51 = __ZN6vectorIPcE3addERKS0_(_50, _0);
      var _52 = HEAP[_s_addr];
      var _53 = _strspn(_52, __str12+0*1);
      var _54 = HEAP[_s_addr];
      var _55 = _54+_53;
      HEAP[_s_addr] = _55;
      __label__ = 11; break;
    case 11: // _bb10
      var _56 = HEAP[_s_addr];
      var _57 = HEAP[_56];
      var _58 = 0+(_57 != 0);
      if (_58) { __label__ = 0; break; } else { __label__ = 13; break; }
    case 13: // _bb11
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __Z12listincludesPKcS0_i(_list, _needle, _needlelen) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _list_addr = Pointer_make([0], 0);
      var _needle_addr = Pointer_make([0], 0);
      var _needlelen_addr;
      var _retval;
      var _0;
      var _iftmp_178;
      var _s = Pointer_make([0], 0);
      var _offset;
      var _elem = Pointer_make([0], 0);
      var _len;
      var __alloca_point_ = 0;
      HEAP[_list_addr] = _list;
      HEAP[_needle_addr] = _needle;
      _needlelen_addr = _needlelen;
      var _1 = HEAP[_list_addr];
      HEAP[_s] = _1;
      var _2 = HEAP[_s];
      var _3 = _strspn(_2, __str12+0*1);
      var _4 = HEAP[_s];
      var _5 = _4+_3;
      HEAP[_s] = _5;
      _offset = 0;
      __label__ = 14; break;
    case 0: // _bb
      var _6 = HEAP[_s];
      HEAP[_elem] = _6;
      var _7 = HEAP[_s];
      var _8 = HEAP[_7];
      var _9 = 0+(_8 == 34);
      if (_9) { __label__ = 1; break; } else { __label__ = 3; break; }
    case 1: // _bb1
      var _10 = HEAP[_s];
      var _11 = _10+1;
      HEAP[_s] = _11;
      var _12 = HEAP[_s];
      var _13 = _strcspn(_12, __str13+0*1);
      var _14 = HEAP[_s];
      var _15 = _14+_13;
      HEAP[_s] = _15;
      var _16 = HEAP[_s];
      var _17 = HEAP[_16];
      var _18 = 0+(_17 == 34);
      var _19 = _18;
      var _21 = _16+_19;
      HEAP[_s] = _21;
      __label__ = 7; break;
    case 3: // _bb2
      var _22 = HEAP[_s];
      var _23 = _strcspn(_22, __str14+0*1);
      var _24 = HEAP[_s];
      var _25 = _24+_23;
      HEAP[_s] = _25;
      __label__ = 7; break;
    case 7: // _bb3
      var _26 = HEAP[_s];
      var _27 = _26;
      var _28 = HEAP[_elem];
      var _29 = _28;
      _len = _27 - _29;
      var _32 = HEAP[_28];
      var _33 = 0+(_32 == 34);
      if (_33) { __label__ = 6; break; } else { __label__ = 10; break; }
    case 6: // _bb4
      var _34 = HEAP[_elem];
      var _35 = _34+1;
      HEAP[_elem] = _35;
      var _36 = HEAP[_s];
      var _38 = HEAP[0 + _36+-1];
      var _39 = 0+(_38 == 34);
      if (_39) { __label__ = 4; break; } else { __label__ = 8; break; }
    case 4: // _bb5
      _iftmp_178 = 2;
      __label__ = 5; break;
    case 8: // _bb6
      _iftmp_178 = 1;
      __label__ = 5; break;
    case 5: // _bb7
      _len = _len - _iftmp_178;
      __label__ = 10; break;
    case 10: // _bb8
      var _45 = 0+(_needlelen_addr == _len);
      if (_45) { __label__ = 9; break; } else { __label__ = 13; break; }
    case 9: // _bb9
      var _46 = _len;
      var _47 = HEAP[_needle_addr];
      var _48 = HEAP[_elem];
      var _49 = _strncmp(_47, _48, _46);
      var _50 = 0+(_49 == 0);
      if (_50) { __label__ = 11; break; } else { __label__ = 13; break; }
    case 11: // _bb10
      _0 = _offset;
      __label__ = 12; break;
    case 13: // _bb11
      var _52 = HEAP[_s];
      var _53 = _strspn(_52, __str12+0*1);
      var _54 = HEAP[_s];
      var _55 = _54+_53;
      HEAP[_s] = _55;
      _offset = _offset + 1;
      __label__ = 14; break;
    case 14: // _bb12
      var _58 = HEAP[_s];
      var _59 = HEAP[_58];
      var _60 = 0+(_59 != 0);
      if (_60) { __label__ = 0; break; } else { __label__ = 15; break; }
    case 15: // _bb13
      _0 = -1;
      __label__ = 12; break;
    case 12: // _bb14
      _retval = _0;
      __label__ = 2; break;
    case 2: // _return
      var _retval15 = _retval;
      return _retval15;
  }
}


function __ZN7_stdcmdILi1071EE3runEPcS1_(_list, _elem) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _list_addr = Pointer_make([0], 0);
      var _elem_addr = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_list_addr] = _list;
      HEAP[_elem_addr] = _elem;
      var _0 = HEAP[_elem_addr];
      var _1 = _strlen(_0);
      var _2 = HEAP[_list_addr];
      var _3 = HEAP[_elem_addr];
      var _4 = __Z12listincludesPKcS0_i(_2, _3, _1);
      __Z6intreti(_4);
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __Z7listdelPKcS0_(_s, _del) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _s_addr = Pointer_make([0], 0);
      var _del_addr = Pointer_make([0], 0);
      var _retval = Pointer_make([0], 0);
      var _save_filt_340;
      var _save_eptr_339 = Pointer_make([0], 0);
      var _0 = Pointer_make([0], 0);
      var _1 = Pointer_make([0], 0);
      var _2 = Pointer_make([0], 0);
      var _retval_184;
      var _retval_183;
      var _iftmp_182;
      var _p = Pointer_make([0,0,0], 0);
      var _elem = Pointer_make([0], 0);
      var _len;
      var _eh_exception = Pointer_make([0], 0);
      var _eh_selector;
      var __alloca_point_ = 0;
      HEAP[_s_addr] = _s;
      HEAP[_del_addr] = _del;
      __ZN6vectorIcEC1Ev(_p);
      var _3 = HEAP[_s_addr];
      var _4 = _strspn(_3, __str12+0*1);
      var _5 = HEAP[_s_addr];
      var _6 = _5+_4;
      HEAP[_s_addr] = _6;
      __label__ = 20; break;
    case 0: // _bb
      var _7 = HEAP[_s_addr];
      HEAP[_elem] = _7;
      var _8 = HEAP[_s_addr];
      var _9 = HEAP[_8];
      var _10 = 0+(_9 == 34);
      if (_10) { __label__ = 1; break; } else { __label__ = 3; break; }
    case 1: // _bb1
      var _11 = HEAP[_s_addr];
      var _12 = _11+1;
      HEAP[_s_addr] = _12;
      var _13 = HEAP[_s_addr];
      var _14 = _strcspn(_13, __str13+0*1);
      var _15 = HEAP[_s_addr];
      var _16 = _15+_14;
      HEAP[_s_addr] = _16;
      var _17 = HEAP[_s_addr];
      var _18 = HEAP[_17];
      var _19 = 0+(_18 == 34);
      var _20 = _19;
      var _22 = _17+_20;
      HEAP[_s_addr] = _22;
      __label__ = 7; break;
    case 3: // _bb2
      var _23 = HEAP[_s_addr];
      var _24 = _strcspn(_23, __str14+0*1);
      var _25 = HEAP[_s_addr];
      var _26 = _25+_24;
      HEAP[_s_addr] = _26;
      __label__ = 7; break;
    case 7: // _bb3
      var _27 = HEAP[_s_addr];
      var _28 = _27;
      var _29 = HEAP[_elem];
      var _30 = _29;
      _len = _28 - _30;
      var _33 = HEAP[_29];
      var _34 = 0+(_33 == 34);
      if (_34) { __label__ = 6; break; } else { __label__ = 10; break; }
    case 6: // _bb4
      var _35 = HEAP[_elem];
      var _36 = _35+1;
      HEAP[_elem] = _36;
      var _37 = HEAP[_s_addr];
      var _39 = HEAP[0 + _37+-1];
      var _40 = 0+(_39 == 34);
      if (_40) { __label__ = 4; break; } else { __label__ = 8; break; }
    case 4: // _bb5
      _iftmp_182 = 2;
      __label__ = 5; break;
    case 8: // _bb6
      _iftmp_182 = 1;
      __label__ = 5; break;
    case 5: // _bb7
      _len = _len - _iftmp_182;
      __label__ = 10; break;
    case 10: // _bb8
      var _44 = HEAP[_del_addr];
      var _45 = HEAP[_elem];
      var _47 = __Z12listincludesPKcS0_i(_44, _45, _len);
      var _48 = 0+(_47 < 0);
      _retval_183 = _48;
      var _toBool = 0+(_retval_183 != 0);
      if (_toBool) { __label__ = 9; break; } else { __label__ = 16; break; }
    case 9: // _bb9
      var _51 = __ZNK6vectorIcE5emptyEv(_p);
      var _toBool10 = 0+(_51 != 0);
      var _toBool10not = _toBool10 ^ true;
      _retval_184 = _toBool10not;
      var _toBool12 = 0+(_retval_184 != 0);
      if (_toBool12) { __label__ = 15; break; } else { __label__ = 12; break; }
    case 15: // _bb13
      HEAP[_2] = 32;
      var _53 = (function() { try { return __ZN6vectorIcE3addERKc(_p, _2); __THREW__ = false } catch(e) { __THREW__ = true; } })(); if (!__THREW__) { __label__ = 137; break; } else { __label__ = 134; break; };
    case 137: // _invcont
      __label__ = 12; break;
    case 12: // _bb14
      var _54 = HEAP[_elem];
      (function() { try { return __ZN6vectorIcE3putEPKci(_p, _54, _len); __THREW__ = false } catch(e) { __THREW__ = true; } })(); if (!__THREW__) { __label__ = 140; break; } else { __label__ = 134; break; }
    case 140: // _invcont15
      __label__ = 16; break;
    case 16: // _bb16
      var _56 = HEAP[_s_addr];
      var _57 = _strspn(_56, __str12+0*1);
      var _58 = HEAP[_s_addr];
      var _59 = _58+_57;
      HEAP[_s_addr] = _59;
      __label__ = 20; break;
    case 20: // _bb17
      var _60 = HEAP[_s_addr];
      var _61 = HEAP[_60];
      var _62 = 0+(_61 != 0);
      if (_62) { __label__ = 0; break; } else { __label__ = 17; break; }
    case 17: // _bb18
      HEAP[_1] = 0;
      var _63 = (function() { try { return __ZN6vectorIcE3addERKc(_p, _1); __THREW__ = false } catch(e) { __THREW__ = true; } })(); if (!__THREW__) { __label__ = 131; break; } else { __label__ = 134; break; };
    case 131: // _invcont19
      var _64 = __ZN6vectorIcE6getbufEv(_p);
      var _65 = (function() { try { return __Z9newstringPKc(_64); __THREW__ = false } catch(e) { __THREW__ = true; } })(); if (!__THREW__) { __label__ = 135; break; } else { __label__ = 134; break; };
    case 135: // _invcont20
      HEAP[_0] = _65;
      __ZN6vectorIcED1Ev(_p);
      var _66 = HEAP[_0];
      HEAP[_retval] = _66;
      __label__ = 2; break;
    case 24: // _bb21
      _save_filt_340 = _eh_selector;
      var _eh_value = HEAP[_eh_exception];
      HEAP[_save_eptr_339] = _eh_value;
      (function() { try { return __ZN6vectorIcED1Ev(_p); __THREW__ = false } catch(e) { __THREW__ = true; } })(); if (!__THREW__) { __label__ = 141; break; } else { __label__ = 142; break; }
    case 141: // _invcont22
      var _67 = HEAP[_save_eptr_339];
      HEAP[_eh_exception] = _67;
      _eh_selector = _save_filt_340;
      __label__ = 26; break;
    case 23: // _bb23
      __ZSt9terminatev();
      // unreachable
    case 2: // _return
      var _retval24 = HEAP[_retval];
      return _retval24;
    case 134: // _lpad
      var _eh_ptr = _llvm_eh_exception();
      HEAP[_eh_exception] = _eh_ptr;
      var _eh_ptr25 = HEAP[_eh_exception];
      _eh_selector = _llvm_eh_selector(_eh_ptr25, ___gxx_personality_v0, 0);
      __label__ = 27; break;
    case 142: // _lpad27
      var _eh_ptr28 = _llvm_eh_exception();
      HEAP[_eh_exception] = _eh_ptr28;
      var _eh_ptr29 = HEAP[_eh_exception];
      _eh_selector = _llvm_eh_selector(_eh_ptr29, ___gxx_personality_v0, 1);
      __label__ = 30; break;
    case 27: // _ppad
      __label__ = 24; break;
    case 30: // _ppad31
      __label__ = 23; break;
    case 26: // _Unwind
      var _eh_ptr32 = HEAP[_eh_exception];
      __Unwind_Resume_or_Rethrow(_eh_ptr32);
      // unreachable
  }
}


function __ZN7_stdcmdILi1070EE3runEPcS1_(_list, _del) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _list_addr = Pointer_make([0], 0);
      var _del_addr = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_list_addr] = _list;
      HEAP[_del_addr] = _del;
      var _0 = HEAP[_list_addr];
      var _1 = HEAP[_del_addr];
      var _2 = __Z7listdelPKcS0_(_0, _1);
      HEAP[_commandret] = _2;
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __Z4concPPcib(_w, _n, _space) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _w_addr = Pointer_make([0], 0);
      var _n_addr;
      var _space_addr;
      var _retval = Pointer_make([0], 0);
      var _0 = Pointer_make([0], 0);
      var _iftmp_104;
      var _len;
      var _r = Pointer_make([0], 0);
      var _j;
      var _i;
      var __alloca_point_ = 0;
      HEAP[_w_addr] = _w;
      _n_addr = _n;
      _space_addr = _space;
      var _toBool = 0+(_space_addr != 0);
      if (_toBool) { __label__ = 0; break; } else { __label__ = 1; break; }
    case 0: // _bb
      var _3 = _n_addr - 1;
      _iftmp_104 = __Z3maxIiET_S0_S0_(_3, 0);
      __label__ = 3; break;
    case 1: // _bb1
      _iftmp_104 = 0;
      __label__ = 3; break;
    case 3: // _bb2
      _len = _iftmp_104;
      _j = 0;
      __label__ = 6; break;
    case 7: // _bb3
      var _6 = HEAP[_w_addr];
      var _9 = HEAP[0 + _6+_j];
      var _10 = _strlen(_9);
      _len = _10 + _len;
      _j = _j + 1;
      __label__ = 6; break;
    case 6: // _bb4
      var _17 = 0+(_j < _n_addr);
      if (_17) { __label__ = 7; break; } else { __label__ = 4; break; }
    case 4: // _bb5
      var _19 = __Z9newstringPKcj(__str3+0*1, _len);
      HEAP[_r] = _19;
      _i = 0;
      __label__ = 13; break;
    case 8: // _bb6
      var _20 = HEAP[_w_addr];
      var _23 = HEAP[0 + _20+_i];
      var _24 = HEAP[_r];
      var _25 = _strcat(_24, _23);
      var _27 = _n_addr - 1;
      var _29 = 0+(_27 == _i);
      if (_29) { __label__ = 14; break; } else { __label__ = 5; break; }
    case 5: // _bb7
      var _toBool8 = 0+(_space_addr != 0);
      if (_toBool8) { __label__ = 9; break; } else { __label__ = 11; break; }
    case 9: // _bb9
      var _31 = HEAP[_r];
      var _32 = _strcat(_31, __str15+0*1);
      __label__ = 11; break;
    case 11: // _bb10
      _i = _i + 1;
      __label__ = 13; break;
    case 13: // _bb11
      var _37 = 0+(_i < _n_addr);
      if (_37) { __label__ = 8; break; } else { __label__ = 14; break; }
    case 14: // _bb12
      var _38 = HEAP[_r];
      HEAP[_0] = _38;
      var _39 = HEAP[_0];
      HEAP[_retval] = _39;
      __label__ = 2; break;
    case 2: // _return
      var _retval13 = HEAP[_retval];
      return _retval13;
  }
}


function __Z10concatwordPPcPi(_args, _numargs) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _args_addr = Pointer_make([0], 0);
      var _numargs_addr = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_args_addr] = _args;
      HEAP[_numargs_addr] = _numargs;
      var _0 = HEAP[_numargs_addr];
      var _1 = HEAP[_0];
      var _2 = HEAP[_args_addr];
      var _3 = __Z4concPPcib(_2, _1, 0);
      HEAP[_commandret] = _3;
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __Z9pushidentR5identPc(_id, _val) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _id_addr = Pointer_make([0], 0);
      var _val_addr = Pointer_make([0], 0);
      var _iftmp_32 = Pointer_make([0], 0);
      var _stack = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_id_addr] = _id;
      HEAP[_val_addr] = _val;
      var _0 = HEAP[_id_addr];
      var _2 = HEAP[0 + _0+1*1];
      var _3 = 0+(_2 != 5);
      if (_3) { __label__ = 6; break; } else { __label__ = 0; break; }
    case 0: // _bb
      var _4 = __Znwj(8);
      HEAP[_stack] = _4;
      var _6 = HEAP[_id_addr];
      var _7 = _6+8*1;
      var _9 = HEAP[0 + _7+0*1];
      var _11 = _6+7*1;
      var _13 = HEAP[0 + _11+0*1];
      var _14 = 0+(_9 == _13);
      if (_14) { __label__ = 1; break; } else { __label__ = 3; break; }
    case 1: // _bb1
      var _15 = HEAP[_id_addr];
      var _16 = _15+7*1;
      var _18 = HEAP[0 + _16+0*1];
      var _19 = __Z9newstringPKc(_18);
      HEAP[_iftmp_32] = _19;
      __label__ = 7; break;
    case 3: // _bb2
      var _20 = HEAP[_id_addr];
      var _21 = _20+7*1;
      var _23 = HEAP[0 + _21+0*1];
      HEAP[_iftmp_32] = _23;
      __label__ = 7; break;
    case 7: // _bb3
      var _24 = HEAP[_stack];
      var _25 = _24+0*1;
      var _26 = HEAP[_iftmp_32];
      HEAP[_25] = _26;
      var _27 = HEAP[_id_addr];
      var _28 = _27+6*1;
      var _29 = _28;
      var _31 = HEAP[_29];
      var _32 = HEAP[_stack];
      HEAP[0 + _32+1*1] = _31;
      var _34 = HEAP[_id_addr];
      var _35 = _34+6*1;
      var _36 = _35;
      var _37 = _36;
      var _38 = HEAP[_stack];
      HEAP[_37] = _38;
      var _39 = HEAP[_id_addr];
      var _40 = _39+7*1;
      var _41 = _40+0*1;
      var _42 = HEAP[_val_addr];
      HEAP[_41] = _42;
      __label__ = 6; break;
    case 6: // _bb4
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __ZN14hashtableentryIPKc5identED1Ev(_this) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _this_addr = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_this_addr] = _this;
      var _0 = HEAP[_this_addr];
      var _1 = _0+__struct_hashtableentry_const_char__ident_____FLATTENER[1];
      __ZN5identD1Ev(_1);
      __label__ = 0; break;
    case 0: // _bb
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __ZN7hashsetI14hashtableentryIPKc5identEE5chainD1Ev(_this) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _this_addr = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_this_addr] = _this;
      var _0 = HEAP[_this_addr];
      var _1 = _0+__struct_hashset_hashtableentry_const_char___ident_____chain____FLATTENER[0];
      __ZN14hashtableentryIPKc5identED1Ev(_1);
      __label__ = 0; break;
    case 0: // _bb
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __ZN7hashsetI14hashtableentryIPKc5identEE10chainchunkC1Ev(_this) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _this_addr = Pointer_make([0], 0);
      var _save_filt_350;
      var _save_eptr_349 = Pointer_make([0], 0);
      var _0 = Pointer_make([0], 0);
      var _1;
      var _2 = Pointer_make([0], 0);
      var _3 = Pointer_make([0], 0);
      var _retval_245 = Pointer_make([0], 0);
      var _eh_exception = Pointer_make([0], 0);
      var _eh_selector;
      var __alloca_point_ = 0;
      HEAP[_this_addr] = _this;
      var _4 = HEAP[_this_addr];
      var _5 = _4+__struct_hashset_hashtableentry_const_char___ident_____chainchunk____FLATTENER[0];
      var _6 = _5+0*13;
      HEAP[_3] = _6;
      var _7 = HEAP[_3];
      HEAP[_2] = _7;
      _1 = 63;
      __label__ = 1; break;
    case 0: // _bb
      var _8 = HEAP[_2];
      (function() { try { return __ZN7hashsetI14hashtableentryIPKc5identEE5chainC1Ev(_8); __THREW__ = false } catch(e) { __THREW__ = true; } })(); if (!__THREW__) { __label__ = 137; break; } else { __label__ = 134; break; }
    case 137: // _invcont
      var _9 = HEAP[_2];
      var _10 = _9+13*1;
      HEAP[_2] = _10;
      _1 = _1 - 1;
      __label__ = 1; break;
    case 1: // _bb1
      var _14 = 0+(_1 != -1);
      if (_14) { __label__ = 0; break; } else { __label__ = 3; break; }
    case 3: // _bb2
      var _15 = HEAP[_3];
      HEAP[_retval_245] = _15;
      __label__ = 2; break;
    case 7: // _bb3
      _save_filt_350 = _eh_selector;
      var _eh_value = HEAP[_eh_exception];
      HEAP[_save_eptr_349] = _eh_value;
      var _16 = HEAP[_3];
      var _17 = 0+(_16 != 0);
      if (_17) { __label__ = 6; break; } else { __label__ = 5; break; }
    case 6: // _bb4
      var _19 = 63 - _1;
      var _20 = _19 * 52;
      var _21 = HEAP[_3];
      var _22 = _21;
      var _23 = _20 + _22;
      var _24 = _23;
      HEAP[_0] = _24;
      __label__ = 4; break;
    case 4: // _bb5
      var _25 = HEAP[_0];
      var _26 = HEAP[_3];
      var _27 = 0+(_25 == _26);
      if (_27) { __label__ = 5; break; } else { __label__ = 8; break; }
    case 8: // _bb6
      var _28 = HEAP[_0];
      var _29 = _28+13*-1;
      HEAP[_0] = _29;
      var _30 = HEAP[_0];
      __ZN7hashsetI14hashtableentryIPKc5identEE5chainD1Ev(_30);
      __label__ = 4; break;
    case 5: // _bb7
      var _31 = HEAP[_save_eptr_349];
      HEAP[_eh_exception] = _31;
      _eh_selector = _save_filt_350;
      __label__ = 26; break;
    case 2: // _return
      return;
    case 134: // _lpad
      var _eh_ptr = _llvm_eh_exception();
      HEAP[_eh_exception] = _eh_ptr;
      var _eh_ptr8 = HEAP[_eh_exception];
      _eh_selector = _llvm_eh_selector(_eh_ptr8, ___gxx_personality_v0, 0);
      __label__ = 27; break;
    case 27: // _ppad
      __label__ = 7; break;
    case 26: // _Unwind
      var _eh_ptr10 = HEAP[_eh_exception];
      __Unwind_Resume_or_Rethrow(_eh_ptr10);
      // unreachable
  }
}


function __ZN7hashsetI14hashtableentryIPKc5identEE6insertEj(_this, _h) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _this_addr = Pointer_make([0], 0);
      var _h_addr;
      var _retval = Pointer_make([0], 0);
      var _save_filt_348;
      var _save_eptr_347 = Pointer_make([0], 0);
      var _0 = Pointer_make([0], 0);
      var _1 = Pointer_make([0], 0);
      var _c = Pointer_make([0], 0);
      var _chunk = Pointer_make([0], 0);
      var _i;
      var _eh_exception = Pointer_make([0], 0);
      var _eh_selector;
      var __alloca_point_ = 0;
      HEAP[_this_addr] = _this;
      _h_addr = _h;
      var _2 = HEAP[_this_addr];
      var _4 = HEAP[0 + _2+4*1];
      var _5 = 0+(_4 == 0);
      if (_5) { __label__ = 0; break; } else { __label__ = 4; break; }
    case 0: // _bb
      var _6 = __Znwj(3332);
      HEAP[_1] = _6;
      var _8 = HEAP[_1];
      (function() { try { return __ZN7hashsetI14hashtableentryIPKc5identEE10chainchunkC1Ev(_8); __THREW__ = false } catch(e) { __THREW__ = true; } })(); if (!__THREW__) { __label__ = 137; break; } else { __label__ = 134; break; }
    case 137: // _invcont
      var _9 = HEAP[_1];
      HEAP[_chunk] = _9;
      var _10 = HEAP[_this_addr];
      var _12 = HEAP[0 + _10+3*1];
      var _13 = HEAP[_chunk];
      HEAP[0 + _13+__struct_hashset_hashtableentry_const_char___ident_____chainchunk____FLATTENER[1]] = _12;
      var _15 = HEAP[_this_addr];
      var _16 = _15+3*1;
      var _17 = HEAP[_chunk];
      HEAP[_16] = _17;
      _i = 0;
      __label__ = 7; break;
    case 1: // _bb1
      _save_filt_348 = _eh_selector;
      var _eh_value = HEAP[_eh_exception];
      HEAP[_save_eptr_347] = _eh_value;
      var _18 = HEAP[_1];
      __ZdlPv(_18);
      var _20 = HEAP[_save_eptr_347];
      HEAP[_eh_exception] = _20;
      _eh_selector = _save_filt_348;
      __label__ = 26; break;
    case 3: // _bb2
      var _22 = _i;
      var _24 = _i + 1;
      var _25 = HEAP[_chunk];
      var _26 = _25+__struct_hashset_hashtableentry_const_char___ident_____chainchunk____FLATTENER[0];
      var _27 = _26+_24*13;
      var _29 = _25+__struct_hashset_hashtableentry_const_char___ident_____chainchunk____FLATTENER[0];
      var _30 = _29+_22*13;
      HEAP[0 + _30+__struct_hashset_hashtableentry_const_char___ident_____chain____FLATTENER[1]] = _27;
      _i = _i + 1;
      __label__ = 7; break;
    case 7: // _bb3
      var _35 = 0+(_i <= 62);
      if (_35) { __label__ = 3; break; } else { __label__ = 6; break; }
    case 6: // _bb4
      var _36 = HEAP[_this_addr];
      var _38 = HEAP[0 + _36+4*1];
      var _39 = HEAP[_chunk];
      var _40 = _39+__struct_hashset_hashtableentry_const_char___ident_____chainchunk____FLATTENER[0];
      var _41 = _40+63*13;
      HEAP[0 + _41+__struct_hashset_hashtableentry_const_char___ident_____chain____FLATTENER[1]] = _38;
      var _43 = HEAP[_chunk];
      var _44 = _43+__struct_hashset_hashtableentry_const_char___ident_____chainchunk____FLATTENER[0];
      var _45 = _44+0*13;
      var _46 = HEAP[_this_addr];
      HEAP[0 + _46+4*1] = _45;
      __label__ = 4; break;
    case 4: // _bb5
      var _48 = HEAP[_this_addr];
      var _50 = HEAP[0 + _48+4*1];
      HEAP[_c] = _50;
      var _51 = HEAP[_this_addr];
      var _53 = HEAP[0 + _51+4*1];
      var _55 = HEAP[0 + _53+__struct_hashset_hashtableentry_const_char___ident_____chain____FLATTENER[1]];
      HEAP[0 + _51+4*1] = _55;
      var _58 = HEAP[_this_addr];
      var _60 = HEAP[0 + _58+2*1];
      var _63 = HEAP[0 + _60+_h_addr];
      var _64 = HEAP[_c];
      HEAP[0 + _64+__struct_hashset_hashtableentry_const_char___ident_____chain____FLATTENER[1]] = _63;
      var _66 = HEAP[_this_addr];
      var _68 = HEAP[0 + _66+2*1];
      var _70 = _68+_h_addr;
      var _71 = HEAP[_c];
      HEAP[_70] = _71;
      var _72 = HEAP[_this_addr];
      var _74 = HEAP[0 + _72+1*1];
      var _75 = _74 + 1;
      HEAP[0 + _72+1*1] = _75;
      var _78 = HEAP[_c];
      HEAP[_0] = _78;
      var _79 = HEAP[_0];
      HEAP[_retval] = _79;
      __label__ = 2; break;
    case 2: // _return
      var _retval6 = HEAP[_retval];
      return _retval6;
    case 134: // _lpad
      var _eh_ptr = _llvm_eh_exception();
      HEAP[_eh_exception] = _eh_ptr;
      var _eh_ptr7 = HEAP[_eh_exception];
      _eh_selector = _llvm_eh_selector(_eh_ptr7, ___gxx_personality_v0, 0);
      __label__ = 27; break;
    case 27: // _ppad
      __label__ = 1; break;
    case 26: // _Unwind
      var _eh_ptr9 = HEAP[_eh_exception];
      __Unwind_Resume_or_Rethrow(_eh_ptr9);
      // unreachable
  }
}


function __ZN9hashtableIPKc5identE6insertERKS1_j(_this, _key, _h) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _this_addr = Pointer_make([0], 0);
      var _key_addr = Pointer_make([0], 0);
      var _h_addr;
      var _retval = Pointer_make([0], 0);
      var _0 = Pointer_make([0], 0);
      var _c = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_this_addr] = _this;
      HEAP[_key_addr] = _key;
      _h_addr = _h;
      var _1 = HEAP[_this_addr];
      var _2 = _1+0*5;
      var _4 = __ZN7hashsetI14hashtableentryIPKc5identEE6insertEj(_2, _h_addr);
      HEAP[_c] = _4;
      var _5 = HEAP[_key_addr];
      var _6 = HEAP[_5];
      var _7 = HEAP[_c];
      var _8 = _7+__struct_hashset_hashtableentry_const_char___ident_____chain____FLATTENER[0];
      HEAP[0 + _8+__struct_hashtableentry_const_char__ident_____FLATTENER[0]] = _6;
      var _10 = HEAP[_c];
      var _11 = _10+__struct_hashset_hashtableentry_const_char___ident_____chain____FLATTENER[0];
      HEAP[_0] = _11;
      var _12 = HEAP[_0];
      HEAP[_retval] = _12;
      __label__ = 2; break;
    case 2: // _return
      var _retval1 = HEAP[_retval];
      return _retval1;
  }
}


function __ZN9hashtableIPKc5identE6accessERKS1_RKS2_(_this, _key, _data) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _this_addr = Pointer_make([0], 0);
      var _key_addr = Pointer_make([0], 0);
      var _data_addr = Pointer_make([0], 0);
      var _retval = Pointer_make([0], 0);
      var _0 = Pointer_make([0], 0);
      var _retval_228;
      var _h;
      var _c = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_this_addr] = _this;
      HEAP[_key_addr] = _key;
      HEAP[_data_addr] = _data;
      var _1 = HEAP[_key_addr];
      var _2 = HEAP[_1];
      var _3 = __ZL6hthashPKc(_2);
      var _4 = HEAP[_this_addr];
      var _5 = _4+0*5;
      var _7 = HEAP[0 + _5+0*1];
      var _8 = _7 - 1;
      _h = _3 & _8;
      var _11 = _4+0*5;
      var _13 = HEAP[0 + _11+2*1];
      var _16 = HEAP[0 + _13+_h];
      HEAP[_c] = _16;
      __label__ = 7; break;
    case 0: // _bb
      var _17 = HEAP[_c];
      var _18 = _17+__struct_hashset_hashtableentry_const_char___ident_____chain____FLATTENER[0];
      var _19 = HEAP[_key_addr];
      var _20 = HEAP[_19];
      _retval_228 = __Z5htcmpIcPKc5identEbPKT_RK14hashtableentryIT0_T1_E(_20, _18);
      var _toBool = 0+(_retval_228 != 0);
      if (_toBool) { __label__ = 1; break; } else { __label__ = 3; break; }
    case 1: // _bb1
      var _23 = HEAP[_c];
      var _24 = _23+__struct_hashset_hashtableentry_const_char___ident_____chain____FLATTENER[0];
      var _25 = _24+__struct_hashtableentry_const_char__ident_____FLATTENER[1];
      HEAP[_0] = _25;
      __label__ = 4; break;
    case 3: // _bb2
      var _26 = HEAP[_c];
      var _28 = HEAP[0 + _26+__struct_hashset_hashtableentry_const_char___ident_____chain____FLATTENER[1]];
      HEAP[_c] = _28;
      __label__ = 7; break;
    case 7: // _bb3
      var _29 = HEAP[_c];
      var _30 = 0+(_29 != 0);
      if (_30) { __label__ = 0; break; } else { __label__ = 6; break; }
    case 6: // _bb4
      var _31 = HEAP[_this_addr];
      var _32 = HEAP[_key_addr];
      var _34 = __ZN9hashtableIPKc5identE6insertERKS1_j(_31, _32, _h);
      var _35 = _34+__struct_hashtableentry_const_char__ident_____FLATTENER[1];
      var _36 = HEAP[_data_addr];
      var _37 = __ZN5identaSERKS_(_35, _36);
      HEAP[_0] = _37;
      __label__ = 4; break;
    case 4: // _bb5
      var _38 = HEAP[_0];
      HEAP[_retval] = _38;
      __label__ = 2; break;
    case 2: // _return
      var _retval6 = HEAP[_retval];
      return _retval6;
  }
}


function __Z8addidentPKcP5ident(_name, _id) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _name_addr = Pointer_make([0], 0);
      var _id_addr = Pointer_make([0], 0);
      var _save_filt_346;
      var _save_eptr_345 = Pointer_make([0], 0);
      var _0 = Pointer_make([0], 0);
      var _eh_exception = Pointer_make([0], 0);
      var _eh_selector;
      var __alloca_point_ = 0;
      HEAP[_name_addr] = _name;
      HEAP[_id_addr] = _id;
      var _1 = HEAP[_idents];
      var _2 = 0+(_1 == 0);
      if (_2) { __label__ = 0; break; } else { __label__ = 3; break; }
    case 0: // _bb
      var _3 = __Znwj(20);
      HEAP[_0] = _3;
      var _5 = HEAP[_0];
      (function() { try { return __ZN9hashtableIPKc5identEC1Ei(_5, 1024); __THREW__ = false } catch(e) { __THREW__ = true; } })(); if (!__THREW__) { __label__ = 137; break; } else { __label__ = 134; break; }
    case 137: // _invcont
      var _6 = HEAP[_0];
      HEAP[_idents] = _6;
      __label__ = 3; break;
    case 1: // _bb1
      _save_filt_346 = _eh_selector;
      var _eh_value = HEAP[_eh_exception];
      HEAP[_save_eptr_345] = _eh_value;
      var _7 = HEAP[_0];
      __ZdlPv(_7);
      var _9 = HEAP[_save_eptr_345];
      HEAP[_eh_exception] = _9;
      _eh_selector = _save_filt_346;
      __label__ = 26; break;
    case 3: // _bb2
      var _11 = HEAP[_idents];
      var _12 = HEAP[_id_addr];
      var _13 = __ZN9hashtableIPKc5identE6accessERKS1_RKS2_(_11, _name_addr, _12);
      __label__ = 2; break;
    case 2: // _return
      return;
    case 134: // _lpad
      var _eh_ptr = _llvm_eh_exception();
      HEAP[_eh_exception] = _eh_ptr;
      var _eh_ptr3 = HEAP[_eh_exception];
      _eh_selector = _llvm_eh_selector(_eh_ptr3, ___gxx_personality_v0, 0);
      __label__ = 27; break;
    case 27: // _ppad
      __label__ = 1; break;
    case 26: // _Unwind
      var _eh_ptr5 = HEAP[_eh_exception];
      __Unwind_Resume_or_Rethrow(_eh_ptr5);
      // unreachable
  }
}


function __Z6aliasaPKcPc(_name, _action) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _name_addr = Pointer_make([0], 0);
      var _action_addr = Pointer_make([0], 0);
      var _iftmp_41;
      var _b = Pointer_make([0], 0);
      var _b1 = Pointer_make([0,0,0,0,0,0,0,0,0,0,0], 0);
      var __alloca_point_ = 0;
      HEAP[_name_addr] = _name;
      HEAP[_action_addr] = _action;
      var _0 = HEAP[_idents];
      var _1 = __ZN9hashtableIPKc5identE6accessERKS1_(_0, _name_addr);
      HEAP[_b] = _1;
      var _2 = HEAP[_b];
      var _3 = 0+(_2 == 0);
      if (_3) { __label__ = 0; break; } else { __label__ = 10; break; }
    case 0: // _bb
      var _4 = HEAP[_persistidents];
      var _toBool = 0+(_4 != 0);
      if (_toBool) { __label__ = 3; break; } else { __label__ = 7; break; }
    case 3: // _bb2
      _iftmp_41 = 1;
      __label__ = 6; break;
    case 7: // _bb3
      _iftmp_41 = 0;
      __label__ = 6; break;
    case 6: // _bb4
      var _5 = HEAP[_name_addr];
      var _6 = __Z9newstringPKc(_5);
      var _7 = HEAP[_action_addr];
      __ZN5identC1EiPKcPci(_b1, 5, _6, _7, _iftmp_41);
      var _9 = HEAP[_overrideidents];
      var _toBool5 = 0+(_9 != 0);
      if (_toBool5) { __label__ = 8; break; } else { __label__ = 5; break; }
    case 8: // _bb6
      HEAP[0 + _b1+5*1] = 0;
      __label__ = 5; break;
    case 5: // _bb7
      var _11 = HEAP[_idents];
      var _12 = _b1+2*1;
      var _13 = __ZN9hashtableIPKc5identE6accessERKS1_RKS2_(_11, _12, _b1);
      __label__ = 31; break;
    case 10: // _bb8
      var _14 = HEAP[_b];
      var _16 = HEAP[0 + _14+1*1];
      var _17 = 0+(_16 != 5);
      if (_17) { __label__ = 9; break; } else { __label__ = 14; break; }
    case 9: // _bb9
      var _18 = HEAP[_name_addr];
      __Z7conoutfiPKcz(4, __str16+0*1, _18);
      var _19 = HEAP[_action_addr];
      var _20 = 0+(_19 != 0);
      if (_20) { __label__ = 11; break; } else { __label__ = 13; break; }
    case 11: // _bb10
      var _21 = HEAP[_action_addr];
      __ZdaPv(_21);
      __label__ = 13; break;
    case 13: // _bb11
      __label__ = 31; break;
    case 14: // _bb12
      var _22 = HEAP[_b];
      var _23 = _22+7*1;
      var _25 = HEAP[0 + _23+0*1];
      var _27 = _22+8*1;
      var _29 = HEAP[0 + _27+0*1];
      var _30 = 0+(_25 != _29);
      if (_30) { __label__ = 15; break; } else { __label__ = 21; break; }
    case 15: // _bb13
      var _31 = HEAP[_b];
      var _32 = _31+7*1;
      var _34 = HEAP[0 + _32+0*1];
      var _35 = 0+(_34 != 0);
      if (_35) { __label__ = 12; break; } else { __label__ = 21; break; }
    case 12: // _bb14
      var _36 = HEAP[_b];
      var _37 = _36+7*1;
      var _39 = HEAP[0 + _37+0*1];
      __ZdaPv(_39);
      __label__ = 21; break;
    case 21: // _bb15
      var _40 = HEAP[_b];
      var _41 = _40+7*1;
      var _42 = _41+0*1;
      var _43 = HEAP[_action_addr];
      HEAP[_42] = _43;
      var _44 = HEAP[_overrideidents];
      var _toBool16 = 0+(_44 != 0);
      if (_toBool16) { __label__ = 20; break; } else { __label__ = 17; break; }
    case 20: // _bb17
      var _45 = HEAP[_b];
      HEAP[0 + _45+5*1] = 0;
      __label__ = 31; break;
    case 17: // _bb18
      var _47 = HEAP[_b];
      var _49 = HEAP[0 + _47+5*1];
      var _50 = 0+(_49 != 2147483647);
      if (_50) { __label__ = 18; break; } else { __label__ = 19; break; }
    case 18: // _bb19
      var _51 = HEAP[_b];
      HEAP[0 + _51+5*1] = 2147483647;
      __label__ = 19; break;
    case 19: // _bb20
      var _53 = HEAP[_persistidents];
      var _toBool21 = 0+(_53 != 0);
      if (_toBool21) { __label__ = 25; break; } else { __label__ = 32; break; }
    case 25: // _bb22
      var _54 = HEAP[_b];
      var _56 = HEAP[0 + _54+10*1];
      var _57 = _56 & 1;
      var _58 = _57;
      var _toBool23 = 0+(_58 != 0);
      var _toBool23not = _toBool23 ^ true;
      var _toBool23not24 = _toBool23not;
      var _toBool26 = 0+(_toBool23not24 != 0);
      if (_toBool26) { __label__ = 33; break; } else { __label__ = 34; break; }
    case 33: // _bb27
      var _59 = HEAP[_b];
      var _61 = HEAP[0 + _59+10*1];
      var _62 = _61 | 1;
      HEAP[0 + _59+10*1] = _62;
      __label__ = 34; break;
    case 34: // _bb28
      __label__ = 31; break;
    case 32: // _bb29
      var _65 = HEAP[_b];
      var _67 = HEAP[0 + _65+10*1];
      var _68 = _67 & 1;
      var _69 = _68;
      var _toBool30 = 0+(_69 != 0);
      if (_toBool30) { __label__ = 35; break; } else { __label__ = 31; break; }
    case 35: // _bb31
      var _70 = HEAP[_b];
      var _72 = HEAP[0 + _70+10*1];
      var _73 = _72 & -2;
      HEAP[0 + _70+10*1] = _73;
      __label__ = 31; break;
    case 31: // _bb32
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __Z5aliasPKcS0_(_name, _action) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _name_addr = Pointer_make([0], 0);
      var _action_addr = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_name_addr] = _name;
      HEAP[_action_addr] = _action;
      var _0 = HEAP[_action_addr];
      var _1 = __Z9newstringPKc(_0);
      var _2 = HEAP[_name_addr];
      __Z6aliasaPKcPc(_2, _1);
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __Z8newidentPKc(_name) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _name_addr = Pointer_make([0], 0);
      var _retval = Pointer_make([0], 0);
      var _0 = Pointer_make([0], 0);
      var _iftmp_34;
      var _id = Pointer_make([0], 0);
      var _init = Pointer_make([0,0,0,0,0,0,0,0,0,0,0], 0);
      var __alloca_point_ = 0;
      HEAP[_name_addr] = _name;
      var _1 = HEAP[_idents];
      var _2 = __ZN9hashtableIPKc5identE6accessERKS1_(_1, _name_addr);
      HEAP[_id] = _2;
      var _3 = HEAP[_id];
      var _4 = 0+(_3 == 0);
      if (_4) { __label__ = 0; break; } else { __label__ = 6; break; }
    case 0: // _bb
      var _5 = HEAP[_persistidents];
      var _toBool = 0+(_5 != 0);
      if (_toBool) { __label__ = 1; break; } else { __label__ = 3; break; }
    case 1: // _bb1
      _iftmp_34 = 1;
      __label__ = 7; break;
    case 3: // _bb2
      _iftmp_34 = 0;
      __label__ = 7; break;
    case 7: // _bb3
      var _6 = __Z9newstringPKc(__str3+0*1);
      var _7 = HEAP[_name_addr];
      var _8 = __Z9newstringPKc(_7);
      __ZN5identC1EiPKcPci(_init, 5, _8, _6, _iftmp_34);
      var _10 = HEAP[_idents];
      var _11 = _init+2*1;
      var _12 = __ZN9hashtableIPKc5identE6accessERKS1_RKS2_(_10, _11, _init);
      HEAP[_id] = _12;
      __label__ = 6; break;
    case 6: // _bb4
      var _13 = HEAP[_id];
      HEAP[_0] = _13;
      var _14 = HEAP[_0];
      HEAP[_retval] = _14;
      __label__ = 2; break;
    case 2: // _return
      var _retval5 = HEAP[_retval];
      return _retval5;
  }
}


function __Z5pushaPKcPc(_name, _action) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _name_addr = Pointer_make([0], 0);
      var _action_addr = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_name_addr] = _name;
      HEAP[_action_addr] = _action;
      var _0 = HEAP[_name_addr];
      var _1 = __Z8newidentPKc(_0);
      var _2 = HEAP[_action_addr];
      __Z9pushidentR5identPc(_1, _2);
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __Z4pushPcS_(_name, _action) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _name_addr = Pointer_make([0], 0);
      var _action_addr = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_name_addr] = _name;
      HEAP[_action_addr] = _action;
      var _0 = HEAP[_action_addr];
      var _1 = __Z9newstringPKc(_0);
      var _2 = HEAP[_name_addr];
      __Z5pushaPKcPc(_2, _1);
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __ZN7hashsetI14hashtableentryIPKc5identEE10chainchunkD1Ev(_this) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _this_addr = Pointer_make([0], 0);
      var _0 = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_this_addr] = _this;
      var _1 = HEAP[_this_addr];
      var _2 = _1+__struct_hashset_hashtableentry_const_char___ident_____chainchunk____FLATTENER[0];
      var _3 = 0+(_2 != 0);
      if (_3) { __label__ = 0; break; } else { __label__ = 7; break; }
    case 0: // _bb
      var _4 = HEAP[_this_addr];
      var _5 = _4+__struct_hashset_hashtableentry_const_char___ident_____chainchunk____FLATTENER[0];
      var _6 = _5+832*1;
      HEAP[_0] = _6;
      __label__ = 1; break;
    case 1: // _bb1
      var _8 = HEAP[_this_addr];
      var _9 = _8+__struct_hashset_hashtableentry_const_char___ident_____chainchunk____FLATTENER[0];
      var _10 = _9+0*13;
      var _11 = HEAP[_0];
      var _12 = 0+(_11 == _10);
      if (_12) { __label__ = 7; break; } else { __label__ = 3; break; }
    case 3: // _bb2
      var _13 = HEAP[_0];
      var _14 = _13+13*-1;
      HEAP[_0] = _14;
      var _15 = HEAP[_0];
      __ZN7hashsetI14hashtableentryIPKc5identEE5chainD1Ev(_15);
      __label__ = 1; break;
    case 7: // _bb3
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __Z10addcommandPKcPFvvES0_(_name, _fun, _narg) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _name_addr = Pointer_make([0], 0);
      var _fun_addr = Pointer_make([0], 0);
      var _narg_addr = Pointer_make([0], 0);
      var _retval;
      var _save_filt_352;
      var _save_eptr_351 = Pointer_make([0], 0);
      var _0;
      var _1 = Pointer_make([0], 0);
      var _c = Pointer_make([0,0,0,0,0,0,0,0,0,0,0], 0);
      var _eh_exception = Pointer_make([0], 0);
      var _eh_selector;
      var __alloca_point_ = 0;
      HEAP[_name_addr] = _name;
      HEAP[_fun_addr] = _fun;
      HEAP[_narg_addr] = _narg;
      var _2 = HEAP[_idents];
      var _3 = 0+(_2 == 0);
      if (_3) { __label__ = 0; break; } else { __label__ = 3; break; }
    case 0: // _bb
      var _4 = __Znwj(20);
      HEAP[_1] = _4;
      var _6 = HEAP[_1];
      (function() { try { return __ZN9hashtableIPKc5identEC1Ei(_6, 1024); __THREW__ = false } catch(e) { __THREW__ = true; } })(); if (!__THREW__) { __label__ = 137; break; } else { __label__ = 134; break; }
    case 137: // _invcont
      var _7 = HEAP[_1];
      HEAP[_idents] = _7;
      __label__ = 3; break;
    case 1: // _bb1
      _save_filt_352 = _eh_selector;
      var _eh_value = HEAP[_eh_exception];
      HEAP[_save_eptr_351] = _eh_value;
      var _8 = HEAP[_1];
      __ZdlPv(_8);
      var _10 = HEAP[_save_eptr_351];
      HEAP[_eh_exception] = _10;
      _eh_selector = _save_filt_352;
      __label__ = 26; break;
    case 3: // _bb2
      var _12 = HEAP[_name_addr];
      var _13 = HEAP[_narg_addr];
      var _14 = HEAP[_fun_addr];
      __ZN5identC1EiPKcS1_PvS2_i(_c, 3, _12, _13, _14, 0, 0);
      var _16 = HEAP[_idents];
      var _17 = __ZN9hashtableIPKc5identE6accessERKS1_RKS2_(_16, _name_addr, _c);
      _0 = 0;
      _retval = _0;
      __label__ = 2; break;
    case 2: // _return
      var _retval34 = _retval;
      return _retval34;
    case 134: // _lpad
      var _eh_ptr = _llvm_eh_exception();
      HEAP[_eh_exception] = _eh_ptr;
      var _eh_ptr5 = HEAP[_eh_exception];
      _eh_selector = _llvm_eh_selector(_eh_ptr5, ___gxx_personality_v0, 0);
      __label__ = 27; break;
    case 27: // _ppad
      __label__ = 1; break;
    case 26: // _Unwind
      var _eh_ptr7 = HEAP[_eh_exception];
      __Unwind_Resume_or_Rethrow(_eh_ptr7);
      // unreachable
  }
}


function __Z9svariablePKcS0_PPcPFvvEi(_name, _cur, _storage, _fun, _flags) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _name_addr = Pointer_make([0], 0);
      var _cur_addr = Pointer_make([0], 0);
      var _storage_addr = Pointer_make([0], 0);
      var _fun_addr = Pointer_make([0], 0);
      var _flags_addr;
      var _retval = Pointer_make([0], 0);
      var _save_filt_354;
      var _save_eptr_353 = Pointer_make([0], 0);
      var _0 = Pointer_make([0], 0);
      var _1 = Pointer_make([0], 0);
      var _v = Pointer_make([0,0,0,0,0,0,0,0,0,0,0], 0);
      var _eh_exception = Pointer_make([0], 0);
      var _eh_selector;
      var __alloca_point_ = 0;
      HEAP[_name_addr] = _name;
      HEAP[_cur_addr] = _cur;
      HEAP[_storage_addr] = _storage;
      HEAP[_fun_addr] = _fun;
      _flags_addr = _flags;
      var _2 = HEAP[_idents];
      var _3 = 0+(_2 == 0);
      if (_3) { __label__ = 0; break; } else { __label__ = 3; break; }
    case 0: // _bb
      var _4 = __Znwj(20);
      HEAP[_1] = _4;
      var _6 = HEAP[_1];
      (function() { try { return __ZN9hashtableIPKc5identEC1Ei(_6, 1024); __THREW__ = false } catch(e) { __THREW__ = true; } })(); if (!__THREW__) { __label__ = 137; break; } else { __label__ = 134; break; }
    case 137: // _invcont
      var _7 = HEAP[_1];
      HEAP[_idents] = _7;
      __label__ = 3; break;
    case 1: // _bb1
      _save_filt_354 = _eh_selector;
      var _eh_value = HEAP[_eh_exception];
      HEAP[_save_eptr_353] = _eh_value;
      var _8 = HEAP[_1];
      __ZdlPv(_8);
      var _10 = HEAP[_save_eptr_353];
      HEAP[_eh_exception] = _10;
      _eh_selector = _save_filt_354;
      __label__ = 26; break;
    case 3: // _bb2
      var _12 = HEAP[_cur_addr];
      var _13 = __Z9newstringPKc(_12);
      var _14 = HEAP[_name_addr];
      var _15 = HEAP[_storage_addr];
      var _16 = HEAP[_fun_addr];
      var _17 = _16;
      __ZN5identC1EiPKcPcPS2_Pvi(_v, 2, _14, _13, _15, _17, _flags_addr);
      var _19 = HEAP[_idents];
      var _20 = __ZN9hashtableIPKc5identE6accessERKS1_RKS2_(_19, _name_addr, _v);
      var _21 = _v+7*1;
      var _22 = _21+0*1;
      var _24 = _22;
      var _26 = HEAP[_24];
      HEAP[_0] = _26;
      var _27 = HEAP[_0];
      HEAP[_retval] = _27;
      __label__ = 2; break;
    case 2: // _return
      var _retval3 = HEAP[_retval];
      return _retval3;
    case 134: // _lpad
      var _eh_ptr = _llvm_eh_exception();
      HEAP[_eh_exception] = _eh_ptr;
      var _eh_ptr4 = HEAP[_eh_exception];
      _eh_selector = _llvm_eh_selector(_eh_ptr4, ___gxx_personality_v0, 0);
      __label__ = 27; break;
    case 27: // _ppad
      __label__ = 1; break;
    case 26: // _Unwind
      var _eh_ptr6 = HEAP[_eh_exception];
      __Unwind_Resume_or_Rethrow(_eh_ptr6);
      // unreachable
  }
}


function __Z9fvariablePKcfffPfPFvvEi(_name, _min, _cur, _max, _storage, _fun, _flags) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _name_addr = Pointer_make([0], 0);
      var _min_addr;
      var _cur_addr;
      var _max_addr;
      var _storage_addr = Pointer_make([0], 0);
      var _fun_addr = Pointer_make([0], 0);
      var _flags_addr;
      var _retval;
      var _save_filt_356;
      var _save_eptr_355 = Pointer_make([0], 0);
      var _0;
      var _1 = Pointer_make([0], 0);
      var _v = Pointer_make([0,0,0,0,0,0,0,0,0,0,0], 0);
      var _eh_exception = Pointer_make([0], 0);
      var _eh_selector;
      var __alloca_point_ = 0;
      HEAP[_name_addr] = _name;
      _min_addr = _min;
      _cur_addr = _cur;
      _max_addr = _max;
      HEAP[_storage_addr] = _storage;
      HEAP[_fun_addr] = _fun;
      _flags_addr = _flags;
      var _2 = HEAP[_idents];
      var _3 = 0+(_2 == 0);
      if (_3) { __label__ = 0; break; } else { __label__ = 3; break; }
    case 0: // _bb
      var _4 = __Znwj(20);
      HEAP[_1] = _4;
      var _6 = HEAP[_1];
      (function() { try { return __ZN9hashtableIPKc5identEC1Ei(_6, 1024); __THREW__ = false } catch(e) { __THREW__ = true; } })(); if (!__THREW__) { __label__ = 137; break; } else { __label__ = 134; break; }
    case 137: // _invcont
      var _7 = HEAP[_1];
      HEAP[_idents] = _7;
      __label__ = 3; break;
    case 1: // _bb1
      _save_filt_356 = _eh_selector;
      var _eh_value = HEAP[_eh_exception];
      HEAP[_save_eptr_355] = _eh_value;
      var _8 = HEAP[_1];
      __ZdlPv(_8);
      var _10 = HEAP[_save_eptr_355];
      HEAP[_eh_exception] = _10;
      _eh_selector = _save_filt_356;
      __label__ = 26; break;
    case 3: // _bb2
      var _12 = HEAP[_name_addr];
      var _13 = _min_addr;
      var _14 = _cur_addr;
      var _15 = _max_addr;
      var _16 = HEAP[_storage_addr];
      var _17 = HEAP[_fun_addr];
      var _18 = _17;
      __ZN5identC1EiPKcfffPfPvi(_v, 1, _12, _13, _14, _15, _16, _18, _flags_addr);
      var _20 = HEAP[_idents];
      var _21 = __ZN9hashtableIPKc5identE6accessERKS1_RKS2_(_20, _name_addr, _v);
      _0 = _cur_addr;
      _retval = _0;
      __label__ = 2; break;
    case 2: // _return
      var _retval3 = _retval;
      return _retval3;
    case 134: // _lpad
      var _eh_ptr = _llvm_eh_exception();
      HEAP[_eh_exception] = _eh_ptr;
      var _eh_ptr4 = HEAP[_eh_exception];
      _eh_selector = _llvm_eh_selector(_eh_ptr4, ___gxx_personality_v0, 0);
      __label__ = 27; break;
    case 27: // _ppad
      __label__ = 1; break;
    case 26: // _Unwind
      var _eh_ptr6 = HEAP[_eh_exception];
      __Unwind_Resume_or_Rethrow(_eh_ptr6);
      // unreachable
  }
}


function __Z8variablePKciiiPiPFvvEi(_name, _min, _cur, _max, _storage, _fun, _flags) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _name_addr = Pointer_make([0], 0);
      var _min_addr;
      var _cur_addr;
      var _max_addr;
      var _storage_addr = Pointer_make([0], 0);
      var _fun_addr = Pointer_make([0], 0);
      var _flags_addr;
      var _retval;
      var _save_filt_358;
      var _save_eptr_357 = Pointer_make([0], 0);
      var _0;
      var _1 = Pointer_make([0], 0);
      var _v = Pointer_make([0,0,0,0,0,0,0,0,0,0,0], 0);
      var _eh_exception = Pointer_make([0], 0);
      var _eh_selector;
      var __alloca_point_ = 0;
      HEAP[_name_addr] = _name;
      _min_addr = _min;
      _cur_addr = _cur;
      _max_addr = _max;
      HEAP[_storage_addr] = _storage;
      HEAP[_fun_addr] = _fun;
      _flags_addr = _flags;
      var _2 = HEAP[_idents];
      var _3 = 0+(_2 == 0);
      if (_3) { __label__ = 0; break; } else { __label__ = 3; break; }
    case 0: // _bb
      var _4 = __Znwj(20);
      HEAP[_1] = _4;
      var _6 = HEAP[_1];
      (function() { try { return __ZN9hashtableIPKc5identEC1Ei(_6, 1024); __THREW__ = false } catch(e) { __THREW__ = true; } })(); if (!__THREW__) { __label__ = 137; break; } else { __label__ = 134; break; }
    case 137: // _invcont
      var _7 = HEAP[_1];
      HEAP[_idents] = _7;
      __label__ = 3; break;
    case 1: // _bb1
      _save_filt_358 = _eh_selector;
      var _eh_value = HEAP[_eh_exception];
      HEAP[_save_eptr_357] = _eh_value;
      var _8 = HEAP[_1];
      __ZdlPv(_8);
      var _10 = HEAP[_save_eptr_357];
      HEAP[_eh_exception] = _10;
      _eh_selector = _save_filt_358;
      __label__ = 26; break;
    case 3: // _bb2
      var _12 = HEAP[_name_addr];
      var _13 = _min_addr;
      var _14 = _cur_addr;
      var _15 = _max_addr;
      var _16 = HEAP[_storage_addr];
      var _17 = HEAP[_fun_addr];
      var _18 = _17;
      __ZN5identC1EiPKciiiPiPvi(_v, 0, _12, _13, _14, _15, _16, _18, _flags_addr);
      var _20 = HEAP[_idents];
      var _21 = __ZN9hashtableIPKc5identE6accessERKS1_RKS2_(_20, _name_addr, _v);
      _0 = _cur_addr;
      _retval = _0;
      __label__ = 2; break;
    case 2: // _return
      var _retval3 = _retval;
      return _retval3;
    case 134: // _lpad
      var _eh_ptr = _llvm_eh_exception();
      HEAP[_eh_exception] = _eh_ptr;
      var _eh_ptr4 = HEAP[_eh_exception];
      _eh_selector = _llvm_eh_selector(_eh_ptr4, ___gxx_personality_v0, 0);
      __label__ = 27; break;
    case 27: // _ppad
      __label__ = 1; break;
    case 26: // _Unwind
      var _eh_ptr6 = HEAP[_eh_exception];
      __Unwind_Resume_or_Rethrow(_eh_ptr6);
      // unreachable
  }
}


function __Z41__static_initialization_and_destruction_0ii(___initialize_p, ___priority) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var ___initialize_p_addr;
      var ___priority_addr;
      var __alloca_point_ = 0;
      ___initialize_p_addr = ___initialize_p;
      ___priority_addr = ___priority;
      var _1 = 0+(___initialize_p_addr == 1);
      if (_1) { __label__ = 0; break; } else { __label__ = 3; break; }
    case 0: // _bb
      var _3 = 0+(___priority_addr == 65535);
      if (_3) { __label__ = 1; break; } else { __label__ = 3; break; }
    case 1: // _bb1
      var _4 = __Z10addcommandPKcPFvvES0_(__str17+0*1, __Z4pushPcS_, __str18+0*1);
      HEAP[__ZL12__dummy_push] = _4;
      var _5 = __Z10addcommandPKcPFvvES0_(__str19+0*1, __Z3popPc, __str20+0*1);
      HEAP[__ZL11__dummy_pop] = _5;
      var _6 = __Z10addcommandPKcPFvvES0_(__str21+0*1, __Z8resetvarPc, __str20+0*1);
      HEAP[__ZL16__dummy_resetvar] = _6;
      var _7 = __Z10addcommandPKcPFvvES0_(__str22+0*1, __Z5aliasPKcS0_, __str18+0*1);
      HEAP[__ZL13__dummy_alias] = _7;
      __ZN6vectorIPS_IcEEC1Ev(__ZL8wordbufs);
      var _8 = ___cxa_atexit(___tcf_2, 0, ___dso_handle);
      var _9 = __Z8variablePKciiiPiPFvvEi(__str23+0*1, 25, 0, 0, __numargs, 0, 0);
      HEAP[__numargs] = _9;
      var _10 = __Z10addcommandPKcPFvvES0_(__str24+0*1, __ZN7_stdcmdILi846EE3runEPcS1_S1_, __str25+0*1);
      HEAP[__ZN7_stdcmdILi846EE4initE] = _10;
      var _11 = __Z10addcommandPKcPFvvES0_(__str26+0*1, __ZN7_stdcmdILi847EE3runEPcS1_S1_, __str25+0*1);
      HEAP[__ZN7_stdcmdILi847EE4initE] = _11;
      var _12 = __Z10addcommandPKcPFvvES0_(__str27+0*1, __ZN7_stdcmdILi860EE3runEPcPiS1_, __str28+0*1);
      HEAP[__ZN7_stdcmdILi860EE4initE] = _12;
      var _13 = __Z10addcommandPKcPFvvES0_(__str29+0*1, __ZN7_stdcmdILi874EE3runEPcPiS1_S1_, __str30+0*1);
      HEAP[__ZN7_stdcmdILi874EE4initE] = _13;
      var _14 = __Z10addcommandPKcPFvvES0_(__str31+0*1, __ZN7_stdcmdILi875EE3runEPcS1_, __str18+0*1);
      HEAP[__ZN7_stdcmdILi875EE4initE] = _14;
      var _15 = __Z10addcommandPKcPFvvES0_(__str32+0*1, __Z6concatPKc, __str33+0*1);
      HEAP[__ZL14__dummy_concat] = _15;
      var _16 = __Z10addcommandPKcPFvvES0_(__str34+0*1, __Z6resultPKc, __str20+0*1);
      HEAP[__ZL14__dummy_result] = _16;
      var _17 = __Z10addcommandPKcPFvvES0_(__str35+0*1, __Z10concatwordPPcPi, __str36+0*1);
      HEAP[__ZL18__dummy_concatword] = _17;
      var _18 = __Z10addcommandPKcPFvvES0_(__str37+0*1, __Z6formatPPcPi, __str36+0*1);
      HEAP[__ZL14__dummy_format] = _18;
      var _19 = __Z10addcommandPKcPFvvES0_(__str38+0*1, __Z2atPcPi, __str39+0*1);
      HEAP[__ZL10__dummy_at] = _19;
      var _20 = __Z10addcommandPKcPFvvES0_(__str40+0*1, __Z6substrPcPiS_, __str28+0*1);
      HEAP[__ZL14__dummy_substr] = _20;
      var _21 = __Z10addcommandPKcPFvvES0_(__str41+0*1, __ZN7_stdcmdILi973EE3runEPc, __str20+0*1);
      HEAP[__ZN7_stdcmdILi973EE4initE] = _21;
      var _22 = __Z10addcommandPKcPFvvES0_(__str42+0*1, __Z9getalias_Pc, __str20+0*1);
      HEAP[__ZL17__dummy_getalias_] = _22;
      var _23 = __Z10addcommandPKcPFvvES0_(__str43+0*1, __Z10prettylistPKcS0_, __str18+0*1);
      HEAP[__ZL18__dummy_prettylist] = _23;
      var _24 = __Z10addcommandPKcPFvvES0_(__str44+0*1, __ZN7_stdcmdILi1070EE3runEPcS1_, __str18+0*1);
      HEAP[__ZN7_stdcmdILi1070EE4initE] = _24;
      var _25 = __Z10addcommandPKcPFvvES0_(__str45+0*1, __ZN7_stdcmdILi1071EE3runEPcS1_, __str18+0*1);
      HEAP[__ZN7_stdcmdILi1071EE4initE] = _25;
      var _26 = __Z10addcommandPKcPFvvES0_(__str46+0*1, __ZN7_stdcmdILi1072EE3runEPcS1_S1_, __str25+0*1);
      HEAP[__ZN7_stdcmdILi1072EE4initE] = _26;
      var _27 = __Z10addcommandPKcPFvvES0_(__str47+0*1, __ZN7_stdcmdILi1073EE3runEPcS1_S1_, __str25+0*1);
      HEAP[__ZN7_stdcmdILi1073EE4initE] = _27;
      var _28 = __Z10addcommandPKcPFvvES0_(__str48+0*1, __ZN7_stdcmdILi1075EE3runEPiS1_, __str49+0*1);
      HEAP[__ZN7_stdcmdILi1075EE4initE] = _28;
      var _29 = __Z10addcommandPKcPFvvES0_(__str50+0*1, __ZN7_stdcmdILi1076EE3runEPiS1_, __str49+0*1);
      HEAP[__ZN7_stdcmdILi1076EE4initE] = _29;
      var _30 = __Z10addcommandPKcPFvvES0_(__str51+0*1, __ZN7_stdcmdILi1077EE3runEPiS1_, __str49+0*1);
      HEAP[__ZN7_stdcmdILi1077EE4initE] = _30;
      var _31 = __Z10addcommandPKcPFvvES0_(__str52+0*1, __ZN7_stdcmdILi1078EE3runEPfS1_, __str53+0*1);
      HEAP[__ZN7_stdcmdILi1078EE4initE] = _31;
      var _32 = __Z10addcommandPKcPFvvES0_(__str54+0*1, __ZN7_stdcmdILi1079EE3runEPfS1_, __str53+0*1);
      HEAP[__ZN7_stdcmdILi1079EE4initE] = _32;
      var _33 = __Z10addcommandPKcPFvvES0_(__str55+0*1, __ZN7_stdcmdILi1080EE3runEPfS1_, __str53+0*1);
      HEAP[__ZN7_stdcmdILi1080EE4initE] = _33;
      var _34 = __Z10addcommandPKcPFvvES0_(__str56+0*1, __ZN7_stdcmdILi1081EE3runEPiS1_, __str49+0*1);
      HEAP[__ZN7_stdcmdILi1081EE4initE] = _34;
      var _35 = __Z10addcommandPKcPFvvES0_(__str57+0*1, __ZN7_stdcmdILi1082EE3runEPiS1_, __str49+0*1);
      HEAP[__ZN7_stdcmdILi1082EE4initE] = _35;
      var _36 = __Z10addcommandPKcPFvvES0_(__str58+0*1, __ZN7_stdcmdILi1083EE3runEPiS1_, __str49+0*1);
      HEAP[__ZN7_stdcmdILi1083EE4initE] = _36;
      var _37 = __Z10addcommandPKcPFvvES0_(__str59+0*1, __ZN7_stdcmdILi1084EE3runEPiS1_, __str49+0*1);
      HEAP[__ZN7_stdcmdILi1084EE4initE] = _37;
      var _38 = __Z10addcommandPKcPFvvES0_(__str60+0*1, __ZN7_stdcmdILi1085EE3runEPiS1_, __str49+0*1);
      HEAP[__ZN7_stdcmdILi1085EE4initE] = _38;
      var _39 = __Z10addcommandPKcPFvvES0_(__str61+0*1, __ZN7_stdcmdILi1086EE3runEPiS1_, __str49+0*1);
      HEAP[__ZN7_stdcmdILi1086EE4initE] = _39;
      var _40 = __Z10addcommandPKcPFvvES0_(__str62+0*1, __ZN7_stdcmdILi1087EE3runEPfS1_, __str53+0*1);
      HEAP[__ZN7_stdcmdILi1087EE4initE] = _40;
      var _41 = __Z10addcommandPKcPFvvES0_(__str63+0*1, __ZN7_stdcmdILi1088EE3runEPfS1_, __str53+0*1);
      HEAP[__ZN7_stdcmdILi1088EE4initE] = _41;
      var _42 = __Z10addcommandPKcPFvvES0_(__str64+0*1, __ZN7_stdcmdILi1089EE3runEPfS1_, __str53+0*1);
      HEAP[__ZN7_stdcmdILi1089EE4initE] = _42;
      var _43 = __Z10addcommandPKcPFvvES0_(__str65+0*1, __ZN7_stdcmdILi1090EE3runEPfS1_, __str53+0*1);
      HEAP[__ZN7_stdcmdILi1090EE4initE] = _43;
      var _44 = __Z10addcommandPKcPFvvES0_(__str66+0*1, __ZN7_stdcmdILi1091EE3runEPfS1_, __str53+0*1);
      HEAP[__ZN7_stdcmdILi1091EE4initE] = _44;
      var _45 = __Z10addcommandPKcPFvvES0_(__str67+0*1, __ZN7_stdcmdILi1092EE3runEPfS1_, __str53+0*1);
      HEAP[__ZN7_stdcmdILi1092EE4initE] = _45;
      var _46 = __Z10addcommandPKcPFvvES0_(__str68+0*1, __ZN7_stdcmdILi1093EE3runEPiS1_, __str49+0*1);
      HEAP[__ZN7_stdcmdILi1093EE4initE] = _46;
      var _47 = __Z10addcommandPKcPFvvES0_(__str69+0*1, __ZN7_stdcmdILi1094EE3runEPi, __str70+0*1);
      HEAP[__ZN7_stdcmdILi1094EE4initE] = _47;
      var _48 = __Z10addcommandPKcPFvvES0_(__str71+0*1, __ZN7_stdcmdILi1095EE3runEPiS1_, __str49+0*1);
      HEAP[__ZN7_stdcmdILi1095EE4initE] = _48;
      var _49 = __Z10addcommandPKcPFvvES0_(__str72+0*1, __ZN7_stdcmdILi1096EE3runEPiS1_, __str49+0*1);
      HEAP[__ZN7_stdcmdILi1096EE4initE] = _49;
      var _50 = __Z10addcommandPKcPFvvES0_(__str73+0*1, __ZN7_stdcmdILi1097EE3runEPi, __str70+0*1);
      HEAP[__ZN7_stdcmdILi1097EE4initE] = _50;
      var _51 = __Z10addcommandPKcPFvvES0_(__str74+0*1, __ZN7_stdcmdILi1098EE3runEPiS1_, __str49+0*1);
      HEAP[__ZN7_stdcmdILi1098EE4initE] = _51;
      var _52 = __Z10addcommandPKcPFvvES0_(__str75+0*1, __ZN7_stdcmdILi1099EE3runEPiS1_, __str49+0*1);
      HEAP[__ZN7_stdcmdILi1099EE4initE] = _52;
      var _53 = __Z10addcommandPKcPFvvES0_(__str76+0*1, __ZN7_stdcmdILi1100EE3runEPiS1_, __str49+0*1);
      HEAP[__ZN7_stdcmdILi1100EE4initE] = _53;
      var _54 = __Z10addcommandPKcPFvvES0_(__str77+0*1, __ZN7_stdcmdILi1101EE3runEPiS1_, __str49+0*1);
      HEAP[__ZN7_stdcmdILi1101EE4initE] = _54;
      var _55 = __Z10addcommandPKcPFvvES0_(__str78+0*1, __ZN7_stdcmdILi1102EE3runEPiS1_, __str49+0*1);
      HEAP[__ZN7_stdcmdILi1102EE4initE] = _55;
      var _56 = __Z10addcommandPKcPFvvES0_(__str79+0*1, __ZN7_stdcmdILi1108EE3runEPPcPi, __str36+0*1);
      HEAP[__ZN7_stdcmdILi1108EE4initE] = _56;
      var _57 = __Z10addcommandPKcPFvvES0_(__str80+0*1, __ZN7_stdcmdILi1114EE3runEPPcPi, __str36+0*1);
      HEAP[__ZN7_stdcmdILi1114EE4initE] = _57;
      var _58 = __Z10addcommandPKcPFvvES0_(__str81+0*1, __ZN7_stdcmdILi1116EE3runEPiS1_, __str49+0*1);
      HEAP[__ZN7_stdcmdILi1116EE4initE] = _58;
      var _59 = __Z10addcommandPKcPFvvES0_(__str82+0*1, __ZN7_stdcmdILi1117EE3runEPiS1_, __str49+0*1);
      HEAP[__ZN7_stdcmdILi1117EE4initE] = _59;
      var _60 = __Z10addcommandPKcPFvvES0_(__str83+0*1, __ZN7_stdcmdILi1118EE3runEPfS1_, __str53+0*1);
      HEAP[__ZN7_stdcmdILi1118EE4initE] = _60;
      var _61 = __Z10addcommandPKcPFvvES0_(__str84+0*1, __ZN7_stdcmdILi1119EE3runEPfS1_, __str53+0*1);
      HEAP[__ZN7_stdcmdILi1119EE4initE] = _61;
      var _62 = __Z10addcommandPKcPFvvES0_(__str85+0*1, __ZN7_stdcmdILi1120EE3runEPf, __str86+0*1);
      HEAP[__ZN7_stdcmdILi1120EE4initE] = _62;
      var _63 = __Z10addcommandPKcPFvvES0_(__str87+0*1, __ZN7_stdcmdILi1121EE3runEPf, __str86+0*1);
      HEAP[__ZN7_stdcmdILi1121EE4initE] = _63;
      var _64 = __Z10addcommandPKcPFvvES0_(__str88+0*1, __ZN7_stdcmdILi1122EE3runEPf, __str86+0*1);
      HEAP[__ZN7_stdcmdILi1122EE4initE] = _64;
      var _65 = __Z10addcommandPKcPFvvES0_(__str89+0*1, __ZN7_stdcmdILi1123EE3runEPf, __str86+0*1);
      HEAP[__ZN7_stdcmdILi1123EE4initE] = _65;
      var _66 = __Z10addcommandPKcPFvvES0_(__str90+0*1, __ZN7_stdcmdILi1124EE3runEPf, __str86+0*1);
      HEAP[__ZN7_stdcmdILi1124EE4initE] = _66;
      var _67 = __Z10addcommandPKcPFvvES0_(__str91+0*1, __ZN7_stdcmdILi1125EE3runEPf, __str86+0*1);
      HEAP[__ZN7_stdcmdILi1125EE4initE] = _67;
      var _68 = __Z10addcommandPKcPFvvES0_(__str92+0*1, __ZN7_stdcmdILi1126EE3runEPf, __str86+0*1);
      HEAP[__ZN7_stdcmdILi1126EE4initE] = _68;
      var _69 = __Z10addcommandPKcPFvvES0_(__str93+0*1, __ZN7_stdcmdILi1127EE3runEPfS1_, __str53+0*1);
      HEAP[__ZN7_stdcmdILi1127EE4initE] = _69;
      var _70 = __Z10addcommandPKcPFvvES0_(__str94+0*1, __ZN7_stdcmdILi1128EE3runEPf, __str86+0*1);
      HEAP[__ZN7_stdcmdILi1128EE4initE] = _70;
      var _71 = __Z10addcommandPKcPFvvES0_(__str95+0*1, __ZN7_stdcmdILi1129EE3runEPf, __str86+0*1);
      HEAP[__ZN7_stdcmdILi1129EE4initE] = _71;
      var _72 = __Z10addcommandPKcPFvvES0_(__str96+0*1, __ZN7_stdcmdILi1130EE3runEPf, __str86+0*1);
      HEAP[__ZN7_stdcmdILi1130EE4initE] = _72;
      var _73 = __Z10addcommandPKcPFvvES0_(__str97+0*1, __ZN7_stdcmdILi1131EE3runEPf, __str86+0*1);
      HEAP[__ZN7_stdcmdILi1131EE4initE] = _73;
      var _74 = __Z10addcommandPKcPFvvES0_(__str98+0*1, __ZN7_stdcmdILi1137EE3runEPPcPi, __str36+0*1);
      HEAP[__ZN7_stdcmdILi1137EE4initE] = _74;
      var _75 = __Z10addcommandPKcPFvvES0_(__str99+0*1, __ZN7_stdcmdILi1143EE3runEPPcPi, __str36+0*1);
      HEAP[__ZN7_stdcmdILi1143EE4initE] = _75;
      var _76 = __Z10addcommandPKcPFvvES0_(__str100+0*1, __ZN7_stdcmdILi1149EE3runEPPcPi, __str36+0*1);
      HEAP[__ZN7_stdcmdILi1149EE4initE] = _76;
      var _77 = __Z10addcommandPKcPFvvES0_(__str101+0*1, __ZN7_stdcmdILi1155EE3runEPPcPi, __str36+0*1);
      HEAP[__ZN7_stdcmdILi1155EE4initE] = _77;
      var _78 = __Z10addcommandPKcPFvvES0_(__str102+0*1, __ZN7_stdcmdILi1167EE3runEPPcPi, __str36+0*1);
      HEAP[__ZN7_stdcmdILi1167EE4initE] = _78;
      var _79 = __Z10addcommandPKcPFvvES0_(__str103+0*1, __ZN7_stdcmdILi1182EE3runEPiPPcS1_, __str104+0*1);
      HEAP[__ZN7_stdcmdILi1182EE4initE] = _79;
      var _80 = __Z10addcommandPKcPFvvES0_(__str105+0*1, __ZN7_stdcmdILi1183EE3runEPfPPcPi, __str106+0*1);
      HEAP[__ZN7_stdcmdILi1183EE4initE] = _80;
      var _81 = __Z10addcommandPKcPFvvES0_(__str107+0*1, __ZN7_stdcmdILi1184EE3runEPcPS1_Pi, __str108+0*1);
      HEAP[__ZN7_stdcmdILi1184EE4initE] = _81;
      var _82 = __Z10addcommandPKcPFvvES0_(__str109+0*1, __ZN7_stdcmdILi1186EE3runEPiS1_, __str49+0*1);
      HEAP[__ZN7_stdcmdILi1186EE4initE] = _82;
      var _83 = __Z10addcommandPKcPFvvES0_(__str110+0*1, __ZN7_stdcmdILi1187EE3runEPcS1_, __str18+0*1);
      HEAP[__ZN7_stdcmdILi1187EE4initE] = _83;
      var _84 = __Z10addcommandPKcPFvvES0_(__str111+0*1, __ZN7_stdcmdILi1188EE3runEPcS1_, __str18+0*1);
      HEAP[__ZN7_stdcmdILi1188EE4initE] = _84;
      var _85 = __Z10addcommandPKcPFvvES0_(__str112+0*1, __ZN7_stdcmdILi1189EE3runEPcS1_, __str18+0*1);
      HEAP[__ZN7_stdcmdILi1189EE4initE] = _85;
      var _86 = __Z10addcommandPKcPFvvES0_(__str113+0*1, __ZN7_stdcmdILi1190EE3runEPcS1_, __str18+0*1);
      HEAP[__ZN7_stdcmdILi1190EE4initE] = _86;
      var _87 = __Z10addcommandPKcPFvvES0_(__str114+0*1, __ZN7_stdcmdILi1191EE3runEPcS1_, __str18+0*1);
      HEAP[__ZN7_stdcmdILi1191EE4initE] = _87;
      var _88 = __Z10addcommandPKcPFvvES0_(__str115+0*1, __ZN7_stdcmdILi1192EE3runEPcS1_, __str18+0*1);
      HEAP[__ZN7_stdcmdILi1192EE4initE] = _88;
      var _89 = __Z10addcommandPKcPFvvES0_(__str116+0*1, __ZN7_stdcmdILi1193EE3runEPcS1_, __str18+0*1);
      HEAP[__ZN7_stdcmdILi1193EE4initE] = _89;
      var _90 = __Z10addcommandPKcPFvvES0_(__str117+0*1, __ZN7_stdcmdILi1194EE3runEPc, __str33+0*1);
      HEAP[__ZN7_stdcmdILi1194EE4initE] = _90;
      var _91 = __Z10addcommandPKcPFvvES0_(__str118+0*1, __ZN7_stdcmdILi1195EE3runEPc, __str33+0*1);
      HEAP[__ZN7_stdcmdILi1195EE4initE] = _91;
      var _92 = __Z10addcommandPKcPFvvES0_(__str119+0*1, __ZN7_stdcmdILi1196EE3runEPcS1_, __str18+0*1);
      HEAP[__ZN7_stdcmdILi1196EE4initE] = _92;
      var _93 = __Z10addcommandPKcPFvvES0_(__str120+0*1, __ZN7_stdcmdILi1197EE3runEPc, __str20+0*1);
      HEAP[__ZN7_stdcmdILi1197EE4initE] = _93;
      var _94 = __Z10addcommandPKcPFvvES0_(__str121+0*1, __ZN7_stdcmdILi1223EE3runEPcS1_S1_, __str25+0*1);
      HEAP[__ZN7_stdcmdILi1223EE4initE] = _94;
      __ZN6vectorI5clineEC1Ev(_conlines);
      var _95 = ___cxa_atexit(___tcf_3, 0, ___dso_handle);
      var _96 = __Z8variablePKciiiPiPFvvEi(__str122+0*1, 10, 200, 1000, _maxcon, __Z10var_maxconv, 1);
      HEAP[_maxcon] = _96;
      var _97 = __Z8variablePKciiiPiPFvvEi(__str123+0*1, 0, 0, 1024, _somevar, 0, 1);
      HEAP[_somevar] = _97;
      __label__ = 3; break;
    case 3: // _bb2
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __Z3popPc(_name) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _name_addr = Pointer_make([0], 0);
      var _0 = Pointer_make([0], 0);
      var _id = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_name_addr] = _name;
      var _1 = HEAP[_name_addr];
      HEAP[_0] = _1;
      var _2 = HEAP[_idents];
      var _3 = __ZN9hashtableIPKc5identE6accessERKS1_(_2, _0);
      HEAP[_id] = _3;
      var _4 = HEAP[_id];
      var _5 = 0+(_4 != 0);
      if (_5) { __label__ = 0; break; } else { __label__ = 1; break; }
    case 0: // _bb
      var _6 = HEAP[_id];
      __Z8popidentR5ident(_6);
      __label__ = 1; break;
    case 1: // _bb1
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __ZN7_stdcmdILi846EE3runEPcS1_S1_(_cond, _t, _f) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _cond_addr = Pointer_make([0], 0);
      var _t_addr = Pointer_make([0], 0);
      var _f_addr = Pointer_make([0], 0);
      var _iftmp_154 = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_cond_addr] = _cond;
      HEAP[_t_addr] = _t;
      HEAP[_f_addr] = _f;
      var _0 = HEAP[_cond_addr];
      var _2 = HEAP[0 + _0];
      var _3 = 0+(_2 == 0);
      if (_3) { __label__ = 8; break; } else { __label__ = 0; break; }
    case 0: // _bb
      var _4 = HEAP[_cond_addr];
      var _5 = __ZL9isintegerPc(_4);
      var _toBool = 0+(_5 != 0);
      var _toBoolnot = _toBool ^ true;
      var _toBoolnot1 = _toBoolnot;
      var _toBool3 = 0+(_toBoolnot1 != 0);
      if (_toBool3) { __label__ = 4; break; } else { __label__ = 6; break; }
    case 6: // _bb4
      var _6 = HEAP[_cond_addr];
      var _7 = __ZL8parseintPKc(_6);
      var _8 = 0+(_7 != 0);
      if (_8) { __label__ = 4; break; } else { __label__ = 8; break; }
    case 4: // _bb5
      var _9 = HEAP[_t_addr];
      HEAP[_iftmp_154] = _9;
      __label__ = 5; break;
    case 8: // _bb6
      var _10 = HEAP[_f_addr];
      HEAP[_iftmp_154] = _10;
      __label__ = 5; break;
    case 5: // _bb7
      var _11 = HEAP[_iftmp_154];
      var _12 = __Z10executeretPKc(_11);
      HEAP[_commandret] = _12;
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __ZN7_stdcmdILi860EE3runEPcPiS1_(_var, _n, _body) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _var_addr = Pointer_make([0], 0);
      var _n_addr = Pointer_make([0], 0);
      var _body_addr = Pointer_make([0], 0);
      var _id = Pointer_make([0], 0);
      var _i;
      var __alloca_point_ = 0;
      HEAP[_var_addr] = _var;
      HEAP[_n_addr] = _n;
      HEAP[_body_addr] = _body;
      var _0 = HEAP[_n_addr];
      var _1 = HEAP[_0];
      var _2 = 0+(_1 <= 0);
      if (_2) { __label__ = 10; break; } else { __label__ = 0; break; }
    case 0: // _bb
      var _3 = HEAP[_var_addr];
      var _4 = __Z8newidentPKc(_3);
      HEAP[_id] = _4;
      var _5 = HEAP[_id];
      var _7 = HEAP[0 + _5+1*1];
      var _8 = 0+(_7 != 5);
      if (_8) { __label__ = 10; break; } else { __label__ = 1; break; }
    case 1: // _bb1
      _i = 0;
      __label__ = 8; break;
    case 3: // _bb2
      var _10 = 0+(_i != 0);
      if (_10) { __label__ = 7; break; } else { __label__ = 6; break; }
    case 7: // _bb3
      var _11 = HEAP[_id];
      var _12 = _11+7*1;
      var _14 = HEAP[0 + _12+0*1];
      var _16 = _sprintf(_14, __str2+0*1, _i);
      __label__ = 4; break;
    case 6: // _bb4
      var _17 = __Z9newstringPKcj(__str148+0*1, 16);
      var _18 = HEAP[_id];
      __Z9pushidentR5identPc(_18, _17);
      __label__ = 4; break;
    case 4: // _bb5
      var _19 = HEAP[_body_addr];
      var _20 = __Z7executePKc(_19);
      _i = _i + 1;
      __label__ = 8; break;
    case 8: // _bb6
      var _23 = HEAP[_n_addr];
      var _24 = HEAP[_23];
      var _26 = 0+(_24 > _i);
      if (_26) { __label__ = 3; break; } else { __label__ = 5; break; }
    case 5: // _bb7
      var _27 = HEAP[_id];
      __Z8popidentR5ident(_27);
      __label__ = 10; break;
    case 10: // _bb8
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __ZN7_stdcmdILi874EE3runEPcPiS1_S1_(_var, _n, _cond, _body) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _var_addr = Pointer_make([0], 0);
      var _n_addr = Pointer_make([0], 0);
      var _cond_addr = Pointer_make([0], 0);
      var _body_addr = Pointer_make([0], 0);
      var _retval_156;
      var _id = Pointer_make([0], 0);
      var _i;
      var __alloca_point_ = 0;
      HEAP[_var_addr] = _var;
      HEAP[_n_addr] = _n;
      HEAP[_cond_addr] = _cond;
      HEAP[_body_addr] = _body;
      var _0 = HEAP[_n_addr];
      var _1 = HEAP[_0];
      var _2 = 0+(_1 <= 0);
      if (_2) { __label__ = 9; break; } else { __label__ = 0; break; }
    case 0: // _bb
      var _3 = HEAP[_var_addr];
      var _4 = __Z8newidentPKc(_3);
      HEAP[_id] = _4;
      var _5 = HEAP[_id];
      var _7 = HEAP[0 + _5+1*1];
      var _8 = 0+(_7 != 5);
      if (_8) { __label__ = 9; break; } else { __label__ = 1; break; }
    case 1: // _bb1
      _i = 0;
      __label__ = 5; break;
    case 3: // _bb2
      var _10 = 0+(_i != 0);
      if (_10) { __label__ = 7; break; } else { __label__ = 6; break; }
    case 7: // _bb3
      var _11 = HEAP[_id];
      var _12 = _11+7*1;
      var _14 = HEAP[0 + _12+0*1];
      var _16 = _sprintf(_14, __str2+0*1, _i);
      __label__ = 4; break;
    case 6: // _bb4
      var _17 = __Z9newstringPKcj(__str148+0*1, 16);
      var _18 = HEAP[_id];
      __Z9pushidentR5identPc(_18, _17);
      __label__ = 4; break;
    case 4: // _bb5
      var _19 = HEAP[_cond_addr];
      var _20 = __Z7executePKc(_19);
      var _21 = 0+(_20 == 0);
      _retval_156 = _21;
      var _toBool = 0+(_retval_156 != 0);
      if (_toBool) { __label__ = 10; break; } else { __label__ = 8; break; }
    case 8: // _bb6
      var _24 = HEAP[_body_addr];
      var _25 = __Z7executePKc(_24);
      _i = _i + 1;
      __label__ = 5; break;
    case 5: // _bb7
      var _28 = HEAP[_n_addr];
      var _29 = HEAP[_28];
      var _31 = 0+(_29 > _i);
      if (_31) { __label__ = 3; break; } else { __label__ = 10; break; }
    case 10: // _bb8
      var _32 = HEAP[_id];
      __Z8popidentR5ident(_32);
      __label__ = 9; break;
    case 9: // _bb9
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __ZN7_stdcmdILi875EE3runEPcS1_(_cond, _body) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _cond_addr = Pointer_make([0], 0);
      var _body_addr = Pointer_make([0], 0);
      var _retval_157;
      var __alloca_point_ = 0;
      HEAP[_cond_addr] = _cond;
      HEAP[_body_addr] = _body;
      __label__ = 1; break;
    case 0: // _bb
      var _0 = HEAP[_body_addr];
      var _1 = __Z7executePKc(_0);
      __label__ = 1; break;
    case 1: // _bb1
      var _2 = HEAP[_cond_addr];
      var _3 = __Z7executePKc(_2);
      var _4 = 0+(_3 != 0);
      _retval_157 = _4;
      var _toBool = 0+(_retval_157 != 0);
      if (_toBool) { __label__ = 0; break; } else { __label__ = 3; break; }
    case 3: // _bb2
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __ZN7_stdcmdILi1072EE3runEPcS1_S1_(_var, _list, _body) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _var_addr = Pointer_make([0], 0);
      var _list_addr = Pointer_make([0], 0);
      var _body_addr = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_var_addr] = _var;
      HEAP[_list_addr] = _list;
      HEAP[_body_addr] = _body;
      var _0 = HEAP[_var_addr];
      var _1 = HEAP[_list_addr];
      var _2 = HEAP[_body_addr];
      __Z8looplistPKcS0_S0_b(_0, _1, _2, 1);
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __ZN7_stdcmdILi1073EE3runEPcS1_S1_(_var, _list, _body) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _var_addr = Pointer_make([0], 0);
      var _list_addr = Pointer_make([0], 0);
      var _body_addr = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_var_addr] = _var;
      HEAP[_list_addr] = _list;
      HEAP[_body_addr] = _body;
      var _0 = HEAP[_var_addr];
      var _1 = HEAP[_list_addr];
      var _2 = HEAP[_body_addr];
      __Z8looplistPKcS0_S0_b(_0, _1, _2, 0);
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __ZN7_stdcmdILi1108EE3runEPPcPi(_args, _numargs) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _args_addr = Pointer_make([0], 0);
      var _numargs_addr = Pointer_make([0], 0);
      var _val;
      var _i;
      var __alloca_point_ = 0;
      HEAP[_args_addr] = _args;
      HEAP[_numargs_addr] = _numargs;
      _val = 1;
      _i = 0;
      __label__ = 3; break;
    case 0: // _bb
      var _0 = HEAP[_args_addr];
      var _3 = HEAP[0 + _0+_i];
      _val = __Z7executePKc(_3);
      var _6 = 0+(_val == 0);
      if (_6) { __label__ = 7; break; } else { __label__ = 1; break; }
    case 1: // _bb1
      _i = _i + 1;
      __label__ = 3; break;
    case 3: // _bb2
      var _9 = HEAP[_numargs_addr];
      var _10 = HEAP[_9];
      var _12 = 0+(_10 > _i);
      if (_12) { __label__ = 0; break; } else { __label__ = 7; break; }
    case 7: // _bb3
      __Z6intreti(_val);
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __ZN7_stdcmdILi1114EE3runEPPcPi(_args, _numargs) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _args_addr = Pointer_make([0], 0);
      var _numargs_addr = Pointer_make([0], 0);
      var _val;
      var _i;
      var __alloca_point_ = 0;
      HEAP[_args_addr] = _args;
      HEAP[_numargs_addr] = _numargs;
      _val = 0;
      _i = 0;
      __label__ = 3; break;
    case 0: // _bb
      var _0 = HEAP[_args_addr];
      var _3 = HEAP[0 + _0+_i];
      _val = __Z7executePKc(_3);
      var _6 = 0+(_val != 0);
      if (_6) { __label__ = 7; break; } else { __label__ = 1; break; }
    case 1: // _bb1
      _i = _i + 1;
      __label__ = 3; break;
    case 3: // _bb2
      var _9 = HEAP[_numargs_addr];
      var _10 = HEAP[_9];
      var _12 = 0+(_10 > _i);
      if (_12) { __label__ = 0; break; } else { __label__ = 7; break; }
    case 7: // _bb3
      __Z6intreti(_val);
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __ZN7_stdcmdILi1167EE3runEPPcPi(_args, _numargs) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _args_addr = Pointer_make([0], 0);
      var _numargs_addr = Pointer_make([0], 0);
      var _retval_193;
      var _i;
      var __alloca_point_ = 0;
      HEAP[_args_addr] = _args;
      HEAP[_numargs_addr] = _numargs;
      _i = 0;
      __label__ = 4; break;
    case 0: // _bb
      var _0 = HEAP[_args_addr];
      var _3 = HEAP[0 + _0+_i];
      var _4 = __Z7executePKc(_3);
      var _5 = 0+(_4 != 0);
      _retval_193 = _5;
      var _toBool = 0+(_retval_193 != 0);
      if (_toBool) { __label__ = 1; break; } else { __label__ = 6; break; }
    case 1: // _bb1
      var _9 = _i + 1;
      var _10 = HEAP[_numargs_addr];
      var _11 = HEAP[_10];
      var _12 = 0+(_9 < _11);
      if (_12) { __label__ = 3; break; } else { __label__ = 7; break; }
    case 3: // _bb2
      var _14 = _i + 1;
      var _15 = HEAP[_args_addr];
      var _17 = HEAP[0 + _15+_14];
      var _18 = __Z10executeretPKc(_17);
      HEAP[_commandret] = _18;
      __label__ = 7; break;
    case 7: // _bb3
      __label__ = 8; break;
    case 6: // _bb4
      _i = _i + 2;
      __label__ = 4; break;
    case 4: // _bb5
      var _21 = HEAP[_numargs_addr];
      var _22 = HEAP[_21];
      var _24 = 0+(_22 > _i);
      if (_24) { __label__ = 0; break; } else { __label__ = 8; break; }
    case 8: // _bb6
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __ZN7_stdcmdILi1182EE3runEPiPPcS1_(_val, _args, _numargs) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _val_addr = Pointer_make([0], 0);
      var _args_addr = Pointer_make([0], 0);
      var _numargs_addr = Pointer_make([0], 0);
      var _retval_194;
      var _i;
      var __alloca_point_ = 0;
      HEAP[_val_addr] = _val;
      HEAP[_args_addr] = _args;
      HEAP[_numargs_addr] = _numargs;
      _i = 1;
      __label__ = 7; break;
    case 0: // _bb
      var _0 = HEAP[_args_addr];
      var _3 = HEAP[0 + _0+_i];
      var _4 = __ZL8parseintPKc(_3);
      var _5 = HEAP[_val_addr];
      var _6 = HEAP[_5];
      var _7 = 0+(_4 == _6);
      _retval_194 = _7;
      var _toBool = 0+(_retval_194 != 0);
      if (_toBool) { __label__ = 1; break; } else { __label__ = 3; break; }
    case 1: // _bb1
      var _11 = _i + 1;
      var _12 = HEAP[_args_addr];
      var _14 = HEAP[0 + _12+_11];
      var _15 = __Z10executeretPKc(_14);
      HEAP[_commandret] = _15;
      __label__ = 8; break;
    case 3: // _bb2
      _i = _i + 2;
      __label__ = 7; break;
    case 7: // _bb3
      var _19 = _i + 1;
      var _20 = HEAP[_numargs_addr];
      var _21 = HEAP[_20];
      var _22 = 0+(_19 < _21);
      if (_22) { __label__ = 0; break; } else { __label__ = 6; break; }
    case 6: // _bb4
      var _23 = HEAP[_numargs_addr];
      var _24 = HEAP[_23];
      var _26 = 0+(_24 > _i);
      if (_26) { __label__ = 4; break; } else { __label__ = 8; break; }
    case 4: // _bb5
      var _27 = HEAP[_args_addr];
      var _30 = HEAP[0 + _27+_i];
      var _31 = __Z10executeretPKc(_30);
      HEAP[_commandret] = _31;
      __label__ = 8; break;
    case 8: // _bb6
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __ZN7_stdcmdILi1183EE3runEPfPPcPi(_val, _args, _numargs) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _val_addr = Pointer_make([0], 0);
      var _args_addr = Pointer_make([0], 0);
      var _numargs_addr = Pointer_make([0], 0);
      var _retval_195;
      var _i;
      var __alloca_point_ = 0;
      HEAP[_val_addr] = _val;
      HEAP[_args_addr] = _args;
      HEAP[_numargs_addr] = _numargs;
      _i = 1;
      __label__ = 7; break;
    case 0: // _bb
      var _0 = HEAP[_args_addr];
      var _3 = HEAP[0 + _0+_i];
      var _4 = __ZL10parsefloatPKc(_3);
      var _5 = HEAP[_val_addr];
      var _6 = HEAP[_5];
      var _7 = 0+(_4 == _6);
      _retval_195 = _7;
      var _toBool = 0+(_retval_195 != 0);
      if (_toBool) { __label__ = 1; break; } else { __label__ = 3; break; }
    case 1: // _bb1
      var _11 = _i + 1;
      var _12 = HEAP[_args_addr];
      var _14 = HEAP[0 + _12+_11];
      var _15 = __Z10executeretPKc(_14);
      HEAP[_commandret] = _15;
      __label__ = 8; break;
    case 3: // _bb2
      _i = _i + 2;
      __label__ = 7; break;
    case 7: // _bb3
      var _19 = _i + 1;
      var _20 = HEAP[_numargs_addr];
      var _21 = HEAP[_20];
      var _22 = 0+(_19 < _21);
      if (_22) { __label__ = 0; break; } else { __label__ = 6; break; }
    case 6: // _bb4
      var _23 = HEAP[_numargs_addr];
      var _24 = HEAP[_23];
      var _26 = 0+(_24 > _i);
      if (_26) { __label__ = 4; break; } else { __label__ = 8; break; }
    case 4: // _bb5
      var _27 = HEAP[_args_addr];
      var _30 = HEAP[0 + _27+_i];
      var _31 = __Z10executeretPKc(_30);
      HEAP[_commandret] = _31;
      __label__ = 8; break;
    case 8: // _bb6
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __ZN7_stdcmdILi1184EE3runEPcPS1_Pi(_val, _args, _numargs) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _val_addr = Pointer_make([0], 0);
      var _args_addr = Pointer_make([0], 0);
      var _numargs_addr = Pointer_make([0], 0);
      var _i;
      var __alloca_point_ = 0;
      HEAP[_val_addr] = _val;
      HEAP[_args_addr] = _args;
      HEAP[_numargs_addr] = _numargs;
      _i = 1;
      __label__ = 7; break;
    case 0: // _bb
      var _0 = HEAP[_args_addr];
      var _3 = HEAP[0 + _0+_i];
      var _4 = HEAP[_val_addr];
      var _5 = _strcmp(_3, _4);
      var _6 = 0+(_5 == 0);
      if (_6) { __label__ = 1; break; } else { __label__ = 3; break; }
    case 1: // _bb1
      var _8 = _i + 1;
      var _9 = HEAP[_args_addr];
      var _11 = HEAP[0 + _9+_8];
      var _12 = __Z10executeretPKc(_11);
      HEAP[_commandret] = _12;
      __label__ = 8; break;
    case 3: // _bb2
      _i = _i + 2;
      __label__ = 7; break;
    case 7: // _bb3
      var _16 = _i + 1;
      var _17 = HEAP[_numargs_addr];
      var _18 = HEAP[_17];
      var _19 = 0+(_16 < _18);
      if (_19) { __label__ = 0; break; } else { __label__ = 6; break; }
    case 6: // _bb4
      var _20 = HEAP[_numargs_addr];
      var _21 = HEAP[_20];
      var _23 = 0+(_21 > _i);
      if (_23) { __label__ = 4; break; } else { __label__ = 8; break; }
    case 4: // _bb5
      var _24 = HEAP[_args_addr];
      var _27 = HEAP[0 + _24+_i];
      var _28 = __Z10executeretPKc(_27);
      HEAP[_commandret] = _28;
      __label__ = 8; break;
    case 8: // _bb6
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __Z8popidentR5ident(_id) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _id_addr = Pointer_make([0], 0);
      var _stack = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_id_addr] = _id;
      var _0 = HEAP[_id_addr];
      var _2 = HEAP[0 + _0+1*1];
      var _3 = 0+(_2 != 5);
      if (_3) { __label__ = 1; break; } else { __label__ = 0; break; }
    case 0: // _bb
      var _4 = HEAP[_id_addr];
      var _5 = _4+6*1;
      var _6 = _5;
      var _8 = HEAP[_6];
      var _9 = 0+(_8 == 0);
      if (_9) { __label__ = 1; break; } else { __label__ = 3; break; }
    case 1: // _bb1
      __label__ = 8; break;
    case 3: // _bb2
      var _10 = HEAP[_id_addr];
      var _11 = _10+7*1;
      var _13 = HEAP[0 + _11+0*1];
      var _15 = _10+8*1;
      var _17 = HEAP[0 + _15+0*1];
      var _18 = 0+(_13 != _17);
      if (_18) { __label__ = 7; break; } else { __label__ = 4; break; }
    case 7: // _bb3
      var _19 = HEAP[_id_addr];
      var _20 = _19+7*1;
      var _22 = HEAP[0 + _20+0*1];
      var _23 = 0+(_22 != 0);
      if (_23) { __label__ = 6; break; } else { __label__ = 4; break; }
    case 6: // _bb4
      var _24 = HEAP[_id_addr];
      var _25 = _24+7*1;
      var _27 = HEAP[0 + _25+0*1];
      __ZdaPv(_27);
      __label__ = 4; break;
    case 4: // _bb5
      var _28 = HEAP[_id_addr];
      var _29 = _28+6*1;
      var _30 = _29;
      var _32 = HEAP[_30];
      HEAP[_stack] = _32;
      var _33 = HEAP[_stack];
      var _35 = HEAP[0 + _33+0*1];
      var _36 = HEAP[_id_addr];
      var _37 = _36+7*1;
      HEAP[0 + _37+0*1] = _35;
      var _39 = HEAP[_stack];
      var _41 = HEAP[0 + _39+1*1];
      var _42 = HEAP[_id_addr];
      var _43 = _42+6*1;
      var _44 = _43;
      HEAP[_44] = _41;
      var _46 = HEAP[_stack];
      __ZdlPv(_46);
      __label__ = 8; break;
    case 8: // _bb6
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __Z10executeretPKc(_p) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _p_addr = Pointer_make([0], 0);
      var _retval = Pointer_make([0], 0);
      var _0 = Pointer_make([0], 0);
      var _1 = Pointer_make([0], 0);
      var _2 = Pointer_make([0], 0);
      var _retval_141;
      var _3;
      var _retval_140;
      var _iftmp_137 = Pointer_make([0], 0);
      var _iftmp_136 = Pointer_make([0], 0);
      var _iftmp_127;
      var _iftmp_122;
      var _iftmp_119 = Pointer_make([0], 0);
      var _retval_116;
      var _4 = Pointer_make([0], 0);
      var _iftmp_114 = Pointer_make([0], 0);
      var _MAXWORDS;
      var _w = Pointer_make([0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0], 0);
      var _retval1 = Pointer_make([0], 0);
      var _cont;
      var _numargs;
      var _infix = Pointer_make([0], 0);
      var _c = Pointer_make([0], 0);
      var _i;
      var _id = Pointer_make([0], 0);
      var _rv = Pointer_make([0], 0);
      var _v = Pointer_make([0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0], 0);
      var _nstor = Pointer_make([0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0], 0);
      var _n;
      var _wn;
      var _cargs = Pointer_make([0], 0);
      var _a = Pointer_make([0], 0);
      var _rv67 = Pointer_make([0], 0);
      var _val;
      var _wasoverriding;
      var _wasexecuting = Pointer_make([0], 0);
      var _i103;
      var _argname = Pointer_make([0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0], 0);
      var _rv112 = Pointer_make([0], 0);
      var _i119;
      var _j;
      var __alloca_point_ = 0;
      HEAP[_p_addr] = _p;
      _MAXWORDS = 25;
      HEAP[_retval1] = 0;
      _cont = 1;
      __label__ = 36; break;
    case 0: // _bb
      _numargs = 25;
      HEAP[_infix] = 0;
      _i = 0;
      __label__ = 4; break;
    case 3: // _bb2
      var _5 = _i;
      var _7 = __Z9parsewordRPKciRi(_p_addr, _i, _infix);
      HEAP[0 + _w+_5*1] = _7;
      var _11 = HEAP[0 + _w+_i*1];
      var _12 = 0+(_11 == 0);
      if (_12) { __label__ = 7; break; } else { __label__ = 6; break; }
    case 7: // _bb3
      _numargs = _i;
      __label__ = 8; break;
    case 6: // _bb4
      _i = _i + 1;
      __label__ = 4; break;
    case 4: // _bb5
      var _17 = 0+(_i <= 24);
      if (_17) { __label__ = 3; break; } else { __label__ = 8; break; }
    case 8: // _bb6
      var _18 = HEAP[_p_addr];
      var _20 = _strcspn(_18, __str124+0*1);
      var _21 = _18+_20;
      HEAP[_p_addr] = _21;
      var _22 = HEAP[_p_addr];
      var _23 = HEAP[_22];
      var _24 = 0+(_23 != 0);
      _cont = _24;
      var _26 = _22+1;
      HEAP[_p_addr] = _26;
      var _28 = HEAP[0 + _w+0*1];
      HEAP[_c] = _28;
      var _29 = HEAP[_c];
      var _30 = 0+(_29 == 0);
      if (_30) { __label__ = 10; break; } else { __label__ = 5; break; }
    case 5: // _bb7
      var _31 = HEAP[_c];
      var _32 = HEAP[_31];
      var _33 = 0+(_32 == 0);
      if (_33) { __label__ = 10; break; } else { __label__ = 9; break; }
    case 10: // _bb8
      __label__ = 36; break;
    case 9: // _bb9
      var _34 = HEAP[_retval1];
      var _35 = 0+(_34 != 0);
      if (_35) { __label__ = 11; break; } else { __label__ = 15; break; }
    case 11: // _bb10
      var _36 = HEAP[_retval1];
      var _37 = 0+(_36 != 0);
      if (_37) { __label__ = 13; break; } else { __label__ = 14; break; }
    case 13: // _bb11
      var _38 = HEAP[_retval1];
      __ZdaPv(_38);
      __label__ = 14; break;
    case 14: // _bb12
      HEAP[_retval1] = 0;
      __label__ = 15; break;
    case 15: // _bb13
      var _39 = HEAP[_infix];
      var _40 = 0+(_39 != 0);
      if (_40) { __label__ = 12; break; } else { __label__ = 19; break; }
    case 12: // _bb14
      var _41 = HEAP[_infix];
      if (_41 == 61) {
  __label__ = 21; break;
}
else {
__label__ = 18; break;
}

    case 21: // _bb15
      var _43 = 0+(_numargs > 2);
      if (_43) { __label__ = 16; break; } else { __label__ = 20; break; }
    case 16: // _bb16
      var _45 = HEAP[0 + _w+2*1];
      HEAP[_iftmp_114] = _45;
      __label__ = 17; break;
    case 20: // _bb17
      var _46 = __Z9newstringPKc(__str3+0*1);
      HEAP[_iftmp_114] = _46;
      __label__ = 17; break;
    case 17: // _bb18
      var _47 = HEAP[_c];
      var _48 = HEAP[_iftmp_114];
      __Z6aliasaPKcPc(_47, _48);
      HEAP[0 + _w+2*1] = 0;
      __label__ = 18; break;
    case 18: // _bb19
      __label__ = 37; break;
    case 19: // _bb20
      var _50 = HEAP[_c];
      HEAP[_4] = _50;
      var _51 = HEAP[_idents];
      var _52 = __ZN9hashtableIPKc5identE6accessERKS1_(_51, _4);
      HEAP[_id] = _52;
      var _53 = HEAP[_id];
      var _54 = 0+(_53 == 0);
      if (_54) { __label__ = 24; break; } else { __label__ = 34; break; }
    case 24: // _bb21
      var _55 = HEAP[_c];
      var _56 = __ZL9isintegerPc(_55);
      var _toBool = 0+(_56 != 0);
      var _toBoolnot = _toBool ^ true;
      _retval_116 = _toBoolnot;
      var _toBool23 = 0+(_retval_116 != 0);
      if (_toBool23) { __label__ = 22; break; } else { __label__ = 38; break; }
    case 22: // _bb24
      var _58 = HEAP[_c];
      __Z7conoutfiPKcz(4, __str125+0*1, _58);
      __label__ = 38; break;
    case 38: // _bb25
      var _59 = HEAP[_c];
      var _60 = __Z9newstringPKc(_59);
      HEAP[_rv] = _60;
      var _61 = HEAP[_rv];
      var _62 = 0+(_61 != 0);
      if (_62) { __label__ = 39; break; } else { __label__ = 33; break; }
    case 39: // _bb26
      var _63 = HEAP[_rv];
      HEAP[_retval1] = _63;
      __label__ = 33; break;
    case 33: // _bb27
      __label__ = 37; break;
    case 34: // _bb28
      var _64 = HEAP[_id];
      var _66 = HEAP[0 + _64+1*1];
      if (_66 == 0) {
  __label__ = 112; break;
}
else if (_66 == 1) {
  __label__ = 113; break;
}
else if (_66 == 2) {
  __label__ = 114; break;
}
else if (_66 == 3) {
  __label__ = 32; break;
}
else if (_66 == 4) {
  __label__ = 32; break;
}
else if (_66 == 5) {
  __label__ = 115; break;
}
else {
__label__ = 37; break;
}

    case 32: // _bb29
      _n = 0;
      _wn = 0;
      HEAP[_cargs] = 0;
      var _67 = HEAP[_id];
      var _69 = HEAP[0 + _67+1*1];
      var _70 = 0+(_69 == 4);
      if (_70) { __label__ = 40; break; } else { __label__ = 35; break; }
    case 40: // _bb30
      var _71 = _n;
      var _72 = HEAP[_id];
      var _73 = _72+8*1;
      var _75 = HEAP[0 + _73+0*1];
      HEAP[0 + _v+_71*1] = _75;
      _n = _n + 1;
      __label__ = 35; break;
    case 35: // _bb31
      var _79 = HEAP[_id];
      var _80 = _79+7*1;
      var _82 = HEAP[0 + _80+0*1];
      HEAP[_a] = _82;
      __label__ = 41; break;
    case 31: // _bb32
      var _83 = HEAP[_a];
      var _84 = HEAP[_83];
      if (_84 == 67) {
  __label__ = 116; break;
}
else if (_84 == 86) {
  __label__ = 117; break;
}
else if (_84 == 102) {
  __label__ = 118; break;
}
else if (_84 == 105) {
  __label__ = 119; break;
}
else if (_84 == 115) {
  __label__ = 111; break;
}
else {
__label__ = 120; break;
}

    case 111: // _bb33
      var _85 = _n;
      _wn = _wn + 1;
      var _90 = 0+(_wn < _numargs);
      if (_90) { __label__ = 42; break; } else { __label__ = 43; break; }
    case 42: // _bb34
      var _93 = HEAP[0 + _w+_wn*1];
      HEAP[_iftmp_119] = _93;
      __label__ = 44; break;
    case 43: // _bb35
      HEAP[_iftmp_119] = __str3+0*1;
      __label__ = 44; break;
    case 44: // _bb36
      var _94 = _v+_85*1;
      var _95 = HEAP[_iftmp_119];
      HEAP[_94] = _95;
      __label__ = 45; break;
    case 119: // _bb37
      var _96 = _n;
      _wn = _wn + 1;
      var _101 = 0+(_wn < _numargs);
      if (_101) { __label__ = 46; break; } else { __label__ = 47; break; }
    case 46: // _bb38
      var _104 = HEAP[0 + _w+_wn*1];
      _iftmp_122 = __ZL8parseintPKc(_104);
      __label__ = 48; break;
    case 47: // _bb39
      _iftmp_122 = 0;
      __label__ = 48; break;
    case 48: // _bb40
      var _106 = _nstor+_96*1;
      var _107 = _106;
      HEAP[_107] = _iftmp_122;
      var _109 = _n;
      var _111 = _nstor+_n*1;
      var _112 = _111;
      var _113 = _v+_109*1;
      HEAP[_113] = _112;
      __label__ = 45; break;
    case 118: // _bb41
      var _115 = _n;
      _wn = _wn + 1;
      var _120 = 0+(_wn < _numargs);
      if (_120) { __label__ = 49; break; } else { __label__ = 50; break; }
    case 49: // _bb42
      var _123 = HEAP[0 + _w+_wn*1];
      _iftmp_127 = __ZL10parsefloatPKc(_123);
      __label__ = 51; break;
    case 50: // _bb43
      _iftmp_127 = 0.000000e+00;
      __label__ = 51; break;
    case 51: // _bb44
      var _125 = _nstor+_115*1;
      var _126 = _125;
      var _127 = _126;
      HEAP[_127] = _iftmp_127;
      var _129 = _n;
      var _131 = _nstor+_n*1;
      var _132 = _131;
      var _133 = _132;
      var _134 = _v+_129*1;
      HEAP[_134] = _133;
      __label__ = 45; break;
    case 117: // _bb45
      var _137 = _v+_n*1;
      var _138 = _w+0*1;
      var _139 = _138+1;
      HEAP[_137] = _139;
      _n = _n + 1;
      var _143 = _n;
      var _145 = _numargs - 1;
      var _146 = _nstor+_143*1;
      HEAP[0 + _146] = _145;
      var _148 = _n;
      var _150 = _nstor+_n*1;
      var _151 = _150;
      var _152 = _v+_148*1;
      HEAP[_152] = _151;
      __label__ = 45; break;
    case 116: // _bb46
      var _154 = HEAP[_cargs];
      var _155 = 0+(_154 == 0);
      if (_155) { __label__ = 52; break; } else { __label__ = 53; break; }
    case 52: // _bb47
      var _157 = _numargs - 1;
      var _158 = _w+0*1;
      var _159 = _158+1;
      var _160 = __Z4concPPcib(_159, _157, 1);
      HEAP[_cargs] = _160;
      __label__ = 53; break;
    case 53: // _bb48
      var _162 = _v+_n*1;
      var _163 = HEAP[_cargs];
      HEAP[_162] = _163;
      __label__ = 45; break;
    case 120: // _bb49
      __Z5fatalPKcz(__str126+0*1);
      __label__ = 45; break;
    case 45: // _bb50
      var _164 = HEAP[_a];
      var _165 = _164+1;
      HEAP[_a] = _165;
      _n = _n + 1;
      __label__ = 41; break;
    case 41: // _bb51
      var _168 = HEAP[_a];
      var _169 = HEAP[_168];
      var _170 = 0+(_169 != 0);
      if (_170) { __label__ = 31; break; } else { __label__ = 54; break; }
    case 54: // _bb52
      if (_n == 0) {
  __label__ = 121; break;
}
else if (_n == 1) {
  __label__ = 122; break;
}
else if (_n == 2) {
  __label__ = 123; break;
}
else if (_n == 3) {
  __label__ = 124; break;
}
else if (_n == 4) {
  __label__ = 125; break;
}
else if (_n == 5) {
  __label__ = 126; break;
}
else if (_n == 6) {
  __label__ = 127; break;
}
else if (_n == 7) {
  __label__ = 128; break;
}
else if (_n == 8) {
  __label__ = 129; break;
}
else {
__label__ = 130; break;
}

    case 121: // _bb53
      var _172 = HEAP[_id];
      var _173 = _172+6*1;
      var _175 = HEAP[0 + _173];
      _175();
      __label__ = 55; break;
    case 122: // _bb54
      var _176 = HEAP[_id];
      var _177 = _176+6*1;
      var _179 = HEAP[0 + _177];
      var _180 = _179;
      var _182 = HEAP[0 + _v+0*1];
      _180(_182);
      __label__ = 55; break;
    case 123: // _bb55
      var _183 = HEAP[_id];
      var _184 = _183+6*1;
      var _186 = HEAP[0 + _184];
      var _187 = _186;
      var _189 = HEAP[0 + _v+1*1];
      var _191 = HEAP[0 + _v+0*1];
      _187(_191, _189);
      __label__ = 55; break;
    case 124: // _bb56
      var _192 = HEAP[_id];
      var _193 = _192+6*1;
      var _195 = HEAP[0 + _193];
      var _196 = _195;
      var _198 = HEAP[0 + _v+2*1];
      var _200 = HEAP[0 + _v+1*1];
      var _202 = HEAP[0 + _v+0*1];
      _196(_202, _200, _198);
      __label__ = 55; break;
    case 125: // _bb57
      var _203 = HEAP[_id];
      var _204 = _203+6*1;
      var _206 = HEAP[0 + _204];
      var _207 = _206;
      var _209 = HEAP[0 + _v+3*1];
      var _211 = HEAP[0 + _v+2*1];
      var _213 = HEAP[0 + _v+1*1];
      var _215 = HEAP[0 + _v+0*1];
      _207(_215, _213, _211, _209);
      __label__ = 55; break;
    case 126: // _bb58
      var _216 = HEAP[_id];
      var _217 = _216+6*1;
      var _219 = HEAP[0 + _217];
      var _220 = _219;
      var _222 = HEAP[0 + _v+4*1];
      var _224 = HEAP[0 + _v+3*1];
      var _226 = HEAP[0 + _v+2*1];
      var _228 = HEAP[0 + _v+1*1];
      var _230 = HEAP[0 + _v+0*1];
      _220(_230, _228, _226, _224, _222);
      __label__ = 55; break;
    case 127: // _bb59
      var _231 = HEAP[_id];
      var _232 = _231+6*1;
      var _234 = HEAP[0 + _232];
      var _235 = _234;
      var _237 = HEAP[0 + _v+5*1];
      var _239 = HEAP[0 + _v+4*1];
      var _241 = HEAP[0 + _v+3*1];
      var _243 = HEAP[0 + _v+2*1];
      var _245 = HEAP[0 + _v+1*1];
      var _247 = HEAP[0 + _v+0*1];
      _235(_247, _245, _243, _241, _239, _237);
      __label__ = 55; break;
    case 128: // _bb60
      var _248 = HEAP[_id];
      var _249 = _248+6*1;
      var _251 = HEAP[0 + _249];
      var _252 = _251;
      var _254 = HEAP[0 + _v+6*1];
      var _256 = HEAP[0 + _v+5*1];
      var _258 = HEAP[0 + _v+4*1];
      var _260 = HEAP[0 + _v+3*1];
      var _262 = HEAP[0 + _v+2*1];
      var _264 = HEAP[0 + _v+1*1];
      var _266 = HEAP[0 + _v+0*1];
      _252(_266, _264, _262, _260, _258, _256, _254);
      __label__ = 55; break;
    case 129: // _bb61
      var _267 = HEAP[_id];
      var _268 = _267+6*1;
      var _270 = HEAP[0 + _268];
      var _271 = _270;
      var _273 = HEAP[0 + _v+7*1];
      var _275 = HEAP[0 + _v+6*1];
      var _277 = HEAP[0 + _v+5*1];
      var _279 = HEAP[0 + _v+4*1];
      var _281 = HEAP[0 + _v+3*1];
      var _283 = HEAP[0 + _v+2*1];
      var _285 = HEAP[0 + _v+1*1];
      var _287 = HEAP[0 + _v+0*1];
      _271(_287, _285, _283, _281, _279, _277, _275, _273);
      __label__ = 55; break;
    case 130: // _bb62
      __Z5fatalPKcz(__str127+0*1);
      __label__ = 55; break;
    case 55: // _bb63
      var _288 = HEAP[_cargs];
      var _289 = 0+(_288 != 0);
      if (_289) { __label__ = 56; break; } else { __label__ = 57; break; }
    case 56: // _bb64
      var _290 = HEAP[_cargs];
      var _291 = 0+(_290 != 0);
      if (_291) { __label__ = 58; break; } else { __label__ = 57; break; }
    case 58: // _bb65
      var _292 = HEAP[_cargs];
      __ZdaPv(_292);
      __label__ = 57; break;
    case 57: // _bb66
      var _293 = HEAP[_commandret];
      HEAP[_rv67] = _293;
      var _294 = HEAP[_rv67];
      var _295 = 0+(_294 != 0);
      if (_295) { __label__ = 59; break; } else { __label__ = 60; break; }
    case 59: // _bb68
      var _296 = HEAP[_rv67];
      HEAP[_retval1] = _296;
      __label__ = 60; break;
    case 60: // _bb69
      HEAP[_commandret] = 0;
      __label__ = 37; break;
    case 112: // _bb70
      var _298 = 0+(_numargs <= 1);
      if (_298) { __label__ = 61; break; } else { __label__ = 62; break; }
    case 61: // _bb71
      var _299 = HEAP[_id];
      var _301 = HEAP[0 + _299+10*1];
      var _302 = _301 & 4;
      var _303 = 0+(_302 == 0);
      if (_303) { __label__ = 63; break; } else { __label__ = 64; break; }
    case 64: // _bb72
      var _304 = HEAP[_id];
      var _305 = _304+4*1;
      var _307 = HEAP[0 + _305];
      var _308 = 0+(_307 != 16777215);
      if (_308) { __label__ = 63; break; } else { __label__ = 65; break; }
    case 65: // _bb73
      var _309 = HEAP[_id];
      var _310 = _309+9*1;
      var _312 = HEAP[0 + _310];
      var _313 = HEAP[_312];
      var _314 = _313 & 255;
      var _316 = _309+9*1;
      var _318 = HEAP[0 + _316];
      var _319 = HEAP[_318];
      var _320 = _319 >> 8;
      var _321 = _320 & 255;
      var _322 = HEAP[_id];
      var _323 = _322+9*1;
      var _325 = HEAP[0 + _323];
      var _326 = HEAP[_325];
      var _327 = _326 >> 16;
      var _328 = _327 & 255;
      var _330 = _322+9*1;
      var _332 = HEAP[0 + _330];
      var _333 = HEAP[_332];
      var _334 = HEAP[_c];
      __Z7conoutfPKcz(__str128+0*1, _334, _333, _328, _321, _314);
      __label__ = 66; break;
    case 63: // _bb74
      var _335 = HEAP[_id];
      var _336 = _335+9*1;
      var _338 = HEAP[0 + _336];
      var _339 = HEAP[_338];
      var _342 = HEAP[0 + _335+10*1];
      var _343 = _342 & 4;
      var _344 = 0+(_343 != 0);
      if (_344) { __label__ = 67; break; } else { __label__ = 68; break; }
    case 67: // _bb75
      HEAP[_iftmp_136] = __str129+0*1;
      __label__ = 69; break;
    case 68: // _bb76
      HEAP[_iftmp_136] = __str130+0*1;
      __label__ = 69; break;
    case 69: // _bb77
      var _345 = HEAP[_iftmp_136];
      var _346 = HEAP[_c];
      __Z7conoutfPKcz(_345, _346, _339);
      __label__ = 66; break;
    case 66: // _bb78
      __label__ = 70; break;
    case 62: // _bb79
      var _348 = HEAP[0 + _w+1*1];
      _val = __ZL8parseintPKc(_348);
      var _350 = HEAP[_id];
      var _352 = HEAP[0 + _350+10*1];
      var _353 = _352 & 4;
      var _354 = 0+(_353 != 0);
      if (_354) { __label__ = 71; break; } else { __label__ = 72; break; }
    case 71: // _bb80
      var _356 = 0+(_numargs > 2);
      if (_356) { __label__ = 73; break; } else { __label__ = 72; break; }
    case 73: // _bb81
      _val = _val << 16;
      var _360 = HEAP[0 + _w+2*1];
      var _361 = __ZL8parseintPKc(_360);
      var _362 = _361 << 8;
      _val = _362 | _val;
      var _366 = 0+(_numargs > 3);
      if (_366) { __label__ = 74; break; } else { __label__ = 72; break; }
    case 74: // _bb82
      var _368 = HEAP[0 + _w+3*1];
      var _369 = __ZL8parseintPKc(_368);
      _val = _369 | _val;
      __label__ = 72; break;
    case 72: // _bb83
      var _372 = HEAP[_id];
      __Z13setvarcheckedP5identi(_372, _val);
      __label__ = 70; break;
    case 70: // _bb84
      __label__ = 37; break;
    case 113: // _bb85
      var _375 = 0+(_numargs <= 1);
      if (_375) { __label__ = 75; break; } else { __label__ = 76; break; }
    case 75: // _bb86
      var _376 = HEAP[_id];
      var _377 = _376+9*1;
      var _378 = _377;
      var _380 = HEAP[_378];
      var _381 = HEAP[_380];
      var _382 = __Z8floatstrf(_381);
      var _383 = HEAP[_c];
      __Z7conoutfPKcz(__str131+0*1, _383, _382);
      __label__ = 77; break;
    case 76: // _bb87
      var _385 = HEAP[0 + _w+1*1];
      var _386 = __ZL10parsefloatPKc(_385);
      var _387 = HEAP[_id];
      __Z14setfvarcheckedP5identf(_387, _386);
      __label__ = 77; break;
    case 77: // _bb88
      __label__ = 37; break;
    case 114: // _bb89
      var _389 = 0+(_numargs <= 1);
      if (_389) { __label__ = 78; break; } else { __label__ = 79; break; }
    case 78: // _bb90
      var _390 = HEAP[_id];
      var _391 = _390+9*1;
      var _392 = _391;
      var _394 = HEAP[_392];
      var _395 = HEAP[_394];
      var _397 = _390+9*1;
      var _398 = _397;
      var _400 = HEAP[_398];
      var _401 = HEAP[_400];
      var _402 = _strchr(_401, 34);
      var _403 = 0+(_402 != 0);
      if (_403) { __label__ = 80; break; } else { __label__ = 81; break; }
    case 80: // _bb91
      HEAP[_iftmp_137] = __str132;
      __label__ = 82; break;
    case 81: // _bb92
      HEAP[_iftmp_137] = __str133;
      __label__ = 82; break;
    case 82: // _bb93
      var _404 = HEAP[_iftmp_137];
      var _405 = _404;
      var _406 = HEAP[_c];
      __Z7conoutfPKcz(_405, _406, _395);
      __label__ = 83; break;
    case 79: // _bb94
      var _408 = HEAP[0 + _w+1*1];
      var _409 = HEAP[_id];
      __Z14setsvarcheckedP5identPKc(_409, _408);
      __label__ = 83; break;
    case 83: // _bb95
      __label__ = 37; break;
    case 115: // _bb96
      var _411 = HEAP[0 + _w+0*1];
      var _412 = 0+(_411 != 0);
      if (_412) { __label__ = 84; break; } else { __label__ = 85; break; }
    case 84: // _bb97
      var _414 = HEAP[0 + _w+0*1];
      __ZdaPv(_414);
      __label__ = 85; break;
    case 85: // _bb98
      var _415 = HEAP[__ZGVZ10executeretPKcE6argids];
      var _416 = 0+(_415 == 0);
      if (_416) { __label__ = 86; break; } else { __label__ = 87; break; }
    case 86: // _bb99
      var _417 = ___cxa_guard_acquire(__ZGVZ10executeretPKcE6argids);
      var _418 = 0+(_417 != 0);
      _retval_140 = _418;
      var _toBool100 = 0+(_retval_140 != 0);
      if (_toBool100) { __label__ = 88; break; } else { __label__ = 87; break; }
    case 88: // _bb101
      _3 = 0;
      __ZN6vectorIP5identEC1Ev(__ZZ10executeretPKcE6argids);
      _3 = 1;
      ___cxa_guard_release(__ZGVZ10executeretPKcE6argids);
      var _421 = ___cxa_atexit(___tcf_1, 0, ___dso_handle);
      __label__ = 87; break;
    case 87: // _bb102
      _i103 = 1;
      __label__ = 89; break;
    case 92: // _bb104
      var _422 = __ZNK6vectorIP5identE6lengthEv(__ZZ10executeretPKcE6argids);
      var _424 = 0+(_422 < _i103);
      _retval_141 = _424;
      var _toBool105 = 0+(_retval_141 != 0);
      if (_toBool105) { __label__ = 90; break; } else { __label__ = 91; break; }
    case 90: // _bb106
      var _427 = _argname+0*1;
      __ZN15stringformatterC1EPc(_2, _427);
      __ZN15stringformatterclEPKcz(_2, __str134+0*1, _i103);
      var _429 = _argname+0*1;
      var _430 = __Z8newidentPKc(_429);
      HEAP[_1] = _430;
      var _431 = __ZN6vectorIP5identE3addERKS1_(__ZZ10executeretPKcE6argids, _1);
      __label__ = 91; break;
    case 91: // _bb107
      var _434 = HEAP[0 + _w+_i103*1];
      var _436 = _i103 - 1;
      var _437 = __ZN6vectorIP5identEixEi(__ZZ10executeretPKcE6argids, _436);
      var _438 = HEAP[_437];
      __Z9pushidentR5identPc(_438, _434);
      _i103 = _i103 + 1;
      __label__ = 89; break;
    case 89: // _bb108
      var _443 = 0+(_i103 < _numargs);
      if (_443) { __label__ = 92; break; } else { __label__ = 93; break; }
    case 93: // _bb109
      var _445 = _numargs - 1;
      HEAP[__numargs] = _445;
      _wasoverriding = HEAP[_overrideidents];
      var _447 = HEAP[_id];
      var _449 = HEAP[0 + _447+5*1];
      var _450 = 0+(_449 != 2147483647);
      if (_450) { __label__ = 94; break; } else { __label__ = 95; break; }
    case 94: // _bb110
      HEAP[_overrideidents] = 1;
      __label__ = 95; break;
    case 95: // _bb111
      var _451 = HEAP[_id];
      var _452 = _451+8*1;
      var _454 = HEAP[0 + _452+0*1];
      HEAP[_wasexecuting] = _454;
      var _455 = HEAP[_id];
      var _456 = _455+7*1;
      var _458 = HEAP[0 + _456+0*1];
      var _460 = _455+8*1;
      HEAP[0 + _460+0*1] = _458;
      var _462 = HEAP[_id];
      var _463 = _462+7*1;
      var _465 = HEAP[0 + _463+0*1];
      var _466 = __Z10executeretPKc(_465);
      HEAP[_rv112] = _466;
      var _467 = HEAP[_rv112];
      var _468 = 0+(_467 != 0);
      if (_468) { __label__ = 96; break; } else { __label__ = 97; break; }
    case 96: // _bb113
      var _469 = HEAP[_rv112];
      HEAP[_retval1] = _469;
      __label__ = 97; break;
    case 97: // _bb114
      var _470 = HEAP[_id];
      var _471 = _470+8*1;
      var _473 = HEAP[0 + _471+0*1];
      var _475 = _470+7*1;
      var _477 = HEAP[0 + _475+0*1];
      var _478 = 0+(_473 != _477);
      if (_478) { __label__ = 98; break; } else { __label__ = 99; break; }
    case 98: // _bb115
      var _479 = HEAP[_id];
      var _480 = _479+8*1;
      var _482 = HEAP[0 + _480+0*1];
      var _483 = HEAP[_wasexecuting];
      var _484 = 0+(_482 != _483);
      if (_484) { __label__ = 100; break; } else { __label__ = 99; break; }
    case 100: // _bb116
      var _485 = HEAP[_id];
      var _486 = _485+8*1;
      var _488 = HEAP[0 + _486+0*1];
      var _489 = 0+(_488 != 0);
      if (_489) { __label__ = 101; break; } else { __label__ = 99; break; }
    case 101: // _bb117
      var _490 = HEAP[_id];
      var _491 = _490+8*1;
      var _493 = HEAP[0 + _491+0*1];
      __ZdaPv(_493);
      __label__ = 99; break;
    case 99: // _bb118
      var _494 = HEAP[_id];
      var _495 = _494+8*1;
      var _496 = _495+0*1;
      var _497 = HEAP[_wasexecuting];
      HEAP[_496] = _497;
      HEAP[_overrideidents] = _wasoverriding;
      _i119 = 1;
      __label__ = 102; break;
    case 103: // _bb120
      var _500 = _i119 - 1;
      var _501 = __ZN6vectorIP5identEixEi(__ZZ10executeretPKcE6argids, _500);
      var _502 = HEAP[_501];
      __Z8popidentR5ident(_502);
      _i119 = _i119 + 1;
      __label__ = 102; break;
    case 102: // _bb121
      var _507 = 0+(_i119 < _numargs);
      if (_507) { __label__ = 103; break; } else { __label__ = 104; break; }
    case 104: // _bb122
      __label__ = 36; break;
    case 37: // _bb123
      _j = 0;
      __label__ = 105; break;
    case 109: // _bb124
      var _510 = HEAP[0 + _w+_j*1];
      var _511 = 0+(_510 != 0);
      if (_511) { __label__ = 106; break; } else { __label__ = 107; break; }
    case 106: // _bb125
      var _514 = HEAP[0 + _w+_j*1];
      var _515 = 0+(_514 != 0);
      if (_515) { __label__ = 108; break; } else { __label__ = 107; break; }
    case 108: // _bb126
      var _518 = HEAP[0 + _w+_j*1];
      __ZdaPv(_518);
      __label__ = 107; break;
    case 107: // _bb127
      _j = _j + 1;
      __label__ = 105; break;
    case 105: // _bb128
      var _523 = 0+(_j < _numargs);
      if (_523) { __label__ = 109; break; } else { __label__ = 36; break; }
    case 36: // _bb129
      var _toBool130 = 0+(_cont != 0);
      if (_toBool130) { __label__ = 0; break; } else { __label__ = 110; break; }
    case 110: // _bb131
      var _525 = HEAP[_retval1];
      HEAP[_0] = _525;
      var _526 = HEAP[_0];
      HEAP[_retval] = _526;
      __label__ = 2; break;
    case 2: // _return
      var _retval132 = HEAP[_retval];
      return _retval132;
  }
}


function __Z9parsewordRPKciRi(_p, _arg, _infix) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _p_addr = Pointer_make([0], 0);
      var _arg_addr;
      var _infix_addr = Pointer_make([0], 0);
      var _retval = Pointer_make([0], 0);
      var _0 = Pointer_make([0], 0);
      var _word = Pointer_make([0], 0);
      var _s = Pointer_make([0], 0);
      var _end = Pointer_make([0], 0);
      var _s5 = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_p_addr] = _p;
      _arg_addr = _arg;
      HEAP[_infix_addr] = _infix;
      __label__ = 0; break;
    case 0: // _bb
      var _1 = HEAP[_p_addr];
      var _2 = HEAP[_1];
      var _4 = HEAP[_1];
      var _5 = _strspn(_4, __str152+0*1);
      var _6 = _2+_5;
      var _7 = HEAP[_p_addr];
      HEAP[_7] = _6;
      var _8 = HEAP[_p_addr];
      var _9 = HEAP[_8];
      var _11 = HEAP[0 + _9];
      var _12 = 0+(_11 != 47);
      if (_12) { __label__ = 3; break; } else { __label__ = 1; break; }
    case 1: // _bb1
      var _13 = HEAP[_p_addr];
      var _14 = HEAP[_13];
      var _16 = HEAP[0 + _14+1];
      var _17 = 0+(_16 != 47);
      if (_17) { __label__ = 3; break; } else { __label__ = 7; break; }
    case 3: // _bb2
      var _18 = HEAP[_p_addr];
      var _19 = HEAP[_18];
      var _20 = HEAP[_19];
      var _21 = 0+(_20 == 34);
      if (_21) { __label__ = 6; break; } else { __label__ = 10; break; }
    case 7: // _bb3
      var _22 = HEAP[_p_addr];
      var _23 = HEAP[_22];
      var _25 = HEAP[_22];
      var _26 = _strcspn(_25, __str150+0*1);
      var _27 = _23+_26;
      var _28 = HEAP[_p_addr];
      HEAP[_28] = _27;
      __label__ = 0; break;
    case 6: // _bb4
      var _29 = HEAP[_p_addr];
      var _30 = HEAP[_29];
      var _31 = _30+1;
      HEAP[_29] = _31;
      var _33 = HEAP[_p_addr];
      var _34 = HEAP[_33];
      var _35 = __Z11parsestringPKc(_34);
      HEAP[_end] = _35;
      var _36 = HEAP[_end];
      var _37 = _36;
      var _38 = HEAP[_p_addr];
      var _39 = HEAP[_38];
      var _40 = _39;
      var _41 = _37 - _40;
      var _42 = __Z9newstringj(_41);
      HEAP[_s5] = _42;
      var _43 = HEAP[_p_addr];
      var _44 = HEAP[_43];
      var _45 = HEAP[_s5];
      var _46 = HEAP[_end];
      var _47 = __Z12escapestringPcPKcS1_(_45, _44, _46);
      var _48 = HEAP[_s5];
      HEAP[0 + _48+_47] = 0;
      var _50 = HEAP[_p_addr];
      var _51 = HEAP[_end];
      HEAP[_50] = _51;
      var _52 = HEAP[_p_addr];
      var _53 = HEAP[_52];
      var _54 = HEAP[_53];
      var _55 = 0+(_54 == 34);
      if (_55) { __label__ = 8; break; } else { __label__ = 5; break; }
    case 8: // _bb6
      var _56 = HEAP[_p_addr];
      var _57 = HEAP[_56];
      var _58 = _57+1;
      HEAP[_56] = _58;
      __label__ = 5; break;
    case 5: // _bb7
      var _60 = HEAP[_s5];
      HEAP[_0] = _60;
      __label__ = 34; break;
    case 10: // _bb8
      var _61 = HEAP[_p_addr];
      var _62 = HEAP[_61];
      var _63 = HEAP[_62];
      var _64 = 0+(_63 == 40);
      if (_64) { __label__ = 9; break; } else { __label__ = 11; break; }
    case 9: // _bb9
      var _65 = HEAP[_p_addr];
      var _66 = __Z8parseexpRPKci(_65, 41);
      HEAP[_0] = _66;
      __label__ = 34; break;
    case 11: // _bb10
      var _67 = HEAP[_p_addr];
      var _68 = HEAP[_67];
      var _69 = HEAP[_68];
      var _70 = 0+(_69 == 91);
      if (_70) { __label__ = 13; break; } else { __label__ = 14; break; }
    case 13: // _bb11
      var _71 = HEAP[_p_addr];
      var _72 = __Z8parseexpRPKci(_71, 93);
      HEAP[_0] = _72;
      __label__ = 34; break;
    case 14: // _bb12
      var _73 = HEAP[_p_addr];
      var _74 = HEAP[_73];
      HEAP[_word] = _74;
      __label__ = 15; break;
    case 15: // _bb13
      var _75 = HEAP[_p_addr];
      var _76 = HEAP[_75];
      var _78 = HEAP[_75];
      var _79 = _strcspn(_78, __str153+0*1);
      var _80 = _76+_79;
      var _81 = HEAP[_p_addr];
      HEAP[_81] = _80;
      var _82 = HEAP[_p_addr];
      var _83 = HEAP[_82];
      var _85 = HEAP[0 + _83];
      var _86 = 0+(_85 != 47);
      if (_86) { __label__ = 21; break; } else { __label__ = 12; break; }
    case 12: // _bb14
      var _87 = HEAP[_p_addr];
      var _88 = HEAP[_87];
      var _90 = HEAP[0 + _88+1];
      var _91 = 0+(_90 == 47);
      if (_91) { __label__ = 21; break; } else { __label__ = 16; break; }
    case 21: // _bb15
      __label__ = 18; break;
    case 16: // _bb16
      var _92 = HEAP[_p_addr];
      var _93 = HEAP[_92];
      var _95 = HEAP[0 + _93+1];
      var _96 = 0+(_95 == 0);
      if (_96) { __label__ = 20; break; } else { __label__ = 17; break; }
    case 20: // _bb17
      var _97 = HEAP[_p_addr];
      var _98 = HEAP[_97];
      var _99 = _98+1;
      HEAP[_97] = _99;
      __label__ = 18; break;
    case 17: // _bb18
      var _101 = HEAP[_p_addr];
      var _102 = HEAP[_101];
      var _103 = _102+2;
      HEAP[_101] = _103;
      __label__ = 15; break;
    case 18: // _bb19
      var _105 = HEAP[_p_addr];
      var _106 = HEAP[_105];
      var _107 = HEAP[_word];
      var _108 = 0+(_106 == _107);
      if (_108) { __label__ = 19; break; } else { __label__ = 24; break; }
    case 19: // _bb20
      HEAP[_0] = 0;
      __label__ = 34; break;
    case 24: // _bb21
      var _110 = 0+(_arg_addr == 1);
      if (_110) { __label__ = 25; break; } else { __label__ = 38; break; }
    case 25: // _bb22
      var _111 = HEAP[_p_addr];
      var _112 = HEAP[_111];
      var _113 = _112;
      var _114 = HEAP[_word];
      var _115 = _114;
      var _116 = _113 - _115;
      var _117 = 0+(_116 == 1);
      if (_117) { __label__ = 23; break; } else { __label__ = 38; break; }
    case 23: // _bb23
      var _118 = HEAP[_word];
      var _119 = HEAP[_118];
      if (_119 == 61) {
  __label__ = 22; break;
}
else {
__label__ = 38; break;
}

    case 22: // _bb24
      var _120 = HEAP[_word];
      var _121 = HEAP[_120];
      var _122 = _121;
      var _123 = HEAP[_infix_addr];
      HEAP[_123] = _122;
      __label__ = 38; break;
    case 38: // _bb25
      var _124 = HEAP[_p_addr];
      var _125 = HEAP[_124];
      var _126 = _125;
      var _127 = HEAP[_word];
      var _128 = _127;
      var _129 = _126 - _128;
      var _131 = __Z9newstringPKcj(_127, _129);
      HEAP[_s] = _131;
      var _132 = HEAP[_s];
      var _133 = HEAP[_132];
      var _134 = 0+(_133 == 36);
      if (_134) { __label__ = 39; break; } else { __label__ = 33; break; }
    case 39: // _bb26
      var _135 = HEAP[_s];
      var _136 = __Z6lookupPc(_135);
      HEAP[_0] = _136;
      __label__ = 34; break;
    case 33: // _bb27
      var _137 = HEAP[_s];
      HEAP[_0] = _137;
      __label__ = 34; break;
    case 34: // _bb28
      var _138 = HEAP[_0];
      HEAP[_retval] = _138;
      __label__ = 2; break;
    case 2: // _return
      var _retval29 = HEAP[_retval];
      return _retval29;
  }
}


function __Z7executePKc(_p) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _p_addr = Pointer_make([0], 0);
      var _retval;
      var _0;
      var _ret = Pointer_make([0], 0);
      var _i;
      var __alloca_point_ = 0;
      HEAP[_p_addr] = _p;
      var _1 = HEAP[_p_addr];
      var _2 = __Z10executeretPKc(_1);
      HEAP[_ret] = _2;
      _i = 0;
      var _3 = HEAP[_ret];
      var _4 = 0+(_3 != 0);
      if (_4) { __label__ = 0; break; } else { __label__ = 3; break; }
    case 0: // _bb
      var _5 = HEAP[_ret];
      _i = __ZL8parseintPKc(_5);
      var _7 = HEAP[_ret];
      var _8 = 0+(_7 != 0);
      if (_8) { __label__ = 1; break; } else { __label__ = 3; break; }
    case 1: // _bb1
      var _9 = HEAP[_ret];
      __ZdaPv(_9);
      __label__ = 3; break;
    case 3: // _bb2
      _0 = _i;
      _retval = _0;
      __label__ = 2; break;
    case 2: // _return
      var _retval3 = _retval;
      return _retval3;
  }
}


function _main() {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _retval;
      var _0;
      var __alloca_point_ = 0;
      var _1 = _puts(__str50+0*1);
      var _2 = __Z7executePKc(__str135+0*1);
      var _3 = __Z7executePKc(__str136+0*1);
      var _4 = __Z7executePKc(__str137+0*1);
      var _5 = __Z7executePKc(__str138+0*1);
      var _6 = __Z7executePKc(__str139+0*1);
      var _7 = __Z6getvarPKc(__str123+0*1);
      var _8 = _printf(__str140+0*1, _7);
      var _9 = __Z7executePKc(__str141+0*1);
      var _10 = __Z7executePKc(__str142+0*1);
      var _11 = __Z7executePKc(__str143+0*1);
      var _12 = __Z7executePKc(__str144+0*1);
      var _13 = __Z7executePKc(__str145+0*1);
      var _14 = __Z7executePKc(__str146+0*1);
      var _15 = __Z7executePKc(__str147+0*1);
      var _16 = _puts(__str50+0*1);
      _0 = 0;
      _retval = _0;
      __label__ = 2; break;
    case 2: // _return
      var _retval1 = _retval;
      return _retval1;
  }
}


function __Z8looplistPKcS0_S0_b(_var, _list, _body, _search) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _var_addr = Pointer_make([0], 0);
      var _list_addr = Pointer_make([0], 0);
      var _body_addr = Pointer_make([0], 0);
      var _search_addr;
      var _iftmp_173;
      var _retval_172;
      var _retval_171;
      var _id = Pointer_make([0], 0);
      var _n;
      var _s = Pointer_make([0], 0);
      var _start = Pointer_make([0], 0);
      var _end = Pointer_make([0], 0);
      var _val = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_var_addr] = _var;
      HEAP[_list_addr] = _list;
      HEAP[_body_addr] = _body;
      _search_addr = _search;
      var _0 = HEAP[_var_addr];
      var _1 = __Z8newidentPKc(_0);
      HEAP[_id] = _1;
      var _2 = HEAP[_id];
      var _4 = HEAP[0 + _2+1*1];
      var _5 = 0+(_4 != 5);
      if (_5) { __label__ = 0; break; } else { __label__ = 7; break; }
    case 0: // _bb
      var _toBool = 0+(_search_addr != 0);
      if (_toBool) { __label__ = 1; break; } else { __label__ = 3; break; }
    case 1: // _bb1
      __Z6intreti(-1);
      __label__ = 3; break;
    case 3: // _bb2
      __label__ = 111; break;
    case 7: // _bb3
      _n = 0;
      var _7 = HEAP[_list_addr];
      HEAP[_s] = _7;
      __label__ = 6; break;
    case 6: // _bb4
      var _8 = HEAP[_s];
      var _9 = _strspn(_8, __str12+0*1);
      var _10 = HEAP[_s];
      var _11 = _10+_9;
      HEAP[_s] = _11;
      var _12 = HEAP[_s];
      var _13 = HEAP[_12];
      var _14 = 0+(_13 == 0);
      if (_14) { __label__ = 4; break; } else { __label__ = 9; break; }
    case 4: // _bb5
      var _toBool6 = 0+(_search_addr != 0);
      if (_toBool6) { __label__ = 5; break; } else { __label__ = 10; break; }
    case 5: // _bb7
      __Z6intreti(-1);
      __label__ = 10; break;
    case 10: // _bb8
      __label__ = 35; break;
    case 9: // _bb9
      var _16 = HEAP[_s];
      HEAP[_start] = _16;
      var _17 = HEAP[_s];
      var _18 = HEAP[_17];
      var _19 = 0+(_18 == 34);
      if (_19) { __label__ = 11; break; } else { __label__ = 13; break; }
    case 11: // _bb10
      var _20 = HEAP[_s];
      var _21 = _20+1;
      HEAP[_s] = _21;
      var _22 = HEAP[_s];
      var _23 = _strcspn(_22, __str13+0*1);
      var _24 = HEAP[_s];
      var _25 = _24+_23;
      HEAP[_s] = _25;
      var _26 = HEAP[_s];
      var _27 = HEAP[_26];
      var _28 = 0+(_27 == 34);
      var _29 = _28;
      var _31 = _26+_29;
      HEAP[_s] = _31;
      __label__ = 14; break;
    case 13: // _bb11
      var _32 = HEAP[_s];
      var _33 = _strcspn(_32, __str14+0*1);
      var _34 = HEAP[_s];
      var _35 = _34+_33;
      HEAP[_s] = _35;
      __label__ = 14; break;
    case 14: // _bb12
      var _36 = HEAP[_s];
      HEAP[_end] = _36;
      var _37 = HEAP[_start];
      var _38 = HEAP[_37];
      var _39 = 0+(_38 == 34);
      if (_39) { __label__ = 15; break; } else { __label__ = 21; break; }
    case 15: // _bb13
      var _40 = HEAP[_start];
      var _41 = _40+1;
      HEAP[_start] = _41;
      var _42 = HEAP[_end];
      var _44 = HEAP[0 + _42+-1];
      var _45 = 0+(_44 == 34);
      if (_45) { __label__ = 12; break; } else { __label__ = 21; break; }
    case 12: // _bb14
      var _46 = HEAP[_end];
      var _47 = _46+-1;
      HEAP[_end] = _47;
      __label__ = 21; break;
    case 21: // _bb15
      var _48 = HEAP[_end];
      var _49 = _48;
      var _50 = HEAP[_start];
      var _51 = _50;
      var _52 = _49 - _51;
      var _54 = __Z9newstringPKcj(_50, _52);
      HEAP[_val] = _54;
      _n = _n + 1;
      var _58 = 0+(_n != 1);
      _retval_171 = _58;
      var _toBool16 = 0+(_retval_171 != 0);
      if (_toBool16) { __label__ = 20; break; } else { __label__ = 17; break; }
    case 20: // _bb17
      var _61 = HEAP[_id];
      var _63 = HEAP[0 + _61+2*1];
      var _64 = HEAP[_val];
      __Z6aliasaPKcPc(_63, _64);
      __label__ = 18; break;
    case 17: // _bb18
      var _65 = HEAP[_id];
      var _66 = HEAP[_val];
      __Z9pushidentR5identPc(_65, _66);
      __label__ = 18; break;
    case 18: // _bb19
      var _67 = HEAP[_body_addr];
      var _68 = __Z7executePKc(_67);
      var _69 = 0+(_68 == 0);
      if (_69) { __label__ = 39; break; } else { __label__ = 19; break; }
    case 19: // _bb20
      var _toBool21 = 0+(_search_addr != 0);
      var _toBool21not = _toBool21 ^ true;
      var _toBool21not22 = _toBool21not;
      var _toBool24 = 0+(_toBool21not22 != 0);
      if (_toBool24) { __label__ = 39; break; } else { __label__ = 38; break; }
    case 38: // _bb25
      _iftmp_173 = 1;
      __label__ = 33; break;
    case 39: // _bb26
      _iftmp_173 = 0;
      __label__ = 33; break;
    case 33: // _bb27
      _retval_172 = _iftmp_173;
      var _toBool28 = 0+(_retval_172 != 0);
      if (_toBool28) { __label__ = 32; break; } else { __label__ = 40; break; }
    case 32: // _bb29
      var _74 = _n - 1;
      __Z6intreti(_74);
      __label__ = 35; break;
    case 40: // _bb30
      __label__ = 6; break;
    case 35: // _bb31
      var _76 = 0+(_n != 0);
      if (_76) { __label__ = 31; break; } else { __label__ = 111; break; }
    case 31: // _bb32
      var _77 = HEAP[_id];
      __Z8popidentR5ident(_77);
      __label__ = 111; break;
    case 111: // _bb33
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __Z8parseexpRPKci(_p, _right) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _p_addr = Pointer_make([0], 0);
      var _right_addr;
      var _retval = Pointer_make([0], 0);
      var _iftmp_98 = Pointer_make([0], 0);
      var _0 = Pointer_make([0], 0);
      var _1 = Pointer_make([0], 0);
      var _2 = Pointer_make([0], 0);
      var _3 = Pointer_make([0], 0);
      var _4 = Pointer_make([0], 0);
      var _5 = Pointer_make([0], 0);
      var _retval_92;
      var _wordbuf = Pointer_make([0], 0);
      var _left;
      var _s = Pointer_make([0], 0);
      var _brak;
      var _n;
      var _c;
      var _end = Pointer_make([0], 0);
      var _ret = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_p_addr] = _p;
      _right_addr = _right;
      var _6 = HEAP[__ZL7bufnest];
      var _7 = __ZNK6vectorIPS_IcEE6lengthEv(__ZL8wordbufs);
      var _8 = 0+(_6 >= _7);
      _retval_92 = _8;
      var _10 = _6 + 1;
      HEAP[__ZL7bufnest] = _10;
      var _toBool = 0+(_retval_92 != 0);
      if (_toBool) { __label__ = 0; break; } else { __label__ = 1; break; }
    case 0: // _bb
      var _12 = __Znwj(12);
      HEAP[_4] = _12;
      var _14 = HEAP[_4];
      __ZN6vectorIcEC1Ev(_14);
      var _15 = HEAP[_4];
      HEAP[_5] = _15;
      var _16 = __ZN6vectorIPS_IcEE3addERKS1_(__ZL8wordbufs, _5);
      __label__ = 1; break;
    case 1: // _bb1
      var _17 = HEAP[__ZL7bufnest];
      var _18 = _17 - 1;
      var _19 = __ZN6vectorIPS_IcEEixEi(__ZL8wordbufs, _18);
      var _20 = HEAP[_19];
      HEAP[_wordbuf] = _20;
      var _21 = HEAP[_p_addr];
      var _22 = HEAP[_21];
      var _23 = HEAP[_22];
      _left = _23;
      var _25 = _22+1;
      HEAP[_21] = _25;
      _brak = 1;
      __label__ = 19; break;
    case 3: // _bb2
      var _27 = HEAP[_p_addr];
      var _28 = HEAP[_27];
      _n = _strcspn(_28, __str149+0*1);
      var _30 = _n;
      var _31 = HEAP[_p_addr];
      var _32 = HEAP[_31];
      var _33 = HEAP[_wordbuf];
      __ZN6vectorIcE3putEPKci(_33, _32, _30);
      var _34 = HEAP[_p_addr];
      var _35 = HEAP[_34];
      var _37 = _35+_n;
      HEAP[_34] = _37;
      var _39 = HEAP[_p_addr];
      var _40 = HEAP[_39];
      var _41 = HEAP[_40];
      _c = _41;
      var _43 = _40+1;
      HEAP[_39] = _43;
      if (_c == 0) {
  __label__ = 15; break;
}
else if (_c == 13) {
  __label__ = 7; break;
}
else if (_c == 34) {
  __label__ = 5; break;
}
else if (_c == 40) {
  __label__ = 12; break;
}
else if (_c == 41) {
  __label__ = 20; break;
}
else if (_c == 47) {
  __label__ = 11; break;
}
else if (_c == 64) {
  __label__ = 6; break;
}
else if (_c == 91) {
  __label__ = 12; break;
}
else if (_c == 93) {
  __label__ = 20; break;
}
else {
__label__ = 18; break;
}

    case 7: // _bb3
      __label__ = 19; break;
    case 6: // _bb4
      var _47 = 0+(_left == 91);
      if (_47) { __label__ = 4; break; } else { __label__ = 8; break; }
    case 4: // _bb5
      var _48 = HEAP[_p_addr];
      var _49 = _brak;
      var _50 = HEAP[_wordbuf];
      __Z10parsemacroRPKciR6vectorIcE(_48, _49, _50);
      __label__ = 19; break;
    case 8: // _bb6
      __label__ = 18; break;
    case 5: // _bb7
      var _52 = _c;
      HEAP[_3] = _52;
      var _53 = HEAP[_wordbuf];
      var _54 = __ZN6vectorIcE3addERKc(_53, _3);
      var _55 = HEAP[_p_addr];
      var _56 = HEAP[_55];
      var _57 = __Z11parsestringPKc(_56);
      HEAP[_end] = _57;
      var _58 = HEAP[_end];
      var _59 = _58;
      var _60 = HEAP[_p_addr];
      var _61 = HEAP[_60];
      var _62 = _61;
      var _63 = _59 - _62;
      var _65 = HEAP[_60];
      var _66 = HEAP[_wordbuf];
      __ZN6vectorIcE3putEPKci(_66, _65, _63);
      var _67 = HEAP[_p_addr];
      var _68 = HEAP[_end];
      HEAP[_67] = _68;
      var _69 = HEAP[_p_addr];
      var _70 = HEAP[_69];
      var _71 = HEAP[_70];
      var _72 = 0+(_71 == 34);
      if (_72) { __label__ = 10; break; } else { __label__ = 9; break; }
    case 10: // _bb8
      var _73 = HEAP[_p_addr];
      var _74 = HEAP[_73];
      var _75 = _74+1;
      HEAP[_73] = _75;
      var _77 = HEAP[_wordbuf];
      var _78 = __ZN6vectorIcE3addERKc(_77, _74);
      __label__ = 9; break;
    case 9: // _bb9
      __label__ = 19; break;
    case 11: // _bb10
      var _79 = HEAP[_p_addr];
      var _80 = HEAP[_79];
      var _81 = HEAP[_80];
      var _82 = 0+(_81 == 47);
      if (_82) { __label__ = 13; break; } else { __label__ = 14; break; }
    case 13: // _bb11
      var _83 = HEAP[_p_addr];
      var _84 = HEAP[_83];
      var _86 = HEAP[_83];
      var _87 = _strcspn(_86, __str150+0*1);
      var _88 = _84+_87;
      var _89 = HEAP[_p_addr];
      HEAP[_89] = _88;
      __label__ = 19; break;
    case 14: // _bb12
      __label__ = 18; break;
    case 15: // _bb13
      var _90 = HEAP[_p_addr];
      var _91 = HEAP[_90];
      var _92 = _91+-1;
      HEAP[_90] = _92;
      __Z7conoutfiPKcz(4, __str151+0*1, _right_addr);
      var _95 = HEAP[_wordbuf];
      __ZN6vectorIcE7setsizeEi(_95, 0);
      var _96 = HEAP[__ZL7bufnest];
      var _97 = _96 - 1;
      HEAP[__ZL7bufnest] = _97;
      HEAP[_2] = 0;
      __label__ = 34; break;
    case 12: // _bb14
      var _100 = 0+(_c == _left);
      if (_100) { __label__ = 21; break; } else { __label__ = 16; break; }
    case 21: // _bb15
      _brak = _brak + 1;
      __label__ = 16; break;
    case 16: // _bb16
      __label__ = 18; break;
    case 20: // _bb17
      var _105 = 0+(_c == _right_addr);
      if (_105) { __label__ = 17; break; } else { __label__ = 18; break; }
    case 17: // _bb18
      _brak = _brak - 1;
      __label__ = 18; break;
    case 18: // _bb19
      var _109 = _c;
      HEAP[_1] = _109;
      var _110 = HEAP[_wordbuf];
      var _111 = __ZN6vectorIcE3addERKc(_110, _1);
      __label__ = 19; break;
    case 19: // _bb20
      var _113 = 0+(_brak != 0);
      if (_113) { __label__ = 3; break; } else { __label__ = 24; break; }
    case 24: // _bb21
      var _114 = HEAP[_wordbuf];
      var _115 = __ZN6vectorIcE3popEv(_114);
      var _117 = 0+(_left == 40);
      if (_117) { __label__ = 25; break; } else { __label__ = 39; break; }
    case 25: // _bb22
      HEAP[_0] = 0;
      var _118 = HEAP[_wordbuf];
      var _119 = __ZN6vectorIcE3addERKc(_118, _0);
      var _120 = HEAP[_wordbuf];
      var _121 = __ZN6vectorIcE6getbufEv(_120);
      var _122 = __Z10executeretPKc(_121);
      HEAP[_ret] = _122;
      var _123 = HEAP[_wordbuf];
      var _124 = __ZN6vectorIcE3popEv(_123);
      var _125 = HEAP[_ret];
      var _126 = 0+(_125 == 0);
      if (_126) { __label__ = 23; break; } else { __label__ = 22; break; }
    case 23: // _bb23
      var _127 = __Z9newstringPKc(__str3+0*1);
      HEAP[_iftmp_98] = _127;
      __label__ = 38; break;
    case 22: // _bb24
      var _128 = HEAP[_ret];
      HEAP[_iftmp_98] = _128;
      __label__ = 38; break;
    case 38: // _bb25
      var _129 = HEAP[_iftmp_98];
      HEAP[_s] = _129;
      __label__ = 33; break;
    case 39: // _bb26
      var _130 = HEAP[_wordbuf];
      var _131 = __ZNK6vectorIcE6lengthEv(_130);
      var _132 = HEAP[_wordbuf];
      var _133 = __ZN6vectorIcE6getbufEv(_132);
      var _134 = __Z9newstringPKcj(_133, _131);
      HEAP[_s] = _134;
      __label__ = 33; break;
    case 33: // _bb27
      var _135 = HEAP[_wordbuf];
      __ZN6vectorIcE7setsizeEi(_135, 0);
      var _136 = HEAP[__ZL7bufnest];
      var _137 = _136 - 1;
      HEAP[__ZL7bufnest] = _137;
      var _138 = HEAP[_s];
      HEAP[_2] = _138;
      __label__ = 34; break;
    case 34: // _bb28
      var _139 = HEAP[_2];
      HEAP[_retval] = _139;
      __label__ = 2; break;
    case 2: // _return
      var _retval29 = HEAP[_retval];
      return _retval29;
  }
}


function __Z10parsemacroRPKciR6vectorIcE(_p, _level, _wordbuf) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _p_addr = Pointer_make([0], 0);
      var _level_addr;
      var _wordbuf_addr = Pointer_make([0], 0);
      var _0 = Pointer_make([0], 0);
      var _1;
      var _retval_87;
      var _2 = Pointer_make([0], 0);
      var _retval_84;
      var _escape;
      var _alias = Pointer_make([0], 0);
      var _ret = Pointer_make([0], 0);
      var _sub = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_p_addr] = _p;
      _level_addr = _level;
      HEAP[_wordbuf_addr] = _wordbuf;
      _escape = 1;
      __label__ = 1; break;
    case 0: // _bb
      var _3 = HEAP[_p_addr];
      var _4 = HEAP[_3];
      var _5 = _4+1;
      HEAP[_3] = _5;
      _escape = _escape + 1;
      __label__ = 1; break;
    case 1: // _bb1
      var _9 = HEAP[_p_addr];
      var _10 = HEAP[_9];
      var _11 = HEAP[_10];
      var _12 = 0+(_11 == 64);
      if (_12) { __label__ = 0; break; } else { __label__ = 3; break; }
    case 3: // _bb2
      var _15 = 0+(_level_addr > _escape);
      if (_15) { __label__ = 7; break; } else { __label__ = 5; break; }
    case 7: // _bb3
      __label__ = 4; break;
    case 6: // _bb4
      HEAP[_2] = 64;
      var _16 = HEAP[_wordbuf_addr];
      var _17 = __ZN6vectorIcE3addERKc(_16, _2);
      __label__ = 4; break;
    case 4: // _bb5
      _escape = _escape - 1;
      var _21 = 0+(_escape != -1);
      _retval_84 = _21;
      var _toBool = 0+(_retval_84 != 0);
      if (_toBool) { __label__ = 6; break; } else { __label__ = 8; break; }
    case 8: // _bb6
      __label__ = 39; break;
    case 5: // _bb7
      var _24 = HEAP[_p_addr];
      var _25 = HEAP[_24];
      var _26 = HEAP[_25];
      var _27 = 0+(_26 == 40);
      if (_27) { __label__ = 10; break; } else { __label__ = 21; break; }
    case 10: // _bb8
      var _28 = HEAP[_p_addr];
      var _29 = __Z8parseexpRPKci(_28, 41);
      HEAP[_ret] = _29;
      var _30 = HEAP[_ret];
      var _31 = 0+(_30 != 0);
      if (_31) { __label__ = 9; break; } else { __label__ = 12; break; }
    case 9: // _bb9
      var _32 = HEAP[_ret];
      HEAP[_sub] = _32;
      __label__ = 13; break;
    case 11: // _bb10
      var _33 = HEAP[_sub];
      var _35 = _33+1;
      HEAP[_sub] = _35;
      var _36 = HEAP[_wordbuf_addr];
      var _37 = __ZN6vectorIcE3addERKc(_36, _33);
      __label__ = 13; break;
    case 13: // _bb11
      var _38 = HEAP[_sub];
      var _39 = HEAP[_38];
      var _40 = 0+(_39 != 0);
      if (_40) { __label__ = 11; break; } else { __label__ = 14; break; }
    case 14: // _bb12
      var _41 = HEAP[_ret];
      var _42 = 0+(_41 != 0);
      if (_42) { __label__ = 15; break; } else { __label__ = 12; break; }
    case 15: // _bb13
      var _43 = HEAP[_ret];
      __ZdaPv(_43);
      __label__ = 12; break;
    case 12: // _bb14
      __label__ = 39; break;
    case 21: // _bb15
      var _44 = HEAP[__ZGVZ10parsemacroRPKciR6vectorIcEE5ident];
      var _45 = 0+(_44 == 0);
      if (_45) { __label__ = 16; break; } else { __label__ = 18; break; }
    case 16: // _bb16
      var _46 = ___cxa_guard_acquire(__ZGVZ10parsemacroRPKciR6vectorIcEE5ident);
      var _47 = 0+(_46 != 0);
      _retval_87 = _47;
      var _toBool17 = 0+(_retval_87 != 0);
      if (_toBool17) { __label__ = 17; break; } else { __label__ = 18; break; }
    case 17: // _bb18
      _1 = 0;
      __ZN6vectorIcEC1Ev(__ZZ10parsemacroRPKciR6vectorIcEE5ident);
      _1 = 1;
      ___cxa_guard_release(__ZGVZ10parsemacroRPKciR6vectorIcEE5ident);
      var _50 = ___cxa_atexit(___tcf_0, 0, ___dso_handle);
      __label__ = 18; break;
    case 18: // _bb19
      __ZN6vectorIcE7setsizeEi(__ZZ10parsemacroRPKciR6vectorIcEE5ident, 0);
      __label__ = 24; break;
    case 19: // _bb20
      var _51 = HEAP[_p_addr];
      var _52 = HEAP[_51];
      var _53 = _52+1;
      HEAP[_51] = _53;
      var _55 = __ZN6vectorIcE3addERKc(__ZZ10parsemacroRPKciR6vectorIcEE5ident, _52);
      __label__ = 24; break;
    case 24: // _bb21
      var _56 = HEAP[_p_addr];
      var _57 = HEAP[_56];
      var _58 = HEAP[_57];
      var _59 = _58;
      var _60 = _isalnum(_59);
      var _61 = 0+(_60 != 0);
      if (_61) { __label__ = 19; break; } else { __label__ = 25; break; }
    case 25: // _bb22
      var _62 = HEAP[_p_addr];
      var _63 = HEAP[_62];
      var _64 = HEAP[_63];
      var _65 = 0+(_64 == 95);
      if (_65) { __label__ = 19; break; } else { __label__ = 23; break; }
    case 23: // _bb23
      HEAP[_0] = 0;
      var _66 = __ZN6vectorIcE3addERKc(__ZZ10parsemacroRPKciR6vectorIcEE5ident, _0);
      var _67 = __ZN6vectorIcE6getbufEv(__ZZ10parsemacroRPKciR6vectorIcEE5ident);
      var _68 = __Z8getaliasPKc(_67);
      HEAP[_alias] = _68;
      __label__ = 38; break;
    case 22: // _bb24
      var _69 = HEAP[_alias];
      var _71 = _69+1;
      HEAP[_alias] = _71;
      var _72 = HEAP[_wordbuf_addr];
      var _73 = __ZN6vectorIcE3addERKc(_72, _69);
      __label__ = 38; break;
    case 38: // _bb25
      var _74 = HEAP[_alias];
      var _75 = HEAP[_74];
      var _76 = 0+(_75 != 0);
      if (_76) { __label__ = 22; break; } else { __label__ = 39; break; }
    case 39: // _bb26
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __ZN7hashsetI14hashtableentryIPKc5identEE12deletechunksEv(_this) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _this_addr = Pointer_make([0], 0);
      var _0 = Pointer_make([0], 0);
      var _nextchunk = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_this_addr] = _this;
      __label__ = 7; break;
    case 0: // _bb
      var _1 = HEAP[_this_addr];
      var _3 = HEAP[0 + _1+3*1];
      var _5 = HEAP[0 + _3+__struct_hashset_hashtableentry_const_char___ident_____chainchunk____FLATTENER[1]];
      HEAP[_nextchunk] = _5;
      var _6 = HEAP[_this_addr];
      var _8 = HEAP[0 + _6+3*1];
      HEAP[_0] = _8;
      var _9 = HEAP[_0];
      var _10 = 0+(_9 != 0);
      if (_10) { __label__ = 1; break; } else { __label__ = 3; break; }
    case 1: // _bb1
      var _11 = HEAP[_0];
      __ZN7hashsetI14hashtableentryIPKc5identEE10chainchunkD1Ev(_11);
      var _12 = HEAP[_0];
      __ZdlPv(_12);
      __label__ = 3; break;
    case 3: // _bb2
      var _14 = HEAP[_this_addr];
      var _15 = _14+3*1;
      var _16 = HEAP[_nextchunk];
      HEAP[_15] = _16;
      __label__ = 7; break;
    case 7: // _bb3
      var _17 = HEAP[_this_addr];
      var _19 = HEAP[0 + _17+3*1];
      var _20 = 0+(_19 != 0);
      if (_20) { __label__ = 0; break; } else { __label__ = 6; break; }
    case 6: // _bb4
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __ZN7hashsetI14hashtableentryIPKc5identEE5clearEv(_this) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _this_addr = Pointer_make([0], 0);
      var _i;
      var __alloca_point_ = 0;
      HEAP[_this_addr] = _this;
      var _0 = HEAP[_this_addr];
      var _2 = HEAP[0 + _0+1*1];
      var _3 = 0+(_2 == 0);
      if (_3) { __label__ = 6; break; } else { __label__ = 0; break; }
    case 0: // _bb
      _i = 0;
      __label__ = 3; break;
    case 1: // _bb1
      var _4 = HEAP[_this_addr];
      var _6 = HEAP[0 + _4+2*1];
      HEAP[0 + _6+_i] = 0;
      _i = _i + 1;
      __label__ = 3; break;
    case 3: // _bb2
      var _11 = HEAP[_this_addr];
      var _13 = HEAP[0 + _11+0*1];
      var _15 = 0+(_13 > _i);
      if (_15) { __label__ = 1; break; } else { __label__ = 7; break; }
    case 7: // _bb3
      var _16 = HEAP[_this_addr];
      HEAP[0 + _16+1*1] = 0;
      var _18 = HEAP[_this_addr];
      HEAP[0 + _18+4*1] = 0;
      var _20 = HEAP[_this_addr];
      __ZN7hashsetI14hashtableentryIPKc5identEE12deletechunksEv(_20);
      __label__ = 6; break;
    case 6: // _bb4
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __Z10clearstackR5ident(_id) {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _id_addr = Pointer_make([0], 0);
      var _stack = Pointer_make([0], 0);
      var _tmp = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      HEAP[_id_addr] = _id;
      var _0 = HEAP[_id_addr];
      var _1 = _0+6*1;
      var _2 = _1;
      var _4 = HEAP[_2];
      HEAP[_stack] = _4;
      __label__ = 7; break;
    case 0: // _bb
      var _5 = HEAP[_stack];
      var _7 = HEAP[0 + _5+0*1];
      var _8 = 0+(_7 != 0);
      if (_8) { __label__ = 1; break; } else { __label__ = 3; break; }
    case 1: // _bb1
      var _9 = HEAP[_stack];
      var _11 = HEAP[0 + _9+0*1];
      __ZdaPv(_11);
      __label__ = 3; break;
    case 3: // _bb2
      var _12 = HEAP[_stack];
      HEAP[_tmp] = _12;
      var _13 = HEAP[_stack];
      var _15 = HEAP[0 + _13+1*1];
      HEAP[_stack] = _15;
      var _16 = HEAP[_tmp];
      __ZdlPv(_16);
      __label__ = 7; break;
    case 7: // _bb3
      var _18 = HEAP[_stack];
      var _19 = 0+(_18 != 0);
      if (_19) { __label__ = 0; break; } else { __label__ = 6; break; }
    case 6: // _bb4
      var _20 = HEAP[_id_addr];
      var _21 = _20+6*1;
      var _22 = _21;
      HEAP[_22] = 0;
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}


function __Z13clear_commandv() {
  var __label__ = 151; /* _entry */
  while(1) switch(__label__) {
    case 151: // _entry
      var _i;
      var _enumc = Pointer_make([0], 0);
      var _i2 = Pointer_make([0], 0);
      var __alloca_point_ = 0;
      _i = 0;
      __label__ = 21; break;
    case 0: // _bb
      var _0 = HEAP[_idents];
      var _1 = _0+0*5;
      var _3 = HEAP[0 + _1+2*1];
      var _6 = HEAP[0 + _3+_i];
      HEAP[_enumc] = _6;
      __label__ = 15; break;
    case 1: // _bb1
      var _8 = HEAP[_enumc];
      var _9 = __ZN9hashtableIPKc5identE7getdataEPv(_8);
      HEAP[_i2] = _9;
      var _10 = HEAP[_enumc];
      var _11 = __ZN9hashtableIPKc5identE7getnextEPv(_10);
      HEAP[_enumc] = _11;
      var _13 = HEAP[_i2];
      var _15 = HEAP[0 + _13+1*1];
      var _16 = 0+(_15 == 5);
      if (_16) { __label__ = 7; break; } else { __label__ = 15; break; }
    case 7: // _bb3
      var _17 = HEAP[_i2];
      var _19 = HEAP[0 + _17+2*1];
      var _20 = 0+(_19 != 0);
      if (_20) { __label__ = 6; break; } else { __label__ = 5; break; }
    case 6: // _bb4
      var _21 = HEAP[_i2];
      var _23 = HEAP[0 + _21+2*1];
      var _24 = 0+(_23 != 0);
      if (_24) { __label__ = 4; break; } else { __label__ = 8; break; }
    case 4: // _bb5
      var _25 = HEAP[_i2];
      var _27 = HEAP[0 + _25+2*1];
      __ZdaPv(_27);
      __label__ = 8; break;
    case 8: // _bb6
      var _28 = HEAP[_i2];
      HEAP[0 + _28+2*1] = 0;
      __label__ = 5; break;
    case 5: // _bb7
      var _30 = HEAP[_i2];
      var _31 = _30+7*1;
      var _33 = HEAP[0 + _31+0*1];
      var _34 = 0+(_33 != 0);
      if (_34) { __label__ = 10; break; } else { __label__ = 13; break; }
    case 10: // _bb8
      var _35 = HEAP[_i2];
      var _36 = _35+7*1;
      var _38 = HEAP[0 + _36+0*1];
      var _39 = 0+(_38 != 0);
      if (_39) { __label__ = 9; break; } else { __label__ = 11; break; }
    case 9: // _bb9
      var _40 = HEAP[_i2];
      var _41 = _40+7*1;
      var _43 = HEAP[0 + _41+0*1];
      __ZdaPv(_43);
      __label__ = 11; break;
    case 11: // _bb10
      var _44 = HEAP[_i2];
      var _45 = _44+7*1;
      HEAP[0 + _45+0*1] = 0;
      __label__ = 13; break;
    case 13: // _bb11
      var _47 = HEAP[_i2];
      var _48 = _47+6*1;
      var _49 = _48;
      var _51 = HEAP[_49];
      var _52 = 0+(_51 != 0);
      if (_52) { __label__ = 14; break; } else { __label__ = 15; break; }
    case 14: // _bb12
      var _53 = HEAP[_i2];
      __Z10clearstackR5ident(_53);
      __label__ = 15; break;
    case 15: // _bb13
      var _54 = HEAP[_enumc];
      var _55 = 0+(_54 != 0);
      if (_55) { __label__ = 1; break; } else { __label__ = 12; break; }
    case 12: // _bb14
      _i = _i + 1;
      __label__ = 21; break;
    case 21: // _bb15
      var _58 = HEAP[_idents];
      var _59 = _58+0*5;
      var _61 = HEAP[0 + _59+0*1];
      var _63 = 0+(_61 > _i);
      if (_63) { __label__ = 0; break; } else { __label__ = 16; break; }
    case 16: // _bb16
      var _64 = HEAP[_idents];
      var _65 = 0+(_64 != 0);
      if (_65) { __label__ = 20; break; } else { __label__ = 17; break; }
    case 20: // _bb17
      var _66 = HEAP[_idents];
      var _67 = _66+0*5;
      __ZN7hashsetI14hashtableentryIPKc5identEE5clearEv(_67);
      __label__ = 17; break;
    case 17: // _bb18
      __label__ = 2; break;
    case 2: // _return
      return;
  }
}

// === Auto-generated postamble setup entry stuff ===

function run(args) {
  var argc = args.length+1;
  var argv = [Pointer_make(intArrayFromString("/bin/this.program")) ];
  for (var i = 0; i < argc-1; i = i + 1) {
    argv.push(Pointer_make(intArrayFromString(args[i])));
  }
  argv = Pointer_make(argv);

  __globalConstructor__();
}

try {
  run(this.arguments ? arguments : []);
} catch (e) {
  print("Fatal exception: " + e.stack);
  throw e;
}



