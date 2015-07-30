#include<stdio.h>
#include<emscripten.h>
#include<assert.h>

extern "C" {

void EMSCRIPTEN_KEEPALIVE finish(int result) {
  REPORT_RESULT();
}

int counter = 0;
int nesting = 0;
int acall_counter = 0;

void EMSCRIPTEN_KEEPALIVE acall(void *arg) {
  assert(nesting == 0);
  printf("an async call %d\n", ++acall_counter);
}

void iter() {
  printf("frame: %d\n", ++counter);

  emscripten_async_call(acall, 0, counter % 2); // 0 or 1 millis, try to execute this *before* the sync callback finishes, which would be bad

  // ensure we don't 'recurse' with the main loop sending is back in before the synchronous operation callback finishes the rest of this trace
  assert(nesting == 0);
  nesting++;
  emscripten_sleep(500);
  assert(nesting == 1);
  nesting = 2;

  if (counter % 3 == 2) {
    // second sleep every 3rd frame, to test sleep when returning from a sleep
    printf(" (second sleep)\n");
    assert(nesting == 2);
    nesting = 3;
    emscripten_sleep(1000);
    assert(nesting == 3);
  };

  nesting = 0;

  if (counter == 10) {
    assert(acall_counter == 9 /* the tenth is about to execute, but not yet! */ );
    finish(121); // if we got here without hitting any assertions, all is well
    emscripten_cancel_main_loop();
  }
}

int main() {
  emscripten_set_main_loop(iter, 3, 0);
}

}

