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

Uint32 SDLCALL report_result(Uint32 interval, void *param) {
  static int reported = 0;
  if (reported) return 0;
  reported = 1;
  SDL_Quit();
  int result = *(int *)param;
  printf("%p %d\n", param, result);
  REPORT_RESULT(result);
  return 0;
}

void nop(void) {}

int main(int argc, char** argv) {
  SDL_Init(SDL_INIT_TIMER);

  Uint32 ticks1 = SDL_GetTicks();
  SDL_Delay(5); // busy-wait
  Uint32 ticks2 = SDL_GetTicks();
  if (ticks2 < ticks1 + 4) {
    printf("not enough ticks from busy-wait\n");
    REPORT_RESULT(9);
    return 0;
  }

  int badret = 4;
  int goodret = 5;

  SDL_TimerID badtimer = SDL_AddTimer(500, report_result, &badret);
  SDL_TimerID goodtimer = SDL_AddTimer(1000, report_result, &goodret);
  SDL_RemoveTimer(badtimer);

  emscripten_set_main_loop(nop, 0, 0);

  return 0;
}
