#include<stdio.h>
#include<assert.h>
#include<emscripten.h>

int frame = 0;

void final(void*) {
  assert(frame == 10);
  int result = 0;
  REPORT_RESULT();
}

void looper() {
  frame++;
  printf("frame: %d\n", frame);
  if (frame == 5) {
    emscripten_cancel_main_loop();
    emscripten_set_main_loop(looper, 100, 1);
  } else if (frame == 10) {
    emscripten_cancel_main_loop();
    emscripten_async_call(final, (void*)0, 1000);
  }
}

int main() {
  emscripten_set_main_loop(looper, 5, 1);
}

