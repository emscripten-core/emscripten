// EMSCRIPTEN_START_ASM
function instantiate(asmLibraryArg, wasmMemory, wasmTable) {
 function asmFunc(global, env, buffer) {
  var a = env.memory;
  var b = wasmTable;
  var c = new global.Int8Array(buffer);
  var d = new global.Int16Array(buffer);
  var e = new global.Int32Array(buffer);
  var f = new global.Uint8Array(buffer);
  var g = new global.Uint16Array(buffer);
  var h = new global.Uint32Array(buffer);
  var i = new global.Float32Array(buffer);
  var j = new global.Float64Array(buffer);
  var k = global.Math.imul;
  var l = global.Math.fround;
  var m = global.Math.abs;
  var n = global.Math.clz32;
  var o = global.Math.min;
  var p = global.Math.max;
  var q = global.Math.floor;
  var r = global.Math.ceil;
  var s = global.Math.sqrt;
  var t = env.abort;
  var u = global.NaN;
  var v = global.Infinity;
  var w = env.fd_write;
  var x = env.emscripten_memcpy_big;
  var y = 5245632;
  var z = 0;
  
// EMSCRIPTEN_START_FUNCS
function E(a, b, d) {
 var g = 0, h = 0;
 if (d >>> 0 >= 512) {
  x(a | 0, b | 0, d | 0) | 0;
  return;
 }
 g = a + d | 0;
 a : {
  if (!((a ^ b) & 3)) {
   b : {
    if ((d | 0) < 1) {
     d = a;
     break b;
    }
    if (!(a & 3)) {
     d = a;
     break b;
    }
    d = a;
    while (1) {
     c[d | 0] = f[b | 0];
     b = b + 1 | 0;
     d = d + 1 | 0;
     if (d >>> 0 >= g >>> 0) {
      break b;
     }
     if (d & 3) {
      continue;
     }
     break;
    }
   }
   a = g & -4;
   c : {
    if (a >>> 0 < 64) {
     break c;
    }
    h = a + -64 | 0;
    if (d >>> 0 > h >>> 0) {
     break c;
    }
    while (1) {
     e[d >> 2] = e[b >> 2];
     e[d + 4 >> 2] = e[b + 4 >> 2];
     e[d + 8 >> 2] = e[b + 8 >> 2];
     e[d + 12 >> 2] = e[b + 12 >> 2];
     e[d + 16 >> 2] = e[b + 16 >> 2];
     e[d + 20 >> 2] = e[b + 20 >> 2];
     e[d + 24 >> 2] = e[b + 24 >> 2];
     e[d + 28 >> 2] = e[b + 28 >> 2];
     e[d + 32 >> 2] = e[b + 32 >> 2];
     e[d + 36 >> 2] = e[b + 36 >> 2];
     e[d + 40 >> 2] = e[b + 40 >> 2];
     e[d + 44 >> 2] = e[b + 44 >> 2];
     e[d + 48 >> 2] = e[b + 48 >> 2];
     e[d + 52 >> 2] = e[b + 52 >> 2];
     e[d + 56 >> 2] = e[b + 56 >> 2];
     e[d + 60 >> 2] = e[b + 60 >> 2];
     b = b - -64 | 0;
     d = d - -64 | 0;
     if (d >>> 0 <= h >>> 0) {
      continue;
     }
     break;
    }
   }
   if (d >>> 0 >= a >>> 0) {
    break a;
   }
   while (1) {
    e[d >> 2] = e[b >> 2];
    b = b + 4 | 0;
    d = d + 4 | 0;
    if (d >>> 0 < a >>> 0) {
     continue;
    }
    break;
   }
   break a;
  }
  if (g >>> 0 < 4) {
   d = a;
   break a;
  }
  h = g + -4 | 0;
  if (h >>> 0 < a >>> 0) {
   d = a;
   break a;
  }
  d = a;
  while (1) {
   c[d | 0] = f[b | 0];
   c[d + 1 | 0] = f[b + 1 | 0];
   c[d + 2 | 0] = f[b + 2 | 0];
   c[d + 3 | 0] = f[b + 3 | 0];
   b = b + 4 | 0;
   d = d + 4 | 0;
   if (d >>> 0 <= h >>> 0) {
    continue;
   }
   break;
  }
 }
 if (d >>> 0 < g >>> 0) {
  while (1) {
   c[d | 0] = f[b | 0];
   b = b + 1 | 0;
   d = d + 1 | 0;
   if ((g | 0) != (d | 0)) {
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
 var d = 0, f = 0, g = 0, h = 0, i = 0, j = 0;
 d = y - 32 | 0;
 y = d;
 f = e[a + 28 >> 2];
 e[d + 16 >> 2] = f;
 g = e[a + 20 >> 2];
 e[d + 28 >> 2] = c;
 e[d + 24 >> 2] = b;
 b = g - f | 0;
 e[d + 20 >> 2] = b;
 f = b + c | 0;
 i = 2;
 b = d + 16 | 0;
 a : {
  b : {
   c : {
    if (!D(w(e[a + 60 >> 2], d + 16 | 0, 2, d + 12 | 0) | 0)) {
     while (1) {
      g = e[d + 12 >> 2];
      if ((g | 0) == (f | 0)) {
       break c;
      }
      if ((g | 0) <= -1) {
       break b;
      }
      h = e[b + 4 >> 2];
      j = g >>> 0 > h >>> 0;
      b = j ? b + 8 | 0 : b;
      h = g - (j ? h : 0) | 0;
      e[b >> 2] = h + e[b >> 2];
      e[b + 4 >> 2] = e[b + 4 >> 2] - h;
      f = f - g | 0;
      i = i - j | 0;
      if (!D(w(e[a + 60 >> 2], b | 0, i | 0, d + 12 | 0) | 0)) {
       continue;
      }
      break;
     }
    }
    e[d + 12 >> 2] = -1;
    if ((f | 0) != -1) {
     break b;
    }
   }
   b = e[a + 44 >> 2];
   e[a + 28 >> 2] = b;
   e[a + 20 >> 2] = b;
   e[a + 16 >> 2] = b + e[a + 48 >> 2];
   a = c;
   break a;
  }
  e[a + 28 >> 2] = 0;
  e[a + 16 >> 2] = 0;
  e[a + 20 >> 2] = 0;
  e[a >> 2] = e[a >> 2] | 32;
  a = 0;
  if ((i | 0) == 2) {
   break a;
  }
  a = c - e[b + 4 >> 2] | 0;
 }
 y = d + 32 | 0;
 return a | 0;
}
function B(a, d) {
 var g = 0, h = 0, i = 0, j = 0, k = 0;
 j = 1024;
 g = e[d + 16 >> 2];
 a : {
  if (!g) {
   if (C(d)) {
    break a;
   }
   g = e[d + 16 >> 2];
  }
  i = e[d + 20 >> 2];
  if (g - i >>> 0 < a >>> 0) {
   return b[e[d + 36 >> 2]](d, 1024, a) | 0;
  }
  b : {
   if (c[d + 75 | 0] < 0) {
    break b;
   }
   h = a;
   while (1) {
    g = h;
    if (!g) {
     break b;
    }
    h = g + -1 | 0;
    if (f[h + 1024 | 0] != 10) {
     continue;
    }
    break;
   }
   h = b[e[d + 36 >> 2]](d, 1024, g) | 0;
   if (h >>> 0 < g >>> 0) {
    break a;
   }
   a = a - g | 0;
   j = g + 1024 | 0;
   i = e[d + 20 >> 2];
   k = g;
  }
  E(i, j, a);
  e[d + 20 >> 2] = e[d + 20 >> 2] + a;
  h = a + k | 0;
 }
 return h;
}
function L(a) {
 var d = 0, f = 0, g = 0;
 d = y - 16 | 0;
 y = d;
 c[d + 15 | 0] = 10;
 f = e[a + 16 >> 2];
 a : {
  if (!f) {
   if (C(a)) {
    break a;
   }
   f = e[a + 16 >> 2];
  }
  g = e[a + 20 >> 2];
  if (!(c[a + 75 | 0] == 10 | g >>> 0 >= f >>> 0)) {
   e[a + 20 >> 2] = g + 1;
   c[g | 0] = 10;
   break a;
  }
  if ((b[e[a + 36 >> 2]](a, d + 15 | 0, 1) | 0) != 1) {
   break a;
  }
 }
 y = d + 16 | 0;
}
function J() {
 var a = 0, b = 0, c = 0;
 b = 1024;
 while (1) {
  a = b;
  b = a + 4 | 0;
  c = e[a >> 2];
  if (!((c ^ -1) & c + -16843009 & -2139062144)) {
   continue;
  }
  break;
 }
 if (!(c & 255)) {
  return a - 1024 | 0;
 }
 while (1) {
  c = f[a + 1 | 0];
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
 a = e[260];
 a;
 b = J();
 a : {
  if ((((M(b, a) | 0) != (b | 0) ? -1 : 0) | 0) < 0) {
   break a;
  }
  b : {
   if (f[a + 75 | 0] == 10) {
    break b;
   }
   b = e[a + 20 >> 2];
   if (b >>> 0 >= h[a + 16 >> 2]) {
    break b;
   }
   e[a + 20 >> 2] = b + 1;
   c[b | 0] = 10;
   break a;
  }
  L(a);
 }
}
function C(a) {
 var b = 0;
 b = f[a + 74 | 0];
 c[a + 74 | 0] = b + -1 | b;
 b = e[a >> 2];
 if (b & 8) {
  e[a >> 2] = b | 32;
  return -1;
 }
 e[a + 4 >> 2] = 0;
 e[a + 8 >> 2] = 0;
 b = e[a + 44 >> 2];
 e[a + 28 >> 2] = b;
 e[a + 20 >> 2] = b;
 e[a + 16 >> 2] = b + e[a + 48 >> 2];
 return 0;
}
function M(a, b) {
 var c = 0, d = 0;
 c = a;
 d = c;
 a : {
  if (e[b + 76 >> 2] <= -1) {
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
 z = 0;
 return 0;
}
function D(a) {
 if (!a) {
  return 0;
 }
 e[300] = a;
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
 return t() | 0;
}
function G(a) {
 a = a | 0;
 return 0;
}
function O() {}




// EMSCRIPTEN_END_FUNCS

  b[1] = G;
  b[2] = H;
  b[3] = F;
  function A() {
   return buffer.byteLength / 65536 | 0;
  }
  return {
   "__wasm_call_ctors": O,
   "main": N,
   "__growWasmMemory": I
  };
 }
 return asmFunc({
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
 }, asmLibraryArg, wasmMemory.buffer);
}


// EMSCRIPTEN_END_ASM



