#include <emscripten/bind.h>
#include <emscripten/val.h>
int main() {
  void* const x = nullptr;
  emscripten::val v(x); // void* const
}
