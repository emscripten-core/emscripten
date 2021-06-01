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

void fail()
{
  cout << "fail\n";
}

void pass()
{
  cout << "pass\n";
}

void test(string message)
{
  cout << "test:\n" << message << "\n";
}

void ensure(bool value)
{
  if (value)
    pass();
  else
    fail();
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

void ensure_js(string js_code)
{
  js_code.append(";");
  const char* js_code_pointer = js_code.c_str();
  ensure(EM_ASM_INT({
    var js_code = UTF8ToString($0);
    return eval(js_code);
  }, js_code_pointer));
}

void ensure_js_throws(string js_code, string error_type)
{
  js_code.append(";");
  const char* js_code_pointer = js_code.c_str();
  const char* error_type_pointer = error_type.c_str();
  ensure(EM_ASM_INT({
    var js_code = UTF8ToString($0);
    var error_type = UTF8ToString($1);
    try {
      eval(js_code);
    }
    catch(error_thrown)
    {
      return error_thrown.name === error_type;
    }
    return false;
  }, js_code_pointer, error_type_pointer));
}

EMSCRIPTEN_BINDINGS(tests) {
  register_vector<int64_t>("Int64Vector");
  register_vector<uint64_t>("UInt64Vector");
}

int main()
{
  const int64_t max_int64_t = numeric_limits<int64_t>::max();
  const int64_t min_int64_t = numeric_limits<int64_t>::min();
  const uint64_t max_uint64_t = numeric_limits<uint64_t>::max();

  printf("start\n");

  test("vector<int64_t>");
  val::global().set("v64", val(vector<int64_t>{1, 2, 3, -4}));
  ensure_js("v64.get(0) === 1n");
  ensure_js("v64.get(1) === 2n");
  ensure_js("v64.get(2) === 3n");
  ensure_js("v64.get(3) === -4n");

  execute_js("v64.push_back(1234n)");
  ensure_js("v64.size() === 5");
  ensure_js("v64.get(4) === 1234n");

  test("vector<int64_t> Cannot convert number to int64_t");
  ensure_js_throws("v64.push_back(1234)", "TypeError");

  test("vector<int64_t> Cannot convert bigint that is too big");
  ensure_js_throws("v64.push_back(12345678901234567890123456n)", "TypeError");

  test("vector<uint64_t>");
  val::global().set("vU64", val(vector<uint64_t>{1, 2, 3, 4}));
  ensure_js("vU64.get(0) === 1n");
  ensure_js("vU64.get(1) === 2n");
  ensure_js("vU64.get(2) === 3n");
  ensure_js("vU64.get(3) === 4n");

  execute_js("vU64.push_back(1234n)");
  ensure_js("vU64.size() === 5");
  ensure_js("vU64.get(4) === 1234n");

  test("vector<uint64_t> Cannot convert number to uint64_t");
  ensure_js_throws("vU64.push_back(1234)", "TypeError");

  test("vector<uint64_t> Cannot convert bigint that is too big");
  ensure_js_throws("vU64.push_back(12345678901234567890123456n)", "TypeError");

  test("vector<uint64_t> Cannot convert bigint that is negative");
  ensure_js_throws("vU64.push_back(-1n)", "TypeError");

  printf("end\n");
  return 0;
}
