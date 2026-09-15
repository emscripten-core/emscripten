/*
 * Copyright 2026 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

#include <assert.h>
#include <stdlib.h>
#include <emscripten.h>
#include <emscripten/eventloop.h>

static int frames;

static void resume(void* unused) {
  emscripten_resume_main_loop();
}

static void tick(void) {
  ++frames;
  if (frames == 2) {
    emscripten_pause_main_loop();
    emscripten_set_timeout(resume, 10, NULL);
  } else if (frames == 4) {
    assert(emscripten_set_main_loop_timing(EM_TIMING_SETTIMEOUT, 1) == 0);
  } else if (frames == 6) {
    assert(emscripten_set_main_loop_timing(EM_TIMING_SETIMMEDIATE, 0) == 0);
  } else if (frames == 8) {
    emscripten_cancel_main_loop();
    emscripten_set_main_loop(tick, 0, 0);
    assert(emscripten_set_main_loop_timing(EM_TIMING_SETIMMEDIATE, 0) == 0);
  } else if (frames == 16) {
    int mode = EM_ASM_INT({ return Module['testMode']; });
    if (mode >= 2) {
      EM_ASM({ abort('expected scheduler abort'); });
    } else if (mode == 1) {
      // Exit with a registered loop, without first cancelling it.
      emscripten_force_exit(0);
    } else {
      emscripten_cancel_main_loop();
      exit(0);
    }
  }
}

int main(void) {
  emscripten_set_main_loop(tick, 0, 0);
  assert(emscripten_set_main_loop_timing(EM_TIMING_SETIMMEDIATE, 0) == 0);
}
