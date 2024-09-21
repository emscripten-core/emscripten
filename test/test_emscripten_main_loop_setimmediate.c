// Copyright 2016 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <emscripten.h>

int numFrames = 0;

void looper() {
  static double frame0 = 0;
  if (!frame0) frame0 = emscripten_get_now();

  double start = emscripten_get_now();

  ++numFrames;
  if (numFrames % 10 == 0) printf("Frame %d\n", numFrames);
  if (numFrames == 100) {
    double now = emscripten_get_now();
    double msecsPerFrame = (now - frame0) / (numFrames-1); // Sub one to account for intervals vs endpoints
    printf("Avg. msecs/frame: %f\n", msecsPerFrame);
    emscripten_cancel_main_loop();
    // Expecting to run extremely fast unthrottled, and certainly not bounded by
    // vsync, so less than common 16.667 msecs per frame (this is assuming 60hz
    // display)
    exit((msecsPerFrame < 5) ? 0 : 1);
  }
}

int main() {
  emscripten_set_main_loop(looper, 1, 0);
  emscripten_set_main_loop_timing(EM_TIMING_SETIMMEDIATE, 0);
  return 99;
}
