// Copyright 2023 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <emscripten.h>
#include <emscripten/bind.h>

using namespace emscripten;

void voidFunc() { emscripten_sleep(0); }

int intFunc(int i) {
  emscripten_sleep(0);
  return i + 1;
}

class MyClass {
public:
  MyClass() {}

  void voidMethod() { emscripten_sleep(0); }

  int intMethod(int i) {
    emscripten_sleep(0);
    return i + 1;
  }

  static void voidClass() { emscripten_sleep(0); }

  static int intClass(int i) {
    emscripten_sleep(0);
    return i + 1;
  }
};

int stdFunction(const MyClass& target, int i) {
  emscripten_sleep(0);
  return i + 1;
}

EM_ASYNC_JS(void, jsSuspend, (), {
  await new Promise(resolve => {
    Module.unsuspendResolve = resolve;
  });
});

void suspend() {
  jsSuspend();
};

void unsuspend() {
  EM_ASM({
    Module.unsuspendResolve();
  });
}

EMSCRIPTEN_BINDINGS(xxx) {
  function("voidFunc", &voidFunc, async());
  function("intFunc", &intFunc, async());
  function("unsuspend", &unsuspend);
  function("suspend", &suspend, async());

  class_<MyClass>("MyClass")
    .constructor<>()
    .function("voidMethod", &MyClass::voidMethod, async())
    .function("intMethod", &MyClass::intMethod, async())
    .function("stdMethod",
              std::function<int(const MyClass&, int)>(&stdFunction),
              async())
    .function("lambdaMethod",
              select_overload<int(MyClass&, int)>([](MyClass& self, int i) {
                emscripten_sleep(0);
                return i + 1;
              }),
              async())
    .class_function("voidClass", &MyClass::voidClass, async())
    .class_function("intClass", &MyClass::intClass, async());
}

EM_ASYNC_JS(void, test, (), {
  async function check(promise, expected) {
    assert(promise instanceof Promise);
    var actual = await promise;
    assert(actual === expected);
  }
  try {
    await check(Module.intFunc(1), 2);
    var myClass = new Module.MyClass();
    await check(myClass.voidMethod());
    await check(myClass.intMethod(1), 2);
    await check(myClass.stdMethod(1), 2);
    await check(myClass.lambdaMethod(1), 2);
    await check(Module.MyClass.voidClass());
    await check(Module.MyClass.intClass(1), 2);

    setTimeout(Module.unsuspend);
    await Module.suspend();

    out('done');
  } catch (e) {
    out('Failed: ' + e.stack);
  }
});

int main() { test(); }
