//====================================================================================
// Auto-generated code. Original source + changes are
// sphere flake bvh raytracer (c) 2005, thierry berger-perrin <tbptbp@gmail.com>
//                            (c) 2010, azakai (Emscripten)
// this code is released under the GNU Public License.
//
// Optimized using Closure Compiler
//====================================================================================

function l() {
  return function() {
  }
}
function m() {
}
FUNCTION_TABLE = [];
var o = [];
function q(a, b) {
  if(!a) {
    b = "Assertion failed: " + b;
    print(b + ":\n" + Error().stack);
    throw"Assertion: " + b;
  }
}
ALLOC_NORMAL = 0;
ALLOC_STACK = 1;
ALLOC_STATIC = 2;
function r(a, b, c) {
  function d(g) {
    if(!g || g.length === undefined || typeof g === "function") {
      return[g]
    }
    return g.map(d).reduce(function(i, j) {
      return i.concat(j)
    }, [])
  }
  b = b ? b : 0;
  if(a === HEAP) {
    return b
  }
  a = d(a);
  c = [_malloc, aa, u][c ? c : ALLOC_STATIC](Math.max(a.length - b, 1));
  for(var e = 0;e < a.length - b;e++) {
    var h = a[b + e];
    if(typeof h === "number" || typeof h === "boolean") {
      IHEAP[c + e] = h;
      FHEAP[c + e] = h
    }else {
      HEAP[c + e] = h
    }
  }
  return c
}
function ba(a) {
  a = {e:IHEAP, d:a};
  for(var b = "", c = 0, d;;) {
    if(a.d + c >= a.e.length) {
      break
    }
    d = String.fromCharCode(a.e[a.d + c]);
    if(d == "\u0000") {
      break
    }
    b += d;
    c += 1
  }
  return b
}
PAGE_SIZE = 4096;
if(!this.f) {
  _malloc = u;
  _free = l()
}
__Znwm = __Znam = __Znaj = __Znwj = _malloc;
__ZdaPv = __ZdlPv = _free;
function ia() {
  HEAP = x("(null)");
  this.TOTAL_MEMORY || (TOTAL_MEMORY = 52428800);
  if(this.Int32Array) {
    IHEAP = new Int32Array(TOTAL_MEMORY);
    for(var a = 0;a < HEAP.length;a++) {
      IHEAP[a] = HEAP[a]
    }
  }else {
    IHEAP = HEAP
  }
  FHEAP = this.Float64Array ? new Float64Array(TOTAL_MEMORY) : HEAP;
  STACK_ROOT = STACKTOP = Math.ceil(10 / PAGE_SIZE) * PAGE_SIZE;
  this.TOTAL_STACK || (TOTAL_STACK = 1048576);
  STACK_MAX = STACK_ROOT + TOTAL_STACK;
  STATICTOP = Math.ceil(STACK_MAX / PAGE_SIZE) * PAGE_SIZE
}
function ja(a, b, c) {
  for(var d = 0;d < c;d++) {
    HEAP[a + d] = HEAP[b + d];
    IHEAP[a + d] = IHEAP[b + d];
    FHEAP[a + d] = FHEAP[b + d]
  }
}
_llvm_memcpy_p0i8_p0i8_i32 = _llvm_memcpy_i64 = ja;
_llvm_memset_p0i8_i32 = function(a, b, c) {
  for(var d = 0;d < c;d++) {
    HEAP[a + d] = IHEAP[a + d] = FHEAP[a + d] = b
  }
};
PRINTBUFFER = "";
function x(a) {
  for(var b = [], c = 0;c < a.length;) {
    b.push(a.charCodeAt(c));
    c += 1
  }
  b.push(0);
  return b
}
function aa(a) {
  var b = STACKTOP;
  q(a > 0);
  STACKTOP += a;
  STACKTOP = Math.ceil(STACKTOP / 4) * 4;
  q(STACKTOP < STACK_ROOT + STACK_MAX);
  return b
}
function u(a) {
  var b = STATICTOP;
  q(a > 0);
  STATICTOP += a;
  STATICTOP = Math.ceil(STATICTOP / 4) * 4;
  return b
}
var ka = [0, 1, 2, 3, 4, 5, 6, 7, 8, 12, 13, 14, 15, 16];
this.__defineGetter__("_screen", function() {
  delete _screen;
  return _screen = r([0], 0, ALLOC_STATIC)
});
this.__defineGetter__("__ZStL8__ioinit", function() {
  delete __ZStL8__ioinit;
  return __ZStL8__ioinit = r([0], 0, ALLOC_STATIC)
});
var la = 0;
this.__defineGetter__("__ZL5light", function() {
  delete __ZL5light;
  return __ZL5light = r([0, 0, 0], 0, ALLOC_STATIC)
});
this.__defineGetter__("__ZL4pool", function() {
  delete __ZL4pool;
  return __ZL4pool = r([0], 0, ALLOC_STATIC)
});
this.__defineGetter__("__ZL3end", function() {
  delete __ZL3end;
  return __ZL3end = r([0], 0, ALLOC_STATIC)
});
this.__defineGetter__("__ZL4grid", function() {
  delete __ZL4grid;
  return __ZL4grid = r([[-1, -0.3333333333333333], [0.3333333333333333, -1], [-0.3333333333333333, 1], [1, 0.3333333333333333]], 0, ALLOC_STATIC)
});
m = function() {
  B()
};
_sqrt = Math.sqrt;
__ZNSt8ios_base4InitC1Ev = function() {
  __ZSt4cout = 1;
  __ZSt4cerr = 2
};
___cxa_atexit = function(a) {
  o.push(a)
};
__ZNSt8ios_base4InitD1Ev = function() {
  __ZSt4cout = 1;
  __ZSt4cerr = 2
};
_cos = Math.cos;
_sin = Math.sin;
_SDL_LockSurface = function(a) {
  var b = SDL_SURFACES[a];
  b.a = b.c.getImageData(0, 0, b.width, b.height);
  for(var c = b.a.data.length, d = 0;d < c;d++) {
    IHEAP[b.b + d] = b.a.data[d]
  }
  IHEAP[a + 5] = b.b
};
_SDL_UnlockSurface = function(a) {
  a = SDL_SURFACES[a];
  for(var b = a.a.data.length, c = 0;c < b;c++) {
    a.a.data[c] = IHEAP[a.b + c]
  }
  for(c = 0;c < b / 4;c++) {
    a.a.data[c * 4 + 3] = 255
  }
  a.c.putImageData(a.a, 0, 0);
  a.a = null
};
_SDL_Flip = l();
_SDL_Init = function() {
  SDL_SURFACES = {};
  return 1
};
_SDL_SetVideoMode = function(a, b, c, d, e) {
  c = _malloc(14);
  SDL_SURFACES[c] = {width:a, height:b, canvas:e, c:e.getContext("2d"), g:c, b:_malloc(a * b * 4)};
  return c
};
_SDL_Delay = l();
_SDL_Quit = function() {
  return 1
};
function B() {
  C(1, 65535)
}
FUNCTION_TABLE[0] = B;
FUNCTION_TABLE[1] = function(a, b) {
  return b
};
FUNCTION_TABLE[2] = l();
function E(a, b) {
  return FHEAP[a] < FHEAP[b] ? b : a
}
FUNCTION_TABLE[3] = E;
function F(a, b) {
  return IHEAP[a] < IHEAP[b] ? b : a
}
FUNCTION_TABLE[4] = F;
var ma = 5;
FUNCTION_TABLE[5] = function() {
  __ZNSt8ios_base4InitD1Ev(__ZStL8__ioinit)
};
function G(a, b, c, d) {
  FHEAP[a] = b;
  FHEAP[a + 1] = c;
  FHEAP[a + 2] = d
}
FUNCTION_TABLE[6] = G;
function H(a, b, c) {
  G(a, FHEAP[b] + FHEAP[c], FHEAP[b + 1] + FHEAP[c + 1], FHEAP[b + 2] + FHEAP[c + 2])
}
FUNCTION_TABLE[7] = H;
function I(a, b, c) {
  G(a, FHEAP[b] - FHEAP[c], FHEAP[b + 1] - FHEAP[c + 1], FHEAP[b + 2] - FHEAP[c + 2])
}
FUNCTION_TABLE[8] = I;
function J(a, b) {
  G(a, 0 - FHEAP[b], 0 - FHEAP[b + 1], 0 - FHEAP[b + 2])
}
FUNCTION_TABLE[9] = J;
function K(a, b, c) {
  G(a, FHEAP[b] * c, FHEAP[b + 1] * c, FHEAP[b + 2] * c)
}
FUNCTION_TABLE[10] = K;
function L(a, b, c) {
  G(a, FHEAP[b + 1] * FHEAP[c + 2] - FHEAP[b + 2] * FHEAP[c + 1], FHEAP[b + 2] * FHEAP[c] - FHEAP[b] * FHEAP[c + 2], FHEAP[b] * FHEAP[c + 1] - FHEAP[b + 1] * FHEAP[c])
}
FUNCTION_TABLE[11] = L;
function M(a, b) {
  return FHEAP[a] * FHEAP[b] + FHEAP[a + 1] * FHEAP[b + 1] + FHEAP[a + 2] * FHEAP[b + 2]
}
FUNCTION_TABLE[12] = M;
function P(a, b) {
  FHEAP[a] = FHEAP[b];
  FHEAP[a + 1] = FHEAP[b + 1];
  FHEAP[a + 2] = FHEAP[b + 2]
}
FUNCTION_TABLE[13] = P;
function Q(a, b, c) {
  FHEAP[a] = FHEAP[b];
  FHEAP[a + 1] = FHEAP[b + 1];
  FHEAP[a + 2] = FHEAP[b + 2];
  a = a + 3;
  FHEAP[a] = FHEAP[c];
  FHEAP[a + 1] = FHEAP[c + 1];
  FHEAP[a + 2] = FHEAP[c + 2]
}
FUNCTION_TABLE[14] = Q;
function R(a) {
  G(a, 0, 0, 0);
  FHEAP[a + 3] = Infinity
}
FUNCTION_TABLE[15] = R;
FUNCTION_TABLE[16] = l();
function S(a, b, c) {
  FHEAP[a] = FHEAP[b];
  FHEAP[a + 1] = FHEAP[b + 1];
  FHEAP[a + 2] = FHEAP[b + 2];
  FHEAP[a + 3] = c
}
FUNCTION_TABLE[17] = S;
function T(a, b, c) {
  var d = STACKTOP;
  STACKTOP += 3;
  var e = 1 / FHEAP[b + 3];
  I(d, c, b);
  K(a, d, e);
  STACKTOP = d
}
FUNCTION_TABLE[18] = T;
FUNCTION_TABLE[19] = l();
function U(a, b, c, d) {
  FHEAP[a] = FHEAP[b];
  FHEAP[a + 1] = FHEAP[b + 1];
  FHEAP[a + 2] = FHEAP[b + 2];
  FHEAP[a + 3] = FHEAP[b + 3];
  b = a + 4;
  FHEAP[b] = FHEAP[c];
  FHEAP[b + 1] = FHEAP[c + 1];
  FHEAP[b + 2] = FHEAP[c + 2];
  FHEAP[b + 3] = FHEAP[c + 3];
  IHEAP[a + 8] = d
}
FUNCTION_TABLE[20] = U;
function V(a, b) {
  var c = STACKTOP;
  STACKTOP += 12;
  var d = c + 3, e = c + 6, h = c + 9;
  W(d, b);
  if((FHEAP[d] * FHEAP[d] != 1 != 0 & FHEAP[d + 1] * FHEAP[d + 1] != 1 != 0 & FHEAP[d + 2] * FHEAP[d + 2] != 1) != 0) {
    var g = a + 3;
    FHEAP[g] = FHEAP[d];
    FHEAP[g + 1] = FHEAP[d + 1];
    FHEAP[g + 2] = FHEAP[d + 2];
    if(FHEAP[d + 1] * FHEAP[d + 1] > FHEAP[d] * FHEAP[d]) {
      if(FHEAP[d + 1] * FHEAP[d + 1] > FHEAP[d + 2] * FHEAP[d + 2]) {
        FHEAP[a + 3 + 1] = 0 - FHEAP[a + 3 + 1]
      }else {
        FHEAP[a + 3 + 2] = 0 - FHEAP[a + 3 + 2]
      }
    }else {
      if(FHEAP[d + 2] * FHEAP[d + 2] > FHEAP[d] * FHEAP[d]) {
        FHEAP[a + 3 + 2] = 0 - FHEAP[a + 3 + 2]
      }else {
        FHEAP[a + 3] = 0 - FHEAP[a + 3]
      }
    }
  }else {
    G(c, FHEAP[d + 2], FHEAP[d], FHEAP[d + 1]);
    g = a + 3;
    FHEAP[g] = FHEAP[c];
    FHEAP[g + 1] = FHEAP[c + 1];
    FHEAP[g + 2] = FHEAP[c + 2]
  }
  FHEAP[a] = FHEAP[d];
  FHEAP[a + 1] = FHEAP[d + 1];
  FHEAP[a + 2] = FHEAP[d + 2];
  d = a + 6;
  L(e, a, a + 3);
  FHEAP[d] = FHEAP[e];
  FHEAP[d + 1] = FHEAP[e + 1];
  FHEAP[d + 2] = FHEAP[e + 2];
  e = a + 3;
  L(h, a, a + 6);
  FHEAP[e] = FHEAP[h];
  FHEAP[e + 1] = FHEAP[h + 1];
  FHEAP[e + 2] = FHEAP[h + 2];
  STACKTOP = c
}
FUNCTION_TABLE[21] = V;
function X(a) {
  return M(a, a)
}
FUNCTION_TABLE[22] = X;
function W(a, b) {
  var c = 1 / _sqrt(X(b));
  K(a, b, c)
}
FUNCTION_TABLE[23] = W;
function C(a, b) {
  var c = STACKTOP;
  STACKTOP += 3;
  var d = a == 1;
  a:do {
    if(d) {
      if(b != 65535) {
        break a
      }
      __ZNSt8ios_base4InitC1Ev(__ZStL8__ioinit);
      ___cxa_atexit(ma, 0, la);
      G(c, -0.5, -0.65, 0.9);
      W(__ZL5light, c)
    }
  }while(0);
  STACKTOP = c
}
FUNCTION_TABLE[24] = C;
function Y(a, b) {
  var c = STACKTOP;
  STACKTOP += 3;
  var d, e, h;
  I(c, a, b);
  d = M(b + 3, c);
  e = d * d - X(c) + FHEAP[a + 3] * FHEAP[a + 3];
  if(e < 0) {
    d = Infinity
  }else {
    h = _sqrt(e);
    e = d + h;
    d = d - h;
    if(e < 0) {
      d = Infinity
    }else {
      d = d = d > 0 ? d : e
    }
  }
  d = d;
  STACKTOP = c;
  return d
}
FUNCTION_TABLE[25] = Y;
function Z(a, b) {
  var c = STACKTOP;
  STACKTOP += 9;
  var d = c + 3, e, h, g = c + 6;
  h = IHEAP[__ZL4pool];
  a:for(;;) {
    if(!(h < IHEAP[__ZL3end])) {
      break a
    }
    e = Y(h, a) >= FHEAP[b + 3];
    if(e != 0) {
      h += 9 * IHEAP[h + 8]
    }else {
      e = Y(h + 4, a);
      if(FHEAP[b + 3] > e) {
        FHEAP[b + 3] = e;
        K(c, a + 3, e);
        H(d, a, c);
        e = b;
        T(g, h + 4, d);
        FHEAP[e] = FHEAP[g];
        FHEAP[e + 1] = FHEAP[g + 1];
        FHEAP[e + 2] = FHEAP[g + 2]
      }
      h += 9
    }
  }
  STACKTOP = c
}
FUNCTION_TABLE[26] = Z;
function na(a, b) {
  var c, d, e, h;
  e = IHEAP[__ZL4pool];
  a:for(;;) {
    if(!(e < IHEAP[__ZL3end])) {
      c = 7;
      break a
    }
    d = Y(e, a) >= FHEAP[b + 3];
    if(d != 0) {
      e += 9 * IHEAP[e + 8]
    }else {
      h = Y(e + 4, a);
      if(FHEAP[b + 3] > h) {
        c = 4;
        break a
      }
      e += 9
    }
  }
  if(c == 4) {
    FHEAP[b + 3] = h
  }
}
FUNCTION_TABLE[27] = na;
function oa(a, b) {
  var c = STACKTOP;
  STACKTOP += 29;
  var d;
  d = c + 3;
  var e = c + 6, h = c + 9, g = c + 12, i = c + 15, j, k = c + 19, n = c + 25;
  R(i);
  Z(b, i);
  j = FHEAP[i + 3] != Infinity ? 0 - M(i, __ZL5light) : 0;
  if(j <= 0) {
    d = 0
  }else {
    J(g, __ZL5light);
    K(e, i, 1.0E-12);
    K(c, b + 3, FHEAP[i + 3]);
    H(d, b, c);
    H(h, d, e);
    Q(k, h, g);
    R(n);
    na(k, n);
    d = d = FHEAP[n + 3] == Infinity ? j : 0
  }
  d = d;
  STACKTOP = c;
  return d
}
FUNCTION_TABLE[28] = oa;
function $(a, b, c, d, e, h) {
  var g = STACKTOP;
  STACKTOP += 67;
  var i = g + 3, j = g + 6, k = g + 9, n = g + 12, s = g + 15, ca = g + 18, y = g + 21, v = g + 24, da = g + 27, ea = g + 30, fa = g + 33, ga = g + 36, ha = g + 39, t = g + 42, p = g + 43, D, z = g + 44, w = g + 48, A = g + 52, N = g + 61, O = g + 64;
  a = a;
  c = c;
  S(w, d, h * 2);
  S(z, d, h);
  D = a;
  if(D != 0) {
    a = b > 1 ? c : 1;
    U(D, w, z, a)
  }
  a = D + 9;
  if(!(b <= 1)) {
    IHEAP[p] = 1;
    IHEAP[t] = Math.floor((c - 9) / 9);
    c = IHEAP[F(t, p)];
    V(A, e);
    t = h / 3;
    z = p = 0;
    a:for(;;) {
      if(!(z <= 5)) {
        break a
      }
      w = _cos(p);
      K(ga, A + 6, w);
      w = _sin(p);
      K(ea, A + 3, w);
      K(da, e, -0.2);
      H(fa, da, ea);
      H(ha, fa, ga);
      W(N, ha);
      K(y, N, h + t);
      H(v, d, y);
      a = $(a, b - 1, c, v, N, t);
      p += 1.0471975511965976;
      z += 1
    }
    p -= 0.3490658503988659;
    y = 0;
    a:for(;;) {
      if(!(y <= 2)) {
        break a
      }
      v = _cos(p);
      K(s, A + 6, v);
      v = _sin(p);
      K(k, A + 3, v);
      K(j, e, 0.6);
      H(n, j, k);
      H(ca, n, s);
      W(O, ca);
      K(g, O, h + t);
      H(i, d, g);
      a = $(a, b - 1, c, i, O, t);
      p += 2.0943951023931953;
      y += 1
    }
  }
  b = a;
  STACKTOP = g;
  return b
}
FUNCTION_TABLE[29] = $;
function pa(a, b, c) {
  var d = STACKTOP;
  STACKTOP += 35;
  var e = d + 3, h;
  h = d + 6;
  var g = d + 9, i = d + 10, j = d + 11, k = d + 17, n = d + 29, s = d + 32;
  FHEAP[g] = a;
  FHEAP[i] = b;
  G(h, 0, 0, -4.5);
  P(j, h);
  h = k;
  b = 3;
  a:for(;;) {
    if(b == -1) {
      break a
    }
    h += 3;
    b -= 1
  }
  b = 0;
  a:for(;;) {
    if(!(b <= 3)) {
      break a
    }
    h = b;
    G(e, FHEAP[__ZL4grid + b * 2] * 0.5 + FHEAP[g] / -2, FHEAP[__ZL4grid + b * 2 + 1] * 0.5 + FHEAP[i] / -2, 0);
    h = k + h * 3;
    FHEAP[h] = FHEAP[e];
    FHEAP[h + 1] = FHEAP[e + 1];
    FHEAP[h + 2] = FHEAP[e + 2];
    b += 1
  }
  G(n, 0, c, FHEAP[E(g, i)]);
  _SDL_LockSurface(IHEAP[_screen]);
  c = a;
  a:for(;;) {
    if(c == 0) {
      break a
    }
    g = e = 0;
    b:for(;;) {
      if(!(g <= 3)) {
        break b
      }
      H(d, n, k + g * 3);
      i = j + 3;
      W(s, d);
      FHEAP[i] = FHEAP[s];
      FHEAP[i + 1] = FHEAP[s + 1];
      FHEAP[i + 2] = FHEAP[s + 2];
      e = oa(IHEAP[__ZL4pool], j) + e;
      g += 1
    }
    g = 0;
    b:for(;;) {
      if(!(g <= 2)) {
        break b
      }
      IHEAP[IHEAP[IHEAP[_screen] + ka[5]] + Math.floor((a - FHEAP[n + 1]) * a * 4 + FHEAP[n] * 4) + g] = Math.floor(e * 64);
      g += 1
    }
    FHEAP[n] += 1;
    c -= 1
  }
  _SDL_UnlockSurface(IHEAP[_screen]);
  STACKTOP = d
}
FUNCTION_TABLE[30] = pa;
function qa(a, b) {
  var c = STACKTOP;
  STACKTOP += 11;
  var d = c + 3, e = c + 6, h, g, i, j = c + 9, k = c + 10;
  if(a == 2) {
    IHEAP[k] = 2;
    g = Math.floor(Number(ba(IHEAP[b + 1])));
    IHEAP[j] = g;
    j = IHEAP[F(j, k)]
  }else {
    j = 6
  }
  j = j;
  k = 9;
  i = j;
  a:for(;;) {
    i -= 1;
    g = i > 1;
    if(g == 0) {
      break a
    }
    k = (k + 1) * 9
  }
  k += 1;
  h = k;
  g = i = __Znaj(h * 68);
  h = h - 1;
  a:for(;;) {
    if(h == -1) {
      break a
    }
    g += 9;
    h -= 1
  }
  IHEAP[__ZL4pool] = i;
  IHEAP[__ZL3end] = IHEAP[__ZL4pool] + 9 * k;
  G(d, 0.25, 1, -0.5);
  W(e, d);
  G(c, 0, 0, 0);
  $(IHEAP[__ZL4pool], j, k, c, e, 1);
  return; // XXX
  _SDL_Init(32);
  d = _SDL_SetVideoMode(512, 512, 32, 0);
  IHEAP[_screen] = d;
  d = 511;
  a:for(;;) {
    if(!(d >= 0)) {
      break a
    }
    pa(512, 512, d);
    d -= 1
  }
  STACKTOP = c;
  return 0
}
FUNCTION_TABLE[31] = qa;
function ra(a) {
  function b() {
    for(var h = 0;h < 0;h++) {
      d.push(0)
    }
  }
  ia();
  var c = a.length + 1, d = [r(x("/bin/this.program"), null)];
  b();
  for(var e = 0;e < c - 1;e += 1) {
    d.push(r(x(a[e]), null));
    b()
  }
  d = r(d, null);
  m();
  for(qa(c, d);o.length > 0;) {
    a = o.pop();
    if(typeof a === "number") {
      a = FUNCTION_TABLE[a]
    }
    a()
  }
}

run = ra;
__ZL10trace_lineiii = pa;

