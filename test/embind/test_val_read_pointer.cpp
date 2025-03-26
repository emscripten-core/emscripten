#include <emscripten/bind.h>
#include <emscripten.h>

using namespace emscripten;

struct ValueObject {
  ValueObject(): value(43) {};
  int value;
};

struct ValueArray {
  ValueArray(): x(44) {};
  int x;
};

struct Foo {
  Foo(): value(45) {};
  int value;
};

enum Enum { ONE, TWO };

EMSCRIPTEN_BINDINGS(xxx) {
  value_object<ValueObject>("ValueObject")
    .field("value", &ValueObject::value);
  
  value_array<ValueArray>("ValueArray")
    .element(&ValueArray::x);
  
  class_<Foo>("Foo")
    .property("value", &Foo::value);

  enum_<Enum>("Enum")
    .value("ONE", ONE)
    .value("TWO", TWO);
}


int main() {
  EM_ASM(
    globalThis["passthrough"] = (arg) => {
      return arg;
    };
    globalThis["passthroughValueObject"] = (arg) => {
      return arg.value;
    };
    globalThis["passthroughValueArray"] = (arg) => {
      return arg[0];
    };
    globalThis["passthroughClass"] = (arg) => {
      const value = arg.value;
      arg.delete();
      return value;
    };
    globalThis["passthroughMemoryView"] = (arg) => {
      return arg[2];
    };
  );

  // These tests execute all the various readValueFromPointer functions for each
  // of the different binding types.
  assert(val::global().call<bool>("passthrough", true));
  assert(val::global().call<int>("passthrough", 42) == 42);
  assert(val::global().call<double>("passthrough", 42.2) == 42.2);
  ValueObject vo;
  assert(val::global().call<int>("passthroughValueObject", vo) == 43);
  ValueArray va;
  assert(val::global().call<int>("passthroughValueArray", va) == 44);
  Foo foo;
  assert(val::global().call<int>("passthroughClass", foo) == 45);
  assert(val::global().call<std::string>("passthrough", val("abc")) == "abc");
  std::string str = "hello";
  assert(val::global().call<std::string>("passthrough", str) == "hello");
  std::wstring wstr = L"abc";
  assert(val::global().call<std::wstring>("passthrough", wstr) == L"abc");
  static const int data[] = {0, 1, 2};
  assert(val::global().call<int>("passthroughMemoryView", typed_memory_view(3, data)) == 2);
  assert(val::global().call<Enum>("passthrough", ONE) == ONE);

  return 0;
}
