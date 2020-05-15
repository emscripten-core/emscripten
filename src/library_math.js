mergeInto(LibraryManager.library, {
  emscripten_math_cbrt: function(x) {
    return Math.cbrt(x);
  },
  emscripten_math_pow: function(x, y) {
    return Math.pow(x, y);
  },
  emscripten_math_random: function(x) {
    return Math.random(x);
  },
  emscripten_math_sign: function(x) {
    return Math.sign(x);
  },
  emscripten_math_sqrt: function(x) {
    return Math.sqrt(x);
  },
  emscripten_math_exp: function(x) {
    return Math.exp(x);
  },
  emscripten_math_expm1: function(x) {
    return Math.expm1(x);
  },
  emscripten_math_log: function(x) {
    return Math.log(x);
  },
  emscripten_math_log1p: function(x) {
    return Math.log1p(x);
  },
  emscripten_math_log10: function(x) {
    return Math.log10(x);
  },
  emscripten_math_log2: function(x) {
    return Math.log2(x);
  },
  emscripten_math_round: function(x) {
    return Math.round(x);
  },
  emscripten_math_acos: function(x) {
    return Math.acos(x);
  },
  emscripten_math_acosh: function(x) {
    return Math.acosh(x);
  },
  emscripten_math_asin: function(x) {
    return Math.asin(x);
  },
  emscripten_math_asinh: function(x) {
    return Math.asinh(x);
  },
  emscripten_math_atan: function(x) {
    return Math.atan(x);
  },
  emscripten_math_atanh: function(x) {
    return Math.atanh(x);
  },
  emscripten_math_atan2: function(y, x) {
    return Math.atan2(y, x);
  },
  emscripten_math_cos: function(x) {
    return Math.cos(x);
  },
  emscripten_math_cosh: function(x) {
    return Math.cosh(x);
  },
  emscripten_math_hypot: function(count, varargs) {
    var args = [];
    for(var i = 0; i < count; ++i) args.push(HEAPF64[(varargs>>3) + i]);
    return Math.hypot.apply(null, args);
  },
  emscripten_math_sin: function(x) {
    return Math.sin(x);
  },
  emscripten_math_sinh: function(x) {
    return Math.sinh(x);
  },
  emscripten_math_tan: function(x) {
    return Math.tan(x);
  },
  emscripten_math_tanh: function(x) {
    return Math.tanh(x);
  }
});
