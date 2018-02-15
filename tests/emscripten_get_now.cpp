#include <stdio.h>
#include "emscripten.h"

#ifndef REPORT_RESULT
// To be able to run this test outside the browser harness in node.js/spidermonkey:
#define REPORT_RESULT(result)
#endif

int result = 0;

int main() {
  // This code tests three things:
  // a) Calling emscripten_get_now(), time actually proceeds.
  // b) Values returned by emscripten_get_now() are strictly nondecreasing.
  // c) emscripten_get_now() is able to return sub-millisecond precision timer values.
  double smallest_delta = 1000.f;
  for(int x = 0; x < 1000; ++x) { // Have several attempts to find a good small delta, i.e. give time to JS engine to warm up the code and so on.
    double t = emscripten_get_now();
    double t2 = t;
    while (t2 == t) {
      t2 = emscripten_get_now();
    }
    if (t2 < t && t2 - t < 1000.f) { // Timer must be monotonous.
      printf("Timer is not monotonous!\n");
      smallest_delta = t2 - t;
      break;
    }
    if (t2 > t) { // Must pass less than a millisecond between two calls.
      smallest_delta = t2 - t;
      break;
    }
  }
  
  printf("Timer resolution (smallest delta): %.20f msecs\n", smallest_delta);

  if (smallest_delta < 5.f) {
    printf("Timer resolution is good (or as good as spectre allows...).\n");
    result = 1;
  } else {
    printf("Error: Bad timer precision, too large\n");
    result = 0;
  }
  REPORT_RESULT(result);
  return 0;
}
