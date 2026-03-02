#include <cassert>
#include <cstddef>
#include <cstring>
#include <functional>
#include <stdexcept>
#include <string>
#include <typeinfo>
#include <utility>

#include <emscripten.h>
#include <emscripten/bind.h>

namespace {

#if __cplusplus >= 202002L
using identity = std::identity;
#else
struct identity {
  template<typename T>
  constexpr T&& operator()(T&& t) const noexcept { return std::forward<T>(t); }
};
#endif

enum class Enum1: int { E1 = 1 };

enum class Enum2: int { E2 = 2 };

struct Struct1 { int n; };

struct Struct2 { double d; };

class Any {
public:
  enum Kind { KIND_ENUM, KIND_STRUCT };

  Any(std::type_info const& type, int enum_value):
    kind_(KIND_ENUM), type_(&type), enum_value_(enum_value) {}

  Any(std::type_info const& type, void const* struct_pointer):
    kind_(KIND_STRUCT), type_(&type), struct_pointer_(copyStruct(type, struct_pointer)) {}

  Any(Any const& other): kind_(other.kind_), type_(other.type_) {
    switch (kind_) {
    case KIND_ENUM:
      enum_value_ = other.enum_value_;
      break;
    case KIND_STRUCT:
      struct_pointer_ = copyStruct(*other.type_, other.struct_pointer_);
      break;
    }
  }

  ~Any() { release(); }

  Any& operator=(Any const& other) {
    if (&other != this) {
      release();
      kind_ = other.kind_;
      type_ = other.type_;
      switch (kind_) {
      case KIND_ENUM:
        enum_value_ = other.enum_value_;
        break;
      case KIND_STRUCT:
        struct_pointer_ = copyStruct(*other.type_, other.struct_pointer_);
        break;
      }
    }
    return *this;
  }

  template<typename T> T get() const;

  template<> Enum1 get() const {
    verify(KIND_ENUM, typeid(Enum1));
    return static_cast<Enum1>(enum_value_);
  }

  template<> Enum2 get() const {
    verify(KIND_ENUM, typeid(Enum2));
    return static_cast<Enum2>(enum_value_);
  }

  template<> Struct1 get() const {
    verify(KIND_STRUCT, typeid(Struct1));
    return *static_cast<Struct1 const*>(struct_pointer_);
  }

  template<> Struct2 get() const {
    verify(KIND_STRUCT, typeid(Struct2));
    return *static_cast<Struct2 const*>(struct_pointer_);
  }

  static Any jsNew(std::string id, emscripten::val const& value) {
    Kind kind;
    std::type_info const* type;
    if (id == "Enum1") {
      kind = KIND_ENUM;
      type = &typeid(Enum1);
    } else if (id == "Enum2") {
      kind = KIND_ENUM;
      type = &typeid(Enum2);
    } else if (id == "Struct1") {
      kind = KIND_STRUCT;
      type = &typeid(Struct1);
    } else if (id == "Struct2") {
      kind = KIND_STRUCT;
      type = &typeid(Struct2);
    } else {
      throw std::runtime_error("unknown type ID " + id);
    }
    switch (kind) {
    case KIND_ENUM:
      return Any(*type, value.dyn_as<int>(*type, identity()));
    case KIND_STRUCT:
      return value.dyn_as<void const*>(*type, [&type](void const* p) { return Any(*type, p); });
    }
    assert(false);
  }

  emscripten::val jsGet() {
    switch (kind_) {
    case Any::KIND_ENUM:
      return emscripten::val::dyn(*type_, enum_value_);
    case Any::KIND_STRUCT:
      return emscripten::val::dyn(*type_, copyStruct(*type_, struct_pointer_));
    }
    assert(false);
  }

private:
  static void const* copyStruct(std::type_info const& type, void const* struct_pointer) {
    std::size_t n;
    if (type == typeid(Struct1)) {
      n = sizeof(Struct1);
    } else if (type == typeid(Struct2)) {
      n = sizeof(Struct2);
    } else {
      assert(false);
    }
    return std::memcpy(new unsigned char[n], struct_pointer, n);
  }

  void verify(Kind kind, std::type_info const& type) const {
    assert(kind_ == kind);
    assert(*type_ == type);
  }

  void release() {
    if (kind_ == KIND_STRUCT) {
      delete[] static_cast<unsigned char const*>(struct_pointer_);
    }
  }

  Kind kind_;
  std::type_info const* type_;
  union {
    int enum_value_;
    void const* struct_pointer_;
  };
};

}

int main() {
  emscripten::enum_<Enum1>("Enum1").value("E1", Enum1::E1);
  emscripten::enum_<Enum2>("Enum2").value("E2", Enum2::E2);
  emscripten::value_object<Struct1>("Struct1").field("n", &Struct1::n);
  emscripten::value_object<Struct2>("Struct2").field("d", &Struct2::d);
  emscripten::class_<Any>("Any").constructor(Any::jsNew).function("get", &Any::jsGet);
  EM_ASM(
    globalThis.AnyEnum1 = new Module.Any("Enum1", Module.Enum1.E1);
    globalThis.AnyEnum2 = new Module.Any("Enum2", Module.Enum2.E2);
    globalThis.AnyStruct1 = new Module.Any("Struct1", {n: 1});
    globalThis.AnyStruct2 = new Module.Any("Struct2", {d: 1.5});
    globalThis.testGetEnum1 = () => globalThis.AnyEnum1.get() === Module.Enum1.E1;
    globalThis.testGetEnum2 = () => globalThis.AnyEnum2.get() === Module.Enum2.E2;
    globalThis.testGetStruct1 = () => globalThis.AnyStruct1.get().n === 1;
    globalThis.testGetStruct2 = () => globalThis.AnyStruct2.get().d === 1.5;
  );
  assert(emscripten::val::global("AnyEnum1").as<Any>().get<Enum1>() == Enum1::E1);
  assert(emscripten::val::global("AnyEnum2").as<Any>().get<Enum2>() == Enum2::E2);
  assert(emscripten::val::global("AnyStruct1").as<Any>().get<Struct1>().n == 1);
  assert(emscripten::val::global("AnyStruct2").as<Any>().get<Struct2>().d == 1.5);
  assert(emscripten::val::global().call<bool>("testGetEnum1"));
  assert(emscripten::val::global().call<bool>("testGetEnum2"));
  assert(emscripten::val::global().call<bool>("testGetStruct1"));
  assert(emscripten::val::global().call<bool>("testGetStruct2"));
}
