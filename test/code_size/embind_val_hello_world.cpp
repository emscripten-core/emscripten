#include <emscripten/val.h>

using emscripten::val;

int main() {
  val::global("console").call<void>("log", val("Hello world! The answer is"), 42);
}
