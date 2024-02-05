// Copyright 2016 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <string.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <stdlib.h>
#include <time.h>
#include <emscripten/fetch.h>

#define TEST_SIZE 512000

int main()
{
  // 1. Populate an IndexedDB file entry with custom data bytes in memory.
  emscripten_fetch_attr_t attr;
  emscripten_fetch_attr_init(&attr);
  strcpy(attr.requestMethod, "EM_IDB_STORE");
  attr.attributes = EMSCRIPTEN_FETCH_REPLACE | EMSCRIPTEN_FETCH_SYNCHRONOUS | EMSCRIPTEN_FETCH_PERSIST_FILE;
  uint8_t *data = (uint8_t*)malloc(TEST_SIZE);
  srand(time(NULL));
  for(int i = 0; i < TEST_SIZE; ++i)
    data[i] = (uint8_t)rand() | 0x40;
  attr.requestData = (char *)data;
  attr.requestDataSize = TEST_SIZE;
  emscripten_fetch_t *fetch = emscripten_fetch(&attr, "myfile.dat");
  assert(fetch->status == 200 && "Initial IndexedDB store of myfile.dat should have succeeded");
  emscripten_fetch_close(fetch);

  // 2. Make sure it is there.
  emscripten_fetch_attr_init(&attr);
  strcpy(attr.requestMethod, "GET");
  attr.attributes = EMSCRIPTEN_FETCH_NO_DOWNLOAD | EMSCRIPTEN_FETCH_SYNCHRONOUS | EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
  fetch = emscripten_fetch(&attr, "myfile.dat");
  assert(fetch->status == 200 && "emscripten_fetch from IndexedDB should have found the file");
  assert(fetch->numBytes == TEST_SIZE);
  assert(fetch->totalBytes == TEST_SIZE);
  assert(fetch->data != 0);
  assert(fetch->data != (char *)data); // We should really have obtained a new copy of the memory.
  assert(!memcmp((char *)data, fetch->data, TEST_SIZE));
  emscripten_fetch_close(fetch);

  // 3. Delete the file from IndexedDB by invoking an Emscripten-specific request "EM_IDB_DELETE".
  emscripten_fetch_attr_init(&attr);
  strcpy(attr.requestMethod, "EM_IDB_DELETE");
  attr.attributes = EMSCRIPTEN_FETCH_SYNCHRONOUS;
  fetch = emscripten_fetch(&attr, "myfile.dat");
  assert(fetch->status == 200 && "Deleting the file from IndexedDB should have succeeded");
  assert(fetch->numBytes == 0);
  assert(fetch->data == 0);
  emscripten_fetch_close(fetch);

  // 4. Now try to get the file again from IndexedDB, and ensure it is no longer available.
  emscripten_fetch_attr_init(&attr);
  strcpy(attr.requestMethod, "GET");
  attr.attributes = EMSCRIPTEN_FETCH_NO_DOWNLOAD | EMSCRIPTEN_FETCH_SYNCHRONOUS | EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
  fetch = emscripten_fetch(&attr, "myfile.dat");
  assert(fetch->status == 404 && "Attempting to GET the file from IndexedDB again should have failed after the file has been deleted");
  assert(fetch->numBytes == 0);
  assert(fetch->data == 0);
  emscripten_fetch_close(fetch);

  printf("Test succeeded!\n");
  return 0;
}
