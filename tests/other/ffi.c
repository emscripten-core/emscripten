/*
 * Copyright 2017 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <math.h>

float add_f (float a, float b) {
    return a + b;
}

long long add_ll (long long a, long long b) {
    return a + b;
}

extern float import_f(float x);

extern long long import_ll(long long x);

int main () {
    float a = 1.2,
          b = import_f((float)3.4),
          c;
    c = add_f(a, b);

    long long d = 1,
              e = import_ll((long long)2),
              f;
    f = add_ll(d, e);
}
