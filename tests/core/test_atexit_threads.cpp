/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <stdlib.h>

extern "C" {
  int __cxa_thread_atexit(void (*dtor)(void *), void *obj, void *dso_symbol);
}

static void cleanA() { printf("A\n"); }
static void cleanB() { printf("B\n"); }
static void cleanCarg(void* x) { printf("C %ld\n", (long)x); }

int main() {
  atexit(cleanA);
  atexit(cleanB);
  __cxa_thread_atexit(cleanCarg, (void*)100, NULL);
  return 0;
}
