// EMSCRIPTEN_START_ASM
function instantiate(H, I, J) {
  var a = 12;
  function b() {
    return a;
  }
  function D(E, F, G) {
    var c = F.memory;
    var d = J;
    var e = new E.Int8Array(G);
    var f = new E.Int16Array(G);
    var g = new E.Int32Array(G);
    var h = new E.Uint8Array(G);
    var i = new E.Uint16Array(G);
    var j = new E.Uint32Array(G);
    var k = new E.Float32Array(G);
    var l = new E.Float64Array(G);
    var m = E.Math.imul;
    var n = E.Math.fround;
    var o = E.Math.abs;
    var p = E.Math.clz32;
    var q = E.Math.min;
    var r = E.Math.max;
    var s = E.Math.floor;
    var t = E.Math.ceil;
    var u = E.Math.sqrt;
    var v = F.abort;
    var w = E.NaN;
    var x = E.Infinity;
    var y = F.fd_write;
    var z = F.emscripten_memcpy_js;
    var A = 5245632;
    var B = 0;
    
// EMSCRIPTEN_START_FUNCS
function N(a, b, c) {
  var d = 0, f = 0;
  if (c >>> 0 >= 512) {
    z(a | 0, b | 0, c | 0) | 0;
    return;
  }
  d = a + c | 0;
  a: {
    if (!((a ^ b) & 3)) {
      b: {
        if ((c | 0) < 1) {
          c = a;
          break b;
        }
        if (!(a & 3)) {
          c = a;
          break b;
        }
        c = a;
        while (1) {
          e[c | 0] = h[b | 0];
          b = b + 1 | 0;
          c = c + 1 | 0;
          if (c >>> 0 >= d >>> 0) {
            break b;
          }
          if (c & 3) {
            continue;
          }
          break;
        }
      }
      a = d & -4;
      c: {
        if (a >>> 0 < 64) {
          break c;
        }
        f = a + -64 | 0;
        if (c >>> 0 > f >>> 0) {
          break c;
        }
        while (1) {
          g[c >> 2] = g[b >> 2];
          g[c + 4 >> 2] = g[b + 4 >> 2];
          g[c + 8 >> 2] = g[b + 8 >> 2];
          g[c + 12 >> 2] = g[b + 12 >> 2];
          g[c + 16 >> 2] = g[b + 16 >> 2];
          g[c + 20 >> 2] = g[b + 20 >> 2];
          g[c + 24 >> 2] = g[b + 24 >> 2];
          g[c + 28 >> 2] = g[b + 28 >> 2];
          g[c + 32 >> 2] = g[b + 32 >> 2];
          g[c + 36 >> 2] = g[b + 36 >> 2];
          g[c + 40 >> 2] = g[b + 40 >> 2];
          g[c + 44 >> 2] = g[b + 44 >> 2];
          g[c + 48 >> 2] = g[b + 48 >> 2];
          g[c + 52 >> 2] = g[b + 52 >> 2];
          g[c + 56 >> 2] = g[b + 56 >> 2];
          g[c + 60 >> 2] = g[b + 60 >> 2];
          b = b - -64 | 0;
          c = c - -64 | 0;
          if (c >>> 0 <= f >>> 0) {
            continue;
          }
          break;
        }
      }
      if (c >>> 0 >= a >>> 0) {
        break a;
      }
      while (1) {
        g[c >> 2] = g[b >> 2];
        b = b + 4 | 0;
        c = c + 4 | 0;
        if (c >>> 0 < a >>> 0) {
          continue;
        }
        break;
      }
      break a;
    }
    if (d >>> 0 < 4) {
      c = a;
      break a;
    }
    f = d + -4 | 0;
    if (f >>> 0 < a >>> 0) {
      c = a;
      break a;
    }
    c = a;
    while (1) {
      e[c | 0] = h[b | 0];
      e[c + 1 | 0] = h[b + 1 | 0];
      e[c + 2 | 0] = h[b + 2 | 0];
      e[c + 3 | 0] = h[b + 3 | 0];
      b = b + 4 | 0;
      c = c + 4 | 0;
      if (c >>> 0 <= f >>> 0) {
        continue;
      }
      break;
    }
  }
  if (c >>> 0 < d >>> 0) {
    while (1) {
      e[c | 0] = h[b | 0];
      b = b + 1 | 0;
      c = c + 1 | 0;
      if ((d | 0) != (c | 0)) {
        continue;
      }
      break;
    }
  }
}

function Q(a, b, c) {
  a = a | 0;
  b = b | 0;
  c = c | 0;
  var d = 0, e = 0, f = 0, h = 0, i = 0, j = 0;
  d = A - 32 | 0;
  A = d;
  e = g[a + 28 >> 2];
  g[d + 16 >> 2] = e;
  f = g[a + 20 >> 2];
  g[d + 28 >> 2] = c;
  g[d + 24 >> 2] = b;
  b = f - e | 0;
  g[d + 20 >> 2] = b;
  e = b + c | 0;
  i = 2;
  b = d + 16 | 0;
  a: {
    b: {
      c: {
        if (!M(y(g[a + 60 >> 2], d + 16 | 0, 2, d + 12 | 0) | 0)) {
          while (1) {
            f = g[d + 12 >> 2];
            if ((f | 0) == (e | 0)) {
              break c;
            }
            if ((f | 0) <= -1) {
              break b;
            }
            h = g[b + 4 >> 2];
            j = f >>> 0 > h >>> 0;
            b = j ? b + 8 | 0 : b;
            h = f - (j ? h : 0) | 0;
            g[b >> 2] = h + g[b >> 2];
            g[b + 4 >> 2] = g[b + 4 >> 2] - h;
            e = e - f | 0;
            i = i - j | 0;
            if (!M(y(g[a + 60 >> 2], b | 0, i | 0, d + 12 | 0) | 0)) {
              continue;
            }
            break;
          }
        }
        g[d + 12 >> 2] = -1;
        if ((e | 0) != -1) {
          break b;
        }
      }
      b = g[a + 44 >> 2];
      g[a + 28 >> 2] = b;
      g[a + 20 >> 2] = b;
      g[a + 16 >> 2] = b + g[a + 48 >> 2];
      a = c;
      break a;
    }
    g[a + 28 >> 2] = 0;
    g[a + 16 >> 2] = 0;
    g[a + 20 >> 2] = 0;
    g[a >> 2] = g[a >> 2] | 32;
    a = 0;
    if ((i | 0) == 2) {
      break a;
    }
    a = c - g[b + 4 >> 2] | 0;
  }
  A = d + 32 | 0;
  return a | 0;
}

function K(a, c) {
  var f = 0, i = 0, j = 0, k = 0, l = 0;
  b();
  k = 1024;
  f = g[c + 16 >> 2];
  a: {
    if (!f) {
      if (L(c)) {
        break a;
      }
      f = g[c + 16 >> 2];
    }
    j = g[c + 20 >> 2];
    if (f - j >>> 0 < a >>> 0) {
      return d[g[c + 36 >> 2]](c, 1024, a) | 0;
    }
    b: {
      if (e[c + 75 | 0] < 0) {
        break b;
      }
      i = a;
      while (1) {
        f = i;
        if (!f) {
          break b;
        }
        i = f + -1 | 0;
        if (h[i + 1024 | 0] != 10) {
          continue;
        }
        break;
      }
      i = d[g[c + 36 >> 2]](c, 1024, f) | 0;
      if (i >>> 0 < f >>> 0) {
        break a;
      }
      a = a - f | 0;
      k = f + 1024 | 0;
      j = g[c + 20 >> 2];
      l = f;
    }
    N(j, k, a);
    g[c + 20 >> 2] = g[c + 20 >> 2] + a;
    i = a + l | 0;
  }
  return i;
}

function U(a) {
  var b = 0, c = 0, f = 0;
  b = A - 16 | 0;
  A = b;
  e[b + 15 | 0] = 10;
  c = g[a + 16 >> 2];
  a: {
    if (!c) {
      if (L(a)) {
        break a;
      }
      c = g[a + 16 >> 2];
    }
    f = g[a + 20 >> 2];
    if (!(e[a + 75 | 0] == 10 | f >>> 0 >= c >>> 0)) {
      g[a + 20 >> 2] = f + 1;
      e[f | 0] = 10;
      break a;
    }
    if ((d[g[a + 36 >> 2]](a, b + 15 | 0, 1) | 0) != 1) {
      break a;
    }
  }
  A = b + 16 | 0;
}

function S() {
  var a = 0, b = 0, c = 0;
  b = 1024;
  while (1) {
    a = b;
    b = a + 4 | 0;
    c = g[a >> 2];
    if (!((c ^ -1) & c + -16843009 & -2139062144)) {
      continue;
    }
    break;
  }
  if (!(c & 255)) {
    return a - 1024 | 0;
  }
  while (1) {
    c = h[a + 1 | 0];
    b = a + 1 | 0;
    a = b;
    if (c) {
      continue;
    }
    break;
  }
  return b - 1024 | 0;
}

function T() {
  var a = 0, b = 0;
  a = g[260];
  a;
  b = S();
  a: {
    if ((((V(b, a) | 0) != (b | 0) ? -1 : 0) | 0) < 0) {
      break a;
    }
    b: {
      if (h[a + 75 | 0] == 10) {
        break b;
      }
      b = g[a + 20 >> 2];
      if (b >>> 0 >= j[a + 16 >> 2]) {
        break b;
      }
      g[a + 20 >> 2] = b + 1;
      e[b | 0] = 10;
      break a;
    }
    U(a);
  }
}

function L(a) {
  var b = 0;
  b = h[a + 74 | 0];
  e[a + 74 | 0] = b + -1 | b;
  b = g[a >> 2];
  if (b & 8) {
    g[a >> 2] = b | 32;
    return -1;
  }
  g[a + 4 >> 2] = 0;
  g[a + 8 >> 2] = 0;
  b = g[a + 44 >> 2];
  g[a + 28 >> 2] = b;
  g[a + 20 >> 2] = b;
  g[a + 16 >> 2] = b + g[a + 48 >> 2];
  return 0;
}

function V(a, b) {
  var c = 0, d = 0;
  c = a;
  d = c;
  a: {
    if (g[b + 76 >> 2] <= -1) {
      b = K(c, b);
      break a;
    }
    b = K(c, b);
  }
  if ((d | 0) == (b | 0)) {
    return a;
  }
  return b;
}

function O(a, b, c, d) {
  a = a | 0;
  b = b | 0;
  c = c | 0;
  d = d | 0;
  B = 0;
  return 0;
}

function M(a) {
  if (!a) {
    return 0;
  }
  g[300] = a;
  return -1;
}

function W(a, b) {
  a = a | 0;
  b = b | 0;
  T();
  return 0;
}

function R(a) {
  a = a | 0;
  return v() | 0;
}

function P(a) {
  a = a | 0;
  return 0;
}

function X() {}

// EMSCRIPTEN_END_FUNCS

    d[1] = P;
    d[2] = Q;
    d[3] = O;
    function C() {
      return G.byteLength / 65536 | 0;
    }
    return {
      __wasm_call_ctors: X,
      main: W,
      __growWasmMemory: R
    };
  }
  return D({
    Int8Array,
    Int16Array,
    Int32Array,
    Uint8Array,
    Uint16Array,
    Uint32Array,
    Float32Array,
    Float64Array,
    NaN,
    Infinity: Infinity,
    Math
  }, H, I.buffer);
}
// EMSCRIPTEN_END_ASM



