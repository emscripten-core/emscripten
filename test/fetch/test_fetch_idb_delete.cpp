// Copyright 2016 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <string.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <emscripten/fetch.h>

int main()
{
  // 1. Populate file to IndexedDB by a GET.
  emscripten_fetch_attr_t attr;
  emscripten_fetch_attr_init(&attr);
  strcpy(attr.requestMethod, "GET");
  attr.attributes = EMSCRIPTEN_FETCH_REPLACE | EMSCRIPTEN_FETCH_SYNCHRONOUS | EMSCRIPTEN_FETCH_PERSIST_FILE;
  emscripten_fetch_t *fetch = emscripten_fetch(&attr, "gears.png");
  printf("fetch->status: %d\n", fetch->status);
  assert(fetch->status == 200 && "Initial XHR GET of gears.png should have succeeded");
  emscripten_fetch_close(fetch);

  // 2. Make sure it is there.
  emscripten_fetch_attr_init(&attr);
  strcpy(attr.requestMethod, "GET");
  attr.attributes = EMSCRIPTEN_FETCH_NO_DOWNLOAD | EMSCRIPTEN_FETCH_SYNCHRONOUS | EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
  fetch = emscripten_fetch(&attr, "gears.png");
  assert(fetch->status == 200 && "emscripten_fetch from IndexedDB should have found the file");
  assert(fetch->numBytes > 0);
  assert(fetch->data != 0);
  emscripten_fetch_close(fetch);

  // 3. Delete the file from IndexedDB by invoking an Emscripten-specific request "EM_IDB_DELETE".
  emscripten_fetch_attr_init(&attr);
  strcpy(attr.requestMethod, "EM_IDB_DELETE");
  attr.attributes = EMSCRIPTEN_FETCH_SYNCHRONOUS;
  fetch = emscripten_fetch(&attr, "gears.png");
  assert(fetch->status == 200 && "Deleting the file from IndexedDB should have succeeded");
  assert(fetch->numBytes == 0);
  assert(fetch->data == 0);
  emscripten_fetch_close(fetch);

  // 4. Now try to get the file again from IndexedDB, and ensure it is no longer available.
  emscripten_fetch_attr_init(&attr);
  strcpy(attr.requestMethod, "GET");
  attr.attributes = EMSCRIPTEN_FETCH_NO_DOWNLOAD | EMSCRIPTEN_FETCH_SYNCHRONOUS | EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
  fetch = emscripten_fetch(&attr, "gears.png");
  assert(fetch->status == 404 && "Attempting to GET the file from IndexedDB again should have failed after the file has been deleted");
  assert(fetch->numBytes == 0);
  assert(fetch->data == 0);
  emscripten_fetch_close(fetch);

  printf("Test succeeded!\n");

  return 0;
}
