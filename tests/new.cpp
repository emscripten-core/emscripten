// Emscripten tests

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

struct Structy { char data[100]; int x; };

int main() {
  int NUM = 100;
  char* allocations[NUM];
  for (int i = 0; i < NUM/2; i++) {
    allocations[i] = (char*){{{ NEW }}};
    assert(allocations[i]);
    if (i > 10 && i%4 == 1 && allocations[i-10]) {
      {{{ DELETE }}}(allocations[i-10]);
      allocations[i-10] = NULL;
    }
  }
  for (int i = NUM/2; i < NUM; i++) {
    allocations[i] = (char*){{{ NEW }}};
    assert(allocations[i]);
    if (i > 10 && i%4 != 1 && allocations[i-10]) {
      {{{ DELETE }}}(allocations[i-10]);
      allocations[i-10] = NULL;
    }
  }
  char* first = allocations[0];
  for (int i = 0; i < NUM; i++) {
    if (allocations[i]) {
      {{{ DELETE }}}(allocations[i]);
    }
  }
  char *last = (char*){{{ NEW }}}; /* should be identical, as we free'd it all */
  char *newer = (char*){{{ NEW }}}; /* should be different */
  printf("*%d,%d*\n", first == last, first == newer);
}

