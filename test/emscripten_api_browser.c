// Copyright 2012 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <stdbool.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <SDL/SDL.h>
#include <emscripten.h>
#include <assert.h>

bool exit_ok = false;
int last = 0;

bool pre1ed = false;
bool pre2ed = false;
void pre1(void *arg) {
  assert(!pre1ed);
  assert(!pre2ed);
  assert((long)arg == 123);
  pre1ed = true;
}
void pre2(void *arg) {
  assert(pre1ed);
  assert(!pre2ed);
  assert((long)arg == 98);
  pre2ed = true;
}

bool fived = false;
void five(void *arg) {
  assert((long)arg == 55);
  fived = true;
  emscripten_resume_main_loop();
}

void argey(void* arg) {
  static int counter = 0;
  assert((long)arg == 17);
  counter++;
  printf("argey: %d\n", counter);
  if (counter == 5) {
    emscripten_cancel_main_loop();
    // The main loop is now done so its ok to run atexit handlers.
    exit_ok = true;
    exit(0);
  }
}

void mainey() {
  static int counter = 0;
  printf("mainey: %d\n", counter++);
  if (counter == 20) {
    emscripten_pause_main_loop();
    emscripten_async_call(five, (void*)55, 1000);
  } else if (counter == 22) { // very soon after 20, so without pausing we fail
    assert(fived);
    emscripten_push_main_loop_blocker(pre1, (void*)123);
    emscripten_push_main_loop_blocker(pre2, (void*)98);
  } else if (counter == 23) {
    assert(pre1ed);
    assert(pre2ed);
    printf("Good!\n");
    emscripten_cancel_main_loop();
    emscripten_set_main_loop_arg(argey, (void*)17, 0, 0);
  }
}

void four(void *arg) {
  assert((long)arg == 43);
  printf("four!\n");
  emscripten_set_main_loop(mainey, 0, 0);
}

EMSCRIPTEN_KEEPALIVE void third() {
  int now = SDL_GetTicks();
  printf("thard! %d\n", now);
  assert(abs(now - last - 1000) < 500);
  emscripten_async_call(four, (void*)43, -1); // triggers requestAnimationFrame
}

void second(void *arg) {
  int now = SDL_GetTicks();
  printf("sacond! %d\n", now);
  assert(abs(now - last - 500) < 250);
  last = now;
  emscripten_async_run_script("Module._third()", 1000);
}

// Should not be called when main return but only once the
// main loops is stopped and the runtime shuts down.
void check_exit_ok() {
  assert(exit_ok == true);
}

int main() {
  SDL_Init(0);
  last = SDL_GetTicks();
  printf("frist! %d\n", last);

  double ratio = emscripten_get_device_pixel_ratio();
  double ratio2 = EM_ASM_DOUBLE({
    return window.devicePixelRatio || 1.0;
  });

  assert(ratio == ratio2);

  atexit(check_exit_ok);

  emscripten_async_call(second, (void*)0, 500);

  return 1;
}

