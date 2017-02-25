
#include <emscripten.h>
#include <math.h>

extern "C" {

EMSCRIPTEN_EXPORT int int_sqrt(int x) {
  return sqrt(x);
}

}

