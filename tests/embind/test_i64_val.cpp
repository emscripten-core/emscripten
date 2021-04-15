// Copyright 2018 The Emscripten Authors.  All rights reserved.
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

void ensure_not(bool value)
{
  ensure(!value);
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

void ensure_js_not(string js_code)
{
  js_code.insert(0, "!( ");
  js_code.append(" )");
  ensure_js(js_code);
}

void throw_js_error(val js_error)
{
  js_error.throw_();
}

EMSCRIPTEN_BINDINGS(test_bindings)
{
  emscripten::function("throw_js_error", &throw_js_error);
}

int main()
{
  printf("start\n");

  test("val(int64_t v)");
  val::global().set("a", val(int64_t(1234)));
  ensure_js("a === 1234n");

  val::global().set("a", val(int64_t(-4321)));
  ensure_js("a === -4321n");

  val::global().set("a", val(int64_t(0x12345678aabbccddL)));
  ensure_js("a === 1311768467732155613n");

  printf("end\n");
  return 0;
}
