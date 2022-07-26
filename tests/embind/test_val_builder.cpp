
#include <array>
#include <emscripten.h>
#include <emscripten/val_builder.h>
#include <iostream>
#include <stdio.h>
#include <string>
#include <vector>

using namespace emscripten;

using VB = emscripten::ValBuilder<32>;

void fail() { std::cout << "fail\n"; }

void pass() { std::cout << "pass\n"; }

void test(std::string message) { std::cout << "test:\n" << message << "\n"; }

void ensure(bool value) {
  if (value)
    pass();
  else
    fail();
}

void ensure_not(bool value) { ensure(!value); }

void ensure_js(std::string js_code) {
  ensure(emscripten_run_script_int(js_code.c_str()));
}

void ensure_js_not(std::string js_code) {
  js_code.insert(0, "!( ");
  js_code.append(" )");
  ensure_js(js_code);
}

namespace tests {

const char LONG_S[] =
    "Class Template Argument Deduction (CTAD) is a C++17 Core Language feature "
    "that reduces code verbosity. C++17’s Standard Library also supports CTAD, "
    "so after upgrading your toolset, you can take advantage of this new "
    "feature when using STL types like std::pair and std::vector. Class "
    "templates in other libraries and your own code will partially benefit "
    "from CTAD automatically, but sometimes they’ll need a bit of new code "
    "(deduction guides) to fully benefit. Fortunately, both using CTAD and "
    "providing deduction guides is pretty easy, despite template "
    "metaprogramming’s fearsome reputation!";

const int BIG_A[] = {
    0x01, 0x61, 0x01, 0x63, 0x00, 0x07, 0x01, 0x61, 0x01, 0x64, 0x00, 0x01,
    0x01, 0x61, 0x01, 0x65, 0x00, 0x01, 0x01, 0x61, 0x01, 0x66, 0x00, 0x0C,
    0x01, 0x61, 0x01, 0x67, 0x00, 0x02, 0x01, 0x61, 0x01, 0x68, 0x00, 0x04,
    0x01, 0x61, 0x01, 0x69, 0x00, 0x01, 0x01, 0x61, 0x01, 0x6A, 0x00, 0x01,
    0x01, 0x61, 0x06, 0x6D, 0x65, 0x6D, 0x6F, 0x72, 0x79, 0x02, 0x01, 0x80,
    0x02, 0x80, 0x80, 0x02, 0x01, 0x61, 0x05, 0x74, 0x61, 0x62, 0x6C, 0x65,
    0x01, 0x70, 0x00, 0xF7, 0x02, 0x03, 0xBF, 0x06, 0xBD, 0x06, 0x00, 0x08,
    0x06, 0x06, 0x04, 0x06, 0x06, 0x58, 0x0B, 0x00, 0x1F, 0x16, 0x5A, 0x02,
    0x13, 0x02, 0x0F, 0x02, 0x02, 0x01, 0x16, 0x04, 0x02, 0x0B, 0x3C, 0x14,
    0x02, 0x29, 0x02, 0x54, 0x0C, 0x06, 0x18, 0x1C, 0x02, 0x04, 0x2F, 0x14,
    0x28, 0x01, 0x21, 0x00, 0x2C, 0x0C, 0x16, 0x2C, 0x0C, 0x02, 0x15, 0x04,
    0x06, 0x2F, 0x0C, 0x00, 0x06, 0x32, 0x02, 0x01, 0x02, 0x06, 0x1B, 0x22,
    0x0F, 0x28, 0x04, 0x49, 0x18, 0x1C, 0x06, 0x10, 0x42, 0x14, 0x02, 0x11,
    0x00, 0x01, 0x02, 0x01, 0x0F, 0x04, 0x23, 0x29, 0x53, 0x00, 0x01, 0x14,
    0x04, 0x01, 0x0F, 0x13, 0x09, 0x17, 0x0B, 0x2D, 0x3F, 0x06, 0x02, 0x00,
    0x56, 0x11, 0x02, 0x14, 0x35, 0x06, 0x04, 0x06, 0x40, 0x0C, 0x55, 0x20,
    0x06, 0x09, 0x09, 0x04, 0x02, 0x2B, 0x11, 0x2B, 0x19, 0x05, 0x18, 0x19,
    0x22, 0x27, 0x04, 0x50, 0x04, 0x0F, 0x01, 0x01, 0x13, 0x3A, 0x18, 0x10,
    0x28, 0x00, 0x31, 0x07, 0x22, 0x01, 0x00, 0x02, 0x02, 0x05, 0x00, 0x03,
    0x02, 0x01, 0x07, 0x36, 0x1A, 0x1B, 0x0C, 0x01, 0x04, 0x04, 0x19, 0x0C,
    0x00, 0x18, 0x00, 0x13, 0x47, 0x19, 0x52, 0x3D, 0x02, 0x18, 0x5C, 0x15,
    0x0D, 0x08, 0x00, 0x00, 0x07, 0x04, 0x02, 0x23, 0x01, 0x01, 0x16, 0x06,
    0x04, 0x02, 0x02, 0x24, 0x24, 0x0D, 0x01, 0x00, 0x34, 0x14, 0x0B, 0x19,
    0x21, 0x02, 0x05, 0x01, 0x45, 0x00, 0x00, 0x03, 0x06, 0x02, 0x1A, 0x07,
    0x07, 0x51, 0x1C, 0x01, 0x04, 0x06, 0x23, 0x13, 0x04, 0x46, 0x06, 0x01,
    0x01, 0x0C, 0x02, 0x23, 0x0C, 0x02, 0x04, 0x03, 0x00, 0x0C, 0x02, 0x01,
    0x01, 0x13, 0x09, 0x01, 0x0C, 0x0C, 0x01, 0x02, 0x48, 0x11, 0x4C, 0x1E,
    0x0A, 0x19, 0x00, 0x44, 0x57, 0x03, 0x33, 0x01, 0x25, 0x2D, 0x33, 0x24,
    0x03, 0x43, 0x15, 0x0F, 0x18, 0x1B, 0x13, 0x09, 0x04, 0x00, 0x1C, 0x1A,
};

void test_cases() {
  printf("start\n");

  test("add() overloads");
  EM_ASM(
    a = [];
  );
  VB va(val::global("a"));
  va.add(1);
  va.add(1.1);
  va.add((char)3);
  va.add((uint8_t)4);
  va.add((uint32_t)-1);
  va.add(1.0f);
  va.add((float)1.1);
  va.finalize();
  ensure_js("a[0] == 1");
  ensure_js("a[1] == 1.1");
  ensure_js("a[2] == 3");
  ensure_js("a[3] == 4");
  ensure_js("a[4] == 4294967295");  // (unsigned)-1
  ensure_js("a[5] == 1.0");
  ensure_js("parseFloat(a[6]).toFixed(1) == 1.1");

  va.add("hello");
  va.add(std::string("world"));
  va.add(val::array());
  va.add(val(10));
  va.finalize();
  ensure_js("a[7] == 'hello'");
  ensure_js("a[8] == 'world'");
  ensure_js("Array.isArray(a[9]) && a[9].length == 0");
  ensure_js("a[10] == 10");

  test("add(array)");
  EM_ASM(
    a = [];
  );
  va.reset_val(val::global("a"));
  std::vector<int> vec{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
  va.add(vec);
  va.add(std::vector<int>{5, 6, 7, 8, 9});
  va.add(std::vector<std::string>{"1", "2", "3"});
  std::vector<std::string> vs{"4", "5", "6"};
  va.add(vs);
  std::array<int, 5> ar{1, 2, 3, 4, 5};
  va.add(ar);
  std::array<std::string, 3> ar1{"4", "99"};
  va.add(ar1);
  char ar2[]{6, 7, 8};
  va.add(ar2);
  va.finalize();
  ensure_js("a[0].length == 16 && a[0][0] == 1 && a[0][2] == 3 && a[0][15] == 16");
  ensure_js("a[1].length == 5 && a[1][0] == 5 && a[1][1] == 6 && a[1][5] == undefined");
  ensure_js("a[2].length == 3 && a[2][0] == '1' && a[2][1] == '2' && a[2][2] == '3' ");
  ensure_js("a[3].length == 3 && a[3][0] == '4' && a[3][1] == '5' && a[3][2] == '6' ");
  ensure_js("a[4].length == 5 && a[4][0] == 1 && a[4][1] == 2 && a[4][4] == 5");
  ensure_js("a[5].length == 3 && a[5][0] == '4' && a[5][1] == '99' && a[5][2] == ''");
  ensure_js("a[6].length == 3 && a[6][0] == 6 && a[6][1] == 7 && a[6][2] == 8");

  test("concat(array&)");
  EM_ASM(
    a = [];
  );
  va.reset_val(val::global("a"));
  va.add(0);
  va.concat(vec);
  va.finalize();
  ensure_js("a[0] == 0");
  ensure_js("a[1] == 1 && a[2] == 2 && a[3] == 3 && a[16] == 16");
  ensure_js("a[17] === undefined");
  ensure_js("a.length == 17");

  va.concat(std::vector<std::string>{"1", "2", "3", "4", "5", "6"});                                                                                                                                                                                                                                                                         
  va.concat(std::vector<float>{1.1, 1.2, 1.3});
  va.concat(std::vector<short>{1, 2, 3});
  va.finalize();
  ensure_js("a.length == 29");
  ensure_js("a[17] == '1' && a[18] == '2' && a[19] == '3' && a[20] == '4'");
  // Floating number is not accurate
  ensure_js("parseFloat(a[23]).toFixed(1) == '1.1' ");
  ensure_js("parseFloat(a[24]).toFixed(1) == '1.2' ");
  ensure_js("parseFloat(a[25]).toFixed(1) == '1.3' ");
  ensure_js("a[26] == 1 && a[27] == 2 && a[28] == 3");

  test("set(k,v)");
  EM_ASM(
    a = {};
  );
  VB vo(val::global("a"));
  vo.set("k1", 1);
  vo.set("k2", 1.1);
  vo.set("k3", "hi");
  vo.set("k4", std::string("world"));
  vo.set("k5", (uint16_t)5);
  val a("ka");
  vo.set(a, "value_a");
  vo.set("k6", val(6));
  vo.finalize();
  ensure_js("a.k1 == 1");
  ensure_js("a.k2 == 1.1");
  ensure_js("a.k3 == 'hi'");
  ensure_js("a.k4 == 'world'");
  ensure_js("a.k5 == 5");
  ensure_js("a.ka == 'value_a'");
  ensure_js("a.k6 == 6");

  test("set(k, array)");
  vo.set("k6", vec);
  vo.set("k7", std::vector<float>{1, 2, 3, 4, 5, 6});
  vo.set("k8", ar2);
  vo.finalize();
  ensure_js("a.k6 instanceof Array && a.k6.length == 16");
  ensure_js("a.k6[0] == 1 && a.k6[1] == 2 && a.k6[9] == 10 && a.k6[15] == 16");
  ensure_js("a.k7 instanceof Array && a.k7.length == 6");
  ensure_js("a.k7[0] == 1 && a.k7[1] == 2 && a.k7[2] == 3 && a.k7[5] == 6");
  ensure_js("a.k8 instanceof Array && a.k8.length == 3");
  ensure_js("a.k8[0] == 6 && a.k8[1] == 7 && a.k8[2] == 8");

  test("set(k, VB)");
  VB vba(ARRAY);
  vba.add(1);
  vo.set("k8", vba);
  vo.finalize();
  ensure_js("a['k8'] instanceof Array && a['k8'][0] == 1");

  VB vbo(OBJECT);
  vbo.set("k1", 2);
  vbo.set("k2", "v2");
  vo.set("k9", vbo);
  vo.finalize();
  ensure_js("a['k9'] instanceof Object");
  ensure_js("a['k9']['k1'] == 2");
  ensure_js("a['k9']['k2'] == 'v2'");

  test("use local scope values");
  {
    std::string s1;
    vo.set("ks1", std::move(s1));
    std::string s2("abc");
    vo.set("ks2", std::move(s2));
    std::string s3("def");
    vo.set("ks3", std::move(s3));
  }
  vo.finalize();
  ensure_js("a.ks1 == ''");
  ensure_js("a.ks2 == 'abc'");
  ensure_js("a.ks3 == 'def'");

  test("use local emscripten::val");
  {
    val o = val::object();
    o.set("k1", 1);
    o.set("k2", "a");
    val a = val::array();
    a.call<void>("push", 1);
    a.call<void>("push", 2);
    vo.set("o", std::move(o));
    vo.set("a", std::move(a));
  }
  vo.finalize();
  ensure_js("a.o.k1 == 1");
  ensure_js("a.o.k2 == 'a'");
  ensure_js("a.a[0]== 1 && a.a[1] == 2");

  printf("end\n");
}

}  // namespace tests

int main() {
  tests::test_cases();
  return 0;
}
