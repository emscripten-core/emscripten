/*
 * Copyright 2015 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include <emscripten.h>

#define DB "THE_DB"

int expected;
int result;

void ok(void* arg)
{
  assert(expected == (int)arg);
  REPORT_RESULT(result);
}

void onerror(void* arg)
{
  assert(expected == (int)arg);
  REPORT_RESULT(999);
}

void onload(void* arg, void* ptr, int num)
{
  assert(expected == (int)arg);
  printf("loaded %s\n", ptr);
  assert(num == strlen(SECRET)+1);
  assert(strcmp(ptr, SECRET) == 0);
  REPORT_RESULT(1);
}

void onbadload(void* arg, void* ptr, int num)
{
  printf("load failed, surprising\n");
  REPORT_RESULT(999);
}

void oncheck(void* arg, int exists)
{
  assert(expected == (int)arg);
  printf("exists? %d\n", exists);
  assert(exists);
  REPORT_RESULT(result);
}

void onchecknope(void* arg, int exists)
{
  assert(expected == (int)arg);
  printf("exists (hopefully not)? %d\n", exists);
  assert(!exists);
  REPORT_RESULT(result);
}

void test() {
  result = STAGE;
#if STAGE == 0
  expected = 12;
  emscripten_idb_async_store(DB, "the_secret", SECRET, strlen(SECRET)+1, (void*)expected, ok, onerror);
  printf("storing %s\n", SECRET);
#elif STAGE == 1
  expected = 31;
  emscripten_idb_async_load(DB, "the_secret", (void*)expected, onload, onerror);
#elif STAGE == 2
  expected = 44;
  emscripten_idb_async_delete(DB, "the_secret", (void*)expected, ok, onerror);
  printf("deleting the_secret\n");
#elif STAGE == 3
  expected = 55;
  emscripten_idb_async_load(DB, "the_secret", (void*)expected, onbadload, ok);
  printf("loading, should fail as we deleted\n");
#elif STAGE == 4
  expected = 66;
  emscripten_idb_async_exists(DB, "the_secret", (void*)expected, oncheck, onerror);
#elif STAGE == 5
  expected = 77;
  emscripten_idb_async_exists(DB, "the_secret", (void*)expected, onchecknope, onerror);
#else
  assert(0);
#endif
}

void never() {
  EM_ASM({ alert('this should never be reached! runtime must not be shut down!') });
  assert(0);
  while (1) {}
}

int main() {
  atexit(never);
  test();
  emscripten_exit_with_live_runtime();
  return 0;
}

