// EMSCRIPTEN_START_ASM
function instantiate(S, T, U) {
 var a = 12;
 function P() {
  return a;
 }
 function Q(V, W, X) {
  var b = W.memory;
  var c = U;
  var d = new V.Int8Array(X);
  var e = new V.Int16Array(X);
  var f = new V.Int32Array(X);
  var g = new V.Uint8Array(X);
  var h = new V.Uint16Array(X);
  var i = new V.Uint32Array(X);
  var j = new V.Float32Array(X);
  var k = new V.Float64Array(X);
  var l = V.Math.imul;
  var m = V.Math.fround;
  var n = V.Math.abs;
  var o = V.Math.clz32;
  var p = V.Math.min;
  var q = V.Math.max;
  var r = V.Math.floor;
  var s = V.Math.ceil;
  var t = V.Math.sqrt;
  var u = W.abort;
  var v = V.NaN;
  var w = V.Infinity;
  var x = W.fd_write;
  var y = W.emscripten_memcpy_big;
  var z = 5245632;
  var A = 0;
  
// EMSCRIPTEN_START_FUNCS
function E(a, b, c) {
 var e = 0, h = 0;
 if (c >>> 0 >= 512) {
  y(a | 0, b | 0, c | 0) | 0;
  return;
 }
 e = a + c | 0;
 a : {
  if (!((a ^ b) & 3)) {
   b : {
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
     d[c | 0] = g[b | 0];
     b = b + 1 | 0;
     c = c + 1 | 0;
     if (c >>> 0 >= e >>> 0) {
      break b;
     }
     if (c & 3) {
      continue;
     }
     break;
    }
   }
   a = e & -4;
   c : {
    if (a >>> 0 < 64) {
     break c;
    }
    h = a + -64 | 0;
    if (c >>> 0 > h >>> 0) {
     break c;
    }
    while (1) {
     f[c >> 2] = f[b >> 2];
     f[c + 4 >> 2] = f[b + 4 >> 2];
     f[c + 8 >> 2] = f[b + 8 >> 2];
     f[c + 12 >> 2] = f[b + 12 >> 2];
     f[c + 16 >> 2] = f[b + 16 >> 2];
     f[c + 20 >> 2] = f[b + 20 >> 2];
     f[c + 24 >> 2] = f[b + 24 >> 2];
     f[c + 28 >> 2] = f[b + 28 >> 2];
     f[c + 32 >> 2] = f[b + 32 >> 2];
     f[c + 36 >> 2] = f[b + 36 >> 2];
     f[c + 40 >> 2] = f[b + 40 >> 2];
     f[c + 44 >> 2] = f[b + 44 >> 2];
     f[c + 48 >> 2] = f[b + 48 >> 2];
     f[c + 52 >> 2] = f[b + 52 >> 2];
     f[c + 56 >> 2] = f[b + 56 >> 2];
     f[c + 60 >> 2] = f[b + 60 >> 2];
     b = b - -64 | 0;
     c = c - -64 | 0;
     if (c >>> 0 <= h >>> 0) {
      continue;
     }
     break;
    }
   }
   if (c >>> 0 >= a >>> 0) {
    break a;
   }
   while (1) {
    f[c >> 2] = f[b >> 2];
    b = b + 4 | 0;
    c = c + 4 | 0;
    if (c >>> 0 < a >>> 0) {
     continue;
    }
    break;
   }
   break a;
  }
  if (e >>> 0 < 4) {
   c = a;
   break a;
  }
  h = e + -4 | 0;
  if (h >>> 0 < a >>> 0) {
   c = a;
   break a;
  }
  c = a;
  while (1) {
   d[c | 0] = g[b | 0];
   d[c + 1 | 0] = g[b + 1 | 0];
   d[c + 2 | 0] = g[b + 2 | 0];
   d[c + 3 | 0] = g[b + 3 | 0];
   b = b + 4 | 0;
   c = c + 4 | 0;
   if (c >>> 0 <= h >>> 0) {
    continue;
   }
   break;
  }
 }
 if (c >>> 0 < e >>> 0) {
  while (1) {
   d[c | 0] = g[b | 0];
   b = b + 1 | 0;
   c = c + 1 | 0;
   if ((e | 0) != (c | 0)) {
    continue;
   }
   break;
  }
 }
}
function H(a, b, c) {
 a = a | 0;
 b = b | 0;
 c = c | 0;
 var d = 0, e = 0, g = 0, h = 0, i = 0, j = 0;
 d = z - 32 | 0;
 z = d;
 e = f[a + 28 >> 2];
 f[d + 16 >> 2] = e;
 g = f[a + 20 >> 2];
 f[d + 28 >> 2] = c;
 f[d + 24 >> 2] = b;
 b = g - e | 0;
 f[d + 20 >> 2] = b;
 e = b + c | 0;
 i = 2;
 b = d + 16 | 0;
 a : {
  b : {
   c : {
    if (!D(x(f[a + 60 >> 2], d + 16 | 0, 2, d + 12 | 0) | 0)) {
     while (1) {
      g = f[d + 12 >> 2];
      if ((g | 0) == (e | 0)) {
       break c;
      }
      if ((g | 0) <= -1) {
       break b;
      }
      h = f[b + 4 >> 2];
      j = g >>> 0 > h >>> 0;
      b = j ? b + 8 | 0 : b;
      h = g - (j ? h : 0) | 0;
      f[b >> 2] = h + f[b >> 2];
      f[b + 4 >> 2] = f[b + 4 >> 2] - h;
      e = e - g | 0;
      i = i - j | 0;
      if (!D(x(f[a + 60 >> 2], b | 0, i | 0, d + 12 | 0) | 0)) {
       continue;
      }
      break;
     }
    }
    f[d + 12 >> 2] = -1;
    if ((e | 0) != -1) {
     break b;
    }
   }
   b = f[a + 44 >> 2];
   f[a + 28 >> 2] = b;
   f[a + 20 >> 2] = b;
   f[a + 16 >> 2] = b + f[a + 48 >> 2];
   a = c;
   break a;
  }
  f[a + 28 >> 2] = 0;
  f[a + 16 >> 2] = 0;
  f[a + 20 >> 2] = 0;
  f[a >> 2] = f[a >> 2] | 32;
  a = 0;
  if ((i | 0) == 2) {
   break a;
  }
  a = c - f[b + 4 >> 2] | 0;
 }
 z = d + 32 | 0;
 return a | 0;
}
function B(a, b) {
 var e = 0, h = 0, i = 0, j = 0, k = 0;
 P();
 j = 1024;
 e = f[b + 16 >> 2];
 a : {
  if (!e) {
   if (C(b)) {
    break a;
   }
   e = f[b + 16 >> 2];
  }
  i = f[b + 20 >> 2];
  if (e - i >>> 0 < a >>> 0) {
   return c[f[b + 36 >> 2]](b, 1024, a) | 0;
  }
  b : {
   if (d[b + 75 | 0] < 0) {
    break b;
   }
   h = a;
   while (1) {
    e = h;
    if (!e) {
     break b;
    }
    h = e + -1 | 0;
    if (g[h + 1024 | 0] != 10) {
     continue;
    }
    break;
   }
   h = c[f[b + 36 >> 2]](b, 1024, e) | 0;
   if (h >>> 0 < e >>> 0) {
    break a;
   }
   a = a - e | 0;
   j = e + 1024 | 0;
   i = f[b + 20 >> 2];
   k = e;
  }
  E(i, j, a);
  f[b + 20 >> 2] = f[b + 20 >> 2] + a;
  h = a + k | 0;
 }
 return h;
}
function L(a) {
 var b = 0, e = 0, g = 0;
 b = z - 16 | 0;
 z = b;
 d[b + 15 | 0] = 10;
 e = f[a + 16 >> 2];
 a : {
  if (!e) {
   if (C(a)) {
    break a;
   }
   e = f[a + 16 >> 2];
  }
  g = f[a + 20 >> 2];
  if (!(d[a + 75 | 0] == 10 | g >>> 0 >= e >>> 0)) {
   f[a + 20 >> 2] = g + 1;
   d[g | 0] = 10;
   break a;
  }
  if ((c[f[a + 36 >> 2]](a, b + 15 | 0, 1) | 0) != 1) {
   break a;
  }
 }
 z = b + 16 | 0;
}
function J() {
 var a = 0, b = 0, c = 0;
 b = 1024;
 while (1) {
  a = b;
  b = a + 4 | 0;
  c = f[a >> 2];
  if (!((c ^ -1) & c + -16843009 & -2139062144)) {
   continue;
  }
  break;
 }
 if (!(c & 255)) {
  return a - 1024 | 0;
 }
 while (1) {
  c = g[a + 1 | 0];
  b = a + 1 | 0;
  a = b;
  if (c) {
   continue;
  }
  break;
 }
 return b - 1024 | 0;
}
function K() {
 var a = 0, b = 0;
 a = f[260];
 a;
 b = J();
 a : {
  if ((((M(b, a) | 0) != (b | 0) ? -1 : 0) | 0) < 0) {
   break a;
  }
  b : {
   if (g[a + 75 | 0] == 10) {
    break b;
   }
   b = f[a + 20 >> 2];
   if (b >>> 0 >= i[a + 16 >> 2]) {
    break b;
   }
   f[a + 20 >> 2] = b + 1;
   d[b | 0] = 10;
   break a;
  }
  L(a);
 }
}
function C(a) {
 var b = 0;
 b = g[a + 74 | 0];
 d[a + 74 | 0] = b + -1 | b;
 b = f[a >> 2];
 if (b & 8) {
  f[a >> 2] = b | 32;
  return -1;
 }
 f[a + 4 >> 2] = 0;
 f[a + 8 >> 2] = 0;
 b = f[a + 44 >> 2];
 f[a + 28 >> 2] = b;
 f[a + 20 >> 2] = b;
 f[a + 16 >> 2] = b + f[a + 48 >> 2];
 return 0;
}
function M(a, b) {
 var c = 0, d = 0;
 c = a;
 d = c;
 a : {
  if (f[b + 76 >> 2] <= -1) {
   b = B(c, b);
   break a;
  }
  b = B(c, b);
 }
 if ((d | 0) == (b | 0)) {
  return a;
 }
 return b;
}
function F(a, b, c, d) {
 a = a | 0;
 b = b | 0;
 c = c | 0;
 d = d | 0;
 A = 0;
 return 0;
}
function D(a) {
 if (!a) {
  return 0;
 }
 f[300] = a;
 return -1;
}
function N(a, b) {
 a = a | 0;
 b = b | 0;
 K();
 return 0;
}
function I(a) {
 a = a | 0;
 return u() | 0;
}
function G(a) {
 a = a | 0;
 return 0;
}
function O() {}




// EMSCRIPTEN_END_FUNCS

  c[1] = G;
  c[2] = H;
  c[3] = F;
  function R() {
   return X.byteLength / 65536 | 0;
  }
  return {
   "__wasm_call_ctors": O,
   "main": N,
   "__growWasmMemory": I
  };
 }
 return Q({
  "Int8Array": Int8Array,
  "Int16Array": Int16Array,
  "Int32Array": Int32Array,
  "Uint8Array": Uint8Array,
  "Uint16Array": Uint16Array,
  "Uint32Array": Uint32Array,
  "Float32Array": Float32Array,
  "Float64Array": Float64Array,
  "NaN": NaN,
  "Infinity": Infinity,
  "Math": Math
 }, S, T.buffer);
}


// EMSCRIPTEN_END_ASM



