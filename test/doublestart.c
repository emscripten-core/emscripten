/*
 * Copyright 2014 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <emscripten.h>

int times = 0;

void later(void* nada) {
  REPORT_RESULT(times);
}

void main_loop(void) {
  static int cnt = 0;
  if (++cnt >= 10) emscripten_cancel_main_loop();
}

int main(void) {
  emscripten_async_call(later, NULL, 2000);
  times++;
  printf("This should only appear once.\n");
  emscripten_set_main_loop(main_loop, 10, 0);
  return 0;
}
