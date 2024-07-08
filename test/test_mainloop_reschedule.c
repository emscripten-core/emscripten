// Copyright 2015 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <stdio.h>
#include <emscripten.h>

int n = 0;
double rate = 0;
double last = -1;

void main_loop(void) {
  emscripten_sleep(0);
  n++;
  double now = emscripten_get_now();
  if (last > 0) {
    double curr = now - last;
    rate = (rate*(n-1)+curr)/n;
    if (n > 15) {
      emscripten_cancel_main_loop();
      int result = rate > 600;
      printf("Final rate: %.2f, success: %d\n", rate, result);
      REPORT_RESULT(result);
      return;
    }
  }
  last = emscripten_get_now();
  printf("Main loop rate: %.2f (over %d)\n", rate, n);
}

int main(void) {
  emscripten_set_main_loop(main_loop, 1, 1);
}

