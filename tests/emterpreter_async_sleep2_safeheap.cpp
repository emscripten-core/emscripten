#include <stdio.h>
#include <emscripten.h>

int result = 0;

extern "C" {

void fix() {
  result = 17;
}

void EMSCRIPTEN_KEEPALIVE callback() {
  EM_ASM_({
    Module.print('callback...');
    Module['dynCall_v']($0);
    Module.print('callback fixed.');
  }, (int)&fix);
}

}

int main(void) {
  EM_ASM({
    setTimeout(function() {
      Module['_callback']();
    }, 1);
  });
  printf("Sleep:\n");
  emscripten_sleep(1000);
  printf("Done!\n");
  REPORT_RESULT();
  return 0;
}

