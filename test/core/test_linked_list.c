/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
struct worker_args {
  int value;
  struct worker_args* next;
};

int main() {
  struct worker_args a;
  struct worker_args b;
  a.value = 60;
  a.next = &b;
  b.value = 900;
  b.next = NULL;
  struct worker_args* c = &a;
  int total = 0;
  while (c) {
    total += c->value;
    c = c->next;
  }

  // Chunk of em
  struct worker_args chunk[10];
  for (int i = 0; i < 9; i++) {
    chunk[i].value = i * 10;
    chunk[i].next = &chunk[i + 1];
  }
  chunk[9].value = 90;
  chunk[9].next = &chunk[0];

  c = chunk;
  do {
    total += c->value;
    c = c->next;
  } while (c != chunk);

  printf("*%d,%p*\n", total, b.next);
  // NULL *is* 0, in C/C++. No JS null! (null == 0 is false, etc.)

  return 0;
}
