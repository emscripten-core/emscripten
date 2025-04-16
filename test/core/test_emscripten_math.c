#include <stdio.h>
#include <emscripten/em_math.h>

int main() {
  printf("%f\n", emscripten_math_acos(0.5));
  printf("%f\n", emscripten_math_acosh(42.0));
  printf("%f\n", emscripten_math_asin(0.5));
  printf("%f\n", emscripten_math_asinh(42.0));
  printf("%f\n", emscripten_math_atan(42.0));
  printf("%f\n", emscripten_math_atan2(42.0, 13.0));
  printf("%f\n", emscripten_math_atanh(0.9));
  printf("%f\n", emscripten_math_cbrt(8.0));
  printf("%f\n", emscripten_math_cos(42.0));
  printf("%f\n", emscripten_math_cosh(0.6));
  printf("%f\n", emscripten_math_exp(2.0));
  printf("%f\n", emscripten_math_expm1(2.0));
  printf("%f\n", emscripten_math_fmod(2.0, 0.75));
  printf("%f\n", emscripten_math_hypot(3, 3.0, 4.0, 5.0));
  printf("%f\n", emscripten_math_log(42.0));
  printf("%f\n", emscripten_math_log10(42.0));
  printf("%f\n", emscripten_math_log1p(42.0));
  printf("%f\n", emscripten_math_log2(42.0));
  printf("%f\n", emscripten_math_pow(2.0, 4.0));
  printf("%d\n", (int)(emscripten_math_random() >= 0 && emscripten_math_random() <= 1));
  printf("%f\n", emscripten_math_round(42.5));
  printf("%f\n", emscripten_math_sign(-42.0));
  printf("%f\n", emscripten_math_sin(42.0));
  printf("%f\n", emscripten_math_sinh(0.6));
  printf("%f\n", emscripten_math_sqrt(10000.0));
  printf("%f\n", emscripten_math_tan(42.0));
  printf("%f\n", emscripten_math_tanh(42.0));
  printf("-\n");
  printf("%f\n", EM_MATH_E);
  printf("%f\n", EM_MATH_LN2);
  printf("%f\n", EM_MATH_LN10);
  printf("%f\n", EM_MATH_LOG2E);
  printf("%f\n", EM_MATH_LOG10E);
  printf("%f\n", EM_MATH_PI);
  printf("%f\n", EM_MATH_SQRT1_2);
  printf("%f\n", EM_MATH_SQRT2);
}
