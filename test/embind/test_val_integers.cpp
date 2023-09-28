// Copyright 2023 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <emscripten.h>
#include <emscripten/val.h>
#include <iostream>
#include <limits>

using namespace emscripten;

bool check_num_on_js_side(std::string value_s) {
  std::string code = std::string("num === ") + value_s + " || console.error(`\t\tFailed: expected " + value_s + ", got ${num}`)";
  return emscripten_run_script_int(code.c_str()) == 1;
}

template <typename T>
bool check_num_on_cpp_side(T expected) {
  T value = emscripten::val::global("num").as<T>();
  if (value != expected) {
    std::cerr << "\t\tFailed: expected " << expected << ", got " << value << std::endl;
    return false;
  }
  return true;
}

template <typename T>
bool test_num(const char *type_name, T value) {
  std::string value_s = std::to_string(value);
  // 64-bit integer must be represented as a BigInt which uses `n` suffix
  if (sizeof(T) > 4) {
    value_s.push_back('n');
  }
  bool ok = true;
  std::cout << "Testing (" << type_name << ") " << value_s << std::endl;
  // Test C++ to JS conversion
  std::cout << "\tC++ to JS via assignment" << std::endl;
  emscripten::val::global().set("num", value);
  ok &= check_num_on_js_side(value_s);
  // Test C++ to JS conversion via function call.
  // Calls use their own argument serialization, so it's best to ensure that works too.
  std::cout << "\tC++ to JS via call" << std::endl;
  emscripten::val::global("setNum")(value);
  ok &= check_num_on_js_side(value_s);
  // Test JS to C++ conversion
  std::cout << "\tJS to C++" << std::endl;
  ok &= check_num_on_cpp_side(value);
  return ok;
}

template <typename T>
bool test_num_limits(const char *type_name) {
  // note: using `&` instead of `&&` to ensure both tests are run
  return test_num<T>(type_name, std::numeric_limits<T>::min()) &
         test_num<T>(type_name, std::numeric_limits<T>::max());
}

#define TEST_NUM_LIMITS(T) ok &= test_num_limits<T>(#T)

int main() {
  bool ok = true;

  EM_ASM({
    globalThis.setNum = num => globalThis.num = num;
  });

  TEST_NUM_LIMITS(char);
  TEST_NUM_LIMITS(unsigned char);
  TEST_NUM_LIMITS(signed char);

  TEST_NUM_LIMITS(short);
  TEST_NUM_LIMITS(unsigned short);

  TEST_NUM_LIMITS(int);
  TEST_NUM_LIMITS(unsigned int);

  TEST_NUM_LIMITS(long);
  TEST_NUM_LIMITS(unsigned long);

#ifdef WASM_BIGINT
  TEST_NUM_LIMITS(long long);
  TEST_NUM_LIMITS(unsigned long long);
#endif

  return !ok;
}
