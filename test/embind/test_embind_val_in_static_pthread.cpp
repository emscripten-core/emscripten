#include <emscripten/val.h>
#include <emscripten/emscripten.h>
#include <thread>

auto BGThread = std::thread([]() {
  auto globalThis = emscripten::val::global("globalThis");
  assert(globalThis.typeOf().as<std::string>() == "object");
  printf("worker\n");
});

int main() {
  BGThread.join();
  printf("done\n");
}
