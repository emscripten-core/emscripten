#include <emscripten.h>

extern "C" {

int EMSCRIPTEN_KEEPALIVE func() {
  return 22;
}

void EMSCRIPTEN_KEEPALIVE report(int result) {
  REPORT_RESULT();
}

}

