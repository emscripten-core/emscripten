// Copyright 2021 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <stdio.h>
#include <iostream>
#include <cmath>
#include <emscripten/bind.h>
#include <emscripten/emscripten.h>
#include <emscripten/val.h>

using namespace emscripten;
using namespace std;

#define assert_js(X) assert(run_js(X))

void test(string message)
{
  cout << "test:\n" << message << "\n";
}


void execute_js(string js_code)
{
  js_code.append(";");
  const char* js_code_pointer = js_code.c_str();
  EM_ASM_INT({
    var js_code = UTF8ToString($0);
    return eval(js_code);
  }, js_code_pointer);
}

int run_js(string js_code)
{
  js_code.append(";");
  const char* js_code_pointer = js_code.c_str();
  return EM_ASM_INT({
    var js_code = UTF8ToString($0);
    return eval(js_code);
  }, js_code_pointer);
}

EMSCRIPTEN_BINDINGS(tests) {
  register_vector<int64_t>("Int64Vector");
  register_vector<uint64_t>("UInt64Vector");
}

extern "C" void ensure_js_throws_with_assertions_enabled(const char* js_code, const char* error_type);

int main()
{
  const int64_t max_int64_t = numeric_limits<int64_t>::max();
  const int64_t min_int64_t = numeric_limits<int64_t>::min();
  const uint64_t max_uint64_t = numeric_limits<uint64_t>::max();

  printf("start\n");

  test("vector<int64_t>");
  val myval(std::vector<int64_t>{1, 2, 3, -4});
  val::global().set("v64", myval);
  assert_js("v64.get(0) === 1n");
  assert_js("v64.get(1) === 2n");
  assert_js("v64.get(2) === 3n");
  assert_js("v64.get(3) === -4n");

  execute_js("v64.push_back(1234n)");
  assert_js("v64.size() === 5");
  assert_js("v64.get(4) === 1234n");

  test("vector<int64_t> Cannot convert bigint that is too big");
  ensure_js_throws_with_assertions_enabled("v64.push_back(12345678901234567890123456n)", "TypeError");

  test("vector<uint64_t>");
  val myval2(vector<uint64_t>{1, 2, 3, 4});
  val::global().set("vU64", myval2);
  assert_js("vU64.get(0) === 1n");
  assert_js("vU64.get(1) === 2n");
  assert_js("vU64.get(2) === 3n");
  assert_js("vU64.get(3) === 4n");

  execute_js("vU64.push_back(1234n)");
  assert_js("vU64.size() === 5");
  assert_js("vU64.get(4) === 1234n");

  execute_js("vU64.push_back(1234)");
  assert_js("vU64.size() === 6");
  assert_js("vU64.get(5) === 1234n");

  test("vector<uint64_t> Cannot convert bigint that is too big");
  ensure_js_throws_with_assertions_enabled("vU64.push_back(12345678901234567890123456n)", "TypeError");

  test("vector<uint64_t> Cannot convert bigint that is negative");
  ensure_js_throws_with_assertions_enabled("vU64.push_back(-1n)", "TypeError");

  myval.call<void>("delete");
  myval2.call<void>("delete");
  printf("end\n");

  return 0;
}
