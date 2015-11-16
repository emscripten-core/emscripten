#include<stdlib.h>
#include<stdio.h>
#include<assert.h>
#include<emscripten.h>

double prevTime = -1.0;
int frame = 0;
bool blockerExecuted = false;

void final(void*) {
  assert(frame == 20);
  int result = 0;
#ifdef REPORT_RESULT
  REPORT_RESULT();
#endif
}

void looper() {
  if (blockerExecuted == false) {
    int result = 1;
#ifdef REPORT_RESULT
    REPORT_RESULT();
#endif
  }

  frame++;
  double curTime = emscripten_get_now();
  double timeSincePrevious = curTime - prevTime;
  printf("frame: %d. dt: %g\n", frame, timeSincePrevious);
  if (timeSincePrevious <= 16.0)
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

  if (frame == 20) {
    emscripten_cancel_main_loop();
    emscripten_async_call(final, (void*)0, 1000);
  }
}

void main_loop_blocker(void*) {
  blockerExecuted = true;
  emscripten_pause_main_loop();
  emscripten_resume_main_loop();
}

int main() {
  prevTime = emscripten_get_now();
  emscripten_push_uncounted_main_loop_blocker(main_loop_blocker, NULL);
  emscripten_set_main_loop(looper, 60, 1);
}
