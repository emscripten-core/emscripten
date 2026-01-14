#include <assert.h>

#include <string>

#include <emscripten.h>
#include <emscripten/val.h>

using namespace emscripten;

int main() {
  val fetch = val::global("fetch");
  std::string url = "data:text/plain,foo";
  val async_response = fetch(url);
  val response = async_response.await();
  val async_text = response.call<val>("text");
  std::string text = async_text.await().as<std::string>();
  assert(text == "foo");
  // This explicit exit() should not be necessary here.
  // TODO(https://github.com/emscripten-core/emscripten/issues/26093)
  exit(0);
  return 0;
}
