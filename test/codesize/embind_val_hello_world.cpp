#include <emscripten/val.h>
#include <string>

using emscripten::val;

int main() {
  val::global("console").call<void>("log", std::string("Hello world! The answer is"), 42);
}
