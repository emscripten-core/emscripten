#include <stdio.h>
#include <memory>
#include <string>
#include <emscripten/bind.h>

using namespace emscripten;

class Test {
 public:
  int function_one(char x, int y) { return 42; }
  int function_two(unsigned char x, int y) { return 43; }
  int function_three(const std::string&) { return 1; }
  int function_four(bool x) { return 2; }

  int const_fn() const { return 0; }

  int getX() const { return x; }
  void setX(int x_) { x = x_; }

  int getY() const { return y; }

  static int static_function(int x) { return 1; }

  static int static_property;

private:
  int x;
  int y;
};

Test class_returning_fn() { return Test(); }

std::unique_ptr<Test> class_unique_ptr_returning_fn() {
  return std::make_unique<Test>();
}

class Foo {
 public:
  void process(const Test& input) {}
};

enum Bar { kValueOne, kValueTwo, kValueThree };

enum EmptyEnum {};

Bar enum_returning_fn() { return kValueOne; }

struct ValArr {
  int x, y, z;
};

struct ValObj {
  Foo foo;
  Bar bar;
};

class ClassWithConstructor {
 public:
  ClassWithConstructor(int, const ValArr&) {}

  int fn(int x) { return 0; }
};

class ClassWithSmartPtrConstructor {
 public:
  ClassWithSmartPtrConstructor(int, const ValArr&) {}

  int fn(int x) { return 0; }
};

int smart_ptr_function(std::shared_ptr<ClassWithSmartPtrConstructor>) {
  return 0;
}

EMSCRIPTEN_DECLARE_VAL_TYPE(CallbackType);

int function_with_callback_param(CallbackType ct) {
  ct(val("hello"));
  return 0;
}

int global_fn(int, int) { return 0; }

class BaseClass {
 public:
  virtual ~BaseClass() = default;
  virtual int fn(int x) { return 0; }
};

class DerivedClass : public BaseClass {
 public:
  int fn(int x) override { return 1; }
  int fn2(int x) { return 2; }
};

EMSCRIPTEN_BINDINGS(Test) {
  class_<Test>("Test")
      .function("functionOne", &Test::function_one)
      .function("functionTwo", &Test::function_two)
      .function("functionThree", &Test::function_three)
      .function("functionFour", &Test::function_four)
      .function("functionFive(x, y)", &Test::function_one)
      .function("functionSix(str)", &Test::function_three)
      .function("constFn", &Test::const_fn)
      .property("x", &Test::getX, &Test::setX)
      .property("y", &Test::getY)
      .class_function("staticFunction", &Test::static_function)
      .class_function("staticFunctionWithParam(x)", &Test::static_function)
      .class_property("staticProperty", &Test::static_property)
	;

  function("class_returning_fn", &class_returning_fn);
  function("class_unique_ptr_returning_fn",
                   &class_unique_ptr_returning_fn);

  constant("an_int", 5);
  constant("a_bool", false);
  constant("an_enum", Bar::kValueOne);
  constant("a_class_instance", Test());

  enum_<Bar>("Bar")
      .value("valueOne", Bar::kValueOne)
      .value("valueTwo", Bar::kValueTwo)
      .value("valueThree", Bar::kValueThree);
  enum_<EmptyEnum>("EmptyEnum");

  function("enum_returning_fn", &enum_returning_fn);

  value_array<ValArr>("ValArr")
      .element(&ValArr::x)
      .element(&ValArr::y)
      .element(&ValArr::z);

  value_array<std::array<Bar, 4>>("ValArrIx")
    .element(emscripten::index<0>())
    .element(emscripten::index<1>())
    .element(emscripten::index<2>())
    .element(emscripten::index<3>());

  value_object<ValObj>("ValObj")
      .field("foo", &ValObj::foo)
      .field("bar", &ValObj::bar);

  register_vector<int>("IntVec");

  class_<Foo>("Foo").function("process", &Foo::process);

  function("global_fn", &global_fn);

  class_<ClassWithConstructor>("ClassWithConstructor")
      .constructor<int, const ValArr&>()
      .function("fn", &ClassWithConstructor::fn);

  class_<ClassWithSmartPtrConstructor>("ClassWithSmartPtrConstructor")
      .smart_ptr_constructor(
          "ClassWithSmartPtrConstructor",
          &std::make_shared<ClassWithSmartPtrConstructor, int, const ValArr&>)
      .function("fn", &ClassWithSmartPtrConstructor::fn);

  function("smart_ptr_function", &smart_ptr_function);
  function("smart_ptr_function_with_params(foo)", &smart_ptr_function);

  function("function_with_callback_param",
           &function_with_callback_param);

  register_type<CallbackType>("(message: string) => void");

  class_<BaseClass>("BaseClass").function("fn", &BaseClass::fn);

  class_<DerivedClass, base<BaseClass>>("DerivedClass")
      .function("fn2", &DerivedClass::fn2);
}

int Test::static_property = 42;

int main() {
  // Main should not be run during TypeScript generation, but should run when
  // the program is run normally.
  printf("main ran\n");
  return 0;
}
