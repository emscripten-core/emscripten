// Copyright 2016 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

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
  printf("Frame %d\n", numFrames);
  if (numFrames == 10) {
    double now = emscripten_get_now();
    // Sub one to account for intervals vs endpoints
    double msecsPerFrame = (now - frame0) / (numFrames-1);
    printf("Avg. msecs/frame: %f\n", msecsPerFrame);
    emscripten_cancel_main_loop();
    // Expecting 500msecs/frame, but allow a lot of leeway. Bad value would be
    // 900msecs/frame (400msecs of processing below and 500msecs of delay)
    exit((msecsPerFrame > 350 && msecsPerFrame < 650) ? 0 : 1);
  }

  // Busy wait 400 msecs.
  double now = start;
  while (now - start < 400) {
    now = emscripten_get_now();
  }
}

int main() {
  // Want to run at 2 fps, or 500msecs/frame.
  emscripten_set_main_loop(looper, 2, 0);
  return 99;
}
