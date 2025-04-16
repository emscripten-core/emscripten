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

#include <emscripten.h>

#define DB "THE_DB"

int main() {
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

  printf("storing %s again\n", SECRET);
  emscripten_idb_store(DB, "the_secret", SECRET, strlen(SECRET)+1, &error);
  assert(!error);
  sum++;

  printf("clearing the store\n");
  emscripten_idb_clear(DB, &error);
  assert(!error);
  sum++;

  printf("last checking\n");
  emscripten_idb_exists(DB, "the_secret", &exists, &error);
  assert(!error);
  assert(!exists);
  sum++;

  // worker-specific code

  printf("save blobs\n");
  const int CHUNK = /*50*/1*1024*1024;
  buffer = malloc(CHUNK);
  char *cbuffer = (char*)buffer;
  cbuffer[0] = 1;
  emscripten_idb_store_blob(DB, "chunk1", buffer, CHUNK, &error);
  cbuffer[0] = 2;
  emscripten_idb_store_blob(DB, "chunk2", buffer, CHUNK, &error);
  assert(!error);
  printf("blobs stored\n");
  cbuffer[0] = 0;
  int blob1, blob2;
  printf("load first\n");
  emscripten_idb_load_blob(DB, "chunk1", &blob1, &error);
  printf("loaded first as %d\n", blob1);
  assert(!error);
  printf("load second\n");
  emscripten_idb_load_blob(DB, "chunk2", &blob2, &error);
  printf("loaded second as %d\n", blob2);
  assert(!error);
  printf("blobs loaded\n");
  emscripten_idb_read_from_blob(blob1, 0, CHUNK, buffer);
  printf("read %d\n", cbuffer[0]);
  assert(cbuffer[0] == 1);
  printf("moving on\n");
  emscripten_idb_read_from_blob(blob2, 0, CHUNK, buffer);
  printf("read %d\n", cbuffer[0]);
  assert(cbuffer[0] == 2);
  //EM_ASM({ alert('freeze') });
  printf("freeing blobs\n");
  emscripten_idb_free_blob(blob1);
  emscripten_idb_free_blob(blob2);

  // finish up

  assert(sum == 8);
  REPORT_RESULT(0);
  return 0;
}
