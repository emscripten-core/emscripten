/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <vector>
#include <stdio.h>

struct S {
  int a;
  float b;
};

void foo(int a, float b) { printf("%d:%.2f\n", a, b); }

int main(int argc, char *argv[]) {
  std::vector<S> ar;
  S s;

  s.a = 789;
  s.b = 123.456f;
  ar.push_back(s);

  s.a = 0;
  s.b = 100.1f;
  ar.push_back(s);

  foo(ar[0].a, ar[0].b);
  foo(ar[1].a, ar[1].b);
}
