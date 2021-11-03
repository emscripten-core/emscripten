#include <emscripten/val.h>

using emscripten::val;

int main() {
  val obj = val::object();
  // Code like below doesn't work like the user might expect.
  // `obj[...] = ...` only modifies the on-stack `val` binding, whereas user probably wanted to set the property on the actual JS object.
  // The correct way to set the property is via `val::set()` instead, like `obj.set("foo", 42)`.
  // Test here that we help catch this mistake and produce a compilation error for the line below.
  obj["foo"] = val(42);
}
