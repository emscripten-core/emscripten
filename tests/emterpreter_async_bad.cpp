#include<stdio.h>
#include<emscripten.h>
#include<assert.h>

extern "C" {

int result = 1;

void finish() {
  REPORT_RESULT();
}

int counter = 0;

void __attribute__((noinline)) run_loop() {
  // infinite main loop, turned async using emterpreter
  while (1) {
    printf("frame: %d\n", ++counter);
    emscripten_sleep(100);
    if (counter == 10) {
      finish();
      break;
    }
  }
}

void __attribute__((noinline)) middle() {
  run_loop();
  printf("after run_loop, counter: %d\n", counter);
  assert(counter == 10);
  result = 99;
}

int main() {
  middle();
}

}

