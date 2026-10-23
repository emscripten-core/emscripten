// Copyright 2026 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

// A value_object writes its fields in the enumeration order of its field
// table, integer-like names first, and a name registered twice keeps only
// its last registration. Both must hold on the generated and the closure
// write paths, and match the read order.

#include <assert.h>
#include <stdio.h>

#include <emscripten/bind.h>
#include <emscripten/emscripten.h>

struct Rec {
  int b;
  int two;
  int a;
  int first;
  int last;
};

static int lastSetterCalls = 0;
static int firstSetterCalls = 0;

int getA(const Rec& r) { return r.a; }
void setA(Rec& r, int v) { r.a = v; }
int getB(const Rec& r) { return r.b; }
void setB(Rec& r, int v) { r.b = v; }
int getTwo(const Rec& r) { return r.two; }
void setTwo(Rec& r, int v) { r.two = v; }
int getFirst(const Rec& r) { return r.first; }
void setFirst(Rec& r, int v) { ++firstSetterCalls; r.first = v; }
int getLast(const Rec& r) { return r.last; }
void setLast(Rec& r, int v) { ++lastSetterCalls; r.last = v; }

int sum(Rec r) {
  return r.b + r.two + r.a + r.first + r.last;
}

int firstCalls() { return firstSetterCalls; }
int lastCalls() { return lastSetterCalls; }

EMSCRIPTEN_BINDINGS(field_order) {
  emscripten::value_object<Rec>("Rec")
      .field("b", &getB, &setB)
      .field("2", &getTwo, &setTwo)
      .field("a", &getA, &setA)
      // "x" registered twice: only the second registration must write.
      .field("x", &getFirst, &setFirst)
      .field("x", &getLast, &setLast);

  emscripten::function("sum", &sum);
  emscripten::function("firstCalls", &firstCalls);
  emscripten::function("lastCalls", &lastCalls);
}

int main() {
  EM_ASM({
    var order = [];
    var o = {};
    var define = (name, value) => Object.defineProperty(o, name, {
      get: () => { order.push(name); return value; }, enumerable: true
    });
    define('b', 1);
    define('2', 20);
    define('a', 300);
    define('x', 4000);
    if (Module['sum'](o) !== 4321) throw 'sum';
    // Integer-like names enumerate first, then insertion order.
    if (order.join() !== '2,b,a,x') throw 'write order: ' + order.join();
    if (Module['firstCalls']() !== 0) throw 'first registration of x wrote';
    if (Module['lastCalls']() !== 1) throw 'last registration of x did not write once';
  });
  printf("done\n");
  return 0;
}
