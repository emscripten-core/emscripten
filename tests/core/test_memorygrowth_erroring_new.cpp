#include <emscripten.h>
#include <stdio.h>
#include <vector>

EMSCRIPTEN_KEEPALIVE extern "C" void allocate_too_much() {
  std::vector<int> x;
  puts("allocating more than TOTAL_MEMORY; this will fail.");
  x.resize(20 * 1024 * 1024);
  puts("oh no, it didn't fail!");
}

int main() {
  EM_ASM({
    // Catch the failure here so we can report it.
    try {
      _allocate_too_much();
      out("no error happened");
    } catch (e) {
      assert(("" + e).indexOf("abort") >= 0, "expect an abort");
      out("an expected error occurred");
    }
  });
}
