/*
 * Copyright 2019 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <emscripten.h>
#include <stdio.h>
#include <stdlib.h>

extern "C" {

EM_JS(void, log_started, (), {
  out("foo_start");
})

EM_JS(void, log_ended, (), {
  out("foo_end");
})

void foo_start(int n) {
  log_started();
  // prevent inlining
  if (n > 10000) foo_start(n - 1);
  if (n > 15000) foo_start(n - 3);
}

void foo_end(int n) {
  log_ended();
  // prevent inlining
  if (n > 20000) foo_end(n - 2);
  if (n > 30000) foo_end(n - 5);
}

} // extern "C"

int main(int argc, char** argv) {
  int x = atoi(argv[1]);
  foo_start(x);
#ifdef CONDITIONAL
  if (x != 42) {
#endif
    emscripten_lazy_load_code();
#ifdef CONDITIONAL
  }
#endif
  foo_end(x); // this can be elided in the first download, only needed after we lazily load the second
  printf("all done, %d", x); // only when we reach here do we need stdio linked in
}
