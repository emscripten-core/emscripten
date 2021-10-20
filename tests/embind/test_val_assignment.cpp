#include <emscripten/val.h>

using emscripten::val;

int main() {
  val obj = val::object();
  obj["foo"] = val(42);
}
