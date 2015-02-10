#include<stdio.h>
#include<emscripten.h>
#include<assert.h>

extern "C" {

void EMSCRIPTEN_KEEPALIVE finish(int result) {
  REPORT_RESULT();
}

int counter = 0;
int nesting = 0;

void iter() {
  printf("frame: %d\n", ++counter);

  // ensure we don't 'recurse' with the main loop sending is back in before the synchronous operation callback finishes the rest of this trace
  assert(nesting == 0);
  nesting++;
  emscripten_sleep(500);
  assert(nesting == 1);
  nesting = 0;

  if (counter == 10) {
    finish(121); // if we got here without hitting any assertions, all is well
    emscripten_cancel_main_loop();
  }
}

int main() {
  emscripten_set_main_loop(iter, 0, 0);
}

}

