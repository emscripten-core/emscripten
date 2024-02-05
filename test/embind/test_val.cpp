// Copyright 2018 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <cstdio>
#include <iostream>
#include <cmath>
#include <emscripten/bind.h>
#include <emscripten/emscripten.h>
#include <emscripten/val.h>

using namespace emscripten;
using namespace std;

void test(string message) {
  cout << "test: " << message << "...\n";
}

#define ensure(X) assert(X)
#define ensure_not(X) assert(!(X))

#define ensure_js(js_code) ensure(emscripten_run_script_int(js_code))
#define ensure_js_not(js_code) ensure_js((string("!( ") + js_code + " )").c_str())

void throw_js_error(val js_error) {
  js_error.throw_();
}

struct Dummy {};

Dummy * makeDummy() {
  return new Dummy();
}

EMSCRIPTEN_BINDINGS(test_bindings) {
  emscripten::class_<Dummy>("Dummy");
  emscripten::function("throw_js_error", &throw_js_error);
  emscripten::function("makeDummy", &makeDummy, emscripten::allow_raw_pointers());
}

int main() {
  printf("start\n");

  test("val array()");
  val::global().set("a", val::array());
  ensure_js("a instanceof Array");
  ensure_js_not("a instanceof Boolean");
  ensure_js_not("a instanceof Number");

  test("template<typename T> val array(const std::vector<T> vec)");
  vector<val> vec1;
  vec1.push_back(val(11));
  vec1.push_back(val("a"));
  vec1.push_back(val::array());
  val::global().set("a", val::array(vec1));
  ensure_js("a instanceof Array");
  ensure_js_not("a instanceof Boolean");
  ensure_js_not("a instanceof Number");
  ensure_js("a[0] == 11");
  ensure_js_not("a[0] == 12");
  ensure_js("a[1] == 'a'");
  ensure_js("a[2] instanceof Array");
  vector<int> vec2;
  vec2.push_back(0);
  vec2.push_back(1);
  vec2.push_back(3);
  val::global().set("a", val::array(vec2));
  ensure_js("a instanceof Array");
  ensure_js_not("a instanceof Number");
  ensure_js("a[0] == 0");
  ensure_js_not("a[0] == 1");
  ensure_js("a[1] == 1");
  ensure_js("a[2] == 3");
  ensure_js_not("a[2] == 2");
  vector<int> vec2_from_iter;
  for (val&& v : val::global("a")) {
    vec2_from_iter.push_back(v.as<int>());
  }
  ensure(vec2 == vec2_from_iter);

  test("template<typename Iter> val array(Iter begin, Iter end)");
  val::global().set("a", val::array(vec1.begin(), vec1.end()));
  ensure_js("a instanceof Array");
  ensure_js_not("a instanceof Boolean");
  ensure_js_not("a instanceof Number");
  ensure_js("a[0] == 11");
  ensure_js_not("a[0] == 12");
  ensure_js("a[1] == 'a'");
  ensure_js("a[2] instanceof Array");
  val::global().set("a", val::array(vec2.begin(), vec2.end()));
  ensure_js("a instanceof Array");
  ensure_js_not("a instanceof Number");
  ensure_js("a[0] == 0");
  ensure_js_not("a[0] == 1");
  ensure_js("a[1] == 1");
  ensure_js("a[2] == 3");
  ensure_js_not("a[2] == 2");
  int arr[] = {1, 2, 3};
  val::global().set("a", val::array(arr, arr + 3));
  ensure_js("a instanceof Array");
  ensure_js_not("a instanceof Number");
  ensure_js("a[0] == 1");
  ensure_js("a[1] == 2");
  ensure_js("a[2] == 3");

  test("val object()");
  val::global().set("a", val::object());
  ensure_js("a instanceof Object");

  // Test emval{From,To}Handle roundtrip.
  {
    val a = val::global("a");
    val a_roundtrip = val::take_ownership(EM_VAL(EM_ASM_PTR({
      return Emval.toHandle(Emval.toValue($0));
    }, a.as_handle())));
    ensure(a == a_roundtrip);
  }

  test("val undefined()");
  val::global().set("a", val::undefined());
  ensure_js("a == undefined");
  ensure_js("a != false");

  test("val null()");
  val::global().set("a", val::null());
  ensure_js("a == null");
  ensure_js("a != false");

  test("val global(const char* name = 0)");
  // best we can probably do is ensure that calling it does not raise an exception
  val::global();
  val::global("addEventListener");

  test("template<typename T> explicit val(T&& value)");
  val::global().set("a", val(false));
  ensure_js("a == false");
  val::global().set("a", val(true));
  ensure_js("a == true");
  val::global().set("a", val(1));
  ensure_js("a == 1");
  val::global().set("a", val(string("1")));
  ensure_js("a == '1'");

  test("val(const char* v)");
  val::global().set("a", val("1"));
  ensure_js("a == '1'");

  test("val()");
  val a;
  val::global().set("a", a);
  ensure_js("a === undefined");
  a = val(1);
  val::global().set("a", a);
  ensure_js("a == 1");
  val ar[2];
  ar[0] = val(1);
  ar[1] = val(2);
  val::global().set("a", val::array(ar, ar + 2));
  ensure_js("a instanceof Array");
  ensure_js_not("a instanceof Number");
  ensure_js("a[0] == 1");
  ensure_js("a[1] == 2");

  test("bool isNull()");
  EM_ASM(
    a = null;
    b = false;
  );
  ensure(val::global("a").isNull());
  ensure_not(val::global("b").isNull());

  test("bool isUndefined()");
  EM_ASM(
    a = undefined;
    b = false;
  );
  ensure(val::global("a").isUndefined());
  ensure_not(val::global("b").isUndefined());

  test("bool isTrue()");
  EM_ASM(
    a = true;
    b = false;
    c = null;
  );
  ensure(val::global("a").isTrue());
  ensure_not(val::global("b").isTrue());
  ensure_not(val::global("c").isTrue());

  test("bool isFalse()");
  EM_ASM(
    a = false;
    b = true;
    c = null;
  );
  ensure(val::global("a").isFalse());
  ensure_not(val::global("b").isFalse());
  ensure_not(val::global("c").isFalse());

  test("bool isNumber()");
  EM_ASM(
    a = 1;
    b = 1.1;
    c = true;
  );
  ensure(val::global("a").isNumber());
  ensure(val::global("b").isNumber());
  ensure_not(val::global("c").isNumber());

  test("bool isString()");
  EM_ASM(
    a = '1';
    b = 1;
    c = true;
  );
  ensure(val::global("a").isString());
  ensure_not(val::global("b").isString());
  ensure_not(val::global("c").isString());

  test("bool isArray()");
  EM_ASM(
    a = [];
    b = [1];
    c = true;
    d = {};
  );
  ensure(val::global("a").isArray());
  ensure(val::global("b").isArray());
  ensure_not(val::global("c").isArray());
  ensure_not(val::global("d").isArray());

  test("val& operator=(val&& v)");
  val source(val::object());
  val target(val::object());
  source.set("val", 1);
  target = std::move(source);
  ensure(target["val"].as<int>() == 1);
  // move and assign to itself
  target.set("val", 2);
  target = std::move(target);
  ensure(target["val"].as<int>() == 2);

  test("bool equals(const val& v)");
  EM_ASM(
    a = 1;
    b = 1;
    c = 2;
    d = true;
    e = '1';
  );
  ensure(val::global("a").equals(val::global("a")));
  ensure(val::global("a").equals(val::global("b")));
  ensure_not(val::global("a").equals(val::global("c")));
  ensure(val::global("a").equals(val::global("d")));
  ensure(val::global("a").equals(val::global("e")));
  ensure_not(val::global("c").equals(val::global("d")));


  test("bool operator==(const val& v)");
  EM_ASM(
    a = 1;
    b = 1;
    c = 2;
    d = true;
    e = '1';
  );
  ensure(val::global("a") == val::global("a"));
  ensure(val::global("a") == val::global("b"));
  ensure_not(val::global("a") == val::global("c"));
  ensure(val::global("a") == val::global("d"));
  ensure(val::global("a") == val::global("e"));
  ensure_not(val::global("c") == val::global("d"));

  test("bool operator!=(const val& v)");
  EM_ASM(
    a = 1;
    b = 1;
    c = 2;
    d = true;
    e = '1';
  );
  ensure_not(val::global("a") != val::global("a"));
  ensure_not(val::global("a") != val::global("b"));
  ensure(val::global("a") != val::global("c"));
  ensure_not(val::global("a") != val::global("d"));
  ensure_not(val::global("a") != val::global("e"));
  ensure(val::global("c") != val::global("d"));

  test("bool strictlyEquals(const val& v)");
  EM_ASM(
    a = 1;
    b = 1;
    c = 2;
    d = true;
    e = '1';
  );
  ensure(val::global("a").strictlyEquals(val::global("a")));
  ensure(val::global("a").strictlyEquals(val::global("b")));
  ensure_not(val::global("a").strictlyEquals(val::global("c")));
  ensure_not(val::global("a").strictlyEquals(val::global("d")));
  ensure_not(val::global("a").strictlyEquals(val::global("e")));
  ensure_not(val::global("c").strictlyEquals(val::global("d")));

  test("bool operator>(const val& v)");
  EM_ASM(
    a = 1;
    b = 1;
    c = 2;
    d = '2';
  );
  ensure_not(val::global("a") > val::global("a"));
  ensure_not(val::global("a") > val::global("b"));
  ensure_not(val::global("a") > val::global("c"));
  ensure_not(val::global("a") > val::global("d"));
  ensure(val::global("c") > val::global("a"));
  ensure(val::global("d") > val::global("a"));

  test("bool operator>= (const val& v)");
  EM_ASM(
    a = 1;
    b = 1;
    c = 2;
    d = '2';
  );
  ensure(val::global("a") >= val::global("a"));
  ensure(val::global("a") >= val::global("b"));
  ensure_not(val::global("a") >= val::global("c"));
  ensure_not(val::global("a") >= val::global("d"));
  ensure(val::global("c") >= val::global("a"));
  ensure(val::global("d") >= val::global("a"));

  test("bool operator<(const val& v)");
  EM_ASM(
    a = 1;
    b = 1;
    c = 2;
    d = '2';
  );
  ensure_not(val::global("a") < val::global("a"));
  ensure_not(val::global("a") < val::global("b"));
  ensure(val::global("a") < val::global("c"));
  ensure(val::global("a") < val::global("d"));
  ensure_not(val::global("c") < val::global("a"));
  ensure_not(val::global("d") < val::global("a"));

  test("bool operator<= (const val& v)");
  EM_ASM(
    a = 1;
    b = 1;
    c = 2;
    d = '2';
  );
  ensure(val::global("a") <= val::global("a"));
  ensure(val::global("a") <= val::global("b"));
  ensure(val::global("a") <= val::global("c"));
  ensure(val::global("a") <= val::global("d"));
  ensure_not(val::global("c") <= val::global("a"));
  ensure_not(val::global("d") <= val::global("a"));

  test("bool operator!()");
  EM_ASM(
    a = true;
    b = false;
    c = null;
    d = undefined;
    e = 0;
    f = 1;
    g = "";
    h = '0';
    i = 'false';
  );
  ensure(!val::global("a") == false);
  ensure(!val::global("b") == true);
  ensure(!val::global("c") == true);
  ensure(!val::global("d") == true);
  ensure(!val::global("e") == true);
  ensure(!val::global("f") == false);
  ensure(!val::global("g") == true);
  ensure(!val::global("h") == false);
  ensure(!val::global("i") == false);
  ensure(!!val::global("a") == true);
  ensure(!!val::global("b") == false);

  test("template<typename... Args> val new_(Args&&... args)");
  EM_ASM(
    A = function() {
      this.value = 2;
    }
  );
  val::global().set("a", val::global("A").new_());
  ensure_js("a instanceof A");
  ensure_js("a.value == 2");
  EM_ASM(
    A = function(arg1, arg2) {
      this.arg1 = arg1;
      this.arg2 = arg2;
    }
  );
  val::global().set("a", val::global("A").new_(val(2), val("b")));
  ensure_js("a instanceof A");
  ensure_js("a.arg1 == 2");
  ensure_js("a.arg2 == 'b'");

  test("template<typename T> val operator[](const T& key)");
  EM_ASM(
    a = 2;
  );
  ensure(val::global()["a"].as<int>() == 2);
  ensure_not(val::global()["a"].as<int>() == 3);
  val k("a");
  ensure(val::global()[k].as<int>() == 2);
  ensure_not(val::global()[k].as<int>() == 3);

  test("template<typename K, typename V> void set(const K& key, const V& value)");
  val::global().set("a", val(2));
  ensure_js("a == 2");
  val::global().set("a", val(3));
  ensure_js("a == 3");
  val::global().set("a", 0);
  ensure_js("a == 0");
  val::global().set("a", false);
  ensure_js("a == false");
  val::global().set("a", 2);
  ensure_js("a == 2");
  val::global().set("a", "b");
  ensure_js("a == 'b'");
  val::global().set(k, 1);
  ensure_js("a == 1");
  val v(3);
  val::global().set(k, v);
  ensure("a == 3");
  ensure(val::global()[k].as<int>() == 3);

  test("template<typename... Args> val operator()(Args&&... args)");
  EM_ASM(
    f = function() {
      return 2;
    };
  );
  ensure(val::global("f")().as<int>() == 2);
  ensure_not(val::global("f")().as<int>() == 3);
  EM_ASM(
    f1 = function(arg1, arg2) {
      return arg1;
    };
    f2 = function(arg1, arg2) {
      return arg2;
    };
  );
  ensure(val::global("f1")(val(2),val("3")).as<int>() == 2);
  ensure(val::global("f2")(val(2),val("3")).as<string>() == "3");

  test("template<typename ReturnValue, typename... Args> ReturnValue call(const char* name, Args&&... args)");
  EM_ASM(
    C = function() {
      this.method = function() { return this; };
    };
    c = new C;
  );
  ensure(val::global("c").call<val>("method") == val::global("c"));
  EM_ASM(
    C = function() {
      this.method = function(arg) { return arg; };
    };
    c = new C;
  );
  ensure(val::global("c").call<int>("method", val(2)) == 2);

  test("template<typename T, typename ...Policies> T as(Policies...)");
  EM_ASM(
    a = 1;
    b = 'b';
  );
  ensure(val::global("a").as<int>() == (int)1);
  ensure(val::global("a").as<double>() == (double)1.0);
  ensure_not(val::global("a").as<double>() == (double)1.1);
  ensure(val::global("b").as<string>() == "b");

  // test:
  // val typeof()
  #if __STRICT_ANSI__
    EM_ASM(
      a = undefined;
      b = null;
      c = true;
      d = 2;
      e = '2';
      f = Symbol();
      g = function () {};
      h = {};
    );
    if (
      val::global("a").typeOf().as<string>() != "undefined" ||
      val::global("a").typeOf().as<string>() == "" ||
      val::global("b").typeOf().as<string>() != "object" ||
      val::global("c").typeOf().as<string>() != "boolean" ||
      val::global("d").typeOf().as<string>() != "number" ||
      val::global("e").typeOf().as<string>() != "string" ||
      val::global("f").typeOf().as<string>() != "symbol" ||
      val::global("g").typeOf().as<string>() != "function" ||
      val::global("h").typeOf().as<string>() != "object"
    ) {
      printf("test:\nval typeof()\nfail\n");
      assert(false);
    }
  #endif

  test("val typeOf()");
  EM_ASM(
    a = undefined;
    b = null;
    c = true;
    d = 2;
    e = '2';
    f = Symbol();
    g = function () {};
    h = {};
  );
  ensure(val::global("a").typeOf().as<string>() == "undefined");
  ensure_not(val::global("a").typeOf().as<string>() == "");
  ensure(val::global("b").typeOf().as<string>() == "object");
  ensure(val::global("c").typeOf().as<string>() == "boolean");
  ensure(val::global("d").typeOf().as<string>() == "number");
  ensure(val::global("e").typeOf().as<string>() == "string");
  ensure(val::global("f").typeOf().as<string>() == "symbol");
  ensure(val::global("g").typeOf().as<string>() == "function");
  ensure(val::global("h").typeOf().as<string>() == "object");
  ensure(val::undefined().typeOf().as<string>() == "undefined");
  ensure(val::null().typeOf().as<string>() == "object");
  ensure(val(true).typeOf().as<string>() == "boolean");
  ensure(val(2).typeOf().as<string>() == "number");
  ensure(val("2").typeOf().as<string>() == "string");
  ensure(val::global().call<val>("Symbol").typeOf().as<string>() == "symbol");
  ensure(val::object().typeOf().as<string>() == "object");

  test("bool instanceof(const val& v)");
  EM_ASM(
    A = function() {};
    B = function() {};
    a = new A;
  );
  ensure(val::global("a").instanceof(val::global("A")));
  ensure_not(val::global("a").instanceof(val::global("B")));

  test("bool in(const val& v)");
  EM_ASM(
    // can't declare like this because i get:
    //   error: use of undeclared identifier 'c'
    // possibly a bug with EM_ASM
    //a = {b: 'bb',c: 'cc'};
    a = {};
    a.b = 'bb';
    a.c = 'cc';
  );
  ensure(val("c").in(val::global("a")));
  ensure_not(val("d").in(val::global("a")));

  test("template<typename T> bool delete_(const T& property)");
  EM_ASM(
    a = {};
    a.b = undefined;
    a[0] = null;
    a[1] = 2;
    a.c = 'c';
  );
  ensure_js("'b' in a");
  ensure_js("0 in a");
  ensure_js("1 in a");
  ensure_js("'c' in a");
  ensure(val::global("a").delete_("b") == true);
  ensure_js_not("'b' in a");
  ensure_js("0 in a");
  ensure_js("1 in a");
  ensure_js("'c' in a");
  ensure(val::global("a").delete_(0) == true);
  ensure(val::global("a").delete_(val(1)) == true);
  ensure(val::global("a").delete_(val("c")) == true);
  ensure_js_not("'b' in a");
  ensure_js_not("0 in a");
  ensure_js_not("1 in a");
  ensure_js_not("'c' in a");

  test("void throw_() const");
  EM_ASM(
    test_val_throw_ = function(error)
    {
      try
      {
        Module.throw_js_error(error);
        return false;
      }
      catch(error_thrown)
      {
        if (error_thrown != error)
          throw error_thrown;
      }
      return true;
    }
  );
  ensure_js("test_val_throw_(new Error)");
  ensure_js("test_val_throw_(Error)");
  ensure_js("test_val_throw_(2)");
  ensure_js("test_val_throw_('message')");
  ensure_js("test_val_throw_(new TypeError('message'))");

  // these tests should probably go elsewhere as it is not a member of val
  test("template<typename T> std::vector<T> vecFromJSArray(const val& v)");
  EM_ASM(
    // can't declare like this because i get:
    //   error: expected ')'
    // possibly a bug with EM_ASM
    //a = [1, '2'];
    a = [];
    a[0] = 1;
    a[1] = '42';
    a[2] = 'b';
    a[3] = new Date(100000);
  );
  const std::vector<val>& aAsArray = vecFromJSArray<val>(val::global("a"));
  ensure(aAsArray.at(0).as<int>() == 1);
  ensure(aAsArray.at(1).as<string>() == "42");
  ensure(aAsArray.at(2).as<string>() == "b");
  ensure(aAsArray.size() == 4);

  test("template<typename T> std::vector<T *> vecFromJSArray(const val& v)");
  EM_ASM(
    b = [];
    b[0] = Module.makeDummy();
    b[1] = Module.makeDummy();
  );
  const std::vector<Dummy *>& bAsArray = vecFromJSArray<Dummy *>(val::global("b"), allow_raw_pointers());
  ensure(bAsArray.size() == 2);
  for (auto *dummy : bAsArray) {
    delete dummy;
  }

  test("template<typename T> std::vector<T> convertJSArrayToNumberVector(const val& v)");

  const std::vector<float>& aAsNumberVectorFloat = convertJSArrayToNumberVector<float>(val::global("a"));
  ensure(aAsNumberVectorFloat.size() == 4);

  ensure(aAsNumberVectorFloat.at(0) == 1.f);
  ensure(aAsNumberVectorFloat.at(1) == 42.f);     // String containing numbers are converted correctly
  ensure(std::isnan(aAsNumberVectorFloat.at(2))); // NaN returned if can not be converted for floats
  ensure(aAsNumberVectorFloat.at(3) == 100000.f); // Date returns milliseconds since epoch

  const std::vector<uint32_t>& aAsNumberVectorUint32_t = convertJSArrayToNumberVector<uint32_t>(val::global("a"));
  ensure(aAsNumberVectorUint32_t.size() == 4);

  ensure(aAsNumberVectorUint32_t.at(0) == 1);
  ensure(aAsNumberVectorUint32_t.at(1) == 42);     // String containing numbers are converted correctly
  ensure(aAsNumberVectorUint32_t.at(2) == 0);      // 0 is returned if can not be converted for integers
  ensure(aAsNumberVectorUint32_t.at(3) == 100000); // Date returns milliseconds since epoch

  test("val u8string(const char* s)");
  val::global().set("a", val::u8string(u8"abc"));
  ensure_js("a == 'abc'");
  val::global().set("a", val::u8string(u8"你好"));
  ensure_js_not("a == 'abc'");
  ensure_js("a == '你好'");
  auto u8_str = val::global()["a"].as<std::string>();
  ensure(u8_str == u8"你好");

  test("val u16string(const char16_t* s)");
  val::global().set("a", val::u16string(u"hello"));
  ensure_js("a == 'hello'");
  val::global().set("a", val::u16string(u"世界"));
  ensure_js_not("a == 'hello'");
  ensure_js("a == '世界'");
  // UTF-16 encoded SMILING FACE WITH OPEN MOUTH (U+1F603)
  const char16_t* s = u"😃 = \U0001F603 is :-D";
  val::global().set("a", val::u16string(s));
  ensure_js("a == '😃 = \U0001F603 is :-D'");

  printf("end\n");
  return 0;
}
