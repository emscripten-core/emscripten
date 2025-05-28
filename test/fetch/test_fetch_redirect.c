// Copyright 2025 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <emscripten/fetch.h>

int fetchSync() {
  emscripten_fetch_attr_t attr;
  emscripten_fetch_attr_init(&attr);
  strcpy(attr.requestMethod, "GET");
  attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY | EMSCRIPTEN_FETCH_SYNCHRONOUS | EMSCRIPTEN_FETCH_REPLACE;
  emscripten_fetch_t *fetch = emscripten_fetch(&attr, "https://httpbin.org/status/307");
  assert(fetch);
  printf("Fetch sync finished with status %d\n", fetch->status);
  assert(fetch->status == 200);
  printf("Downloaded %llu bytes", fetch->numBytes);
  assert(strcmp(fetch->responseUrl, "https://httpbin.org/get") == 0);
  exit(0);
}

void onsuccess(emscripten_fetch_t *fetch) {
  printf("Fetch async finished with status %d\n", fetch->status);
  assert(fetch->status == 200);
  printf("Downloaded %llu bytes", fetch->numBytes);
  assert(strcmp(fetch->responseUrl, "https://httpbin.org/get") == 0);
  emscripten_fetch_close(fetch);
  fetchSync();
}

void onreadystatechange(emscripten_fetch_t *fetch) {
  printf("Fetch readyState %d responseUrl %s\n", fetch->readyState, fetch->responseUrl ? fetch->responseUrl : "is null");
  if (fetch->readyState < 2) {
    assert(NULL == fetch->responseUrl);
  } else {
    assert(0 == strcmp(fetch->responseUrl, "https://httpbin.org/get"));
  }
}

int main() {
  emscripten_fetch_attr_t attr;
  emscripten_fetch_attr_init(&attr);
  strcpy(attr.requestMethod, "GET");
  attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
  attr.onsuccess = onsuccess;
  attr.onreadystatechange = onreadystatechange;
  emscripten_fetch_t *fetch = emscripten_fetch(&attr, "https://httpbin.org/status/307");
  assert(fetch);
  return 99;
}
