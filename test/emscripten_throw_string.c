#include <emscripten/html5.h>

int main() {
  emscripten_throw_string("Hello!");
#ifdef REPORT_RESULT
  REPORT_RESULT(1); // failed
#endif
}
