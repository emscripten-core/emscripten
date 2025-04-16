/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <assert.h>
#include <stdio.h>
#include <emscripten.h>
#include <SDL/SDL.h>

Uint32 start_time;

Uint32 SDLCALL timer_callback(Uint32 interval, void *param) {
  static int reported = 0;
  assert(!reported);
  reported = 1;
  int result = *(int *)param;
  printf("timer_callback: %p %d\n", param, result);
  assert(result == 5);
  Uint32 now = SDL_GetTicks();
  assert(now >= start_time + 500);
  SDL_Quit();
  emscripten_force_exit(0);
}

void nop(void) {}

int main(int argc, char** argv) {
  SDL_Init(SDL_INIT_TIMER);

  Uint32 ticks1 = SDL_GetTicks();
  SDL_Delay(5); // busy-wait
  Uint32 ticks2 = SDL_GetTicks();
  // not enough ticks from busy-wait
  assert(ticks2 >= ticks1 + 4);

  int badret = 4;
  int goodret = 5;

  start_time = SDL_GetTicks();
  SDL_TimerID badtimer = SDL_AddTimer(500, timer_callback, &badret);
  SDL_TimerID goodtimer = SDL_AddTimer(1000, timer_callback, &goodret);
  SDL_RemoveTimer(badtimer);

  return 99;
}
