#include <assert.h>
#include <emscripten.h>
#include <emscripten/val.h>
#include <string>
#include <thread>

using namespace emscripten;

void thread(void) {
  EM_ASM({
    globalProperty = {
      foo: function(value) {
        return value;
      }
    };
  });
  val globalProperty = val::global("globalProperty");
  auto result0 = globalProperty.call<val>("foo", val("bar"));
  assert(result0.as<std::string>() == "bar");
}

int main() {
  std::thread t(thread);
  std::thread t1(thread);
  t.join();
  t1.join();
  return 0;
}
