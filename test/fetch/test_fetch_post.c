// Copyright 2023 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

// Upload a file by POST'ing with ?file=xx in the query string and then
// attempt to download the file

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <emscripten/fetch.h>

void onError(emscripten_fetch_t *fetch) {
  printf("onError: %d %s\n", fetch->status, fetch->statusText);
  assert(false);
}

void onGetSuccess(emscripten_fetch_t *fetch) {
  printf("onGetSuccess URL=%s status=%d numBytes=%llu\n",
         fetch->url,
         fetch->status,
         fetch->numBytes);
  assert(fetch->status == 200);
  printf("data: %.*s\n", (int)fetch->numBytes, fetch->data);
  assert(strncmp(fetch->data, "Hello, world!", fetch->numBytes) == 0);
  exit(0);
}

void doGet() {
  emscripten_fetch_attr_t attr;
  emscripten_fetch_attr_init(&attr);
  attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
  attr.onsuccess = onGetSuccess;
  attr.onerror = onError;
  emscripten_fetch(&attr, "newfile.txt");
}

void onPostSuccess(emscripten_fetch_t *fetch) {
  printf("onPostSuccess URL=%s status=%d\n", fetch->url, fetch->status);
  assert(fetch->status == 200);
  // The data is now available at fetch->data[0] through fetch->data[fetch->numBytes-1];
  emscripten_fetch_close(fetch); // Free data associated with the fetch.

  // Now attempt to GET the POST'ed file
  doGet();
}

int main() {
  // Upload a file using `POST` with `?file=`
  emscripten_fetch_attr_t attr;
  emscripten_fetch_attr_init(&attr);
  strcpy(attr.requestMethod, "POST");
  attr.onsuccess = onPostSuccess;
  attr.onerror = onError;
  attr.requestData = "Hello, world!";
  attr.requestDataSize = strlen(attr.requestData);
  emscripten_fetch(&attr, "?file=newfile.txt");

  // This return code should be ignored
  return 99;
}
