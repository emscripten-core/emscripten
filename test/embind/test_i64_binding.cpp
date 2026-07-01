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

void test(string message)
{
  printf("test: %s\n", message.c_str());
}

void assert_js_eq(string X, string Y) {
  string js_code;
  js_code += "const x = " + X + ";";
  js_code += "const y = " + Y + ";";
  js_code += "assert(x === y, `" + X + ": actual = ${typeof x} ${x}, expected = ${typeof y} ${y}`);";
  emscripten_run_script(js_code.c_str());
}

EMSCRIPTEN_BINDINGS(tests) {
  emscripten::function("int64_min", &numeric_limits<int64_t>::min);
  emscripten::function("int64_max", &numeric_limits<int64_t>::max);
  emscripten::function("uint64_max", &numeric_limits<uint64_t>::max);

  register_vector<int64_t>("Int64Vector");
  register_vector<uint64_t>("UInt64Vector");
}

extern "C" void ensure_js_throws_with_assertions_enabled(const char* js_code, const char* error_type);

int main()
{
  test("limits");

  assert_js_eq("Module.int64_min()", to_string(numeric_limits<int64_t>::min()) + "n");
  assert_js_eq("Module.int64_max()", to_string(numeric_limits<int64_t>::max()) + "n");
  assert_js_eq("Module.uint64_max()", to_string(numeric_limits<uint64_t>::max()) + "n");

  printf("start\n");

  test("vector<int64_t>");
  val myval(std::vector<int64_t>{1, 2, 3, -4});
  val::global().set("v64", myval);
  assert_js_eq("v64.get(0)", "1n");
  assert_js_eq("v64.get(1)", "2n");
  assert_js_eq("v64.get(2)", "3n");
  assert_js_eq("v64.get(3)", "-4n");

  emscripten_run_script("v64.push_back(1234n)");
  assert_js_eq("v64.size()", "5");
  assert_js_eq("v64.get(4)", "1234n");

  test("vector<int64_t> Cannot convert bigint that is too big");
  ensure_js_throws_with_assertions_enabled("v64.push_back(12345678901234567890123456n)", "TypeError");

  test("vector<uint64_t>");
  val myval2(vector<uint64_t>{1, 2, 3, 4});
  val::global().set("vU64", myval2);
  assert_js_eq("vU64.get(0)", "1n");
  assert_js_eq("vU64.get(1)", "2n");
  assert_js_eq("vU64.get(2)", "3n");
  assert_js_eq("vU64.get(3)", "4n");

  emscripten_run_script("vU64.push_back(1234n)");
  assert_js_eq("vU64.size()", "5");
  assert_js_eq("vU64.get(4)", "1234n");

  emscripten_run_script("vU64.push_back(1234)");
  assert_js_eq("vU64.size()", "6");
  assert_js_eq("vU64.get(5)", "1234n");

  test("vector<uint64_t> Cannot convert bigint that is too big");
  ensure_js_throws_with_assertions_enabled("vU64.push_back(12345678901234567890123456n)", "TypeError");

  test("vector<uint64_t> Cannot convert bigint that is negative");
  ensure_js_throws_with_assertions_enabled("vU64.push_back(-1n)", "TypeError");

  myval.call<void>("delete");
  myval2.call<void>("delete");
  printf("end\n");

  return 0;
}
