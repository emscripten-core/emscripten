/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>

class CBase {
  virtual void dummy() {}
};
class CDerived : public CBase {
  int a;
};
class CDerivedest : public CDerived {
  float b;
};

int main() {
  CBase *pa = new CBase;
  CBase *pb = new CDerived;
  CBase *pc = new CDerivedest;

  printf("a1: %d\n", dynamic_cast<CDerivedest *>(pa) != NULL);
  printf("a2: %d\n", dynamic_cast<CDerived *>(pa) != NULL);
  printf("a3: %d\n", dynamic_cast<CBase *>(pa) != NULL);

  printf("b1: %d\n", dynamic_cast<CDerivedest *>(pb) != NULL);
  printf("b2: %d\n", dynamic_cast<CDerived *>(pb) != NULL);
  printf("b3: %d\n", dynamic_cast<CBase *>(pb) != NULL);

  printf("c1: %d\n", dynamic_cast<CDerivedest *>(pc) != NULL);
  printf("c2: %d\n", dynamic_cast<CDerived *>(pc) != NULL);
  printf("c3: %d\n", dynamic_cast<CBase *>(pc) != NULL);

  delete pa;
  delete pb;
  delete pc;

  return 0;
}
