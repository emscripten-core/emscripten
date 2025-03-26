#include <emscripten/html5.h>

int main() {
  emscripten_throw_number(42);
#ifdef REPORT_RESULT
  REPORT_RESULT(1); // failed
#endif
}
