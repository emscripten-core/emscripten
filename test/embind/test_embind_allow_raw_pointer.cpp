#include <emscripten/bind.h>

using namespace emscripten;

class C {};

void onePointerArg(C* ptr) {}
void twoPointerArg(C* ptr1, C* ptr2) {}
void sandwich(int a, C* ptr1, int b) {}

C* pointerRet() { return nullptr; }
C* pointerRetPointerArg(C* ptr) { return nullptr; }

EMSCRIPTEN_BINDINGS(raw_pointers) {
  class_<C>("C");
  function("onePointerArg", &onePointerArg, allow_raw_pointer<arg<0>>());
  function("twoPointerArg", &twoPointerArg, allow_raw_pointer<arg<0>>(), allow_raw_pointer<arg<1>>());
  function("sandwich", &sandwich, allow_raw_pointer<arg<1>>());
  function("pointerRet", &pointerRet, allow_raw_pointer<ret_val>());
  function("pointerRetPointerArg", &pointerRetPointerArg, allow_raw_pointer<ret_val>(), allow_raw_pointer<arg<0>>());
}
