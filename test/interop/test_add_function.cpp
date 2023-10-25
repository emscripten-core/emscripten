// Copyright 2016 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <stdio.h>
#include <stdlib.h>
#include <emscripten.h>

int foo() {
  return 1;
}

int bar() {
  return 2;
}

EMSCRIPTEN_KEEPALIVE
extern "C" int baz() {
  return 3;
}

EM_JS_DEPS(main, "$addFunction,$removeFunction");

int main(int argc, char **argv) {
#if defined(GROWTH)
  EM_ASM({
    // Get an export that isn't in the table (we never took its address in C).
    var baz = wasmExports["baz"];
    var tableSizeBefore = wasmTable.length;
    var bazIndex = addFunction(baz);
    assert(bazIndex >= tableSizeBefore, "we actually added it");
    assert(addFunction(baz) === bazIndex, "we never add it again");
  });
#endif

  int fp = atoi(argv[1]);
  printf("fp: %d\n", fp);
  void (*f)(int) = reinterpret_cast<void (*)(int)>(fp);
  f(7);
  EM_ASM({
    removeFunction($0)
  }, f);

  // We can reuse indexes
  EM_ASM({
    var beforeLength = wasmTable.length;
    for (var i = 0; i < 10; i++) {
      var index = addFunction(function(){}, 'v');
      removeFunction(index);
    }
    assert(wasmTable.length === beforeLength);
  });

  // We guarantee index uniqueness for each function.
  EM_ASM({
    assert(wasmTable.length >= 3);
    assert(addFunction(wasmTable.get(1)) === 1);
    assert(addFunction(wasmTable.get(2)) === 2);
  }, &foo, &bar); // taking the addresses here ensures they are in the table
                  // (the optimizer can't remove these uses) which then lets
                  // us assume the table is of a certain size in the test.
  printf("ok\n");
  return 0;
}
