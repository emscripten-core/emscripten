addToLibrary({
  emscripten_math_cbrt: 'Math.cbrt',
  emscripten_math_pow: 'Math.pow',
  emscripten_math_random: 'Math.random',
  emscripten_math_sign: 'Math.sign',
  emscripten_math_sqrt: 'Math.sqrt',
  emscripten_math_exp: 'Math.exp',
  emscripten_math_expm1: 'Math.expm1',
  emscripten_math_fmod: (x, y) => x % y,
  emscripten_math_log: 'Math.log',
  emscripten_math_log1p: 'Math.log1p',
  emscripten_math_log10: 'Math.log10',
  emscripten_math_log2: 'Math.log2',
  emscripten_math_round: 'Math.round',
  emscripten_math_acos: 'Math.acos',
  emscripten_math_acosh: 'Math.acosh',
  emscripten_math_asin: 'Math.asin',
  emscripten_math_asinh: 'Math.asinh',
  emscripten_math_atan: 'Math.atan',
  emscripten_math_atanh: 'Math.atanh',
  emscripten_math_atan2: 'Math.atan2',
  emscripten_math_cos: 'Math.cos',
  emscripten_math_cosh: 'Math.cosh',
  emscripten_math_hypot: (count, varargs) => {
    var args = [];
    for (var i = 0; i < count; ++i) {
      args.push({{{ makeGetValue('varargs', `i * ${getNativeTypeSize('double')}`, 'double') }}});
    }
    return Math.hypot(...args);
  },
  emscripten_math_sin: 'Math.sin',
  emscripten_math_sinh: 'Math.sinh',
  emscripten_math_tan: 'Math.tan',
  emscripten_math_tanh: 'Math.tanh',
});
