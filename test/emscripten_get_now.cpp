// Copyright 2013 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <assert.h>
#include <stdio.h>
#include "emscripten.h"

const double good_enough = 5.0; // good as we can do, given spectre...

int main() {
  // This code tests three things:
  // a) Calling emscripten_get_now(), time actually proceeds.
  // b) Values returned by emscripten_get_now() are strictly nondecreasing.
  // c) emscripten_get_now() is able to return sub-millisecond precision timer values.
  double smallest_delta = 1000.f;
  double start = emscripten_get_now();
  while (1) { // Have several attempts to find a good small delta, i.e. give time to JS engine to warm up the code and so on.
    double t = emscripten_get_now();
    if (t - start >= 5000) {
      printf("We waited long enough, %f ms\n", t - start);
      break;
    }
    double t2 = t;
    while (t2 == t) {
      t2 = emscripten_get_now();
    }
    double delta = t2 - t;
    if (delta < 0.) { // Timer must be monotonous.
      printf("Timer is not monotonous!\n");
      return 1;
    }
    if (delta < smallest_delta) {
      smallest_delta = delta;
      if (delta <= good_enough) {
        break;
      }
    }
  }

  if (smallest_delta <= 0.) {
    printf("Smallest delta is invalid %f\n", smallest_delta);
    return 1;
  }

  printf("Timer resolution (smallest delta): %.20f msecs\n", smallest_delta);

  if (smallest_delta <= good_enough) {
    printf("Timer resolution is good (or as good as spectre allows...).\n");
    return 0;
  }

  printf("Error: Bad timer precision, too large\n");
  assert(false);
  return 1;
}
