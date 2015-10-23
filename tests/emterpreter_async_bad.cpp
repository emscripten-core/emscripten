#include<stdio.h>
#include<emscripten.h>
#include<assert.h>

extern "C" {

void EMSCRIPTEN_KEEPALIVE finish(int result) {
  REPORT_RESULT();
}

int counter = 0;

void __attribute__((noinline)) run_loop() {
  // infinite main loop, turned async using emterpreter
  while (1) {
    printf("frame: %d\n", ++counter);
    emscripten_sleep(100);
    if (counter == 10) {
      finish(123); // this should not happen, we should fail!
      break;
    }
  }
}

void __attribute__((noinline)) middle() {
  run_loop();
  printf("after run_loop, counter: %d\n", counter);
  assert(counter == 10);
  assert(0); // we should never get here!
}

int main() {
  EM_ASM({
    window.onerror = function(err) {
      assert(err.toString().indexOf('This error happened during an emterpreter-async save or load of the stack') > 0, 'expect good error message');
      // manually REPORT_RESULT; we can't call back into native code at this point, assertions would trigger
      xhr = new XMLHttpRequest();
      xhr.open("GET", "http://localhost:8888/report_result?1");
      xhr.onload = xhr.onerror = function() {
        window.close();
      };
      xhr.send();
    };
  });

  middle();
}

}

