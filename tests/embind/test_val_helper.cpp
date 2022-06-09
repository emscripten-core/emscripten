
#include <array>
#include <emscripten.h>
#include <emscripten/val.h>
#include <emscripten/val_helper.h>
#include <malloc.h>  // for ::mallinfo
#include <iostream>
#include <stdio.h>
#include <string>
#include <vector>

#include <unistd.h>

using namespace emscripten;

namespace Val {
using VH = ValHelper<32>;
}

namespace {

unsigned int get_used_memory() {
  struct mallinfo i = ::mallinfo();
  unsigned int dynamicTop = (unsigned int)sbrk(0);
  return dynamicTop - i.fordblks;
}

}  // namespace

namespace std {

template <class T, std::size_t N>
constexpr std::size_t size(const T (&array)[N]) noexcept {
    return N;
}

}

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

const std::vector<int> veci{1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20};

void test_val() {
  val v = val::object();
  v.set("key1", 1);
  v.set("key2", 2);
  v.set("key3", "333333333333333333333333");
  v.set("key4", 6.69884350);
  v.set("key5", true);
  std::string s("hello world!!");
  v.set("key6", s);
  val v1 = val(1000.1);
  v.set("key7", v1);
  v.set("key8", val::array(std::vector<int>{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
                                            12, 13, 14, 15}));
  v.set("key9", val::array(std::vector<float>{1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7,
                                              8.8, 9.9}));
  v.set("key11", "jjk");
  auto ls =
      std::string("a long long long long long long long long string!!!!!");
  v.set("key12", ls);
  v.set("long_value", LONG_S);
  std::vector<int> biga(BIG_A, BIG_A + std::size(BIG_A));
  v.set("big_array", val::array(biga));

  // embed an array
  val ar = val::array();
  ar.call<void>("push", std::string("string1"));
  ar.call<void>("push", std::string("string2"));
  ar.call<void>("push", std::string("string3"));
  ar.call<void>("push", std::string("string4"));
  ar.call<void>("push", ls);
  for (auto i : veci)
    ar.call<void>("push", i);
  v.set("key_array", ar);

  // embed an object
  val ob = val::object();
  ob.set("subkey1", 1);
  ob.set("subkey2", 1.0f);
  ob.set("subkey3", 9.9);
  ob.set("subkey4", "substring1");
  ob.set("subkey5", std::string("substring2"));
  v.set("key_object", ob);

  // EM_ASM({console.log(Emval.toValue($0));}, v.as_handle());
}

void test_valhelper() {
  Val::VH v;
  v.set("key1", 1);
  v.set("key2", 2);
  v.set("key3", "333333333333333333333333");
  v.set("key4", 6.69884350);
  v.set("key5", true);
  std::string s("hello world!!");
  v.set("key6", s);
  v.set("key7", 1000.1);
  auto vec =
      std::vector<int>{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
  v.set("key8", vec);
  auto vecf = std::vector<float>{1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9};
  v.set("key9", vecf);
  v.set("key11", "jjk");
  std::string ls("a long long long long long long long long string!!!!!");
  v.set("key12", ls);
  v.set("long_value", LONG_S);
  std::vector<int> biga(BIG_A, BIG_A + std::size(BIG_A));
  v.set("big_array", biga);

  // embed an array
  Val::VH ar(Val::ARRAY);
  std::vector<std::string> vs{"string1", "string2", "string3", "string4", ls};
  ar.concat(vs);
  ar.concat(veci);
  v.set("key_array", ar.toval());

  // embed an object
  Val::VH ob;
  ob.set("subkey1", 1);
  ob.set("subkey2", 1.0f);
  ob.set("subkey3", 9.9);
  std::string ss1("substring1");
  ob.set("subkey4", ss1);
  std::string ss2("substring2");
  ob.set("subkey5", ss2);
  v.set("key_object", ob.toval());

  v.finalize();
  // EM_ASM({console.log(Emval.toValue($0));}, v.toval().as_handle());
}

// test_val loop 100000 cost: 1340.400000
// test_valhelper loop 100000 cost: 609.800000

// WITH LONG_S & BIG_A
// test_val loop 100000 cost: 5737.500000
// test_valhelper loop 100000 cost: 1155.300000

void test_perf() {
  const size_t LoopTimes = 100000;
  double start = emscripten_get_now();
  for (size_t i = 0; i < LoopTimes; i++) {
    test_val();
  }
  printf("test_val loop %zu cost: %f\n", LoopTimes, emscripten_get_now() - start);

  start = emscripten_get_now();
  for (size_t i = 0; i < LoopTimes; i++) {
    test_valhelper();
  }
  printf("test_valhelper loop %zu cost: %f\n", LoopTimes, emscripten_get_now() - start);
}

// loop 100000 val biga cost: 4519.156967
// loop 100000 VH biga cost: 214.419598
void test_perf_biga() {
  const size_t LoopTimes = 100000;
  double start = emscripten_get_now();
  for (size_t i = 0; i < LoopTimes; i++) {
    val vo = val::object();
    vo.set("k", val::array(BIG_A, BIG_A+std::size(BIG_A)));
  }
  printf("loop %zu val biga cost: %f\n", LoopTimes, emscripten_get_now() - start);

  start = emscripten_get_now();
  for (size_t i = 0; i < LoopTimes; i++) {
    Val::VH vo;
    vo.set("k", BIG_A);
    vo.finalize();
  }
  printf("loop %zu VH biga cost: %f\n", LoopTimes, emscripten_get_now() - start);
}

void test_cases() {
  printf("start\n");

  test("add() overloads");
  EM_ASM(
    a = [];
  );
  Val::VH va(val::global("a"));
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
  va.attach(val::global("a"));
  std::vector<int> vec{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
  va.add(vec);
  va.add(std::vector<int>{5, 6, 7, 8, 9});
  va.add(std::vector<std::string>{"1", "2", "3"});
  std::vector<std::string> vs{"1", "2", "3"};
  va.add(vs);
  std::array<int, 5> ar{1, 2, 3, 4, 5};
  va.add(ar);
  std::array<std::string, 5> ar1{"4", "5", "6", "99"};
  va.add(ar1);
  char ar2[]{6, 7, 8};
  va.add(ar2);
  va.finalize();
  ensure_js("a[0].length == 16 && a[0][0] == 1 && a[0][2] == 3 && a[0][15] == 16");
  ensure_js("a[1].length == 5 && a[1][0] == 5 && a[1][1] == 6 && a[1][5] == undefined");
  ensure_js("a[2].length == 3 && a[2][0] == '1' && a[2][1] == '2' && a[2][2] == '3' ");
  ensure_js("a[3].length == 3 && a[3][0] == '1' && a[3][1] == '2' && a[3][2] == '3' ");
  ensure_js("a[4].length == 5 && a[4][0] == 1 && a[4][1] == 2 && a[4][4] == 5");
  ensure_js("a[5].length == 5 && a[5][0] == '4' && a[5][1] == '5' && a[5][2] == '6' && a[5][3] == '99' && a[5][4] == ''");
  ensure_js("a[6].length == 3 && a[6][0] == 6 && a[6][1] == 7 && a[6][2] == 8");

  test("concat(array&)");
  EM_ASM(
    a = [];

  );
  va.attach(val::global("a"));
  va.add(0);
  va.concat(vec);
  va.finalize();
  ensure_js("a[0] == 0");
  ensure_js("a[1] == 1 && a[2] == 2 && a[3] == 3 && a[16] == 16");
  ensure_js("a[17] === undefined");

  va.concat(std::vector<std::string>{"1", "2", "3", "4", "5", "6"});                                                                                                                                                                                                                                                                         
  va.concat(std::vector<float>{1.1,1.2,1.3});
  va.concat(std::vector<short>{1,2,3});                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         
  va.finalize();
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
  Val::VH vo(val::global("a"));
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

  test("set(k, VH)");
  Val::VH vha(Val::ARRAY);
  vha.add(1);
  vo.set("k8", vha);
  vo.finalize();
  ensure_js("a['k8'] instanceof Array && a['k8'][0] == 1");

  Val::VH vho(Val::OBJECT);
  vho.set("k1", 2);
  vho.set("k2", "v2");
  vo.set("k9", vho);
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
  auto m1 = get_used_memory();
  tests::test_cases();
  auto m2 = get_used_memory();
  assert(m1 == m2);

  // tests::test_perf();
  // tests::test_perf_biga();
  return 0;
}
