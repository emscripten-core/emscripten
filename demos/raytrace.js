//====================================================================================
// Auto-generated code. Original source + changes are
// sphere flake bvh raytracer (c) 2005, thierry berger-perrin <tbptbp@gmail.com>
//                            (c) 2010, azakai (Emscripten)
// this code is released under the GNU Public License.
//
// Optimized using Closure Compiler
//====================================================================================


// === Auto-generated preamble library stuff ===

function __globalConstructor__() {
}

var __THREW__ = false; // Used in checking for thrown exceptions.

var __ATEXIT__ = [];




function assert(condition, text) {
  if (!condition) {
    var text = "Assertion failed: " + text;
    print(text + ':\n' + (new Error).stack);
    throw "Assertion: " + text;
  }
}

function Pointer_niceify(ptr) {
  return { slab: HEAP, pos: ptr };
}

// Creates a pointer for a certain slab and a certain address in that slab.
// If just a slab is given, will allocate room for it and copy it there. In
// other words, do whatever is necessary in order to return a pointer, that
// points to the slab (and possibly position) we are given.

ALLOC_NORMAL = 0; // Tries to use _malloc()
ALLOC_STACK = 1; // Lives for the duration of the current function call
ALLOC_STATIC = 2; // Cannot be freed

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
  var ret = [_malloc, stackAlloc, staticAlloc][allocator ? allocator : ALLOC_STATIC](Math.max(slab.length - pos, 1));
  for (var i = 0; i < slab.length - pos; i++) {
    HEAP[ret + i] = slab[pos + i];
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

PAGE_SIZE = 4096;
function alignMemoryPage(x) {
  return Math.ceil(x/PAGE_SIZE)*PAGE_SIZE;
}

// If we don't have malloc/free implemented, use a simple implementation.
if (!this._malloc) {
  _malloc = staticAlloc;
  _free = function() { }; // leak!
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

  STACK_STACK = [];
  STACK_ROOT = STACKTOP = alignMemoryPage(10);
  if (!this['TOTAL_STACK']) TOTAL_STACK = 64*1024*100; // Reserved room for stack
  STACK_MAX = STACK_ROOT + TOTAL_STACK;

  STATICTOP = alignMemoryPage(STACK_MAX);
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
  return Pointer_make(ret, 0, ALLOC_STACK); // NB: Stored on the stack
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

function stackAlloc(size) { var ret = STACKTOP; STACKTOP += size; return ret; }
function staticAlloc(size) { var ret = STATICTOP; STATICTOP += size; return ret; }

var _0___FLATTENER = [0,1];
var _struct__1__pthread_mutex_s___FLATTENER = [0,1,2,3,4,5];
var _struct_SDL_BlitMap___FLATTENER = [];
var _struct_SDL_Color___FLATTENER = [0,1,2,3];
var _struct_SDL_Palette___FLATTENER = [0,1];
var _struct_SDL_PixelFormat___FLATTENER = [0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16];
var _struct_SDL_Rect___FLATTENER = [0,1,2,3];
var _struct_SDL_Surface___FLATTENER = [0,1,2,3,4,5,6,7,8,12,13,14,15,16];
var _struct___pthread_slist_t___FLATTENER = [0];
var _struct_basis_t___FLATTENER = [0,3,6];
var _struct_hit_t___FLATTENER = [0,3];
var _struct_node_t___FLATTENER = [0,4,8];
var _struct_private_hwdata___FLATTENER = [];
var _struct_ray_t___FLATTENER = [0,3];
var _struct_sphere_t___FLATTENER = [0,3];
var __struct_std__ios_base__Init____FLATTENER = [0];
var _struct_v_t___FLATTENER = [0,1,2];
var _union__0__45___FLATTENER = [0];
var _union_pthread_attr_t___FLATTENER = [0,1];
var _union_pthread_mutex_t___FLATTENER = [0];
var _union_pthread_mutexattr_t___FLATTENER = [0];
this.__defineGetter__("_screen", function() { delete _screen; _screen = Pointer_make([0], 0, ALLOC_STATIC); return _screen });
this.__defineGetter__("__ZStL8__ioinit", function() { delete __ZStL8__ioinit; __ZStL8__ioinit = Pointer_make([0], 0, ALLOC_STATIC); return __ZStL8__ioinit });
var ___dso_handle = 0; /* external value? */
this.__defineGetter__("__ZL5light", function() { delete __ZL5light; __ZL5light = Pointer_make([0,0,0], 0, ALLOC_STATIC); return __ZL5light });
this.__defineGetter__("__ZL4pool", function() { delete __ZL4pool; __ZL4pool = Pointer_make([0], 0, ALLOC_STATIC); return __ZL4pool });
this.__defineGetter__("__ZL3end", function() { delete __ZL3end; __ZL3end = Pointer_make([0], 0, ALLOC_STATIC); return __ZL3end });
this.__defineGetter__("__ZL4grid", function() { delete __ZL4grid; __ZL4grid = Pointer_make([ [-1.000000e+00, -0.3333333333333333], [0.3333333333333333, -1.000000e+00], [-0.3333333333333333, 1.000000e+00], [1.000000e+00, 0.3333333333333333] ], 0, ALLOC_STATIC); return __ZL4grid });

__globalConstructor__ = function() {
  __GLOBAL__I_screen();
}


_sqrt = function (x) { return Math.sqrt(x) }
__ZNSt8ios_base4InitC1Ev = function () {
    // need valid 'file descriptors'
    __ZSt4cout = 1;
    __ZSt4cerr = 2;
  }
___cxa_atexit = function (func) {
    __ATEXIT__.push(func);
  }
__ZNSt8ios_base4InitD1Ev = function () {
    // need valid 'file descriptors'
    __ZSt4cout = 1;
    __ZSt4cerr = 2;
  }
_cos = function (x) { return Math.cos(x) }
_sin = function (x) { return Math.sin(x) }
_SDL_LockSurface = function (surf) {
    var surfData = SDL_SURFACES[surf];
    surfData.image = surfData.ctx.getImageData(0, 0, surfData.width, surfData.height);
    // Copy pixel data to somewhere accessible to 'C/C++'
    var num = surfData.image.data.length;
    for (var i = 0; i < num; i++) {
      HEAP[surfData.buffer+i] = surfData.image.data[i];
    }
    // Mark in C/C++-accessible SDL structure
    // SDL_Surface has the following fields: Uint32 flags, SDL_PixelFormat *format; int w, h; Uint16 pitch; void *pixels; ...
    // So we have fields all of the same size, and 5 of them before us.
    HEAP[surf + 5*1] = surfData.buffer;
  }
_SDL_UnlockSurface = function (surf) {
    var surfData = SDL_SURFACES[surf];
    // Copy pixel data to image
    var num = surfData.image.data.length;
    for (var i = 0; i < num; i++) {
      surfData.image.data[i] = HEAP[surfData.buffer+i];
    }
    for (var i = 0; i < num/4; i++) {
      surfData.image.data[i*4+3] = 255; // opacity, as canvases blend alpha
    }
    // Copy to canvas
    surfData.ctx.putImageData(surfData.image, 0, 0);
    // Cleanup
    surfData.image = null;
  }
_SDL_Flip = function (surf) {
    // We actually do this in Unlock...
  }
// stub for _atoi
// stub for __Znaj
_SDL_Init = function (what) {
    SDL_SURFACES = {};
    return 1;
  }
_SDL_SetVideoMode = function (width, height, depth, flags, canvas) {
  //                                                      ^^^^^^ a 'canvas' parameter is added here; supply a canvas from JS there
    var surf = _malloc(14*1); // SDL_Surface has 14 fields of quantum size
    SDL_SURFACES[surf] = {
      width: width,
      height: height,
      canvas: canvas,
      ctx: canvas.getContext('2d'),
      surf: surf,
      buffer: _malloc(width*height*4),
    };
    return surf;
  }
_SDL_Delay = function (delay) {
    // No can do... unless you were a generator...
  }
_SDL_Quit = function () {
    return 1;
  }
// stub for i32
// stub for i8_
// stub for i32
// stub for i32
// stub for i32
// stub for i32
// stub for i32
// stub for i32
// stub for i32
// stub for i32
// stub for i32
// stub for i32
// stub for i32
// stub for i32


function __GLOBAL__I_screen() {
  STACK_STACK.push(STACKTOP);
  var __label__;
  __Z41__static_initialization_and_destruction_0ii(1, 65535);
  __label__ = 0; /* _return */ 
  STACKTOP = STACK_STACK.pop();
  return;
}


function __ZnwjPv(_unnamed_arg, ___p) {
  STACK_STACK.push(STACKTOP);
  var __label__;
  var _unnamed_arg_addr;
  var ___p_addr = STACKTOP; STACKTOP += 1;
  var _retval = STACKTOP; STACKTOP += 1;
  var _0 = STACKTOP; STACKTOP += 1;
  var __alloca_point_ = 0;
  _unnamed_arg_addr = _unnamed_arg;
  HEAP[___p_addr] = ___p;
  var _1 = HEAP[___p_addr];
  HEAP[_0] = _1;
  var _2 = HEAP[_0];
  HEAP[_0] = _2;
  var _3 = HEAP[_0];
  HEAP[_retval] = _3;
  __label__ = 0; /* _return */ 
  var _retval1 = HEAP[_retval];
  STACKTOP = STACK_STACK.pop();
  return _retval1;
}


function __ZN3v_tC1Ev(_this) {
  STACK_STACK.push(STACKTOP);
  var __label__;
  var _this_addr = STACKTOP; STACKTOP += 1;
  var __alloca_point_ = 0;
  HEAP[_this_addr] = _this;
  __label__ = 0; /* _return */ 
  STACKTOP = STACK_STACK.pop();
  return;
}


function __ZNK3v_tplERKS_(_agg_result, _this, _v) {
  STACK_STACK.push(STACKTOP);
  var __label__;
  var _this_addr = STACKTOP; STACKTOP += 1;
  var _v_addr = STACKTOP; STACKTOP += 1;
  var __alloca_point_ = 0;
  HEAP[_this_addr] = _this;
  HEAP[_v_addr] = _v;
  var _0 = HEAP[_this_addr];
  var _2 = HEAP[0 + _0+2*1];
  var _3 = HEAP[_v_addr];
  var _5 = HEAP[0 + _3+2*1];
  var _6 = _2 + _5;
  var _7 = HEAP[_this_addr];
  var _9 = HEAP[0 + _7+1*1];
  var _10 = HEAP[_v_addr];
  var _12 = HEAP[0 + _10+1*1];
  var _13 = _9 + _12;
  var _14 = HEAP[_this_addr];
  var _16 = HEAP[0 + _14+0*1];
  var _17 = HEAP[_v_addr];
  var _19 = HEAP[0 + _17+0*1];
  var _20 = _16 + _19;
  __ZN3v_tC1Eddd(_agg_result, _20, _13, _6);
  __label__ = 0; /* _return */ 
  STACKTOP = STACK_STACK.pop();
  return;
}


function __ZNK3v_tmiERKS_(_agg_result, _this, _v) {
  STACK_STACK.push(STACKTOP);
  var __label__;
  var _this_addr = STACKTOP; STACKTOP += 1;
  var _v_addr = STACKTOP; STACKTOP += 1;
  var __alloca_point_ = 0;
  HEAP[_this_addr] = _this;
  HEAP[_v_addr] = _v;
  var _0 = HEAP[_this_addr];
  var _2 = HEAP[0 + _0+2*1];
  var _3 = HEAP[_v_addr];
  var _5 = HEAP[0 + _3+2*1];
  var _6 = _2 - _5;
  var _7 = HEAP[_this_addr];
  var _9 = HEAP[0 + _7+1*1];
  var _10 = HEAP[_v_addr];
  var _12 = HEAP[0 + _10+1*1];
  var _13 = _9 - _12;
  var _14 = HEAP[_this_addr];
  var _16 = HEAP[0 + _14+0*1];
  var _17 = HEAP[_v_addr];
  var _19 = HEAP[0 + _17+0*1];
  var _20 = _16 - _19;
  __ZN3v_tC1Eddd(_agg_result, _20, _13, _6);
  __label__ = 0; /* _return */ 
  STACKTOP = STACK_STACK.pop();
  return;
}


function __ZNK3v_tngEv(_agg_result, _this) {
  STACK_STACK.push(STACKTOP);
  var __label__;
  var _this_addr = STACKTOP; STACKTOP += 1;
  var __alloca_point_ = 0;
  HEAP[_this_addr] = _this;
  var _0 = HEAP[_this_addr];
  var _2 = HEAP[0 + _0+2*1];
  var _3 = -0.000000e+00 - _2;
  var _4 = HEAP[_this_addr];
  var _6 = HEAP[0 + _4+1*1];
  var _7 = -0.000000e+00 - _6;
  var _8 = HEAP[_this_addr];
  var _10 = HEAP[0 + _8+0*1];
  var _11 = -0.000000e+00 - _10;
  __ZN3v_tC1Eddd(_agg_result, _11, _7, _3);
  __label__ = 0; /* _return */ 
  STACKTOP = STACK_STACK.pop();
  return;
}


function __ZNK3v_tmlEd(_agg_result, _this, _d) {
  STACK_STACK.push(STACKTOP);
  var __label__;
  var _this_addr = STACKTOP; STACKTOP += 1;
  var _d_addr;
  var __alloca_point_ = 0;
  HEAP[_this_addr] = _this;
  _d_addr = _d;
  var _0 = HEAP[_this_addr];
  var _2 = HEAP[0 + _0+2*1];
  var _3 = _d_addr;
  var _4 = _2 * _3;
  var _5 = HEAP[_this_addr];
  var _7 = HEAP[0 + _5+1*1];
  var _8 = _d_addr;
  var _9 = _7 * _8;
  var _10 = HEAP[_this_addr];
  var _12 = HEAP[0 + _10+0*1];
  var _13 = _d_addr;
  var _14 = _12 * _13;
  __ZN3v_tC1Eddd(_agg_result, _14, _9, _4);
  __label__ = 0; /* _return */ 
  STACKTOP = STACK_STACK.pop();
  return;
}


function __ZNK3v_t5crossERKS_(_agg_result, _this, _v) {
  STACK_STACK.push(STACKTOP);
  var __label__;
  var _this_addr = STACKTOP; STACKTOP += 1;
  var _v_addr = STACKTOP; STACKTOP += 1;
  var __alloca_point_ = 0;
  HEAP[_this_addr] = _this;
  HEAP[_v_addr] = _v;
  var _0 = HEAP[_this_addr];
  var _2 = HEAP[0 + _0+0*1];
  var _3 = HEAP[_v_addr];
  var _5 = HEAP[0 + _3+1*1];
  var _6 = _2 * _5;
  var _7 = HEAP[_this_addr];
  var _9 = HEAP[0 + _7+1*1];
  var _10 = HEAP[_v_addr];
  var _12 = HEAP[0 + _10+0*1];
  var _13 = _9 * _12;
  var _14 = _6 - _13;
  var _15 = HEAP[_this_addr];
  var _17 = HEAP[0 + _15+2*1];
  var _18 = HEAP[_v_addr];
  var _20 = HEAP[0 + _18+0*1];
  var _21 = _17 * _20;
  var _22 = HEAP[_this_addr];
  var _24 = HEAP[0 + _22+0*1];
  var _25 = HEAP[_v_addr];
  var _27 = HEAP[0 + _25+2*1];
  var _28 = _24 * _27;
  var _29 = _21 - _28;
  var _30 = HEAP[_this_addr];
  var _32 = HEAP[0 + _30+1*1];
  var _33 = HEAP[_v_addr];
  var _35 = HEAP[0 + _33+2*1];
  var _36 = _32 * _35;
  var _37 = HEAP[_this_addr];
  var _39 = HEAP[0 + _37+2*1];
  var _40 = HEAP[_v_addr];
  var _42 = HEAP[0 + _40+1*1];
  var _43 = _39 * _42;
  var _44 = _36 - _43;
  __ZN3v_tC1Eddd(_agg_result, _44, _29, _14);
  __label__ = 0; /* _return */ 
  STACKTOP = STACK_STACK.pop();
  return;
}


function __ZNK3v_t3dotERKS_(_this, _v) {
  STACK_STACK.push(STACKTOP);
  var __label__;
  var _this_addr = STACKTOP; STACKTOP += 1;
  var _v_addr = STACKTOP; STACKTOP += 1;
  var _retval;
  var _0;
  var __alloca_point_ = 0;
  HEAP[_this_addr] = _this;
  HEAP[_v_addr] = _v;
  var _1 = HEAP[_this_addr];
  var _3 = HEAP[0 + _1+0*1];
  var _4 = HEAP[_v_addr];
  var _6 = HEAP[0 + _4+0*1];
  var _7 = _3 * _6;
  var _8 = HEAP[_this_addr];
  var _10 = HEAP[0 + _8+1*1];
  var _11 = HEAP[_v_addr];
  var _13 = HEAP[0 + _11+1*1];
  var _14 = _10 * _13;
  var _15 = _7 + _14;
  var _16 = HEAP[_this_addr];
  var _18 = HEAP[0 + _16+2*1];
  var _19 = HEAP[_v_addr];
  var _21 = HEAP[0 + _19+2*1];
  var _22 = _18 * _21;
  _0 = _15 + _22;
  _retval = _0;
  __label__ = 0; /* _return */ 
  var _retval1 = _retval;
  STACKTOP = STACK_STACK.pop();
  return _retval1;
}


function __ZSt3maxIdERKT_S2_S2_(___a, ___b) {
  STACK_STACK.push(STACKTOP);
  var __label__;
  var ___a_addr = STACKTOP; STACKTOP += 1;
  var ___b_addr = STACKTOP; STACKTOP += 1;
  var _retval = STACKTOP; STACKTOP += 1;
  var _0 = STACKTOP; STACKTOP += 1;
  var __alloca_point_ = 0;
  HEAP[___a_addr] = ___a;
  HEAP[___b_addr] = ___b;
  var _1 = HEAP[___a_addr];
  var _2 = HEAP[_1];
  var _3 = HEAP[___b_addr];
  var _4 = HEAP[_3];
  var _5 = 0+(_2 < _4);
  if (_5) { __label__ = 1; /* _bb */  } else { __label__ = 2; /* _bb1 */  }
  _bb: do { 
    if (__label__ == 1) {
      var _6 = HEAP[___b_addr];
      HEAP[_0] = _6;
      __label__ = 3; /* _bb2 */ break _bb;
    }
    else   if (__label__ == 2) {
      var _7 = HEAP[___a_addr];
      HEAP[_0] = _7;
      __label__ = 3; /* _bb2 */ break _bb;
    }
  } while(0);
  var _8 = HEAP[_0];
  HEAP[_retval] = _8;
  __label__ = 0; /* _return */ 
  var _retval3 = HEAP[_retval];
  STACKTOP = STACK_STACK.pop();
  return _retval3;
}


function __ZSt3maxIiERKT_S2_S2_(___a, ___b) {
  STACK_STACK.push(STACKTOP);
  var __label__;
  var ___a_addr = STACKTOP; STACKTOP += 1;
  var ___b_addr = STACKTOP; STACKTOP += 1;
  var _retval = STACKTOP; STACKTOP += 1;
  var _0 = STACKTOP; STACKTOP += 1;
  var __alloca_point_ = 0;
  HEAP[___a_addr] = ___a;
  HEAP[___b_addr] = ___b;
  var _1 = HEAP[___a_addr];
  var _2 = HEAP[_1];
  var _3 = HEAP[___b_addr];
  var _4 = HEAP[_3];
  var _5 = 0+(_2 < _4);
  if (_5) { __label__ = 1; /* _bb */  } else { __label__ = 2; /* _bb1 */  }
  _bb: do { 
    if (__label__ == 1) {
      var _6 = HEAP[___b_addr];
      HEAP[_0] = _6;
      __label__ = 3; /* _bb2 */ break _bb;
    }
    else   if (__label__ == 2) {
      var _7 = HEAP[___a_addr];
      HEAP[_0] = _7;
      __label__ = 3; /* _bb2 */ break _bb;
    }
  } while(0);
  var _8 = HEAP[_0];
  HEAP[_retval] = _8;
  __label__ = 0; /* _return */ 
  var _retval3 = HEAP[_retval];
  STACKTOP = STACK_STACK.pop();
  return _retval3;
}


function ___tcf_0(_unnamed_arg) {
  STACK_STACK.push(STACKTOP);
  var __label__;
  var _unnamed_arg_addr = STACKTOP; STACKTOP += 1;
  var __alloca_point_ = 0;
  HEAP[_unnamed_arg_addr] = _unnamed_arg;
  __ZNSt8ios_base4InitD1Ev(__ZStL8__ioinit);
  __label__ = 0; /* _return */ 
  STACKTOP = STACK_STACK.pop();
  return;
}


function __ZN3v_tC1Eddd(_this, _a, _b, _c) {
  STACK_STACK.push(STACKTOP);
  var __label__;
  var _this_addr = STACKTOP; STACKTOP += 1;
  var _a_addr;
  var _b_addr;
  var _c_addr;
  var __alloca_point_ = 0;
  HEAP[_this_addr] = _this;
  _a_addr = _a;
  _b_addr = _b;
  _c_addr = _c;
  var _0 = HEAP[_this_addr];
  var _1 = _0+0*1;
  HEAP[_1] = _a_addr;
  var _3 = HEAP[_this_addr];
  var _4 = _3+1*1;
  HEAP[_4] = _b_addr;
  var _6 = HEAP[_this_addr];
  var _7 = _6+2*1;
  HEAP[_7] = _c_addr;
  __label__ = 0; /* _return */ 
  STACKTOP = STACK_STACK.pop();
  return;
}


function __ZN5ray_tC1ERK3v_t(_this, _v) {
  STACK_STACK.push(STACKTOP);
  var __label__;
  var _this_addr = STACKTOP; STACKTOP += 1;
  var _v_addr = STACKTOP; STACKTOP += 1;
  var __alloca_point_ = 0;
  HEAP[_this_addr] = _this;
  HEAP[_v_addr] = _v;
  var _0 = HEAP[_this_addr];
  var _1 = _0+0*3;
  var _2 = HEAP[_v_addr];
  var _3 = _1+0*1;
  var _5 = HEAP[0 + _2+0*1];
  HEAP[_3] = _5;
  var _6 = _1+1*1;
  var _8 = HEAP[0 + _2+1*1];
  HEAP[_6] = _8;
  var _9 = _1+2*1;
  var _11 = HEAP[0 + _2+2*1];
  HEAP[_9] = _11;
  var _12 = HEAP[_this_addr];
  var _13 = _12+1*3;
  __ZN3v_tC1Ev(_13);
  __label__ = 0; /* _return */ 
  STACKTOP = STACK_STACK.pop();
  return;
}


function __ZN5ray_tC1ERK3v_tS2_(_this, _v, _w) {
  STACK_STACK.push(STACKTOP);
  var __label__;
  var _this_addr = STACKTOP; STACKTOP += 1;
  var _v_addr = STACKTOP; STACKTOP += 1;
  var _w_addr = STACKTOP; STACKTOP += 1;
  var __alloca_point_ = 0;
  HEAP[_this_addr] = _this;
  HEAP[_v_addr] = _v;
  HEAP[_w_addr] = _w;
  var _0 = HEAP[_this_addr];
  var _1 = _0+0*3;
  var _2 = HEAP[_v_addr];
  var _3 = _1+0*1;
  var _5 = HEAP[0 + _2+0*1];
  HEAP[_3] = _5;
  var _6 = _1+1*1;
  var _8 = HEAP[0 + _2+1*1];
  HEAP[_6] = _8;
  var _9 = _1+2*1;
  var _11 = HEAP[0 + _2+2*1];
  HEAP[_9] = _11;
  var _12 = HEAP[_this_addr];
  var _13 = _12+1*3;
  var _14 = HEAP[_w_addr];
  var _15 = _13+0*1;
  var _17 = HEAP[0 + _14+0*1];
  HEAP[_15] = _17;
  var _18 = _13+1*1;
  var _20 = HEAP[0 + _14+1*1];
  HEAP[_18] = _20;
  var _21 = _13+2*1;
  var _23 = HEAP[0 + _14+2*1];
  HEAP[_21] = _23;
  __label__ = 0; /* _return */ 
  STACKTOP = STACK_STACK.pop();
  return;
}


function __ZN5hit_tC1Ev(_this) {
  STACK_STACK.push(STACKTOP);
  var __label__;
  var _this_addr = STACKTOP; STACKTOP += 1;
  var __alloca_point_ = 0;
  HEAP[_this_addr] = _this;
  var _0 = HEAP[_this_addr];
  var _1 = _0+_struct_hit_t___FLATTENER[0];
  __ZN3v_tC1Eddd(_1, 0.000000e+00, 0.000000e+00, 0.000000e+00);
  var _2 = HEAP[_this_addr];
  var _3 = _2+_struct_hit_t___FLATTENER[1];
  HEAP[_3] = Infinity;
  __label__ = 0; /* _return */ 
  STACKTOP = STACK_STACK.pop();
  return;
}


function __ZN8sphere_tC1Ev(_this) {
  STACK_STACK.push(STACKTOP);
  var __label__;
  var _this_addr = STACKTOP; STACKTOP += 1;
  var __alloca_point_ = 0;
  HEAP[_this_addr] = _this;
  var _0 = HEAP[_this_addr];
  var _1 = _0+_struct_hit_t___FLATTENER[0];
  __ZN3v_tC1Ev(_1);
  __label__ = 0; /* _return */ 
  STACKTOP = STACK_STACK.pop();
  return;
}


function __ZN8sphere_tC1ERK3v_td(_this, _v, _d) {
  STACK_STACK.push(STACKTOP);
  var __label__;
  var _this_addr = STACKTOP; STACKTOP += 1;
  var _v_addr = STACKTOP; STACKTOP += 1;
  var _d_addr;
  var __alloca_point_ = 0;
  HEAP[_this_addr] = _this;
  HEAP[_v_addr] = _v;
  _d_addr = _d;
  var _0 = HEAP[_this_addr];
  var _1 = _0+_struct_hit_t___FLATTENER[0];
  var _2 = HEAP[_v_addr];
  var _3 = _1+0*1;
  var _5 = HEAP[0 + _2+0*1];
  HEAP[_3] = _5;
  var _6 = _1+1*1;
  var _8 = HEAP[0 + _2+1*1];
  HEAP[_6] = _8;
  var _9 = _1+2*1;
  var _11 = HEAP[0 + _2+2*1];
  HEAP[_9] = _11;
  var _12 = HEAP[_this_addr];
  var _13 = _12+_struct_hit_t___FLATTENER[1];
  HEAP[_13] = _d_addr;
  __label__ = 0; /* _return */ 
  STACKTOP = STACK_STACK.pop();
  return;
}


function __ZNK8sphere_t10get_normalERK3v_t(_agg_result, _this, _v) {
  STACK_STACK.push(STACKTOP);
  var __label__;
  var _this_addr = STACKTOP; STACKTOP += 1;
  var _v_addr = STACKTOP; STACKTOP += 1;
  var _0 = STACKTOP; STACKTOP += 3;
  var __alloca_point_ = 0;
  HEAP[_this_addr] = _this;
  HEAP[_v_addr] = _v;
  var _1 = HEAP[_this_addr];
  var _3 = HEAP[0 + _1+_struct_hit_t___FLATTENER[1]];
  var _4 = 1.000000e+00 / _3;
  var _5 = HEAP[_this_addr];
  var _6 = _5+_struct_hit_t___FLATTENER[0];
  var _7 = HEAP[_v_addr];
  __ZNK3v_tmiERKS_(_0, _7, _6);
  __ZNK3v_tmlEd(_agg_result, _0, _4);
  __label__ = 0; /* _return */ 
  STACKTOP = STACK_STACK.pop();
  return;
}


function __ZN6node_tC1Ev(_this) {
  STACK_STACK.push(STACKTOP);
  var __label__;
  var _this_addr = STACKTOP; STACKTOP += 1;
  var __alloca_point_ = 0;
  HEAP[_this_addr] = _this;
  var _0 = HEAP[_this_addr];
  var _1 = _0+_struct_node_t___FLATTENER[0];
  __ZN8sphere_tC1Ev(_1);
  var _2 = HEAP[_this_addr];
  var _3 = _2+_struct_node_t___FLATTENER[1];
  __ZN8sphere_tC1Ev(_3);
  __label__ = 0; /* _return */ 
  STACKTOP = STACK_STACK.pop();
  return;
}


function __ZN6node_tC1ERK8sphere_tS2_l(_this, _b, _l, _jump) {
  STACK_STACK.push(STACKTOP);
  var __label__;
  var _this_addr = STACKTOP; STACKTOP += 1;
  var _b_addr = STACKTOP; STACKTOP += 1;
  var _l_addr = STACKTOP; STACKTOP += 1;
  var _jump_addr;
  var __alloca_point_ = 0;
  HEAP[_this_addr] = _this;
  HEAP[_b_addr] = _b;
  HEAP[_l_addr] = _l;
  _jump_addr = _jump;
  var _0 = HEAP[_this_addr];
  var _1 = _0+_struct_node_t___FLATTENER[0];
  var _2 = HEAP[_b_addr];
  var _3 = _1+_struct_hit_t___FLATTENER[0];
  var _4 = _2+_struct_hit_t___FLATTENER[0];
  var _5 = _3+0*1;
  var _7 = HEAP[0 + _4+0*1];
  HEAP[_5] = _7;
  var _8 = _3+1*1;
  var _10 = HEAP[0 + _4+1*1];
  HEAP[_8] = _10;
  var _11 = _3+2*1;
  var _13 = HEAP[0 + _4+2*1];
  HEAP[_11] = _13;
  var _14 = _1+_struct_hit_t___FLATTENER[1];
  var _16 = HEAP[0 + _2+_struct_hit_t___FLATTENER[1]];
  HEAP[_14] = _16;
  var _17 = HEAP[_this_addr];
  var _18 = _17+_struct_node_t___FLATTENER[1];
  var _19 = HEAP[_l_addr];
  var _20 = _18+_struct_hit_t___FLATTENER[0];
  var _21 = _19+_struct_hit_t___FLATTENER[0];
  var _22 = _20+0*1;
  var _24 = HEAP[0 + _21+0*1];
  HEAP[_22] = _24;
  var _25 = _20+1*1;
  var _27 = HEAP[0 + _21+1*1];
  HEAP[_25] = _27;
  var _28 = _20+2*1;
  var _30 = HEAP[0 + _21+2*1];
  HEAP[_28] = _30;
  var _31 = _18+_struct_hit_t___FLATTENER[1];
  var _33 = HEAP[0 + _19+_struct_hit_t___FLATTENER[1]];
  HEAP[_31] = _33;
  var _34 = HEAP[_this_addr];
  var _35 = _34+_struct_node_t___FLATTENER[2];
  HEAP[_35] = _jump_addr;
  __label__ = 0; /* _return */ 
  STACKTOP = STACK_STACK.pop();
  return;
}


function __ZN7basis_tC1ERK3v_t(_this, _v) {
  STACK_STACK.push(STACKTOP);
  var __label__;
  var _this_addr = STACKTOP; STACKTOP += 1;
  var _v_addr = STACKTOP; STACKTOP += 1;
  var _0 = STACKTOP; STACKTOP += 3;
  var _n = STACKTOP; STACKTOP += 3;
  var _memtmp = STACKTOP; STACKTOP += 3;
  var _memtmp12 = STACKTOP; STACKTOP += 3;
  var __alloca_point_ = 0;
  HEAP[_this_addr] = _this;
  HEAP[_v_addr] = _v;
  var _1 = HEAP[_this_addr];
  var _2 = _1+0*3;
  __ZN3v_tC1Ev(_2);
  var _3 = HEAP[_this_addr];
  var _4 = _3+1*3;
  __ZN3v_tC1Ev(_4);
  var _5 = HEAP[_this_addr];
  var _6 = _5+2*3;
  __ZN3v_tC1Ev(_6);
  var _7 = HEAP[_v_addr];
  __ZNK3v_t4normEv(_n, _7);
  var _9 = HEAP[0 + _n+0*1];
  var _11 = HEAP[0 + _n+0*1];
  var _12 = _9 * _11;
  var _13 = 0+(_12 != 1.000000e+00);
  var _14 = _13;
  var _16 = HEAP[0 + _n+1*1];
  var _18 = HEAP[0 + _n+1*1];
  var _19 = _16 * _18;
  var _20 = 0+(_19 != 1.000000e+00);
  var _21 = _20;
  var _toBool = 0+(_14 != 0);
  var _toBool1 = 0+(_21 != 0);
  var _22 = _toBool & _toBool1;
  var _23 = _22;
  var _24 = _23;
  var _26 = HEAP[0 + _n+2*1];
  var _28 = HEAP[0 + _n+2*1];
  var _29 = _26 * _28;
  var _30 = 0+(_29 != 1.000000e+00);
  var _31 = _30;
  var _32 = _24 & _31;
  var _33 = 0+(_32 != 0);
  if (_33) { __label__ = 1; /* _bb */  } else { __label__ = 4; /* _bb10 */  }
  _bb: do { 
    if (__label__ == 1) {
      var _34 = HEAP[_this_addr];
      var _35 = _34+1*3;
      var _36 = _35+0*1;
      var _38 = HEAP[0 + _n+0*1];
      HEAP[_36] = _38;
      var _39 = _35+1*1;
      var _41 = HEAP[0 + _n+1*1];
      HEAP[_39] = _41;
      var _42 = _35+2*1;
      var _44 = HEAP[0 + _n+2*1];
      HEAP[_42] = _44;
      var _46 = HEAP[0 + _n+1*1];
      var _48 = HEAP[0 + _n+1*1];
      var _49 = _46 * _48;
      var _51 = HEAP[0 + _n+0*1];
      var _53 = HEAP[0 + _n+0*1];
      var _54 = _51 * _53;
      var _55 = 0+(_49 > _54);
      if (_55) { __label__ = 3; /* _bb2 */  } else { __label__ = 5; /* _bb6 */  }
      _bb2: do { 
        if (__label__ == 3) {
          var _57 = HEAP[0 + _n+1*1];
          var _59 = HEAP[0 + _n+1*1];
          var _60 = _57 * _59;
          var _62 = HEAP[0 + _n+2*1];
          var _64 = HEAP[0 + _n+2*1];
          var _65 = _62 * _64;
          var _66 = 0+(_60 > _65);
          if (_66) { __label__ = 6; /* _bb3 */  } else { __label__ = 7; /* _bb4 */  }
          _bb3: do { 
            if (__label__ == 6) {
              var _67 = HEAP[_this_addr];
              var _68 = _67+1*3;
              var _70 = HEAP[0 + _68+1*1];
              var _71 = -0.000000e+00 - _70;
              var _72 = HEAP[_this_addr];
              var _73 = _72+1*3;
              var _74 = _73+1*1;
              HEAP[_74] = _71;
              __label__ = 8; /* _bb5 */ break _bb3;
            }
            else   if (__label__ == 7) {
              var _75 = HEAP[_this_addr];
              var _76 = _75+1*3;
              var _78 = HEAP[0 + _76+2*1];
              var _79 = -0.000000e+00 - _78;
              var _80 = HEAP[_this_addr];
              var _81 = _80+1*3;
              var _82 = _81+2*1;
              HEAP[_82] = _79;
              __label__ = 8; /* _bb5 */ break _bb3;
            }
          } while(0);
          __label__ = 9; /* _bb9 */ break _bb2;
        }
        else   if (__label__ == 5) {
          var _84 = HEAP[0 + _n+2*1];
          var _86 = HEAP[0 + _n+2*1];
          var _87 = _84 * _86;
          var _89 = HEAP[0 + _n+0*1];
          var _91 = HEAP[0 + _n+0*1];
          var _92 = _89 * _91;
          var _93 = 0+(_87 > _92);
          if (_93) { __label__ = 10; /* _bb7 */  } else { __label__ = 11; /* _bb8 */  }
          _bb7: do { 
            if (__label__ == 10) {
              var _94 = HEAP[_this_addr];
              var _95 = _94+1*3;
              var _97 = HEAP[0 + _95+2*1];
              var _98 = -0.000000e+00 - _97;
              var _99 = HEAP[_this_addr];
              var _100 = _99+1*3;
              var _101 = _100+2*1;
              HEAP[_101] = _98;
              __label__ = 9; /* _bb9 */ break _bb2;
            }
            else   if (__label__ == 11) {
              var _102 = HEAP[_this_addr];
              var _103 = _102+1*3;
              var _105 = HEAP[0 + _103+0*1];
              var _106 = -0.000000e+00 - _105;
              var _107 = HEAP[_this_addr];
              var _108 = _107+1*3;
              var _109 = _108+0*1;
              HEAP[_109] = _106;
              __label__ = 9; /* _bb9 */ break _bb2;
            }
          } while(0);
        }
      } while(0);
      __label__ = 12; /* _bb11 */ break _bb;
    }
    else   if (__label__ == 4) {
      var _111 = HEAP[0 + _n+1*1];
      var _113 = HEAP[0 + _n+0*1];
      var _115 = HEAP[0 + _n+2*1];
      __ZN3v_tC1Eddd(_0, _115, _113, _111);
      var _116 = HEAP[_this_addr];
      var _117 = _116+1*3;
      var _118 = _117+0*1;
      var _120 = HEAP[0 + _0+0*1];
      HEAP[_118] = _120;
      var _121 = _117+1*1;
      var _123 = HEAP[0 + _0+1*1];
      HEAP[_121] = _123;
      var _124 = _117+2*1;
      var _126 = HEAP[0 + _0+2*1];
      HEAP[_124] = _126;
      __label__ = 12; /* _bb11 */ break _bb;
    }
  } while(0);
  var _127 = HEAP[_this_addr];
  var _128 = _127+0*3;
  var _129 = _128+0*1;
  var _131 = HEAP[0 + _n+0*1];
  HEAP[_129] = _131;
  var _132 = _128+1*1;
  var _134 = HEAP[0 + _n+1*1];
  HEAP[_132] = _134;
  var _135 = _128+2*1;
  var _137 = HEAP[0 + _n+2*1];
  HEAP[_135] = _137;
  var _138 = HEAP[_this_addr];
  var _139 = _138+1*3;
  var _140 = HEAP[_this_addr];
  var _141 = _140+0*3;
  var _142 = HEAP[_this_addr];
  var _143 = _142+2*3;
  __ZNK3v_t5crossERKS_(_memtmp, _141, _139);
  var _144 = _143+0*1;
  var _146 = HEAP[0 + _memtmp+0*1];
  HEAP[_144] = _146;
  var _147 = _143+1*1;
  var _149 = HEAP[0 + _memtmp+1*1];
  HEAP[_147] = _149;
  var _150 = _143+2*1;
  var _152 = HEAP[0 + _memtmp+2*1];
  HEAP[_150] = _152;
  var _153 = HEAP[_this_addr];
  var _154 = _153+2*3;
  var _155 = HEAP[_this_addr];
  var _156 = _155+0*3;
  var _157 = HEAP[_this_addr];
  var _158 = _157+1*3;
  __ZNK3v_t5crossERKS_(_memtmp12, _156, _154);
  var _159 = _158+0*1;
  var _161 = HEAP[0 + _memtmp12+0*1];
  HEAP[_159] = _161;
  var _162 = _158+1*1;
  var _164 = HEAP[0 + _memtmp12+1*1];
  HEAP[_162] = _164;
  var _165 = _158+2*1;
  var _167 = HEAP[0 + _memtmp12+2*1];
  HEAP[_165] = _167;
  __label__ = 0; /* _return */ 
  STACKTOP = STACK_STACK.pop();
  return;
}


function __ZNK3v_t6magsqrEv(_this) {
  STACK_STACK.push(STACKTOP);
  var __label__;
  var _this_addr = STACKTOP; STACKTOP += 1;
  var _retval;
  var _0;
  var __alloca_point_ = 0;
  HEAP[_this_addr] = _this;
  var _1 = HEAP[_this_addr];
  var _2 = HEAP[_this_addr];
  _0 = __ZNK3v_t3dotERKS_(_1, _2);
  _retval = _0;
  __label__ = 0; /* _return */ 
  var _retval1 = _retval;
  STACKTOP = STACK_STACK.pop();
  return _retval1;
}


function __ZNK3v_t4normEv(_agg_result, _this) {
  STACK_STACK.push(STACKTOP);
  var __label__;
  var _this_addr = STACKTOP; STACKTOP += 1;
  var __alloca_point_ = 0;
  HEAP[_this_addr] = _this;
  var _0 = HEAP[_this_addr];
  var _1 = __ZNK3v_t6magsqrEv(_0);
  var _2 = _sqrt(_1);
  var _3 = 1.000000e+00 / _2;
  var _4 = HEAP[_this_addr];
  __ZNK3v_tmlEd(_agg_result, _4, _3);
  __label__ = 0; /* _return */ 
  STACKTOP = STACK_STACK.pop();
  return;
}


function __Z41__static_initialization_and_destruction_0ii(___initialize_p, ___priority) {
  STACK_STACK.push(STACKTOP);
  var __label__;
  var ___initialize_p_addr;
  var ___priority_addr;
  var _0 = STACKTOP; STACKTOP += 3;
  var __alloca_point_ = 0;
  ___initialize_p_addr = ___initialize_p;
  ___priority_addr = ___priority;
  var _1 = ___initialize_p_addr;
  var _2 = 0+(_1 == 1);
  if (_2) { __label__ = 1; /* _bb */  } else { __label__ = 3; /* _bb2 */  }
  _bb: do { 
    if (__label__ == 1) {
      var _3 = ___priority_addr;
      var _4 = 0+(_3 == 65535);
      if (_4) { __label__ = 2; /* _bb1 */  } else { __label__ = 3; /* _bb2 */ break _bb; }
      __ZNSt8ios_base4InitC1Ev(__ZStL8__ioinit);
      var _5 = ___cxa_atexit(___tcf_0, 0, ___dso_handle);
      __ZN3v_tC1Eddd(_0, -5.000000e-01, -6.500000e-01, 9.000000e-01);
      __ZNK3v_t4normEv(__ZL5light, _0);
      __label__ = 3; /* _bb2 */ break _bb;
    }
  } while(0);
  __label__ = 0; /* _return */ 
  STACKTOP = STACK_STACK.pop();
  return;
}


function __ZNK8sphere_t9intersectERK5ray_t(_this, _ray) {
  STACK_STACK.push(STACKTOP);
  var __label__;
  var _this_addr = STACKTOP; STACKTOP += 1;
  var _ray_addr = STACKTOP; STACKTOP += 1;
  var _retval;
  var _iftmp_88;
  var _0;
  var _v = STACKTOP; STACKTOP += 3;
  var _b;
  var _disc;
  var _d;
  var _t2;
  var _t1;
  var __alloca_point_ = 0;
  HEAP[_this_addr] = _this;
  HEAP[_ray_addr] = _ray;
  var _1 = HEAP[_ray_addr];
  var _2 = _1+0*3;
  var _3 = HEAP[_this_addr];
  var _4 = _3+_struct_hit_t___FLATTENER[0];
  __ZNK3v_tmiERKS_(_v, _4, _2);
  var _5 = HEAP[_ray_addr];
  var _6 = _5+1*3;
  _b = __ZNK3v_t3dotERKS_(_6, _v);
  var _8 = _b;
  var _9 = _b;
  var _10 = _8 * _9;
  var _11 = __ZNK3v_t6magsqrEv(_v);
  var _12 = _10 - _11;
  var _13 = HEAP[_this_addr];
  var _15 = HEAP[0 + _13+_struct_hit_t___FLATTENER[1]];
  var _16 = HEAP[_this_addr];
  var _18 = HEAP[0 + _16+_struct_hit_t___FLATTENER[1]];
  var _19 = _15 * _18;
  _disc = _12 + _19;
  var _21 = _disc;
  var _22 = 0+(_21 < 0.000000e+00);
  if (_22) { __label__ = 1; /* _bb */  } else { __label__ = 2; /* _bb1 */  }
  _bb: do { 
    if (__label__ == 1) {
      _0 = Infinity;
      __label__ = 10; /* _bb7 */ break _bb;
    }
    else   if (__label__ == 2) {
      _d = _sqrt(_disc);
      var _25 = _b;
      var _26 = _d;
      _t2 = _25 + _26;
      var _28 = _b;
      var _29 = _d;
      _t1 = _28 - _29;
      var _31 = _t2;
      var _32 = 0+(_31 < 0.000000e+00);
      if (_32) { __label__ = 3; /* _bb2 */  } else { __label__ = 6; /* _bb3 */  }
      _bb2: do { 
        if (__label__ == 3) {
          _0 = Infinity;
          __label__ = 10; /* _bb7 */ break _bb;
        }
        else   if (__label__ == 6) {
          var _33 = _t1;
          var _34 = 0+(_33 > 0.000000e+00);
          if (_34) { __label__ = 7; /* _bb4 */  } else { __label__ = 8; /* _bb5 */  }
          _bb4: do { 
            if (__label__ == 7) {
              _iftmp_88 = _t1;
              __label__ = 5; /* _bb6 */ break _bb4;
            }
            else   if (__label__ == 8) {
              _iftmp_88 = _t2;
              __label__ = 5; /* _bb6 */ break _bb4;
            }
          } while(0);
          _0 = _iftmp_88;
          __label__ = 10; /* _bb7 */ break _bb;
        }
      } while(0);
    }
  } while(0);
  _retval = _0;
  __label__ = 0; /* _return */ 
  var _retval8 = _retval;
  STACKTOP = STACK_STACK.pop();
  return _retval8;
}


function __ZN6node_t9intersectILb0EEEvRK5ray_tR5hit_t(_ray, _hit) {
  STACK_STACK.push(STACKTOP);
  var __label__;
  var _ray_addr = STACKTOP; STACKTOP += 1;
  var _hit_addr = STACKTOP; STACKTOP += 1;
  var _0 = STACKTOP; STACKTOP += 3;
  var _1 = STACKTOP; STACKTOP += 3;
  var _retval_114;
  var _p = STACKTOP; STACKTOP += 1;
  var _t;
  var _memtmp = STACKTOP; STACKTOP += 3;
  var __alloca_point_ = 0;
  HEAP[_ray_addr] = _ray;
  HEAP[_hit_addr] = _hit;
  var _2 = HEAP[__ZL4pool];
  HEAP[_p] = _2;
  __label__ = 8; /* _bb5 */ 
  _bb5: while(1) { // _bb5
    var _50 = HEAP[__ZL3end];
    var _51 = HEAP[_p];
    var _52 = 0+(_51 < _50);
    if (_52) { __label__ = 1; /* _bb */  } else { __label__ = 5; /* _bb6 */ break _bb5; }
    var _3 = HEAP[_p];
    var _4 = _3+_struct_node_t___FLATTENER[0];
    var _5 = HEAP[_ray_addr];
    var _6 = __ZNK8sphere_t9intersectERK5ray_t(_4, _5);
    var _7 = HEAP[_hit_addr];
    var _9 = HEAP[0 + _7+_struct_hit_t___FLATTENER[1]];
    var _10 = 0+(_6 >= _9);
    _retval_114 = _10;
    var _12 = _retval_114;
    var _toBool = 0+(_12 != 0);
    if (_toBool) { __label__ = 2; /* _bb1 */  } else { __label__ = 3; /* _bb2 */  }
    _bb1: do { 
      if (__label__ == 2) {
        var _13 = HEAP[_p];
        var _15 = HEAP[0 + _13+_struct_node_t___FLATTENER[2]];
        var _16 = HEAP[_p];
        var _17 = _16+9*_15;
        HEAP[_p] = _17;
        __label__ = 8; /* _bb5 */ continue _bb5;
      }
      else   if (__label__ == 3) {
        var _18 = HEAP[_p];
        var _19 = _18+_struct_node_t___FLATTENER[1];
        var _20 = HEAP[_ray_addr];
        _t = __ZNK8sphere_t9intersectERK5ray_t(_19, _20);
        var _22 = HEAP[_hit_addr];
        var _24 = HEAP[0 + _22+_struct_hit_t___FLATTENER[1]];
        var _25 = _t;
        var _26 = 0+(_24 > _25);
        if (_26) { __label__ = 6; /* _bb3 */  } else { __label__ = 7; /* _bb4 */  }
        _bb3: do { 
          if (__label__ == 6) {
            var _27 = HEAP[_hit_addr];
            var _28 = _27+_struct_hit_t___FLATTENER[1];
            HEAP[_28] = _t;
            var _30 = HEAP[_ray_addr];
            var _31 = _30+1*3;
            __ZNK3v_tmlEd(_0, _31, _t);
            var _33 = HEAP[_ray_addr];
            var _34 = _33+0*3;
            __ZNK3v_tplERKS_(_1, _34, _0);
            var _35 = HEAP[_p];
            var _36 = _35+_struct_node_t___FLATTENER[1];
            var _37 = HEAP[_hit_addr];
            var _38 = _37+_struct_hit_t___FLATTENER[0];
            __ZNK8sphere_t10get_normalERK3v_t(_memtmp, _36, _1);
            var _39 = _38+0*1;
            var _41 = HEAP[0 + _memtmp+0*1];
            HEAP[_39] = _41;
            var _42 = _38+1*1;
            var _44 = HEAP[0 + _memtmp+1*1];
            HEAP[_42] = _44;
            var _45 = _38+2*1;
            var _47 = HEAP[0 + _memtmp+2*1];
            HEAP[_45] = _47;
            __label__ = 7; /* _bb4 */ break _bb3;
          }
        } while(0);
        var _48 = HEAP[_p];
        var _49 = _48+9*1;
        HEAP[_p] = _49;
        __label__ = 8; /* _bb5 */ continue _bb5;
      }
    } while(0);
  }
  __label__ = 0; /* _return */ 
  STACKTOP = STACK_STACK.pop();
  return;
}


function __ZN6node_t9intersectILb1EEEvRK5ray_tR5hit_t(_ray, _hit) {
  STACK_STACK.push(STACKTOP);
  var __label__;
  var _ray_addr = STACKTOP; STACKTOP += 1;
  var _hit_addr = STACKTOP; STACKTOP += 1;
  var _0 = STACKTOP; STACKTOP += 3;
  var _1 = STACKTOP; STACKTOP += 3;
  var _retval_116;
  var _p = STACKTOP; STACKTOP += 1;
  var _t;
  var __alloca_point_ = 0;
  HEAP[_ray_addr] = _ray;
  HEAP[_hit_addr] = _hit;
  var _2 = HEAP[__ZL4pool];
  HEAP[_p] = _2;
  __label__ = 8; /* _bb5 */ 
  _bb5: while(1) { // _bb5
    var _32 = HEAP[__ZL3end];
    var _33 = HEAP[_p];
    var _34 = 0+(_33 < _32);
    if (_34) { __label__ = 1; /* _bb */  } else { __label__ = 5; /* _bb6 */ break _bb5; }
    var _3 = HEAP[_p];
    var _4 = _3+_struct_node_t___FLATTENER[0];
    var _5 = HEAP[_ray_addr];
    var _6 = __ZNK8sphere_t9intersectERK5ray_t(_4, _5);
    var _7 = HEAP[_hit_addr];
    var _9 = HEAP[0 + _7+_struct_hit_t___FLATTENER[1]];
    var _10 = 0+(_6 >= _9);
    _retval_116 = _10;
    var _12 = _retval_116;
    var _toBool = 0+(_12 != 0);
    if (_toBool) { __label__ = 2; /* _bb1 */  } else { __label__ = 3; /* _bb2 */  }
    _bb1: do { 
      if (__label__ == 2) {
        var _13 = HEAP[_p];
        var _15 = HEAP[0 + _13+_struct_node_t___FLATTENER[2]];
        var _16 = HEAP[_p];
        var _17 = _16+9*_15;
        HEAP[_p] = _17;
        __label__ = 8; /* _bb5 */ continue _bb5;
      }
      else   if (__label__ == 3) {
        var _18 = HEAP[_p];
        var _19 = _18+_struct_node_t___FLATTENER[1];
        var _20 = HEAP[_ray_addr];
        _t = __ZNK8sphere_t9intersectERK5ray_t(_19, _20);
        var _22 = HEAP[_hit_addr];
        var _24 = HEAP[0 + _22+_struct_hit_t___FLATTENER[1]];
        var _25 = _t;
        var _26 = 0+(_24 > _25);
        if (_26) { __label__ = 6; /* _bb3 */ break _bb5; } else { __label__ = 7; /* _bb4 */  }
        var _30 = HEAP[_p];
        var _31 = _30+9*1;
        HEAP[_p] = _31;
        __label__ = 8; /* _bb5 */ continue _bb5;
      }
    } while(0);
  }
  _bb3: do { 
    if (__label__ == 6) {
      var _27 = HEAP[_hit_addr];
      var _28 = _27+_struct_hit_t___FLATTENER[1];
      HEAP[_28] = _t;
      __label__ = 5; /* _bb6 */ break _bb3;
    }
  } while(0);
  __label__ = 0; /* _return */ 
  STACKTOP = STACK_STACK.pop();
  return;
}


function __ZL9ray_tracePK6node_tRK5ray_t(_scene, _ray) {
  STACK_STACK.push(STACKTOP);
  var __label__;
  var _scene_addr = STACKTOP; STACKTOP += 1;
  var _ray_addr = STACKTOP; STACKTOP += 1;
  var _retval;
  var _iftmp_90;
  var _0 = STACKTOP; STACKTOP += 3;
  var _1 = STACKTOP; STACKTOP += 3;
  var _2 = STACKTOP; STACKTOP += 3;
  var _3 = STACKTOP; STACKTOP += 3;
  var _4 = STACKTOP; STACKTOP += 3;
  var _5;
  var _iftmp_89;
  var _hit = STACKTOP; STACKTOP += 4;
  var _diffuse;
  var _sray = STACKTOP; STACKTOP += 6;
  var _shit = STACKTOP; STACKTOP += 4;
  var __alloca_point_ = 0;
  HEAP[_scene_addr] = _scene;
  HEAP[_ray_addr] = _ray;
  __ZN5hit_tC1Ev(_hit);
  var _6 = HEAP[_ray_addr];
  __ZN6node_t9intersectILb0EEEvRK5ray_tR5hit_t(_6, _hit);
  var _8 = HEAP[0 + _hit+_struct_hit_t___FLATTENER[1]];
  var _9 = 0+(_8 != Infinity);
  if (_9) { __label__ = 1; /* _bb */  } else { __label__ = 2; /* _bb1 */  }
  _bb: do { 
    if (__label__ == 1) {
      var _10 = _hit+_struct_hit_t___FLATTENER[0];
      var _11 = __ZNK3v_t3dotERKS_(_10, __ZL5light);
      _iftmp_89 = -0.000000e+00 - _11;
      __label__ = 3; /* _bb2 */ break _bb;
    }
    else   if (__label__ == 2) {
      _iftmp_89 = 0.000000e+00;
      __label__ = 3; /* _bb2 */ break _bb;
    }
  } while(0);
  _diffuse = _iftmp_89;
  var _14 = _diffuse;
  var _15 = 0+(_14 <= 0.000000e+00);
  if (_15) { __label__ = 6; /* _bb3 */  } else { __label__ = 7; /* _bb4 */  }
  _bb3: do { 
    if (__label__ == 6) {
      _5 = 0.000000e+00;
      __label__ = 11; /* _bb8 */ break _bb3;
    }
    else   if (__label__ == 7) {
      __ZNK3v_tngEv(_4, __ZL5light);
      var _16 = _hit+_struct_hit_t___FLATTENER[0];
      __ZNK3v_tmlEd(_2, _16, 1.000000e-12);
      var _18 = HEAP[0 + _hit+_struct_hit_t___FLATTENER[1]];
      var _19 = HEAP[_ray_addr];
      var _20 = _19+1*3;
      __ZNK3v_tmlEd(_0, _20, _18);
      var _21 = HEAP[_ray_addr];
      var _22 = _21+0*3;
      __ZNK3v_tplERKS_(_1, _22, _0);
      __ZNK3v_tplERKS_(_3, _1, _2);
      __ZN5ray_tC1ERK3v_tS2_(_sray, _3, _4);
      __ZN5hit_tC1Ev(_shit);
      __ZN6node_t9intersectILb1EEEvRK5ray_tR5hit_t(_sray, _shit);
      var _24 = HEAP[0 + _shit+_struct_hit_t___FLATTENER[1]];
      var _25 = 0+(_24 == Infinity);
      if (_25) { __label__ = 8; /* _bb5 */  } else { __label__ = 5; /* _bb6 */  }
      _bb5: do { 
        if (__label__ == 8) {
          _iftmp_90 = _diffuse;
          __label__ = 10; /* _bb7 */ break _bb5;
        }
        else   if (__label__ == 5) {
          _iftmp_90 = 0.000000e+00;
          __label__ = 10; /* _bb7 */ break _bb5;
        }
      } while(0);
      _5 = _iftmp_90;
      __label__ = 11; /* _bb8 */ break _bb3;
    }
  } while(0);
  _retval = _5;
  __label__ = 0; /* _return */ 
  var _retval9 = _retval;
  STACKTOP = STACK_STACK.pop();
  return _retval9;
}


function __ZL6createP6node_tii3v_tS1_d(_n, _lvl, _dist, _c, _d, _r) {
  STACK_STACK.push(STACKTOP);
  var __label__;
  var _n_addr = STACKTOP; STACKTOP += 1;
  var _lvl_addr;
  var _dist_addr;
  var _r_addr;
  var _retval = STACKTOP; STACKTOP += 1;
  var _0 = STACKTOP; STACKTOP += 3;
  var _1 = STACKTOP; STACKTOP += 3;
  var _2 = STACKTOP; STACKTOP += 3;
  var _3 = STACKTOP; STACKTOP += 3;
  var _4 = STACKTOP; STACKTOP += 3;
  var _5 = STACKTOP; STACKTOP += 3;
  var _6 = STACKTOP; STACKTOP += 3;
  var _7 = STACKTOP; STACKTOP += 3;
  var _8 = STACKTOP; STACKTOP += 3;
  var _9 = STACKTOP; STACKTOP += 3;
  var _10 = STACKTOP; STACKTOP += 3;
  var _11 = STACKTOP; STACKTOP += 3;
  var _12 = STACKTOP; STACKTOP += 3;
  var _13 = STACKTOP; STACKTOP += 3;
  var _14 = STACKTOP; STACKTOP += 1;
  var _15 = STACKTOP; STACKTOP += 1;
  var _16 = STACKTOP; STACKTOP += 1;
  var _iftmp_104;
  var _iftmp_103 = STACKTOP; STACKTOP += 1;
  var _17 = STACKTOP; STACKTOP += 1;
  var _18 = STACKTOP; STACKTOP += 4;
  var _19 = STACKTOP; STACKTOP += 1;
  var _20 = STACKTOP; STACKTOP += 4;
  var _21 = STACKTOP; STACKTOP += 1;
  var _b = STACKTOP; STACKTOP += 9;
  var _nr;
  var _daL;
  var _daU;
  var _a;
  var _i;
  var _ndir = STACKTOP; STACKTOP += 3;
  var _i11;
  var _ndir13 = STACKTOP; STACKTOP += 3;
  var __alloca_point_ = 0;
  HEAP[_n_addr] = _n;
  _lvl_addr = _lvl;
  _dist_addr = _dist;
  _r_addr = _r;
  var _22 = _r_addr;
  var _23 = _22 * 2.000000e+00;
  __ZN8sphere_tC1ERK3v_td(_20, _c, _23);
  HEAP[_21] = _20;
  __ZN8sphere_tC1ERK3v_td(_18, _c, _r_addr);
  HEAP[_19] = _18;
  var _25 = HEAP[_n_addr];
  var _27 = __ZnwjPv(68, _25);
  HEAP[_17] = _27;
  var _29 = HEAP[_17];
  var _30 = 0+(_29 != 0);
  if (_30) { __label__ = 1; /* _bb */  } else { __label__ = 7; /* _bb4 */  }
  _bb: do { 
    if (__label__ == 1) {
      var _31 = _lvl_addr;
      var _32 = 0+(_31 > 1);
      if (_32) { __label__ = 2; /* _bb1 */  } else { __label__ = 3; /* _bb2 */  }
      _bb1: do { 
        if (__label__ == 2) {
          _iftmp_104 = _dist_addr;
          __label__ = 6; /* _bb3 */ break _bb1;
        }
        else   if (__label__ == 3) {
          _iftmp_104 = 1;
          __label__ = 6; /* _bb3 */ break _bb1;
        }
      } while(0);
      var _34 = HEAP[_17];
      var _35 = HEAP[_21];
      var _36 = HEAP[_19];
      __ZN6node_tC1ERK8sphere_tS2_l(_34, _35, _36, _iftmp_104);
      var _38 = HEAP[_17];
      HEAP[_iftmp_103] = _38;
      __label__ = 8; /* _bb5 */ break _bb;
    }
    else   if (__label__ == 7) {
      var _39 = HEAP[_17];
      HEAP[_iftmp_103] = _39;
      __label__ = 8; /* _bb5 */ break _bb;
    }
  } while(0);
  var _40 = HEAP[_iftmp_103];
  var _41 = _40+9*1;
  HEAP[_n_addr] = _41;
  var _42 = _lvl_addr;
  var _43 = 0+(_42 <= 1);
  if (_43) { __label__ = 5; /* _bb6 */  } else { __label__ = 10; /* _bb7 */  }
  _bb6: do { 
    if (__label__ == 5) {
      var _44 = HEAP[_n_addr];
      HEAP[_16] = _44;
      __label__ = 13; /* _bb16 */ break _bb6;
    }
    else   if (__label__ == 10) {
      HEAP[_15] = 1;
      var _45 = _dist_addr;
      var _46 = _45 - 9;
      var _47 = Math.floor(_46 / 9);
      HEAP[_14] = _47;
      var _48 = __ZSt3maxIiERKT_S2_S2_(_14, _15);
      _dist_addr = HEAP[_48];
      __ZN7basis_tC1ERK3v_t(_b, _d);
      var _50 = _r_addr;
      _nr = _50 / 3.000000e+00;
      _daL = 1.0471975511965976;
      _daU = 2.0943951023931953;
      _a = 0.000000e+00;
      _i = 0;
      __label__ = 9; /* _bb9 */ 
      _bb9: while(1) { // _bb9
        var _71 = _i;
        var _72 = 0+(_71 <= 5);
        if (_72) { __label__ = 11; /* _bb8 */  } else { __label__ = 4; /* _bb10 */ break _bb9; }
        var _53 = _cos(_a);
        var _54 = _b+2*3;
        __ZNK3v_tmlEd(_12, _54, _53);
        var _56 = _sin(_a);
        var _57 = _b+1*3;
        __ZNK3v_tmlEd(_10, _57, _56);
        __ZNK3v_tmlEd(_9, _d, -2.000000e-01);
        __ZNK3v_tplERKS_(_11, _9, _10);
        __ZNK3v_tplERKS_(_13, _11, _12);
        __ZNK3v_t4normEv(_ndir, _13);
        var _58 = _r_addr;
        var _59 = _nr;
        var _60 = _58 + _59;
        __ZNK3v_tmlEd(_7, _ndir, _60);
        __ZNK3v_tplERKS_(_8, _c, _7);
        var _61 = _lvl_addr;
        var _62 = _61 - 1;
        var _63 = HEAP[_n_addr];
        var _66 = __ZL6createP6node_tii3v_tS1_d(_63, _62, _dist_addr, _8, _ndir, _nr);
        HEAP[_n_addr] = _66;
        var _67 = _a;
        _a = _67 + 1.0471975511965976;
        var _69 = _i;
        _i = _69 + 1;
        __label__ = 9; /* _bb9 */ continue _bb9;
      }
      var _73 = _a;
      _a = _73 - 0.3490658503988659;
      _i11 = 0;
      __label__ = 14; /* _bb14 */ 
      _bb14: while(1) { // _bb14
        var _94 = _i11;
        var _95 = 0+(_94 <= 2);
        if (_95) { __label__ = 15; /* _bb12 */  } else { __label__ = 16; /* _bb15 */ break _bb14; }
        var _76 = _cos(_a);
        var _77 = _b+2*3;
        __ZNK3v_tmlEd(_5, _77, _76);
        var _79 = _sin(_a);
        var _80 = _b+1*3;
        __ZNK3v_tmlEd(_3, _80, _79);
        __ZNK3v_tmlEd(_2, _d, 6.000000e-01);
        __ZNK3v_tplERKS_(_4, _2, _3);
        __ZNK3v_tplERKS_(_6, _4, _5);
        __ZNK3v_t4normEv(_ndir13, _6);
        var _81 = _r_addr;
        var _82 = _nr;
        var _83 = _81 + _82;
        __ZNK3v_tmlEd(_0, _ndir13, _83);
        __ZNK3v_tplERKS_(_1, _c, _0);
        var _84 = _lvl_addr;
        var _85 = _84 - 1;
        var _86 = HEAP[_n_addr];
        var _89 = __ZL6createP6node_tii3v_tS1_d(_86, _85, _dist_addr, _1, _ndir13, _nr);
        HEAP[_n_addr] = _89;
        var _90 = _a;
        _a = _90 + 2.0943951023931953;
        var _92 = _i11;
        _i11 = _92 + 1;
        __label__ = 14; /* _bb14 */ continue _bb14;
      }
      var _96 = HEAP[_n_addr];
      HEAP[_16] = _96;
      __label__ = 13; /* _bb16 */ break _bb6;
    }
  } while(0);
  var _97 = HEAP[_16];
  HEAP[_retval] = _97;
  __label__ = 0; /* _return */ 
  var _retval17 = HEAP[_retval];
  STACKTOP = STACK_STACK.pop();
  return _retval17;
}


function __ZL10trace_lineiii(_width, _height, _y) {
  STACK_STACK.push(STACKTOP);
  var __label__;
  var _width_addr;
  var _height_addr;
  var _y_addr;
  var _0 = STACKTOP; STACKTOP += 3;
  var _1 = STACKTOP; STACKTOP += 3;
  var _2;
  var _3 = STACKTOP; STACKTOP += 1;
  var _4 = STACKTOP; STACKTOP += 1;
  var _retval_91 = STACKTOP; STACKTOP += 1;
  var _5 = STACKTOP; STACKTOP += 3;
  var _w = STACKTOP; STACKTOP += 1;
  var _h = STACKTOP; STACKTOP += 1;
  var _rcp;
  var _scale;
  var _ray = STACKTOP; STACKTOP += 6;
  var _rgss = STACKTOP; STACKTOP += 12;
  var _scan = STACKTOP; STACKTOP += 3;
  var _i;
  var _j;
  var _g;
  var _idx;
  var _memtmp = STACKTOP; STACKTOP += 3;
  var _k;
  var __alloca_point_ = 0;
  _width_addr = _width;
  _height_addr = _height;
  _y_addr = _y;
  var _6 = _width_addr;
  var _7 = _6;
  HEAP[_w] = _7;
  var _8 = _height_addr;
  var _9 = _8;
  HEAP[_h] = _9;
  _rcp = 5.000000e-01;
  _scale = 6.400000e+01;
  __ZN3v_tC1Eddd(_5, 0.000000e+00, 0.000000e+00, -4.500000e+00);
  __ZN5ray_tC1ERK3v_t(_ray, _5);
  var _10 = _rgss+0*3;
  HEAP[_4] = _10;
  var _11 = HEAP[_4];
  HEAP[_3] = _11;
  _2 = 3;
  __label__ = 2; /* _bb1 */ 
  _bb1: while(1) { // _bb1
    var _17 = _2;
    var _18 = 0+(_17 != -1);
    if (_18) { __label__ = 1; /* _bb */  } else { __label__ = 3; /* _bb2 */ break _bb1; }
    var _12 = HEAP[_3];
    __ZN3v_tC1Ev(_12);
    var _13 = HEAP[_3];
    var _14 = _13+3*1;
    HEAP[_3] = _14;
    var _15 = _2;
    _2 = _15 - 1;
    __label__ = 2; /* _bb1 */ continue _bb1;
  }
  var _19 = HEAP[_4];
  HEAP[_retval_91] = _19;
  _i = 0;
  __label__ = 7; /* _bb4 */ 
  _bb4: while(1) { // _bb4
    var _49 = _i;
    var _50 = 0+(_49 <= 3);
    if (_50) { __label__ = 6; /* _bb3 */  } else { __label__ = 8; /* _bb5 */ break _bb4; }
    var _20 = _i;
    var _22 = __ZL4grid+_i*2;
    var _24 = HEAP[0 + _22+1*1];
    var _25 = _24 * 5.000000e-01;
    var _26 = HEAP[_h];
    var _27 = _26 / -2.000000e+00;
    var _28 = _25 + _27;
    var _30 = __ZL4grid+_i*2;
    var _32 = HEAP[0 + _30+0*1];
    var _33 = _32 * 5.000000e-01;
    var _34 = HEAP[_w];
    var _35 = _34 / -2.000000e+00;
    var _36 = _33 + _35;
    __ZN3v_tC1Eddd(_1, _36, _28, 0.000000e+00);
    var _37 = _rgss+_20*3;
    var _38 = _37+0*1;
    var _40 = HEAP[0 + _1+0*1];
    HEAP[_38] = _40;
    var _41 = _37+1*1;
    var _43 = HEAP[0 + _1+1*1];
    HEAP[_41] = _43;
    var _44 = _37+2*1;
    var _46 = HEAP[0 + _1+2*1];
    HEAP[_44] = _46;
    var _47 = _i;
    _i = _47 + 1;
    __label__ = 7; /* _bb4 */ continue _bb4;
  }
  var _51 = __ZSt3maxIdERKT_S2_S2_(_w, _h);
  var _52 = HEAP[_51];
  var _53 = _y_addr;
  var _54 = _53;
  __ZN3v_tC1Eddd(_scan, 0.000000e+00, _54, _52);
  var _55 = HEAP[_screen];
  var _56 = _SDL_LockSurface(_55);
  _j = _width_addr;
  __label__ = 14; /* _bb14 */ 
  _bb14: while(1) { // _bb14
    var _112 = _j;
    var _113 = 0+(_112 != 0);
    if (_113) { __label__ = 5; /* _bb6 */  } else { __label__ = 16; /* _bb15 */ break _bb14; }
    _g = 0.000000e+00;
    _idx = 0;
    __label__ = 11; /* _bb8 */ 
    _bb8: while(1) { // _bb8
      var _76 = _idx;
      var _77 = 0+(_76 <= 3);
      if (_77) { __label__ = 10; /* _bb7 */  } else { __label__ = 9; /* _bb9 */ break _bb8; }
      var _59 = _rgss+_idx*3;
      __ZNK3v_tplERKS_(_0, _scan, _59);
      var _60 = _ray+1*3;
      __ZNK3v_t4normEv(_memtmp, _0);
      var _61 = _60+0*1;
      var _63 = HEAP[0 + _memtmp+0*1];
      HEAP[_61] = _63;
      var _64 = _60+1*1;
      var _66 = HEAP[0 + _memtmp+1*1];
      HEAP[_64] = _66;
      var _67 = _60+2*1;
      var _69 = HEAP[0 + _memtmp+2*1];
      HEAP[_67] = _69;
      var _70 = HEAP[__ZL4pool];
      var _71 = __ZL9ray_tracePK6node_tRK5ray_t(_70, _ray);
      var _72 = _g;
      _g = _71 + _72;
      var _74 = _idx;
      _idx = _74 + 1;
      __label__ = 11; /* _bb8 */ continue _bb8;
    }
    _k = 0;
    __label__ = 15; /* _bb12 */ 
    _bb12: while(1) { // _bb12
      var _104 = _k;
      var _105 = 0+(_104 <= 2);
      if (_105) { __label__ = 12; /* _bb11 */  } else { __label__ = 17; /* _bb13 */ break _bb12; }
      var _78 = HEAP[_screen];
      var _80 = HEAP[0 + _78+_struct_SDL_Surface___FLATTENER[5]];
      var _81 = _width_addr;
      var _82 = _81;
      var _84 = HEAP[0 + _scan+1*1];
      var _85 = _82 - _84;
      var _86 = _width_addr;
      var _87 = _86;
      var _88 = _85 * _87;
      var _89 = _88 * 4.000000e+00;
      var _91 = HEAP[0 + _scan+0*1];
      var _92 = _91 * 4.000000e+00;
      var _93 = _89 + _92;
      var _94 = Math.floor(_93);
      var _95 = _80+1*_94;
      var _96 = _g;
      var _97 = _96 * 6.400000e+01;
      var _98 = Math.floor(_97);
      var _99 = _98;
      var _101 = _95+1*_k;
      HEAP[_101] = _99;
      var _102 = _k;
      _k = _102 + 1;
      __label__ = 15; /* _bb12 */ continue _bb12;
    }
    var _107 = HEAP[0 + _scan+0*1];
    var _108 = _107 + 1.000000e+00;
    var _109 = _scan+0*1;
    HEAP[_109] = _108;
    var _110 = _j;
    _j = _110 - 1;
    __label__ = 14; /* _bb14 */ continue _bb14;
  }
  var _114 = HEAP[_screen];
  _SDL_UnlockSurface(_114);
  var _115 = HEAP[_screen];
  var _116 = _SDL_Flip(_115);
  __label__ = 0; /* _return */ 
  STACKTOP = STACK_STACK.pop();
  return;
}


function _main(_argc, _argv) {
  STACK_STACK.push(STACKTOP);
  var __label__;
  var _argc_addr;
  var _argv_addr = STACKTOP; STACKTOP += 1;
  var _retval;
  var _0;
  var _1 = STACKTOP; STACKTOP += 3;
  var _2 = STACKTOP; STACKTOP += 3;
  var _3 = STACKTOP; STACKTOP += 3;
  var _4;
  var _5 = STACKTOP; STACKTOP += 1;
  var _6 = STACKTOP; STACKTOP += 1;
  var _retval_108 = STACKTOP; STACKTOP += 1;
  var _count_107;
  var _7 = STACKTOP; STACKTOP += 1;
  var _retval_106;
  var _8 = STACKTOP; STACKTOP += 1;
  var _9 = STACKTOP; STACKTOP += 1;
  var _iftmp_105;
  var _lvl;
  var _count;
  var _dec;
  var _y;
  var __alloca_point_ = 0;
  _argc_addr = _argc;
  HEAP[_argv_addr] = _argv;
  var _10 = _argc_addr;
  var _11 = 0+(_10 == 2);
  if (_11) { __label__ = 1; /* _bb */  } else { __label__ = 2; /* _bb1 */  }
  _bb: do { 
    if (__label__ == 1) {
      HEAP[_9] = 2;
      var _12 = HEAP[_argv_addr];
      var _14 = HEAP[0 + _12+1*1];
      var _15 = _atoi(_14);
      HEAP[_8] = _15;
      var _16 = __ZSt3maxIiERKT_S2_S2_(_8, _9);
      _iftmp_105 = HEAP[_16];
      __label__ = 3; /* _bb2 */ break _bb;
    }
    else   if (__label__ == 2) {
      _iftmp_105 = 6;
      __label__ = 3; /* _bb2 */ break _bb;
    }
  } while(0);
  _lvl = _iftmp_105;
  _count = 9;
  _dec = _lvl;
  __label__ = 7; /* _bb4 */ 
  _bb4: while(1) { // _bb4
    var _23 = _dec;
    _dec = _23 - 1;
    var _25 = _dec;
    var _26 = 0+(_25 > 1);
    _retval_106 = _26;
    var _28 = _retval_106;
    var _toBool = 0+(_28 != 0);
    if (_toBool) { __label__ = 6; /* _bb3 */  } else { __label__ = 8; /* _bb5 */ break _bb4; }
    var _20 = _count;
    var _21 = _20 + 1;
    _count = _21 * 9;
    __label__ = 7; /* _bb4 */ continue _bb4;
  }
  var _29 = _count;
  _count = _29 + 1;
  _count_107 = _count;
  var _32 = _count_107;
  var _33 = _32 * 68;
  var _34 = __Znaj(_33);
  HEAP[_7] = _34;
  var _36 = HEAP[_7];
  HEAP[_6] = _36;
  var _38 = HEAP[_6];
  HEAP[_5] = _38;
  var _39 = _count_107;
  _4 = _39 - 1;
  __label__ = 10; /* _bb7 */ 
  _bb7: while(1) { // _bb7
    var _46 = _4;
    var _47 = 0+(_46 != -1);
    if (_47) { __label__ = 5; /* _bb6 */  } else { __label__ = 11; /* _bb8 */ break _bb7; }
    var _41 = HEAP[_5];
    __ZN6node_tC1Ev(_41);
    var _42 = HEAP[_5];
    var _43 = _42+9*1;
    HEAP[_5] = _43;
    var _44 = _4;
    _4 = _44 - 1;
    __label__ = 10; /* _bb7 */ continue _bb7;
  }
  var _48 = HEAP[_6];
  HEAP[_retval_108] = _48;
  var _49 = HEAP[_retval_108];
  var _50 = _49;
  var _51 = HEAP[_7];
  HEAP[__ZL4pool] = _51;
  var _53 = HEAP[__ZL4pool];
  var _55 = _53+9*_count;
  HEAP[__ZL3end] = _55;
  __ZN3v_tC1Eddd(_2, 2.500000e-01, 1.000000e+00, -5.000000e-01);
  __ZNK3v_t4normEv(_3, _2);
  __ZN3v_tC1Eddd(_1, 0.000000e+00, 0.000000e+00, 0.000000e+00);
  var _56 = HEAP[__ZL4pool];
  var _59 = __ZL6createP6node_tii3v_tS1_d(_56, _lvl, _count, _1, _3, 1.000000e+00);
  return; // XXX
  var _60 = _SDL_Init(32);
  var _61 = _SDL_SetVideoMode(512, 512, 32, 0);
  HEAP[_screen] = _61;
  _y = 511;
  __label__ = 4; /* _bb10 */ 
  _bb10: while(1) { // _bb10
    var _65 = _y;
    var _66 = 0+(_65 >= 0);
    if (_66) { __label__ = 9; /* _bb9 */  } else { __label__ = 12; /* _bb11 */ break _bb10; }
    __ZL10trace_lineiii(512, 512, _y);
    var _63 = _y;
    _y = _63 - 1;
    __label__ = 4; /* _bb10 */ continue _bb10;
  }
  _SDL_Delay(20000);
  _SDL_Quit();
  _0 = 0;
  _retval = _0;
  __label__ = 0; /* _return */ 
  var _retval12 = _retval;
  STACKTOP = STACK_STACK.pop();
  return _retval12;
}

// === Auto-generated postamble setup entry stuff ===

function run(args) {
  __initializeRuntime__();

  var argc = args.length+1;
  function pad() {
    for (var i = 0; i < 1-1; i++) {
      argv.push(0);
    }
  }
  var argv = [Pointer_make(intArrayFromString("/bin/this.program"), null) ];
  pad();
  for (var i = 0; i < argc-1; i = i + 1) {
    argv.push(Pointer_make(intArrayFromString(args[i]), null));
    pad();
  }
  argv = Pointer_make(argv, null);

  __globalConstructor__();

  _main(argc, argv);

  while( __ATEXIT__.length > 0) {
    __ATEXIT__.pop()();
  }
}

