#include <stdio.h>
#include <functional>
#include <emscripten.h>
#include <emscripten/bind.h>

using namespace emscripten;

// Counters to track Value structs lifetimes.
static int default_constructed = 0, copy_constructed = 0,
           move_constructed = 0, destructed = 0;

struct Value {
  std::string name;
  Value() : name("initial") {
    printf("create\n");
    default_constructed++;
  }
  Value(const Value &v) {
    printf("copy\n");
    name = v.name;
    copy_constructed++;
  }
  Value(Value&& v) {
    printf("move\n");
    name = std::move(v.name);
    move_constructed++;
  }
  Value& operator=(Value other) {
    printf("copy assign\n");
    return *this;
  }
  std::string getName() {
    return name;
  }
  void setName(std::string _name) {
    name = _name;
  }
  ~Value() {
    printf("destroy\n");
    destructed++;
  }
};

void resetCounters() {
  default_constructed = 0;
  copy_constructed = 0,
  move_constructed = 0;
  destructed = 0;
}

void assertDefault(int count) {
  assert(count == default_constructed);
  default_constructed = 0;
}

void assertCopy(int count) {
  assert(count == copy_constructed);
  copy_constructed = 0;
}

void assertMove(int count) {
  assert(count == move_constructed);
  move_constructed = 0;
}

void assertDestructed(int count) {
  assert(count == destructed);
  destructed = 0;
}

void assertAllCountsZero() {
  assert(default_constructed == 0);
  assert(copy_constructed == 0);
  assert(move_constructed == 0);
  assert(destructed == 0);
}

struct ValueHolder {
  ValueHolder() {};
  Value value;
  Value* getPtr() {
    return &value;
  }
  Value* getNewPtr() {
    return new Value();
  }
  Value getValue() {
    return value;
  }
  Value& getRef() {
    return value;
  }
  static Value* staticGetPtr() {
    static Value v;
    return &v;
  }
};

Value* value_holder(ValueHolder& target) {
  return target.getNewPtr();
}

Value* _valuePtr;
Value _value;

void initializeValuePtr() {
  _valuePtr = new Value();
}

void resetValue() {
  _value.name = "initial";
}

void deleteValue() {
  delete _valuePtr;
}

Value* getPtr() {
  return _valuePtr;
};

Value* getNewPtr() {
  return new Value();
};

Value getValue() {
  return Value();
};

Value& getRef() {
  return _value;
};

int main() {
  EM_ASM(
    /////////////////////////////////
    // Test RVP with plain functions.
    /////////////////////////////////
    Module.resetCounters();

    console.log("getPtrTakeOwnership");
    var value = Module.getPtrTakeOwnership();
    value.delete();
    Module.assertDefault(1);
    Module.assertDestructed(1);
    Module.assertAllCountsZero();

    console.log("getPtrReference");
    Module.initializeValuePtr();
    value = Module.getPtrReference();
    // Check that both references are updated.
    value.setName("reference");
    value = Module.getPtrReference();
    assert(value.getName() === "reference");
    // TODO: enforce that we can't delete it from JS.
    Module.deleteValue();
    Module.assertDefault(1);
    Module.assertDestructed(1);
    Module.assertAllCountsZero();

    console.log("getValueDefault");
    value = Module.getValueDefault();
    value.delete();
    Module.assertDefault(1);
    Module.assertCopy(1);
    Module.assertDestructed(2);
    Module.assertAllCountsZero();

    console.log("getValueTakeOwnership");
    value = Module.getValueTakeOwnership();
    value.delete();
    Module.assertDefault(1);
    Module.assertMove(1);
    Module.assertDestructed(2);
    Module.assertAllCountsZero();

    console.log("getRefDefault");
    value = Module.getRefDefault();
    value.delete();
    Module.assertCopy(1);
    Module.assertDestructed(1);
    Module.assertAllCountsZero();

    console.log("getRefTakeOwnership");
    value = Module.getRefTakeOwnership();
    value.delete();
    Module.assertMove(1);
    Module.assertDestructed(1);
    Module.assertAllCountsZero();

    console.log("getRefReference");
    value = Module.getRefReference();
    Module.assertAllCountsZero();

    ///////////////////////////////
    // Test RVP with class methods.
    ///////////////////////////////
    console.log("Module.ValueHolder");
    var valueHolder = new Module.ValueHolder();
    Module.resetCounters();

    console.log("valueHolder.getPtrTakeOwnership");
    value = valueHolder.getPtrTakeOwnership();
    value.delete();
    Module.assertDefault(1);
    Module.assertDestructed(1);
    Module.assertAllCountsZero();

    console.log("valueHolder.getPtrReference");
    value = valueHolder.getPtrReference();
    Module.assertAllCountsZero();

    console.log("valueHolder.getPtrWithStdFunctionTakeOwnership");
    value = valueHolder.getPtrWithStdFunctionTakeOwnership();
    value.delete();
    Module.assertDefault(1);
    Module.assertDestructed(1);
    Module.assertAllCountsZero();

    valueHolder.delete();

    ////////////////////////////////////////
    // Test RVP with static class functions.
    ////////////////////////////////////////
    // These bindings use the same machinery as regular function bindings,
    // so don't bother extensively testing them.
    Module.resetCounters();
    console.log("staticGetPtrReference");
    value = Module.ValueHolder.staticGetPtrReference();
    Module.assertDefault(1);
    Module.assertAllCountsZero();
  );
}

EMSCRIPTEN_BINDINGS(xxx) {
  function("resetCounters", &resetCounters);
  function("initializeValuePtr", &initializeValuePtr);
  function("deleteValue", &deleteValue);
  function("assertAllCountsZero", &assertAllCountsZero);
  function("assertDefault", &assertDefault);
  function("assertCopy", &assertCopy);
  function("assertMove", &assertMove);
  function("assertDestructed", &assertDestructed);

  class_<Value>("Value")
    .function("getName", &Value::getName)
    .function("setName", &Value::setName);

  // Skip the test for getPtr with a default return policy since raw pointers
  // must explicitly choose a policy.
  function("getPtrTakeOwnership", &getNewPtr, return_value_policy::take_ownership());
  function("getPtrReference", &getPtr, return_value_policy::reference());

  function("getValueDefault", &getValue);
  function("getValueTakeOwnership", &getValue, return_value_policy::take_ownership());
  // Skip the test for getValue with a reference return policy since it is not
  // allowed.

  function("getRefDefault", &getRef);
  function("getRefTakeOwnership", &getRef, return_value_policy::take_ownership());
  function("getRefReference", &getRef, return_value_policy::reference());

  class_<ValueHolder>("ValueHolder")
    .constructor<>()
    .function("getPtrTakeOwnership", &ValueHolder::getNewPtr, return_value_policy::take_ownership())
    .function("getPtrReference", &ValueHolder::getPtr, return_value_policy::reference())
    .function("getPtrWithStdFunctionTakeOwnership", std::function<Value*(ValueHolder&)>(&value_holder), return_value_policy::take_ownership())
    .class_function("staticGetPtrReference", &ValueHolder::staticGetPtr, return_value_policy::reference());
    ;
}
