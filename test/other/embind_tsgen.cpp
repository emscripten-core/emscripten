#include <memory>
#include <string>
#include <emscripten/bind.h>

using namespace emscripten;

class Test {
 public:
  int function_one(char x, int y) { return 42; }
  int function_two(unsigned char x, int y) { return 43; }
  int function_three(const std::string&) { return 1; }
  int function_four(bool x) { return 2; }

  int const_fn() const { return 0; }
};
int global_fn(int, int) { return 0; }

EMSCRIPTEN_BINDINGS(Test) {
  class_<Test>("Test")
      .function("functionOne", &Test::function_one)
      .function("functionTwo", &Test::function_two)
      .function("functionThree", &Test::function_three)
      .function("functionFour", &Test::function_four)
      .function("constFn", &Test::const_fn)
	;

  function("global_fn", &global_fn);
}

int main() {
  // Main should not be run during TypeScript generation.
  abort();
  return 0;
}
