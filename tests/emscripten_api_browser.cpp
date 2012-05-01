#include<stdio.h>
#include<math.h>
#include<stdlib.h>
#include<SDL.h>
#include<emscripten.h>
#include<assert.h>

int last = 0;

extern "C" {

void mainey() {
  static int counter = 0;
  printf("mainey: %d\n", counter++);
  if (counter == 10) {
    int result = 1;
    REPORT_RESULT();
  }
}

void four() {
  printf("four!\n");
  emscripten_set_main_loop(mainey, 0);
}

void __attribute__((used)) third() {
  int now = SDL_GetTicks();
  printf("thard! %d\n", now);
  assert(fabs(now - last - 1000) < 500);
  emscripten_async_call(four, -1); // triggers requestAnimationFrame
}

void second() {
  int now = SDL_GetTicks();
  printf("sacond! %d\n", now);
  assert(fabs(now - last - 500) < 250);
  last = now;
  emscripten_async_run_script("_third()", 1000);
}

}

void never() {
  int result = 0;
  REPORT_RESULT();
}

int main() {
  SDL_Init(0);
  last = SDL_GetTicks();
  printf("frist! %d\n", last);

  atexit(never); // should never be called - it is wrong to exit the runtime orderly if we have async calls!

  emscripten_async_call(second, 500);

  return 1;
}

