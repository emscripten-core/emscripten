
#include <array>
#include <emscripten.h>
#include <emscripten/val_builder.h>
#include <stdio.h>
#include <string>
#include <vector>

using namespace emscripten;

using VB = emscripten::ValBuilder<32>;

#if _LIBCPP_STD_VER < 17
namespace std {

template <class T, std::size_t N>
constexpr std::size_t size(const T (&array)[N]) noexcept {
    return N;
}

}  // namespace std
#endif  // _LIBCPP_STD_VER

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

void test_valbuilder() {
  VB v;
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
  VB ar(ARRAY);
  std::vector<std::string> vs{"string1", "string2", "string3", "string4", ls};
  ar.concat(vs);
  ar.concat(veci);
  v.set("key_array", ar.toval());

  // embed an object
  VB ob;
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
// test_valbuilder loop 100000 cost: 609.800000

// WITH LONG_S & BIG_A
// test_val loop 100000 cost: 5737.500000
// test_valbuilder loop 100000 cost: 1155.300000

void test_perf() {
  const size_t LoopTimes = 100000;
  double start = emscripten_get_now();
  for (size_t i = 0; i < LoopTimes; i++) {
    test_val();
  }
  printf("test_val loop %zu cost: %f\n", LoopTimes, emscripten_get_now() - start);

  start = emscripten_get_now();
  for (size_t i = 0; i < LoopTimes; i++) {
    test_valbuilder();
  }
  printf("test_valbuilder loop %zu cost: %f\n", LoopTimes, emscripten_get_now() - start);
  printf("\n");
}

EM_JS(emscripten::EM_VAL,
      pushDataEmjs,
      (int v1,
       int v2,
       float v3,
       bool v4,
       const char* v5,
       const char* v6,
       emscripten::EM_VAL v7,
       int v8),
    {
        var o = {};
        o["key1"] = v1;
        o["key2"] = v2;
        o["key3"] = v3;
        o["key4"] = !!v4;
        o["key5"] = readLatin1String(v5);
        o["key6"] = UTF8ToString(v6);
        o["key7"] = Emval.toValue(v7);
        o["key8"] = v8;
        return Emval.toHandle(o);
    }
);

void test_perf_chaining() {
  const size_t LoopTimes = 100000;
  const size_t PreWarmLoopTimes = 10000;
  std::string s("test hello world!!");
  val vi = val(256);

  // pre-warm: val
  for (size_t i = 0; i < PreWarmLoopTimes; i++) {
    // val (non chaining)
    val v = val::object();
    v.set("key1", 1);
    v.set("key2", 2);
    v.set("key3", 1.234f);
    v.set("key4", true);
    v.set("key5", "012345678910");
    v.set("key6", s);
    v.set("key7", vi);
    v.set("key8", 8);
  }

  double start = emscripten_get_now();
  for (size_t i = 0; i < LoopTimes; i++) {
    // val (non chaining)
    val v = val::object();
    v.set("key1", 1);
    v.set("key2", 2);
    v.set("key3", 1.234f);
    v.set("key4", true);
    v.set("key5", "012345678910");
    v.set("key6", s);
    v.set("key7", vi);
    v.set("key8", 8);
  }
  auto val_cost = emscripten_get_now() - start;
  printf("test_chain_val loop %zu cost: %f\n", LoopTimes, val_cost);

  // pre-warm: val_builder
  for (size_t i = 0; i < PreWarmLoopTimes; i++) {
    // use valbuilder (chaining)
    VB v(OBJECT);
    v.set("key1", 1);
    v.set("key2", 2);
    v.set("key3", 1.234f);
    v.set("key4", true);
    v.set("key5", "012345678910");
    v.set("key6", s);
    v.set("key7", vi);
    v.set("key8", 8);
    val o = v.toval();  // finalized
  }

  start = emscripten_get_now();
  for (size_t i = 0; i < LoopTimes; i++) {
    // use valbuilder (chaining)
    VB v(OBJECT);
    v.set("key1", 1);
    v.set("key2", 2);
    v.set("key3", 1.234f);
    v.set("key4", true);
    v.set("key5", "012345678910");
    v.set("key6", s);
    v.set("key7", vi);
    v.set("key8", 8);
    val o = v.toval();  // finalized
  }
  auto vb_cost = emscripten_get_now() - start;
  printf("test_chain_valbuilder loop %zu cost: %f\n", LoopTimes, vb_cost);

  // pre-warm: EM_JS
  for (size_t i = 0; i < PreWarmLoopTimes; i++) {
    // use EM_JS (chaining)
    val v = val::take_ownership(pushDataEmjs(
      1, 2, 1.234f, true, "012345678910", s.c_str(), vi.as_handle(), 8));
  }

  start = emscripten_get_now();
  for (size_t i = 0; i < LoopTimes; i++) {
    // use EM_JS (chaining)
    val v = val::take_ownership(pushDataEmjs(
      1, 2, 1.234f, true, "012345678910", s.c_str(), vi.as_handle(), 8));
  }
  auto emjs_cost = emscripten_get_now() - start;
  printf("test_chain_emjs loop %zu cost: %f\n", LoopTimes, emjs_cost);

  printf("val-vb = %f\n", val_cost - vb_cost);
  printf("val/vb = %f\n", val_cost / vb_cost);
  printf("val/emjs = %f\n", val_cost / emjs_cost);
  printf("\n");
}

// loop 100000 val biga cost: 4519.156967
// loop 100000 VB biga cost: 214.419598
void test_perf_biga() {
  const size_t LoopTimes = 100000;
  double start = emscripten_get_now();
  for (size_t i = 0; i < LoopTimes; i++) {
    val vo = val::object();
    vo.set("k", val::array(BIG_A, BIG_A+std::size(BIG_A)));
  }
  printf("test big-array loop %zu val cost: %f\n", LoopTimes, emscripten_get_now() - start);

  start = emscripten_get_now();
  for (size_t i = 0; i < LoopTimes; i++) {
    VB vo;
    vo.set("k", BIG_A);
    vo.finalize();
  }
  printf("test big-array loop %zu VB cost: %f\n", LoopTimes, emscripten_get_now() - start);
  printf("\n");
}

}  // namespace tests

int main() {
  tests::test_perf();
  tests::test_perf_chaining();
  tests::test_perf_biga();
  return 0;
}

// build command-line:
// ../../emcc -lembind -O2 -o val_builder_benchmark.html val_builder_benchmark.cpp
