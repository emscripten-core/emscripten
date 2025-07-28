// Copyright 2025 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <emscripten/bind.h>
#include <emscripten/console.h>
#include <emscripten/emscripten.h>

using namespace emscripten;

// Simple test class with T type params and returns
template<typename T>
class TestClass {
public:
  TestClass(T v) : val(v) { }
  T getVal() const {
    return val;
  }
  void setVal(T v) {
    val = v;
  }
private:
  T val;
};

// One for each of the embind types
typedef TestClass<float> TestClassFloat;
typedef TestClass<double> TestClassDouble;
typedef TestClass<char> TestClassChar;
typedef TestClass<short> TestClassShort;
typedef TestClass<int> TestClassInt;
typedef TestClass<long> TestClassLong;

// Setup a test for each of the bound types to construct, get and set each
EMSCRIPTEN_BINDINGS(TestClasses) {
  class_<TestClassFloat>("TestClassFloat")
    .constructor<float>()
    .function("getVal", &TestClassFloat::getVal)
    .function("setVal", &TestClassFloat::setVal)
    .property("val", &TestClassFloat::getVal, &TestClassFloat::setVal);
  class_<TestClassDouble>("TestClassDouble")
    .constructor<double>()
    .function("getVal", &TestClassDouble::getVal)
    .function("setVal", &TestClassDouble::setVal)
    .property("val", &TestClassDouble::getVal, &TestClassDouble::setVal);
  class_<TestClassChar>("TestClassChar")
    .constructor<char>()
    .function("getVal", &TestClassChar::getVal)
    .function("setVal", &TestClassChar::setVal)
    .property("val", &TestClassChar::getVal, &TestClassChar::setVal);
  class_<TestClassShort>("TestClassShort")
    .constructor<short>()
    .function("getVal", &TestClassShort::getVal)
    .function("setVal", &TestClassShort::setVal)
    .property("val", &TestClassShort::getVal, &TestClassShort::setVal);
  class_<TestClassInt>("TestClassInt")
    .constructor<int>()
    .function("getVal", &TestClassInt::getVal)
    .function("setVal", &TestClassInt::setVal)
    .property("val", &TestClassInt::getVal, &TestClassInt::setVal);
  class_<TestClassLong>("TestClassLong")
    .constructor<long>()
    .function("getVal", &TestClassLong::getVal)
    .function("setVal", &TestClassLong::setVal)
    .property("val", &TestClassLong::getVal, &TestClassLong::setVal);
}

int main() {
  EM_ASM(
    // Instantiate then get/set
    function runTest(cls) {
      out('Testing: ' + cls.name);
      var c = new cls(Math.PI);
      c.setVal(Math.SQRT2);
      out('getVal: ' + c.getVal());
      c.val = Math.PI;
      out('val: ' + c.val);
      c.delete();
    }
    runTest(Module.TestClassFloat);
    runTest(Module.TestClassDouble);
    runTest(Module.TestClassChar);
    runTest(Module.TestClassShort);
    runTest(Module.TestClassInt);
    runTest(Module.TestClassLong);
  );
  return 0;
}
