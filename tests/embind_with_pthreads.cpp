#include <assert.h>

#include <string>

#include <emscripten.h>
#include <emscripten/val.h>

using namespace emscripten;

int main() {
  EM_ASM({
    globalProperty = {
      foo: function() { return "bar" }
    };
  });
  val globalProperty = val::global("globalProperty");
  auto result = globalProperty.call<std::string>("foo");
  REPORT_RESULT(result == "bar");
}

