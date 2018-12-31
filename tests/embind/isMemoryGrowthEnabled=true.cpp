#include <emscripten/bind.h>

using namespace emscripten;

EMSCRIPTEN_BINDINGS(settings) {
    constant("isMemoryGrowthEnabled", true);
}
