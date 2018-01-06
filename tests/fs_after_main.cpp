#include <stdio.h>
#include <assert.h>
#include <emscripten.h>

// test file operations after main() exits

#define NAME "file.cpp"

EMSCRIPTEN_KEEPALIVE
extern "C" void finish(void*) {
  EM_ASM({
    var printed = Module['extraSecretBuffer'].split('Iteration').length - 1;
    console.log(printed);
    assert(printed == 5, 'should have printed 5 iterations');
  });
  printf("Test passed.\n");
#ifdef REPORT_RESULT
  REPORT_RESULT(0);
#endif
}

EMSCRIPTEN_KEEPALIVE
extern "C" void looper() {
  // exiting main should not cause any weirdness with file opening
  printf("Iteration\n");
  FILE* f = fopen("/dev/stdin", "rb");
  if (!f) {
    printf("Test failed.\n");
#ifdef REPORT_RESULT
    REPORT_RESULT(1);
#endif
  }
  fclose(f);
}

int main() {
  EM_ASM({
    (function() {
      // exiting main should not cause any weirdness with printing
      var realPrint = Module['print'];
      Module['extraSecretBuffer'] = '';
      Module['print'] = function(x) {
        Module['extraSecretBuffer'] += x;
        realPrint(x);
      };
    })();
  });
  printf("Start\n");
  FILE* f = fopen(NAME, "wb");
  fclose(f);
  printf("Looping...\n");
  EM_ASM({
    Module['print']('js');
    var counter = 0;
    function looper() {
      Module['print']('js looping');
      Module['_looper']();
      counter++;
      if (counter < 5) {
        Module['print']('js queueing');
        setTimeout(looper, 1);
      } else {
        Module['print']('js finishing');
        setTimeout(Module['_finish'], 1);
      }
    }
    looper();
  });
}
