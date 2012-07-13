#include<stdio.h>
#include<math.h>
#include<stdlib.h>
#include<SDL.h>
#include<emscripten.h>
#include<assert.h>

int last = 0;

extern "C" {

bool pre1ed = false;
bool pre2ed = false;
void pre1() {
  assert(!pre1ed);
  assert(!pre2ed);
  pre1ed = true;
}
void pre2() {
  assert(pre1ed);
  assert(!pre2ed);
  pre2ed = true;
}

bool fived = false;
void five() {
  fived = true;
  emscripten_resume_main_loop();
}

void mainey() {
  static int counter = 0;
  printf("mainey: %d\n", counter++);
  if (counter == 20) {
    emscripten_pause_main_loop();
    emscripten_async_call(five, 1000);
  } else if (counter == 22) { // very soon after 20, so without pausing we fail
    assert(fived);
    emscripten_push_main_loop_blocker(pre1);
    emscripten_push_main_loop_blocker(pre2);
  } else if (counter == 23) {
    assert(pre1ed);
    assert(pre2ed);
    printf("Good!\n");
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

