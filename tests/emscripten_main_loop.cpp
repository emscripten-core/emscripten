#include<stdlib.h>
#include<stdio.h>
#include<assert.h>
#include<emscripten.h>

double prevTime = -1.0;
int frame = 0;

void final(void*) {
  assert(frame == 100);
  int result = 0;
#ifdef REPORT_RESULT
  REPORT_RESULT();
#endif
}

void looper() {
  frame++;
  double curTime = emscripten_get_now();
  double timeSincePrevious = (prevTime >= 0) ? (curTime - prevTime) : (1000.0/60.0);
  printf("frame: %d. dt: %g\n", frame, timeSincePrevious);
  if (timeSincePrevious <= 1.0)
  {
    printf("Abort: main loop tick was called too quickly after the previous frame!\n");
    int result = 1;
#ifdef REPORT_RESULT
    REPORT_RESULT();
#endif
    emscripten_cancel_main_loop();
    exit(0);
  }
  prevTime = curTime;
  if (frame > 0 && frame < 90 && frame % 10 == 0) {
    emscripten_cancel_main_loop();
    emscripten_set_main_loop(looper, 0, 0);
  } else if (frame == 90) {
    emscripten_cancel_main_loop();
    emscripten_set_main_loop(looper, 100, 1);
  } else if (frame == 100) {
    emscripten_cancel_main_loop();
    emscripten_async_call(final, (void*)0, 1000);
  }
}

int main() {
  emscripten_set_main_loop(looper, 5, 1);
}
