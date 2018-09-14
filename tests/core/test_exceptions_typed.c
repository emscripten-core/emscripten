/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>

class ExFoo {
public:
  int x;
  ExFoo(int x) { this->x = x; printf("*CREATING A FOO\n"); }
  ExFoo(const ExFoo& other)  { x=other.x; printf("*COPYING A FOO\n"); }
  ~ExFoo() { printf("*DESTROYING A FOO (%d)\n", x); }
} ExFooInstance(11);
class ExBar {
public:
  int x;
  ExBar(int x) { this->x = x; printf("*CREATING A BAR\n"); }
  ExBar(const ExBar& other)  { x=other.x; printf("*COPYING A BAR\n"); }
  ~ExBar() { printf("*DESTROYING A BAR (%d)\n", x); }
} ExBarInstance(22);
class ExQuux {
public:
  int x;
  ExQuux(int x) { this->x = x; printf("*CREATING A QUUX\n"); }
  ExQuux(const ExQuux& other)  { x=other.x; printf("*COPYING A QUUX\n"); }
  ~ExQuux() { printf("*DESTROYING A QUUX (%d)\n", x); }
} ExQuuxInstance(33);
class ExChild : public ExQuux {
public:
  ExChild(int x) : ExQuux(x) { printf("*CREATING A CHILD\n"); }
  ExChild(const ExChild& other) : ExQuux(other.x)  { printf("*COPYING CHILD\n"); }
  ~ExChild() { printf("*DESTROYING A CHILD (%d)\n", x); }
} ExChildInstance(44);

void magic(int which) {
  try {
    switch (which) {
      case 0:
        printf("  throwing ExFooInstance\n");
        throw ExFooInstance;
      case 1:
        printf("  throwing ExBarInstance\n");
        throw ExBarInstance;
      case 2:
        printf("  throwing ExQuuxInstance\n");
        throw ExQuuxInstance;
      case 3:
        printf("  throwing ExQuux ptr\n");
        throw &ExQuuxInstance;
      case 4:
        printf("  throwing ExChildInstance\n");
        throw ExChildInstance;
      case 5:
        printf("  throwing ExChildInstance ptr\n");
        throw &ExChildInstance;
      case 6:
        printf("  throwing 42\n");
        throw 42;
      case 7:
        printf("  throwing NULL\n");
        throw (void*)0;
      case 8:
        printf("  not throwing\n");
    }
  } catch (ExQuux e1) {
    printf("inner catch quux: %d\n", e1.x);
  } catch (ExBar e2) {
    printf("inner re-throw: %d\n", e2.x);
    throw;
  }
}

int main() {
  printf("start\n\n\n");
  for (int i = 0; i < 9; i++) {
    printf("test %d\n", i);
    try {
      magic(i);
    } catch (ExFoo e1) {
      printf("outer catch foo: %d\n", e1.x);
    } catch (ExBar& e2) {
      printf("outer catch bar-ref: %d\n", e2.x);
    } catch (ExQuux& e3) {
      printf("outer catch quux-ref: %d\n", e3.x);
    } catch (ExQuux* e4) {
      printf("outer catch quux-ptr: %d\n", e4->x);
    } catch (int e5) {
      printf("outer catch int: %d\n", e5);
    } catch (...) {
      printf("outer catch-all\n");
    }
    printf("\n\n");
  }
  printf("end\n");
  return 0;
}
