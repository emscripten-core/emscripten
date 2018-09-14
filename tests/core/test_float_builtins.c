/*
 * Copyright 2017 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <math.h>
#include <stdio.h>

// Prints the float/double/long double versions of the given function
// e.g.: TEST(fmax) prints fmaxf : fmax : fmaxl
#define TEST(func)                    \
    printf("%f : %f : %Lf\n",         \
        __builtin_##func##f(f1, f2),  \
        __builtin_##func(d1, d2),     \
        __builtin_##func##l(l1, l2));

int test_builtins() {
    float f1 = 0.1234f;
    float f2 = 0.5678f;
    double d1 = 1.0101;
    double d2 = 0.10101;
    long double l1 = 12.0123L;
    long double l2 = 21.3201L;
    TEST(fmax)
    TEST(fmin)
    TEST(fmod)
    return 0;
}

void test_exp_log(double x) {
    double a = exp2(x);
    double b = log10(x);
    float c = log10f(x);
    double d = acos(x);
    printf("%f : %f : %f : %d\n", a, b, c, isnan(d));
}

int main() {
    puts("***start***");
    test_builtins();
    test_exp_log(1234.5678);
    puts("***end***");
    return 0;
}
