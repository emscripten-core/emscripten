// Copyright 2014 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include "test.h"

Parent::Parent(int val) : value(val), attr(6), immutableAttr(8) {
  printf("Parent:%d\n", val);
}

Parent::Parent(Parent* p, Parent* q)
  : value(p->value + q->value), attr(6), immutableAttr(8) {
  printf("Parent:%d\n", value);
}

void Parent::mulVal(int mul) { value *= mul; }

typedef EnumClass::EnumWithinClass EnumClass_EnumWithinClass;
typedef EnumNamespace::EnumInNamespace EnumNamespace_EnumInNamespace;

#include "glue.cpp"

#ifdef BROWSER
int main() {
  printf("main().\n");
  EM_ASM({
    // simple test that everything is functional
    var sme = new Module.Parent(42);
    sme.mulVal(2);
    var got = sme.getVal();
    assert(got === 84, "got: " + got);
  });
  REPORT_RESULT(1);
}
#endif

