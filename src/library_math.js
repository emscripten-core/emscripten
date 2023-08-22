addToLibrary({
  emscripten_math_cbrt: (x) => Math.cbrt(x),
  emscripten_math_pow: (x, y) => Math.pow(x, y),
  emscripten_math_random: () => Math.random(),
  emscripten_math_sign: (x) => Math.sign(x),
  emscripten_math_sqrt: (x) => Math.sqrt(x),
  emscripten_math_exp: (x) => Math.exp(x),
  emscripten_math_expm1: (x) => Math.expm1(x),
  emscripten_math_fmod: (x, y) => x % y,
  emscripten_math_log: (x) => Math.log(x),
  emscripten_math_log1p: (x) => Math.log1p(x),
  emscripten_math_log10: (x) => Math.log10(x),
  emscripten_math_log2: (x) => Math.log2(x),
  emscripten_math_round: (x) => Math.round(x),
  emscripten_math_acos: (x) => Math.acos(x),
  emscripten_math_acosh: (x) => Math.acosh(x),
  emscripten_math_asin: (x) => Math.asin(x),
  emscripten_math_asinh: (x) => Math.asinh(x),
  emscripten_math_atan: (x) => Math.atan(x),
  emscripten_math_atanh: (x) => Math.atanh(x),
  emscripten_math_atan2: (y, x) => Math.atan2(y, x),
  emscripten_math_cos: (x) => Math.cos(x),
  emscripten_math_cosh: (x) => Math.cosh(x),
  emscripten_math_hypot: (count, varargs) => {
    var args = [];
    for (var i = 0; i < count; ++i) {
      args.push({{{ makeGetValue('varargs', `i * ${getNativeTypeSize('double')}`, 'double') }}});
    }
    return Math.hypot.apply(null, args);
  },
  emscripten_math_sin: (x) => Math.sin(x),
  emscripten_math_sinh: (x) => Math.sinh(x),
  emscripten_math_tan: (x) => Math.tan(x),
  emscripten_math_tanh: (x) => Math.tanh(x)
});
