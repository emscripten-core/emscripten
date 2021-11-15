#include <emscripten.h>
#include <stdio.h>
#include <vector>

std::vector<int> x;

EMSCRIPTEN_KEEPALIVE extern "C" void allocate_too_much() {
  puts("allocating more than TOTAL_MEMORY; this will fail.");
  x.resize(20 * 1024 * 1024);
  puts("oh no, it didn't fail!");
}

int main() {
  EM_ASM({
    // Catch the failure here so we can report it.
    try {
      _allocate_too_much();
      out("no abort happened");
    } catch (e) {
      assert(("" + e).indexOf("Aborted") >= 0, "expect an abort from new");
      out("new aborted as expected");
    }
  });
}
