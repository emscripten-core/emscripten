/*
 * Copyright 2015 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include <emscripten/emscripten.h>
#include <emscripten/html5.h>

#define DB "THE_DB"

long expected;
int result;

void ok(void* arg) {
  assert(expected == (long)arg);
}

void onerror(void* arg) {
  assert(expected == (long)arg);
  assert(false);
}

void onload(void* arg, void* ptr, int num) {
  assert(expected == (long)arg);
  printf("loaded %s\n", (char*)ptr);
  assert(num == strlen(SECRET)+1);
  assert(strcmp(ptr, SECRET) == 0);
}

void onbadload(void* arg, void* ptr, int num) {
  printf("load failed, surprising\n");
  assert(false);
}

void oncheck(void* arg, int exists) {
  assert(expected == (long)arg);
  printf("exists? %d\n", exists);
  assert(exists);
}

void onchecknope(void* arg, int exists) {
  assert(expected == (long)arg);
  printf("exists (hopefully not)? %d\n", exists);
  assert(!exists);
}

int main() {
  result = STAGE;
#if STAGE == 0
  expected = 12;
  printf("storing %s\n", SECRET);
  emscripten_idb_async_store(DB, "the_secret", SECRET, strlen(SECRET)+1, (void*)expected, ok, onerror);
#elif STAGE == 1
  expected = 31;
  printf("loading the_secret\n");
  emscripten_idb_async_load(DB, "the_secret", (void*)expected, onload, onerror);
#elif STAGE == 2
  expected = 44;
  printf("deleting the_secret\n");
  emscripten_idb_async_delete(DB, "the_secret", (void*)expected, ok, onerror);
#elif STAGE == 3
  expected = 55;
  printf("loading, should fail as we deleted\n");
  emscripten_idb_async_load(DB, "the_secret", (void*)expected, onbadload, ok);
#elif STAGE == 4
  expected = 66;
  emscripten_idb_async_exists(DB, "the_secret", (void*)expected, oncheck, onerror);
#elif STAGE == 5
  expected = 77;
  emscripten_idb_async_exists(DB, "the_secret", (void*)expected, onchecknope, onerror);
#elif STAGE == 6
  expected = 88;
  printf("clearing\n");
  emscripten_idb_async_clear(DB, (void*)expected, ok, onerror);
#else
  assert(0);
#endif
}

