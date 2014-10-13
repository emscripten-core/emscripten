#include <emscripten.h>

extern "C" {

int EMSCRIPTEN_KEEPALIVE func() {
  return 10;
}

void EMSCRIPTEN_KEEPALIVE report(int result) {
  REPORT_RESULT();
}

}

