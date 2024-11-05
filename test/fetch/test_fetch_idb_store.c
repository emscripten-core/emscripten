// Copyright 2016 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <emscripten/fetch.h>

void success(emscripten_fetch_t *fetch)
{
  printf("IDB store succeeded.\n");
  emscripten_fetch_close(fetch);
}

void failure(emscripten_fetch_t *fetch)
{
  printf("IDB store failed.\n");
  emscripten_fetch_close(fetch);
}

void persistFileToIndexedDB(const char *outputFilename, uint8_t *data, size_t numBytes)
{
  emscripten_fetch_attr_t attr;
  emscripten_fetch_attr_init(&attr);
  strcpy(attr.requestMethod, "EM_IDB_STORE");
  attr.attributes = EMSCRIPTEN_FETCH_REPLACE | EMSCRIPTEN_FETCH_PERSIST_FILE;
  attr.requestData = (char *)data;
  attr.requestDataSize = numBytes;
  attr.onsuccess = success;
  attr.onerror = failure;
  emscripten_fetch(&attr, outputFilename);
}

int main()
{
  // Create data
  uint8_t *data = (uint8_t*)malloc(10240);
  srand(time(NULL));
  for(int i = 0; i < 10240; ++i) data[i] = (uint8_t)rand();

  persistFileToIndexedDB("outputfile.dat", data, 10240);
}
