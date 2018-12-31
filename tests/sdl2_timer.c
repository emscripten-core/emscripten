#include <assert.h>
#include <stdio.h>
#include <emscripten.h>
#include <SDL/SDL.h>

Uint32 SDLCALL report_result(Uint32 interval, void *param) {
  SDL_Quit();
  int result = *(int *)param;
  printf("%p %d\n", param, result);
  REPORT_RESULT();
  return 0;
}

void nop(void) {}

int main(int argc, char** argv) {
  SDL_Init(SDL_INIT_TIMER);

  Uint32 ticks1 = SDL_GetTicks();
  SDL_Delay(5); // busy-wait
  Uint32 ticks2 = SDL_GetTicks();
  assert(ticks2 >= ticks1 + 5);

  int badret = 4;
  int goodret = 5;

  SDL_TimerID badtimer = SDL_AddTimer(500, report_result, &badret);
  SDL_TimerID goodtimer = SDL_AddTimer(1000, report_result, &goodret);
  SDL_RemoveTimer(badtimer);

  emscripten_set_main_loop(nop, 0, 0);

  return 0;
}
