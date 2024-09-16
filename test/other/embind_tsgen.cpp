#include <stdio.h>
#include <memory>
#include <string>
#include <optional>
#include <emscripten/bind.h>

using namespace emscripten;

class Test {
 public:
  int function_one(char x, int y) { return 42; }
  int function_two(unsigned char x, int y) { return 43; }
  int function_three(const std::string&) { return 1; }
  int function_four(bool x) { return 2; }

  long long_fn(unsigned long a) { return 3; }

  int const_fn() const { return 0; }

  int getX() const { return x; }
  void setX(int x_) { x = x_; }

  int getY() const { return y; }

  std::string string_property;

  static int static_function(int x) { return 1; }

  static int static_property;
  static std::string static_string_property;

private:
  int x;
  int y;
};

class Foo {
 public:
  void process(const Test& input) {}
};

Test class_returning_fn() { return Test(); }

std::unique_ptr<Test> class_unique_ptr_returning_fn() {
  return std::make_unique<Test>();
}

enum Bar { kValueOne, kValueTwo, kValueThree };

enum EmptyEnum {};

Bar enum_returning_fn() { return kValueOne; }

struct ValArr {
  int x, y, z;
};

EMSCRIPTEN_DECLARE_VAL_TYPE(CallbackType);

struct ValObj {
  Foo foo;
  Bar bar;
  CallbackType callback;
  ValObj() : callback(val::undefined()) {}
};

class ClassWithConstructor {
 public:
  ClassWithConstructor(int, const ValArr&) {}

  int fn(int x) { return 0; }
};

class ClassWithTwoConstructors {
 public:
  ClassWithTwoConstructors() {}
  ClassWithTwoConstructors(int) {}
};

class ClassWithSmartPtrConstructor {
 public:
  ClassWithSmartPtrConstructor(int, const ValArr&) {}

  int fn(int x) { return 0; }
};

int smart_ptr_function(std::shared_ptr<ClassWithSmartPtrConstructor>) {
  return 0;
}

struct Obj {};
Obj* get_pointer(Obj* ptr) { return ptr; }
Obj* get_nonnull_pointer() { return new Obj(); }

int function_with_callback_param(CallbackType ct) {
  ct(val("hello"));
  return 0;
}

int global_fn(int, int) { return 0; }

std::string string_test(std::string arg) {
  return "hi";
}

std::wstring wstring_test(std::wstring arg) {
  return L"hi";
}

std::optional<int> optional_test(std::optional<Foo> arg) {
  return {};
}

std::optional<int> optional_and_nonoptional_test(std::optional<Foo> arg1, int arg2) {
  return {};
}

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

struct Interface {
  virtual void invoke(const std::string& str) = 0;
  virtual ~Interface() {}
};

struct InterfaceWrapper : public wrapper<Interface> {
  EMSCRIPTEN_WRAPPER(InterfaceWrapper);
  void invoke(const std::string& str) {
      return call<void>("invoke", str);
  }
};

EMSCRIPTEN_BINDINGS(Test) {
  class_<Test>("Test")
      .function("functionOne", &Test::function_one)
      .function("functionTwo", &Test::function_two)
      .function("functionThree", &Test::function_three)
      .function("functionFour", &Test::function_four)
      .function("functionFive(x, y)", &Test::function_one)
      .function("functionSix(str)", &Test::function_three)
      .function("longFn", &Test::long_fn)
      .function("constFn", &Test::const_fn)
      .property("x", &Test::getX, &Test::setX)
      .property("y", &Test::getY)
      .property("stringProperty", &Test::string_property)
      .class_function("staticFunction", &Test::static_function)
      .class_function("staticFunctionWithParam(x)", &Test::static_function)
      .class_property("staticProperty", &Test::static_property)
      .class_property("staticStringProperty", &Test::static_string_property)
	;

  function("class_returning_fn", &class_returning_fn);
  function("class_unique_ptr_returning_fn",
                   &class_unique_ptr_returning_fn);
  class_<Obj>("Obj");
  function("getPointer", &get_pointer, allow_raw_pointers());
  function("getNonnullPointer", &get_nonnull_pointer, allow_raw_pointers(), nonnull<ret_val>());

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
      .field("bar", &ValObj::bar)
      .field("callback", &ValObj::callback);

  register_vector<int>("IntVec");

  register_map<int, int>("MapIntInt");

  class_<Foo>("Foo").function("process", &Foo::process);

  function("global_fn", &global_fn);

  register_optional<int>();
  register_optional<Foo>();
  function("optional_test", &optional_test);
  function("optional_and_nonoptional_test", &optional_and_nonoptional_test);

  function("string_test", &string_test);
  function("wstring_test", &wstring_test);

  class_<ClassWithConstructor>("ClassWithConstructor")
      .constructor<int, const ValArr&>()
      .function("fn", &ClassWithConstructor::fn);

  // The last defined constructor should be used in the definition.
  class_<ClassWithTwoConstructors>("ClassWithTwoConstructors")
      .constructor<>()
      .constructor<int>();

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

  class_<Interface>("Interface")
    .function("invoke", &Interface::invoke, pure_virtual())
    .allow_subclass<InterfaceWrapper>("InterfaceWrapper")
    ;
}

int Test::static_property = 42;
std::string Test::static_string_property = "";

int main() {
  // Main should not be run during TypeScript generation, but should run when
  // the program is run normally.
  printf("main ran\n");
  return 0;
}
