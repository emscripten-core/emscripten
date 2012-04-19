#include<stdio.h>
#include<math.h>
#include<SDL.h>
#include<emscripten.h>
#include<assert.h>

int last = 0;

extern "C" {

void third() {
  int now = SDL_GetTicks();
  printf("thard! %d\n", now);
  assert(fabs(now - last - 1000) < 500);
  int result = 1;
  REPORT_RESULT();
}

void second() {
  int now = SDL_GetTicks();
  printf("sacond! %d\n", now);
  assert(fabs(now - last - 500) < 250);
  last = now;
  emscripten_async_call(third, 1000);
}

}

int main() {
  SDL_Init(0);
  last = SDL_GetTicks();
  printf("frist! %d\n", last);

  emscripten_async_call(second, 500);

  return 1;
}

