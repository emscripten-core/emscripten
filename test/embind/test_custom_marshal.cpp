// Copyright 2019 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <stdio.h>
#include <emscripten.h>
#include <emscripten/bind.h>
#include <emscripten/val.h>
#include <type_traits>
#include <string>

using namespace emscripten;

// Class which wraps int and have no explicit or implicit conversions.
struct IntWrapper {
  int get() const {
    return value;
  }
  static IntWrapper create(int v) {
    return IntWrapper(v);
  }

private:
  explicit IntWrapper(int v) : value(v) {}
  int value;
};

// Need for SFINAE-based specialization testing.
template<typename T> struct IsIntWrapper : std::false_type {};
template<> struct IsIntWrapper<IntWrapper> : std::true_type {};

// We will pass IntWrapper between C++ and JavaScript via IntWrapperIntermediate
// which should be already registered by embind on both C++ and JavaScript sides.
// That way we can write C++ conversions only and use standard JS conversions.
using IntWrapperIntermediate = int;

// Specify custom (un)marshalling for all types satisfying IsIntWrapper.
namespace emscripten {
namespace internal {
// remove_cv/remove_reference are required for TypeID, but not BindingType, see https://github.com/emscripten-core/emscripten/issues/7292
template<typename T>
struct TypeID<T, typename std::enable_if<IsIntWrapper<typename std::remove_cv<typename std::remove_reference<T>::type>::type>::value, void>::type> {
  static constexpr TYPEID get() {
    return TypeID<IntWrapperIntermediate>::get();
  }
};

template<typename T>
struct BindingType<T, typename std::enable_if<IsIntWrapper<T>::value, void>::type> {
  typedef typename BindingType<IntWrapperIntermediate>::WireType WireType;

  constexpr static WireType toWireType(const T& v, rvp::default_tag) {
    return BindingType<IntWrapperIntermediate>::toWireType(v.get(), rvp::default_tag{});
  }
  constexpr static T fromWireType(WireType v) {
    return T::create(BindingType<IntWrapperIntermediate>::fromWireType(v));
  }
};
} // namespace internal
} // namespace emscripten

template<typename T>
void test() {
  IntWrapper x = IntWrapper::create(10);
  val js_func = val::module_property("js_func");
  IntWrapper y = js_func(val(std::forward<T>(x))).as<IntWrapper>();
  printf("C++ got %d\n", y.get());
}

int main(int argc, char **argv) {
  test<IntWrapper>();
  test<IntWrapper&>();
  test<const IntWrapper>();
  test<const IntWrapper&>();
  return 0;
}
