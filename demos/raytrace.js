//====================================================================================
// Auto-generated code. Original source + changes are
// sphere flake bvh raytracer (c) 2005, thierry berger-perrin <tbptbp@gmail.com>
//                            (c) 2010, azakai (Emscripten)
// this code is released under the GNU Public License.
//
// Optimized using Closure Compiler
//====================================================================================

function __globalConstructor__() {
}
var __THREW__ = false, __ATEXIT__ = [];
function assert(b, d) {
  if(!b) {
    d = "Assertion failed: " + d;
    print(d + ":\n" + Error().stack);
    this[alert] && alert(d);
    throw"Assertion: " + d;
  }
}
function Pointer_niceify(b) {
  return{slab:HEAP, pos:b}
}
function Pointer_make(b, d, a) {
  function c(f) {
    if(!f || f.length === undefined || typeof f === "function") {
      return[f]
    }
    return f.map(c).reduce(function(g, h) {
      return g.concat(h)
    }, [])
  }
  d = d ? d : 0;
  if(b === HEAP) {
    return d
  }
  b = c(b);
  a = [_malloc, stackAlloc, staticAlloc][a ? a : ALLOC_STATIC](Math.max(b.length - d, 1));
  for(var e = 0;e < b.length - d;e++) {
    HEAP[a + e] = b[d + e]
  }
  return a
}
function Pointer_stringify(b) {
  b = Pointer_niceify(b);
  for(var d = "", a = 0, c;;) {
    if(b.pos + a >= b.slab.length) {
      break
    }
    c = String.fromCharCode(b.slab[b.pos + a]);
    if(c == "\u0000") {
      break
    }
    d += c;
    a += 1
  }
  return d
}
ALLOC_NORMAL = 0;
ALLOC_STACK = 1;
ALLOC_STATIC = 2;
function alignMemory(b) {
  return Math.ceil(b / 1) * 1
}
PAGE_SIZE = 4096;
function alignMemoryPage(b) {
  return Math.ceil(b / PAGE_SIZE) * PAGE_SIZE
}
function stackEnter() {
  STACK_STACK.push(STACKTOP)
}
function stackExit() {
  STACKTOP = STACK_STACK.pop()
}
function stackAlloc(b) {
  b = alignMemory(b);
  assert(STACKTOP + b - STACKROOT < TOTAL_STACK, "No room on stack!");
  var d = STACKTOP;
  STACKTOP += b;
  return d
}
function staticAlloc(b) {
  b = alignMemory(b);
  assert(STATICTOP + b - STATICROOT < TOTAL_STATIC, "No room for static allocation!");
  var d = STATICTOP;
  STATICTOP += b;
  return d
}
if(!this._malloc) {
  _malloc = staticAlloc;
  _free = function() {
  }
}
__Znwm = __Znam = __Znaj = __Znwj = _malloc;
__ZdaPv = __ZdlPv = _free;
function __initializeRuntime__() {
  HEAP = intArrayFromString("(null)");
  this.TOTAL_STATIC || (TOTAL_STATIC = 6553600);
  STATICROOT = STATICTOP = alignMemoryPage(HEAP.length);
  this.TOTAL_STACK || (TOTAL_STACK = 655360);
  STACK_STACK = [];
  STACKROOT = STACKTOP = alignMemoryPage(STATICROOT + TOTAL_STATIC);
  HEAPTOP = alignMemoryPage(STACKROOT + TOTAL_STACK)
}
function __formatString() {
  for(var b = arguments[0], d = 1, a = [], c = -1;c != 0;) {
    c = HEAP[b];
    next = HEAP[b + 1];
    if(c == "%".charCodeAt(0) && ["d", "u", "f", "."].indexOf(String.fromCharCode(next)) != -1) {
      var e = String(arguments[d]);
      if(next == ".".charCodeAt(0)) {
        var f = parseInt(String.fromCharCode(HEAP[b + 2])), g = e.indexOf(".");
        if(g == -1) {
          g = e.length;
          e += "."
        }
        e += "00000000000";
        e = e.substr(0, g + 1 + f);
        b += 2
      }else {
        if(next == "u".charCodeAt(0)) {
          e = String(unSign(arguments[d], 32))
        }
      }
      e.split("").forEach(function(h) {
        a.push(h.charCodeAt(0))
      });
      d += 1;
      b += 2
    }else {
      if(c == "%".charCodeAt(0) && next == "s".charCodeAt(0)) {
        a = a.concat(String_copy(arguments[d]));
        d += 1;
        b += 2
      }else {
        a.push(c);
        b += 1
      }
    }
  }
  return Pointer_make(a, 0, ALLOC_STACK)
}
function Array_copy(b, d) {
  return HEAP.slice(b, b + d)
}
function String_copy(b, d) {
  return Array_copy(b, _strlen(b)).concat(d ? [0] : [])
}
function _atoi(b) {
  return Math.floor(Number(Pointer_stringify(b)))
}
function _llvm_memcpy_i32(b, d, a) {
  for(var c = 0;c < a;c++) {
    HEAP[b + c] = HEAP[d + c]
  }
}
_llvm_memcpy_i64 = _llvm_memcpy_i32;
PRINTBUFFER = "";
function __print__(b) {
  for(PRINTBUFFER += b;(b = PRINTBUFFER.indexOf("\n")) != -1;) {
    print(PRINTBUFFER.substr(0, b));
    PRINTBUFFER = PRINTBUFFER.substr(b + 1)
  }
}
function jrint(b, d) {
  if(d) {
    b += " : "
  }else {
    d = b;
    b = ""
  }
  print(b + JSON.stringify(d))
}
function intArrayFromString(b) {
  for(var d = [], a = 0;a < b.length;) {
    d.push(b.charCodeAt(a));
    a += 1
  }
  d.push(0);
  return d
}
function unSign(b, d) {
  if(b >= 0) {
    return b
  }
  return 2 * Math.abs(1 << d - 1) + b
}
var _0___FLATTENER = [0, 1], _struct__1__pthread_mutex_s___FLATTENER = [0, 1, 2, 3, 4, 5], _struct_SDL_BlitMap___FLATTENER = [], _struct_SDL_Color___FLATTENER = [0, 1, 2, 3], _struct_SDL_Palette___FLATTENER = [0, 1], _struct_SDL_PixelFormat___FLATTENER = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16], _struct_SDL_Rect___FLATTENER = [0, 1, 2, 3], _struct_SDL_Surface___FLATTENER = [0, 1, 2, 3, 4, 5, 6, 7, 8, 12, 13, 14, 15, 16], _struct___pthread_slist_t___FLATTENER = [0], _struct_basis_t___FLATTENER = 
[0, 3, 6], _struct_hit_t___FLATTENER = [0, 3], _struct_node_t___FLATTENER = [0, 4, 8], _struct_private_hwdata___FLATTENER = [], _struct_ray_t___FLATTENER = [0, 3], _struct_sphere_t___FLATTENER = [0, 3], __struct_std__ios_base__Init____FLATTENER = [0], _struct_v_t___FLATTENER = [0, 1, 2], _union__0__45___FLATTENER = [0], _union_pthread_attr_t___FLATTENER = [0, 1], _union_pthread_mutex_t___FLATTENER = [0], _union_pthread_mutexattr_t___FLATTENER = [0];
this.__defineGetter__("_screen", function() {
  delete _screen;
  return _screen = Pointer_make([0], 0, ALLOC_STATIC)
});
this.__defineGetter__("__ZStL8__ioinit", function() {
  delete __ZStL8__ioinit;
  return __ZStL8__ioinit = Pointer_make([0], 0, ALLOC_STATIC)
});
var ___dso_handle = 0;
this.__defineGetter__("__ZL5light", function() {
  delete __ZL5light;
  return __ZL5light = Pointer_make([0, 0, 0], 0, ALLOC_STATIC)
});
this.__defineGetter__("__ZL4pool", function() {
  delete __ZL4pool;
  return __ZL4pool = Pointer_make([0], 0, ALLOC_STATIC)
});
this.__defineGetter__("__ZL3end", function() {
  delete __ZL3end;
  return __ZL3end = Pointer_make([0], 0, ALLOC_STATIC)
});
this.__defineGetter__("__ZL4grid", function() {
  delete __ZL4grid;
  return __ZL4grid = Pointer_make([[-1, -0.3333333333333333], [0.3333333333333333, -1], [-0.3333333333333333, 1], [1, 0.3333333333333333]], 0, ALLOC_STATIC)
});
__globalConstructor__ = function() {
  __GLOBAL__I_screen()
};
_sqrt = function(b) {
  return Math.sqrt(b)
};
__ZNSt8ios_base4InitC1Ev = function() {
  __ZSt4cout = 1;
  __ZSt4cerr = 2
};
___cxa_atexit = function(b) {
  __ATEXIT__.push(b)
};
__ZNSt8ios_base4InitD1Ev = function() {
  __ZSt4cout = 1;
  __ZSt4cerr = 2
};
_cos = function(b) {
  return Math.cos(b)
};
_sin = function(b) {
  return Math.sin(b)
};
_SDL_LockSurface = function(b) {
  var d = SDL_SURFACES[b];
  d.image = d.ctx.getImageData(0, 0, d.width, d.height);
  for(var a = d.image.data.length, c = 0;c < a;c++) {
    HEAP[d.buffer + c] = d.image.data[c]
  }
  HEAP[b + 5] = d.buffer
};
_SDL_UnlockSurface = function(b) {
  b = SDL_SURFACES[b];
  for(var d = b.image.data.length, a = 0;a < d;a++) {
    b.image.data[a] = HEAP[b.buffer + a]
  }
  for(a = 0;a < d / 4;a++) {
    b.image.data[a * 4 + 3] = 255
  }
  b.ctx.putImageData(b.image, 0, 0);
  b.image = null
};
_SDL_Flip = function() {
};
_SDL_Init = function() {
  SDL_SURFACES = {};
  return 1
};
_SDL_SetVideoMode = function(b, d, a, c, e) {
  a = _malloc(14);
  SDL_SURFACES[a] = {width:b, height:d, canvas:e, ctx:e.getContext("2d"), surf:a, buffer:_malloc(b * d * 4)};
  return a
};
_SDL_Delay = function() {
};
_SDL_Quit = function() {
  return 1
};
function __GLOBAL__I_screen() {
  stackEnter();
  __Z41__static_initialization_and_destruction_0ii(1, 65535);
  stackExit()
}
function __ZnwjPv(b, d) {
  stackEnter();
  var a = Pointer_make([0], 0, ALLOC_STACK), c = Pointer_make([0], 0, ALLOC_STACK), e = Pointer_make([0], 0, ALLOC_STACK);
  HEAP[a] = d;
  HEAP[e] = HEAP[a];
  HEAP[e] = HEAP[e];
  HEAP[c] = HEAP[e];
  a = HEAP[c];
  stackExit();
  return a
}
function __ZN3v_tC1Ev(b) {
  stackEnter();
  var d = Pointer_make([0], 0, ALLOC_STACK);
  HEAP[d] = b;
  stackExit()
}
function __ZNK3v_tplERKS_(b, d, a) {
  stackEnter();
  var c = Pointer_make([0], 0, ALLOC_STACK), e = Pointer_make([0], 0, ALLOC_STACK);
  HEAP[c] = d;
  HEAP[e] = a;
  __ZN3v_tC1Eddd(b, HEAP[0 + HEAP[c] + 0] + HEAP[0 + HEAP[e] + 0], HEAP[0 + HEAP[c] + 1] + HEAP[0 + HEAP[e] + 1], HEAP[0 + HEAP[c] + 2] + HEAP[0 + HEAP[e] + 2]);
  stackExit()
}
function __ZNK3v_tmiERKS_(b, d, a) {
  stackEnter();
  var c = Pointer_make([0], 0, ALLOC_STACK), e = Pointer_make([0], 0, ALLOC_STACK);
  HEAP[c] = d;
  HEAP[e] = a;
  __ZN3v_tC1Eddd(b, HEAP[0 + HEAP[c] + 0] - HEAP[0 + HEAP[e] + 0], HEAP[0 + HEAP[c] + 1] - HEAP[0 + HEAP[e] + 1], HEAP[0 + HEAP[c] + 2] - HEAP[0 + HEAP[e] + 2]);
  stackExit()
}
function __ZNK3v_tngEv(b, d) {
  stackEnter();
  var a = Pointer_make([0], 0, ALLOC_STACK);
  HEAP[a] = d;
  __ZN3v_tC1Eddd(b, 0 - HEAP[0 + HEAP[a] + 0], 0 - HEAP[0 + HEAP[a] + 1], 0 - HEAP[0 + HEAP[a] + 2]);
  stackExit()
}
function __ZNK3v_tmlEd(b, d, a) {
  stackEnter();
  var c = Pointer_make([0], 0, ALLOC_STACK);
  HEAP[c] = d;
  __ZN3v_tC1Eddd(b, HEAP[0 + HEAP[c] + 0] * a, HEAP[0 + HEAP[c] + 1] * a, HEAP[0 + HEAP[c] + 2] * a);
  stackExit()
}
function __ZNK3v_t5crossERKS_(b, d, a) {
  stackEnter();
  var c = Pointer_make([0], 0, ALLOC_STACK), e = Pointer_make([0], 0, ALLOC_STACK);
  HEAP[c] = d;
  HEAP[e] = a;
  __ZN3v_tC1Eddd(b, HEAP[0 + HEAP[c] + 1] * HEAP[0 + HEAP[e] + 2] - HEAP[0 + HEAP[c] + 2] * HEAP[0 + HEAP[e] + 1], HEAP[0 + HEAP[c] + 2] * HEAP[0 + HEAP[e] + 0] - HEAP[0 + HEAP[c] + 0] * HEAP[0 + HEAP[e] + 2], HEAP[0 + HEAP[c] + 0] * HEAP[0 + HEAP[e] + 1] - HEAP[0 + HEAP[c] + 1] * HEAP[0 + HEAP[e] + 0]);
  stackExit()
}
function __ZNK3v_t3dotERKS_(b, d) {
  stackEnter();
  var a = Pointer_make([0], 0, ALLOC_STACK), c = Pointer_make([0], 0, ALLOC_STACK);
  HEAP[a] = b;
  HEAP[c] = d;
  a = HEAP[0 + HEAP[a] + 0] * HEAP[0 + HEAP[c] + 0] + HEAP[0 + HEAP[a] + 1] * HEAP[0 + HEAP[c] + 1] + HEAP[0 + HEAP[a] + 2] * HEAP[0 + HEAP[c] + 2];
  stackExit();
  return a
}
function __ZSt3maxIdERKT_S2_S2_(b, d) {
  stackEnter();
  var a, c = Pointer_make([0], 0, ALLOC_STACK), e = Pointer_make([0], 0, ALLOC_STACK), f = Pointer_make([0], 0, ALLOC_STACK), g = Pointer_make([0], 0, ALLOC_STACK);
  HEAP[c] = b;
  HEAP[e] = d;
  a = 0 + (HEAP[HEAP[c]] < HEAP[HEAP[e]]) ? 1 : 2;
  a:do {
    if(a == 1) {
      HEAP[g] = HEAP[e];
      break a
    }else {
      if(a == 2) {
        HEAP[g] = HEAP[c];
        break a
      }
    }
  }while(0);
  HEAP[f] = HEAP[g];
  a = HEAP[f];
  stackExit();
  return a
}
function __ZSt3maxIiERKT_S2_S2_(b, d) {
  stackEnter();
  var a, c = Pointer_make([0], 0, ALLOC_STACK), e = Pointer_make([0], 0, ALLOC_STACK), f = Pointer_make([0], 0, ALLOC_STACK), g = Pointer_make([0], 0, ALLOC_STACK);
  HEAP[c] = b;
  HEAP[e] = d;
  a = 0 + (HEAP[HEAP[c]] < HEAP[HEAP[e]]) ? 1 : 2;
  a:do {
    if(a == 1) {
      HEAP[g] = HEAP[e];
      break a
    }else {
      if(a == 2) {
        HEAP[g] = HEAP[c];
        break a
      }
    }
  }while(0);
  HEAP[f] = HEAP[g];
  a = HEAP[f];
  stackExit();
  return a
}
function ___tcf_0(b) {
  stackEnter();
  var d = Pointer_make([0], 0, ALLOC_STACK);
  HEAP[d] = b;
  __ZNSt8ios_base4InitD1Ev(__ZStL8__ioinit);
  stackExit()
}
function __ZN3v_tC1Eddd(b, d, a, c) {
  stackEnter();
  var e = Pointer_make([0], 0, ALLOC_STACK);
  HEAP[e] = b;
  HEAP[HEAP[e] + 0] = d;
  HEAP[HEAP[e] + 1] = a;
  HEAP[HEAP[e] + 2] = c;
  stackExit()
}
function __ZN5ray_tC1ERK3v_t(b, d) {
  stackEnter();
  var a = Pointer_make([0], 0, ALLOC_STACK), c = Pointer_make([0], 0, ALLOC_STACK);
  HEAP[a] = b;
  HEAP[c] = d;
  var e = HEAP[a] + 0;
  c = HEAP[c];
  HEAP[e + 0] = HEAP[0 + c + 0];
  HEAP[e + 1] = HEAP[0 + c + 1];
  HEAP[e + 2] = HEAP[0 + c + 2];
  __ZN3v_tC1Ev(HEAP[a] + 3);
  stackExit()
}
function __ZN5ray_tC1ERK3v_tS2_(b, d, a) {
  stackEnter();
  var c = Pointer_make([0], 0, ALLOC_STACK), e = Pointer_make([0], 0, ALLOC_STACK), f = Pointer_make([0], 0, ALLOC_STACK);
  HEAP[c] = b;
  HEAP[e] = d;
  HEAP[f] = a;
  b = HEAP[c] + 0;
  e = HEAP[e];
  HEAP[b + 0] = HEAP[0 + e + 0];
  HEAP[b + 1] = HEAP[0 + e + 1];
  HEAP[b + 2] = HEAP[0 + e + 2];
  c = HEAP[c] + 3;
  f = HEAP[f];
  HEAP[c + 0] = HEAP[0 + f + 0];
  HEAP[c + 1] = HEAP[0 + f + 1];
  HEAP[c + 2] = HEAP[0 + f + 2];
  stackExit()
}
function __ZN5hit_tC1Ev(b) {
  stackEnter();
  var d = Pointer_make([0], 0, ALLOC_STACK);
  HEAP[d] = b;
  __ZN3v_tC1Eddd(HEAP[d] + _struct_hit_t___FLATTENER[0], 0, 0, 0);
  HEAP[HEAP[d] + _struct_hit_t___FLATTENER[1]] = Infinity;
  stackExit()
}
function __ZN8sphere_tC1Ev(b) {
  stackEnter();
  var d = Pointer_make([0], 0, ALLOC_STACK);
  HEAP[d] = b;
  __ZN3v_tC1Ev(HEAP[d] + _struct_hit_t___FLATTENER[0]);
  stackExit()
}
function __ZN8sphere_tC1ERK3v_td(b, d, a) {
  stackEnter();
  var c = Pointer_make([0], 0, ALLOC_STACK), e = Pointer_make([0], 0, ALLOC_STACK);
  HEAP[c] = b;
  HEAP[e] = d;
  b = HEAP[c] + _struct_hit_t___FLATTENER[0];
  e = HEAP[e];
  HEAP[b + 0] = HEAP[0 + e + 0];
  HEAP[b + 1] = HEAP[0 + e + 1];
  HEAP[b + 2] = HEAP[0 + e + 2];
  HEAP[HEAP[c] + _struct_hit_t___FLATTENER[1]] = a;
  stackExit()
}
function __ZNK8sphere_t10get_normalERK3v_t(b, d, a) {
  stackEnter();
  var c = Pointer_make([0], 0, ALLOC_STACK), e = Pointer_make([0], 0, ALLOC_STACK), f = Pointer_make([0, 0, 0], 0, ALLOC_STACK);
  HEAP[c] = d;
  HEAP[e] = a;
  d = 1 / HEAP[0 + HEAP[c] + _struct_hit_t___FLATTENER[1]];
  __ZNK3v_tmiERKS_(f, HEAP[e], HEAP[c] + _struct_hit_t___FLATTENER[0]);
  __ZNK3v_tmlEd(b, f, d);
  stackExit()
}
function __ZN6node_tC1Ev(b) {
  stackEnter();
  var d = Pointer_make([0], 0, ALLOC_STACK);
  HEAP[d] = b;
  __ZN8sphere_tC1Ev(HEAP[d] + _struct_node_t___FLATTENER[0]);
  __ZN8sphere_tC1Ev(HEAP[d] + _struct_node_t___FLATTENER[1]);
  stackExit()
}
function __ZN6node_tC1ERK8sphere_tS2_l(b, d, a, c) {
  stackEnter();
  var e = Pointer_make([0], 0, ALLOC_STACK), f = Pointer_make([0], 0, ALLOC_STACK), g = Pointer_make([0], 0, ALLOC_STACK);
  HEAP[e] = b;
  HEAP[f] = d;
  HEAP[g] = a;
  b = HEAP[e] + _struct_node_t___FLATTENER[0];
  f = HEAP[f];
  d = b + _struct_hit_t___FLATTENER[0];
  a = f + _struct_hit_t___FLATTENER[0];
  HEAP[d + 0] = HEAP[0 + a + 0];
  HEAP[d + 1] = HEAP[0 + a + 1];
  HEAP[d + 2] = HEAP[0 + a + 2];
  HEAP[b + _struct_hit_t___FLATTENER[1]] = HEAP[0 + f + _struct_hit_t___FLATTENER[1]];
  f = HEAP[e] + _struct_node_t___FLATTENER[1];
  g = HEAP[g];
  b = f + _struct_hit_t___FLATTENER[0];
  d = g + _struct_hit_t___FLATTENER[0];
  HEAP[b + 0] = HEAP[0 + d + 0];
  HEAP[b + 1] = HEAP[0 + d + 1];
  HEAP[b + 2] = HEAP[0 + d + 2];
  HEAP[f + _struct_hit_t___FLATTENER[1]] = HEAP[0 + g + _struct_hit_t___FLATTENER[1]];
  HEAP[HEAP[e] + _struct_node_t___FLATTENER[2]] = c;
  stackExit()
}
function __ZN7basis_tC1ERK3v_t(b, d) {
  stackEnter();
  var a, c = Pointer_make([0], 0, ALLOC_STACK);
  a = Pointer_make([0], 0, ALLOC_STACK);
  var e = Pointer_make([0, 0, 0], 0, ALLOC_STACK), f = Pointer_make([0, 0, 0], 0, ALLOC_STACK), g = Pointer_make([0, 0, 0], 0, ALLOC_STACK), h = Pointer_make([0, 0, 0], 0, ALLOC_STACK);
  HEAP[c] = b;
  HEAP[a] = d;
  __ZN3v_tC1Ev(HEAP[c] + 0);
  __ZN3v_tC1Ev(HEAP[c] + 3);
  __ZN3v_tC1Ev(HEAP[c] + 6);
  __ZNK3v_t4normEv(f, HEAP[a]);
  a = 0 + ((0 + (0 + (HEAP[0 + f + 0] * HEAP[0 + f + 0] != 1) != 0) & 0 + (0 + (HEAP[0 + f + 1] * HEAP[0 + f + 1] != 1) != 0) & 0 + (HEAP[0 + f + 2] * HEAP[0 + f + 2] != 1)) != 0) ? 1 : 4;
  a:do {
    if(a == 1) {
      e = HEAP[c] + 3;
      HEAP[e + 0] = HEAP[0 + f + 0];
      HEAP[e + 1] = HEAP[0 + f + 1];
      HEAP[e + 2] = HEAP[0 + f + 2];
      a = 0 + (HEAP[0 + f + 1] * HEAP[0 + f + 1] > HEAP[0 + f + 0] * HEAP[0 + f + 0]) ? 3 : 5;
      b:do {
        if(a == 3) {
          a = 0 + (HEAP[0 + f + 1] * HEAP[0 + f + 1] > HEAP[0 + f + 2] * HEAP[0 + f + 2]) ? 6 : 7;
          c:do {
            if(a == 6) {
              HEAP[HEAP[c] + 3 + 1] = 0 - HEAP[0 + (HEAP[c] + 3) + 1];
              break c
            }else {
              if(a == 7) {
                HEAP[HEAP[c] + 3 + 2] = 0 - HEAP[0 + (HEAP[c] + 3) + 2];
                break c
              }
            }
          }while(0);
          break b
        }else {
          if(a == 5) {
            a = 0 + (HEAP[0 + f + 2] * HEAP[0 + f + 2] > HEAP[0 + f + 0] * HEAP[0 + f + 0]) ? 10 : 11;
            do {
              if(a == 10) {
                HEAP[HEAP[c] + 3 + 2] = 0 - HEAP[0 + (HEAP[c] + 3) + 2];
                break b
              }else {
                if(a == 11) {
                  HEAP[HEAP[c] + 3 + 0] = 0 - HEAP[0 + (HEAP[c] + 3) + 0];
                  break b
                }
              }
            }while(0)
          }
        }
      }while(0);
      break a
    }else {
      if(a == 4) {
        __ZN3v_tC1Eddd(e, HEAP[0 + f + 2], HEAP[0 + f + 0], HEAP[0 + f + 1]);
        a = HEAP[c] + 3;
        HEAP[a + 0] = HEAP[0 + e + 0];
        HEAP[a + 1] = HEAP[0 + e + 1];
        HEAP[a + 2] = HEAP[0 + e + 2];
        break a
      }
    }
  }while(0);
  e = HEAP[c] + 0;
  HEAP[e + 0] = HEAP[0 + f + 0];
  HEAP[e + 1] = HEAP[0 + f + 1];
  HEAP[e + 2] = HEAP[0 + f + 2];
  f = HEAP[c] + 6;
  __ZNK3v_t5crossERKS_(g, HEAP[c] + 0, HEAP[c] + 3);
  HEAP[f + 0] = HEAP[0 + g + 0];
  HEAP[f + 1] = HEAP[0 + g + 1];
  HEAP[f + 2] = HEAP[0 + g + 2];
  g = HEAP[c] + 3;
  __ZNK3v_t5crossERKS_(h, HEAP[c] + 0, HEAP[c] + 6);
  HEAP[g + 0] = HEAP[0 + h + 0];
  HEAP[g + 1] = HEAP[0 + h + 1];
  HEAP[g + 2] = HEAP[0 + h + 2];
  stackExit()
}
function __ZNK3v_t6magsqrEv(b) {
  stackEnter();
  var d = Pointer_make([0], 0, ALLOC_STACK);
  HEAP[d] = b;
  b = __ZNK3v_t3dotERKS_(HEAP[d], HEAP[d]);
  stackExit();
  return b
}
function __ZNK3v_t4normEv(b, d) {
  stackEnter();
  var a = Pointer_make([0], 0, ALLOC_STACK);
  HEAP[a] = d;
  var c = __ZNK3v_t6magsqrEv(HEAP[a]);
  c = 1 / _sqrt(c);
  __ZNK3v_tmlEd(b, HEAP[a], c);
  stackExit()
}
function __Z41__static_initialization_and_destruction_0ii(b, d) {
  stackEnter();
  var a, c = Pointer_make([0, 0, 0], 0, ALLOC_STACK);
  a = 0 + (b == 1) ? 1 : 3;
  a:for(;;) {
    do {
      if(a == 1) {
        if(!(0 + (d == 65535))) {
          a = 3;
          continue a
        }
        __ZNSt8ios_base4InitC1Ev(__ZStL8__ioinit);
        ___cxa_atexit(___tcf_0, 0, ___dso_handle);
        __ZN3v_tC1Eddd(c, -0.5, -0.65, 0.9);
        __ZNK3v_t4normEv(__ZL5light, c);
        a = 3;
        continue a
      }else {
        if(a == 3) {
          break a
        }
      }
    }while(0)
  }
  stackExit()
}
function __ZNK8sphere_t9intersectERK5ray_t(b, d) {
  stackEnter();
  var a, c = Pointer_make([0], 0, ALLOC_STACK), e = Pointer_make([0], 0, ALLOC_STACK), f, g, h = Pointer_make([0, 0, 0], 0, ALLOC_STACK), i;
  HEAP[c] = b;
  HEAP[e] = d;
  __ZNK3v_tmiERKS_(h, HEAP[c] + _struct_hit_t___FLATTENER[0], HEAP[e] + 0);
  e = __ZNK3v_t3dotERKS_(HEAP[e] + 3, h);
  a = e * e;
  h = __ZNK3v_t6magsqrEv(h);
  c = a - h + HEAP[0 + HEAP[c] + _struct_hit_t___FLATTENER[1]] * HEAP[0 + HEAP[c] + _struct_hit_t___FLATTENER[1]];
  a = 0 + (c < 0) ? 1 : 2;
  a:do {
    if(a == 1) {
      g = Infinity;
      break a
    }else {
      if(a == 2) {
        a = _sqrt(c);
        h = e + a;
        i = e - a;
        a = 0 + (h < 0) ? 3 : 6;
        do {
          if(a == 3) {
            g = Infinity;
            break a
          }else {
            if(a == 6) {
              a = 0 + (i > 0) ? 7 : 8;
              c:do {
                if(a == 7) {
                  f = i;
                  break c
                }else {
                  if(a == 8) {
                    f = h;
                    break c
                  }
                }
              }while(0);
              g = f;
              break a
            }
          }
        }while(0)
      }
    }
  }while(0);
  f = g;
  stackExit();
  return f
}
function __ZN6node_t9intersectILb0EEEvRK5ray_tR5hit_t(b, d) {
  stackEnter();
  var a, c = Pointer_make([0], 0, ALLOC_STACK), e = Pointer_make([0], 0, ALLOC_STACK), f = Pointer_make([0, 0, 0], 0, ALLOC_STACK), g = Pointer_make([0, 0, 0], 0, ALLOC_STACK), h, i = Pointer_make([0], 0, ALLOC_STACK), j = Pointer_make([0, 0, 0], 0, ALLOC_STACK);
  HEAP[c] = b;
  HEAP[e] = d;
  HEAP[i] = HEAP[__ZL4pool];
  a:for(;;) {
    if(!(0 + (HEAP[i] < HEAP[__ZL3end]))) {
      break a
    }
    h = 0 + (__ZNK8sphere_t9intersectERK5ray_t(HEAP[i] + _struct_node_t___FLATTENER[0], HEAP[c]) >= HEAP[0 + HEAP[e] + _struct_hit_t___FLATTENER[1]]);
    a = 0 + (h != 0) ? 2 : 3;
    do {
      if(a == 2) {
        HEAP[i] += 9 * HEAP[0 + HEAP[i] + _struct_node_t___FLATTENER[2]];
        continue a
      }else {
        if(a == 3) {
          h = __ZNK8sphere_t9intersectERK5ray_t(HEAP[i] + _struct_node_t___FLATTENER[1], HEAP[c]);
          a = 0 + (HEAP[0 + HEAP[e] + _struct_hit_t___FLATTENER[1]] > h) ? 6 : 7;
          c:for(;;) {
            do {
              if(a == 6) {
                HEAP[HEAP[e] + _struct_hit_t___FLATTENER[1]] = h;
                __ZNK3v_tmlEd(f, HEAP[c] + 3, h);
                __ZNK3v_tplERKS_(g, HEAP[c] + 0, f);
                a = HEAP[e] + _struct_hit_t___FLATTENER[0];
                __ZNK8sphere_t10get_normalERK3v_t(j, HEAP[i] + _struct_node_t___FLATTENER[1], g);
                HEAP[a + 0] = HEAP[0 + j + 0];
                HEAP[a + 1] = HEAP[0 + j + 1];
                HEAP[a + 2] = HEAP[0 + j + 2];
                a = 7;
                continue c
              }else {
                if(a == 7) {
                  HEAP[i] += 9;
                  continue a
                }
              }
            }while(0)
          }
        }
      }
    }while(0)
  }
  stackExit()
}
function __ZN6node_t9intersectILb1EEEvRK5ray_tR5hit_t(b, d) {
  stackEnter();
  var a, c = Pointer_make([0], 0, ALLOC_STACK), e = Pointer_make([0], 0, ALLOC_STACK);
  Pointer_make([0, 0, 0], 0, ALLOC_STACK);
  Pointer_make([0, 0, 0], 0, ALLOC_STACK);
  var f = Pointer_make([0], 0, ALLOC_STACK), g;
  HEAP[c] = b;
  HEAP[e] = d;
  HEAP[f] = HEAP[__ZL4pool];
  a:for(;;) {
    if(!(0 + (HEAP[f] < HEAP[__ZL3end]))) {
      a = 5;
      break a
    }
    a = 0 + (__ZNK8sphere_t9intersectERK5ray_t(HEAP[f] + _struct_node_t___FLATTENER[0], HEAP[c]) >= HEAP[0 + HEAP[e] + _struct_hit_t___FLATTENER[1]]);
    a = 0 + (a != 0) ? 2 : 3;
    do {
      if(a == 2) {
        HEAP[f] += 9 * HEAP[0 + HEAP[f] + _struct_node_t___FLATTENER[2]];
        a = 8;
        continue a
      }else {
        if(a == 3) {
          g = __ZNK8sphere_t9intersectERK5ray_t(HEAP[f] + _struct_node_t___FLATTENER[1], HEAP[c]);
          if(0 + (HEAP[0 + HEAP[e] + _struct_hit_t___FLATTENER[1]] > g)) {
            a = 6;
            break a
          }
          HEAP[f] += 9;
          a = 8;
          continue a
        }
      }
    }while(0)
  }
  a:for(;;) {
    do {
      if(a == 6) {
        HEAP[HEAP[e] + _struct_hit_t___FLATTENER[1]] = g;
        a = 5;
        continue a
      }else {
        if(a == 5) {
          break a
        }
      }
    }while(0)
  }
  stackExit()
}
function __ZL9ray_tracePK6node_tRK5ray_t(b, d) {
  stackEnter();
  var a;
  a = Pointer_make([0], 0, ALLOC_STACK);
  var c = Pointer_make([0], 0, ALLOC_STACK), e, f = Pointer_make([0, 0, 0], 0, ALLOC_STACK), g = Pointer_make([0, 0, 0], 0, ALLOC_STACK), h = Pointer_make([0, 0, 0], 0, ALLOC_STACK), i = Pointer_make([0, 0, 0], 0, ALLOC_STACK), j = Pointer_make([0, 0, 0], 0, ALLOC_STACK), m, l, k = Pointer_make([0, 0, 0, 0], 0, ALLOC_STACK), n = Pointer_make([0, 0, 0, 0, 0, 0], 0, ALLOC_STACK), o = Pointer_make([0, 0, 0, 0], 0, ALLOC_STACK);
  HEAP[a] = b;
  HEAP[c] = d;
  __ZN5hit_tC1Ev(k);
  __ZN6node_t9intersectILb0EEEvRK5ray_tR5hit_t(HEAP[c], k);
  a = 0 + (HEAP[0 + k + _struct_hit_t___FLATTENER[1]] != Infinity) ? 1 : 2;
  a:do {
    if(a == 1) {
      l = 0 - __ZNK3v_t3dotERKS_(k + _struct_hit_t___FLATTENER[0], __ZL5light);
      break a
    }else {
      if(a == 2) {
        l = 0;
        break a
      }
    }
  }while(0);
  l = l;
  a = 0 + (l <= 0) ? 6 : 7;
  a:do {
    if(a == 6) {
      m = 0;
      break a
    }else {
      if(a == 7) {
        __ZNK3v_tngEv(j, __ZL5light);
        __ZNK3v_tmlEd(h, k + _struct_hit_t___FLATTENER[0], 1.0E-12);
        __ZNK3v_tmlEd(f, HEAP[c] + 3, HEAP[0 + k + _struct_hit_t___FLATTENER[1]]);
        __ZNK3v_tplERKS_(g, HEAP[c] + 0, f);
        __ZNK3v_tplERKS_(i, g, h);
        __ZN5ray_tC1ERK3v_tS2_(n, i, j);
        __ZN5hit_tC1Ev(o);
        __ZN6node_t9intersectILb1EEEvRK5ray_tR5hit_t(n, o);
        a = 0 + (HEAP[0 + o + _struct_hit_t___FLATTENER[1]] == Infinity) ? 8 : 5;
        b:do {
          if(a == 8) {
            e = l;
            break b
          }else {
            if(a == 5) {
              e = 0;
              break b
            }
          }
        }while(0);
        m = e;
        break a
      }
    }
  }while(0);
  c = m;
  stackExit();
  return c
}
function __ZL6createP6node_tii3v_tS1_d(b, d, a, c, e, f) {
  stackEnter();
  var g, h = Pointer_make([0], 0, ALLOC_STACK), i = Pointer_make([0], 0, ALLOC_STACK), j = Pointer_make([0, 0, 0], 0, ALLOC_STACK), m = Pointer_make([0, 0, 0], 0, ALLOC_STACK), l = Pointer_make([0, 0, 0], 0, ALLOC_STACK), k = Pointer_make([0, 0, 0], 0, ALLOC_STACK), n = Pointer_make([0, 0, 0], 0, ALLOC_STACK), o = Pointer_make([0, 0, 0], 0, ALLOC_STACK), A = Pointer_make([0, 0, 0], 0, ALLOC_STACK), u = Pointer_make([0, 0, 0], 0, ALLOC_STACK), r = Pointer_make([0, 0, 0], 0, ALLOC_STACK), B = Pointer_make([0, 
  0, 0], 0, ALLOC_STACK), C = Pointer_make([0, 0, 0], 0, ALLOC_STACK), D = Pointer_make([0, 0, 0], 0, ALLOC_STACK), E = Pointer_make([0, 0, 0], 0, ALLOC_STACK), F = Pointer_make([0, 0, 0], 0, ALLOC_STACK), s = Pointer_make([0], 0, ALLOC_STACK), p = Pointer_make([0], 0, ALLOC_STACK), x = Pointer_make([0], 0, ALLOC_STACK), t, q = Pointer_make([0], 0, ALLOC_STACK), v = Pointer_make([0], 0, ALLOC_STACK);
  g = Pointer_make([0, 0, 0, 0], 0, ALLOC_STACK);
  var G = Pointer_make([0], 0, ALLOC_STACK), H = Pointer_make([0, 0, 0, 0], 0, ALLOC_STACK), I = Pointer_make([0], 0, ALLOC_STACK), w = Pointer_make([0, 0, 0, 0, 0, 0, 0, 0, 0], 0, ALLOC_STACK), y = Pointer_make([0, 0, 0], 0, ALLOC_STACK), z = Pointer_make([0, 0, 0], 0, ALLOC_STACK);
  HEAP[h] = b;
  b = a;
  __ZN8sphere_tC1ERK3v_td(H, c, f * 2);
  HEAP[I] = H;
  __ZN8sphere_tC1ERK3v_td(g, c, f);
  HEAP[G] = g;
  g = __ZnwjPv(68, HEAP[h]);
  HEAP[v] = g;
  g = 0 + (HEAP[v] != 0) ? 1 : 7;
  a:do {
    if(g == 1) {
      g = 0 + (d > 1) ? 2 : 3;
      b:do {
        if(g == 2) {
          t = b;
          break b
        }else {
          if(g == 3) {
            t = 1;
            break b
          }
        }
      }while(0);
      __ZN6node_tC1ERK8sphere_tS2_l(HEAP[v], HEAP[I], HEAP[G], t);
      HEAP[q] = HEAP[v];
      break a
    }else {
      if(g == 7) {
        HEAP[q] = HEAP[v];
        break a
      }
    }
  }while(0);
  HEAP[h] = HEAP[q] + 9;
  g = 0 + (d <= 1) ? 5 : 10;
  a:do {
    if(g == 5) {
      HEAP[x] = HEAP[h];
      break a
    }else {
      if(g == 10) {
        HEAP[p] = 1;
        HEAP[s] = Math.floor((b - 9) / 9);
        b = __ZSt3maxIiERKT_S2_S2_(s, p);
        b = HEAP[b];
        __ZN7basis_tC1ERK3v_t(w, e);
        s = f / 3;
        t = p = 0;
        b:for(;;) {
          if(!(0 + (t <= 5))) {
            break b
          }
          q = _cos(p);
          __ZNK3v_tmlEd(E, w + 6, q);
          q = _sin(p);
          __ZNK3v_tmlEd(C, w + 3, q);
          __ZNK3v_tmlEd(B, e, -0.2);
          __ZNK3v_tplERKS_(D, B, C);
          __ZNK3v_tplERKS_(F, D, E);
          __ZNK3v_t4normEv(y, F);
          __ZNK3v_tmlEd(u, y, f + s);
          __ZNK3v_tplERKS_(r, c, u);
          q = __ZL6createP6node_tii3v_tS1_d(HEAP[h], d - 1, b, r, y, s);
          HEAP[h] = q;
          p += 1.0471975511965976;
          t += 1
        }
        p -= 0.3490658503988659;
        u = 0;
        b:for(;;) {
          if(!(0 + (u <= 2))) {
            break b
          }
          r = _cos(p);
          __ZNK3v_tmlEd(o, w + 6, r);
          r = _sin(p);
          __ZNK3v_tmlEd(k, w + 3, r);
          __ZNK3v_tmlEd(l, e, 0.6);
          __ZNK3v_tplERKS_(n, l, k);
          __ZNK3v_tplERKS_(A, n, o);
          __ZNK3v_t4normEv(z, A);
          __ZNK3v_tmlEd(j, z, f + s);
          __ZNK3v_tplERKS_(m, c, j);
          r = __ZL6createP6node_tii3v_tS1_d(HEAP[h], d - 1, b, m, z, s);
          HEAP[h] = r;
          p += 2.0943951023931953;
          u += 1
        }
        HEAP[x] = HEAP[h];
        break a
      }
    }
  }while(0);
  HEAP[i] = HEAP[x];
  d = HEAP[i];
  stackExit();
  return d
}
function __ZL10trace_lineiii(b, d, a) {
  stackEnter();
  var c = Pointer_make([0, 0, 0], 0, ALLOC_STACK), e = Pointer_make([0, 0, 0], 0, ALLOC_STACK), f = Pointer_make([0], 0, ALLOC_STACK), g = Pointer_make([0], 0, ALLOC_STACK), h = Pointer_make([0], 0, ALLOC_STACK), i = Pointer_make([0, 0, 0], 0, ALLOC_STACK), j = Pointer_make([0], 0, ALLOC_STACK), m = Pointer_make([0], 0, ALLOC_STACK), l = Pointer_make([0, 0, 0, 0, 0, 0], 0, ALLOC_STACK), k = Pointer_make([0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0], 0, ALLOC_STACK), n = Pointer_make([0, 0, 0], 0, ALLOC_STACK), 
  o = Pointer_make([0, 0, 0], 0, ALLOC_STACK);
  HEAP[j] = b;
  HEAP[m] = d;
  __ZN3v_tC1Eddd(i, 0, 0, -4.5);
  __ZN5ray_tC1ERK3v_t(l, i);
  HEAP[g] = k + 0;
  HEAP[f] = HEAP[g];
  d = 3;
  a:for(;;) {
    if(!(0 + (d != -1))) {
      break a
    }
    __ZN3v_tC1Ev(HEAP[f]);
    HEAP[f] += 3;
    d -= 1
  }
  HEAP[h] = HEAP[g];
  f = 0;
  a:for(;;) {
    if(!(0 + (f <= 3))) {
      break a
    }
    g = f;
    __ZN3v_tC1Eddd(e, HEAP[0 + (__ZL4grid + f * 2) + 0] * 0.5 + HEAP[j] / -2, HEAP[0 + (__ZL4grid + f * 2) + 1] * 0.5 + HEAP[m] / -2, 0);
    g = k + g * 3;
    HEAP[g + 0] = HEAP[0 + e + 0];
    HEAP[g + 1] = HEAP[0 + e + 1];
    HEAP[g + 2] = HEAP[0 + e + 2];
    f += 1
  }
  e = __ZSt3maxIdERKT_S2_S2_(j, m);
  __ZN3v_tC1Eddd(n, 0, a, HEAP[e]);
  _SDL_LockSurface(HEAP[_screen]);
  a = b;
  a:for(;;) {
    if(!(0 + (a != 0))) {
      break a
    }
    j = e = 0;
    b:for(;;) {
      if(!(0 + (j <= 3))) {
        break b
      }
      __ZNK3v_tplERKS_(c, n, k + j * 3);
      m = l + 3;
      __ZNK3v_t4normEv(o, c);
      HEAP[m + 0] = HEAP[0 + o + 0];
      HEAP[m + 1] = HEAP[0 + o + 1];
      HEAP[m + 2] = HEAP[0 + o + 2];
      e = __ZL9ray_tracePK6node_tRK5ray_t(HEAP[__ZL4pool], l) + e;
      j += 1
    }
    j = 0;
    b:for(;;) {
      if(!(0 + (j <= 2))) {
        break b
      }
      HEAP[HEAP[0 + HEAP[_screen] + _struct_SDL_Surface___FLATTENER[5]] + 1 * Math.floor((b - HEAP[0 + n + 1]) * b * 4 + HEAP[0 + n + 0] * 4) + 1 * j] = Math.floor(e * 64);
      j += 1
    }
    HEAP[n + 0] = HEAP[0 + n + 0] + 1;
    a -= 1
  }
  _SDL_UnlockSurface(HEAP[_screen]);
  _SDL_Flip(HEAP[_screen]);
  stackExit()
}
function _main(b, d) {
  stackEnter();
  var a, c = Pointer_make([0], 0, ALLOC_STACK), e = Pointer_make([0, 0, 0], 0, ALLOC_STACK), f = Pointer_make([0, 0, 0], 0, ALLOC_STACK), g = Pointer_make([0, 0, 0], 0, ALLOC_STACK), h = Pointer_make([0], 0, ALLOC_STACK), i = Pointer_make([0], 0, ALLOC_STACK), j = Pointer_make([0], 0, ALLOC_STACK), m = Pointer_make([0], 0, ALLOC_STACK), l = Pointer_make([0], 0, ALLOC_STACK), k = Pointer_make([0], 0, ALLOC_STACK), n;
  HEAP[c] = d;
  a = 0 + (b == 2) ? 1 : 2;
  a:do {
    if(a == 1) {
      HEAP[k] = 2;
      a = _atoi(HEAP[0 + HEAP[c] + 1]);
      HEAP[l] = a;
      l = __ZSt3maxIiERKT_S2_S2_(l, k);
      n = HEAP[l];
      break a
    }else {
      if(a == 2) {
        n = 6;
        break a
      }
    }
  }while(0);
  l = n;
  k = 9;
  c = l;
  a:for(;;) {
    c -= 1;
    a = 0 + (c > 1);
    if(!(0 + (a != 0))) {
      break a
    }
    k = (k + 1) * 9
  }
  k += 1;
  a = k;
  c = __Znaj(a * 68);
  HEAP[m] = c;
  HEAP[i] = HEAP[m];
  HEAP[h] = HEAP[i];
  a = a - 1;
  a:for(;;) {
    if(!(0 + (a != -1))) {
      break a
    }
    __ZN6node_tC1Ev(HEAP[h]);
    HEAP[h] += 9;
    a -= 1
  }
  HEAP[j] = HEAP[i];
  HEAP[__ZL4pool] = HEAP[m];
  HEAP[__ZL3end] = HEAP[__ZL4pool] + 9 * k;
  __ZN3v_tC1Eddd(f, 0.25, 1, -0.5);
  __ZNK3v_t4normEv(g, f);
  __ZN3v_tC1Eddd(e, 0, 0, 0);
  __ZL6createP6node_tii3v_tS1_d(HEAP[__ZL4pool], l, k, e, g, 1);
}
function run(b) {
  function d() {
    for(var f = 0;f < 0;f++) {
      c.push(0)
    }
  }
  __initializeRuntime__();
  var a = b.length + 1, c = [Pointer_make(intArrayFromString("/bin/this.program"), null, ALLOC_STATIC)];
  d();
  for(var e = 0;e < a - 1;e += 1) {
    c.push(Pointer_make(intArrayFromString(b[e]), null, ALLOC_STATIC));
    d()
  }
  c = Pointer_make(c, null, ALLOC_STATIC);
  __globalConstructor__();
  for(_main(a, c);__ATEXIT__.length > 0;) {
    __ATEXIT__.pop()()
  }
}
;
