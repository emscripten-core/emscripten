/*
 * Copyright 2020 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/**
 * This file contains C functions to access the JavaScript Math API via Emscripten.
 * Please note that accessing these functions is relatively slow, since they each
 * incur a language boundary crossing call from WebAssembly out to JavaScript.

 * These functions are best used in scenarios where small code size is more desirable
 * than performance.

 * See https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Math
 * for details. */

// The following operations have very fast WebAssembly opcodes. Therefore they are not
// exposed as individual functions:

// Math.abs(x) -> f32.abs and f64.abs. (use fabsf() and fabs() from math.h)
// Math.ceil -> f32.ceil and f64.ceil (ceil() and ceilf() in math.h)
// Math.clz32(x) -> i32.clz and i64.clz (call __builtin_clz() and __builtin_clzll())
// Math.floor -> f32.floor and f64.floor (floor() and floorf() in math.h)
// Math.fround -> f64.promote_f32(f32.demote_f64()) (call double d = (double)(float)someDouble;)
// Math.imul(x, y) -> i32.mul and i64.mul (directly multiply two signed integers)
// Math.min -> f32.min and f64.min (fminf() and fmin() in math.h)
// Math.max -> f32.max and f64.max (fmaxf() and fmax() in math.h)
// Math.trunc -> f32.trunc and f64.trunc (truncf() and trunc() in math.h)

// The following constants are available on the JS Math object, mirrored here for convenience.

#define EM_MATH_E 2.718281828459045
#define EM_MATH_LN2 0.6931471805599453
#define EM_MATH_LN10 2.302585092994046
#define EM_MATH_LOG2E 1.4426950408889634
#define EM_MATH_LOG10E 0.4342944819032518
#define EM_MATH_PI 3.141592653589793
#define EM_MATH_SQRT1_2 0.7071067811865476
#define EM_MATH_SQRT2 1.4142135623730951

// The following Math operations do not have native WebAssembly opcodes, and
// are provided here as small sized alternatives to their libc counterparts.

double emscripten_math_acos(double x); // acos() in math.h
double emscripten_math_acosh(double x); // acosh() in math.h
double emscripten_math_asin(double x); // asin() in math.h
double emscripten_math_asinh(double x); // asinh() in math.h
double emscripten_math_atan(double x); // atan() in math.h
double emscripten_math_atan2(double y, double x); // atan2() in math.h
double emscripten_math_atanh(double x); // atanh() in math.h
double emscripten_math_cbrt(double x); // cbrt() in math.h
double emscripten_math_cos(double x); // cos() in math.h
double emscripten_math_cosh(double x); // cosh() in math.h
double emscripten_math_exp(double x); // exp() in math.h
double emscripten_math_expm1(double x); // expm1() in math.h
double emscripten_math_fmod(double x, double y); // fmod() in math.h, not a function on Math, but calls JS "x % y" operator.
double emscripten_math_hypot(int count, ...);  // hypot() in math.h (although only for fixed 2 arguments)
double emscripten_math_log(double x); // log() in math.h
double emscripten_math_log1p(double x); // log1p() in math.h
double emscripten_math_log10(double x); // log10() in math.h
double emscripten_math_log2(double x); // log2() in math.h
double emscripten_math_pow(double x, double y); // pow(x, y) in math.h
double emscripten_math_random(void); // N.b. emscripten_random() in emscripten.h returns a single-precision float!
double emscripten_math_round(double x); // round() in math.h
double emscripten_math_sign(double x); // No equivalent in libc
double emscripten_math_sin(double x); // sin() in math.h
double emscripten_math_sinh(double x); // sinh() in math.h
double emscripten_math_sqrt(double x); // sqrt() in math.h
double emscripten_math_tan(double x); // tan() in math.h
double emscripten_math_tanh(double x); // tanh() in math.h

#ifdef __cplusplus
}
#endif
