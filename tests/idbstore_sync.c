#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

#include <emscripten.h>

#define DB "THE_DB"

void test() {
  void *buffer;
  int num, error, exists;
  int sum = 0;

  printf("storing %s\n", SECRET);
  emscripten_idb_store(DB, "the_secret", SECRET, strlen(SECRET)+1, &error);
  assert(!error);
  sum++;

  printf("checking\n");
  emscripten_idb_exists(DB, "the_secret", &exists, &error);
  assert(!error);
  assert(exists);
  sum++;

  printf("loading\n");
  emscripten_idb_load(DB, "the_secret", &buffer, &num, &error);
  assert(!error);
  char *ptr = buffer;
  printf("loaded %s\n", ptr);
  assert(num == strlen(SECRET)+1);
  assert(strcmp(ptr, SECRET) == 0);
  free(buffer);
  sum++;

  printf("deleting the_secret\n");
  emscripten_idb_delete(DB, "the_secret", &error);
  assert(!error);
  sum++;

  printf("loading, should fail as we deleted\n");
  emscripten_idb_load(DB, "the_secret", &buffer, &num, &error);
  assert(error); // expected error!
  sum++;

  printf("last checking\n");
  emscripten_idb_exists(DB, "the_secret", &exists, &error);
  assert(!error);
  assert(!exists);
  sum++;

  int result = sum;
  REPORT_RESULT();
}

void never() {
  EM_ASM({ alert('this should never be reached! runtime must not be shut down!') });
  assert(0);
  while (1) {}
}

int main() {
  atexit(never);
  test();
  return 0;
}

