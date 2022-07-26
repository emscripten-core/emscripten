// Copyright 2016 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include "emscripten.h"

extern "C" {
void save_me_aimee() { printf("mann\n"); }
}

int main() {
  // EMSCRIPTEN_COMMENT("hello from the source");
  emscripten_run_script("out('hello world' + '!')");
  printf("*%d*\n", emscripten_run_script_int("5*20"));
  printf("*%s*\n", emscripten_run_script_string("'five'+'six'"));
  emscripten_run_script("Module['_save_me_aimee']()");
  //
  double d = 0.1234567891231219886553;
  int len = emscripten_print_double(d, NULL, -1);
  char buffer[len+1];
  buffer[len] = CHAR_MAX;
  emscripten_print_double(d, buffer, len+1);
  assert(buffer[len] == 0); // null terminated
  double e;
  sscanf(buffer, "%lf", &e);
  printf("%.30lf : %s : %.30lf  (%d)\n", d, buffer, e, len);

  buffer[0] = 1;
  buffer[1] = 2;
  buffer[2] = 3;
  int n = emscripten_print_double(d, buffer, 2);
  assert(n == 1);
  assert(buffer[0] == '0'); // touched
  assert(buffer[1] == 0); // touched
  assert(buffer[2] == 3); // untouched
  puts("success\n");

  return 0;
}
