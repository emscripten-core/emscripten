// Copyright 2016 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <string.h>
#include <stdio.h>
#include <emscripten/fetch.h>
#include <emscripten/emscripten.h>

int main() {
  emscripten_fetch_attr_t attr;
  emscripten_fetch_attr_init(&attr);
  strcpy(attr.requestMethod, "GET");
  attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY | EMSCRIPTEN_FETCH_WAITABLE;
  emscripten_fetch_t *fetch = emscripten_fetch(&attr, "myfile.dat"); // Starts as asynchronous.

  EMSCRIPTEN_RESULT ret = EMSCRIPTEN_RESULT_TIMED_OUT;
  while (ret == EMSCRIPTEN_RESULT_TIMED_OUT) {
    // possibly do some other work
    // milliseconds to wait. 0 to just poll, INFINITY=wait until completion
    ret = emscripten_fetch_wait(fetch, 1);
  }
  if (ret != EMSCRIPTEN_RESULT_SUCCESS) {
    printf("Downloading %s failed: %d.\n", fetch->url, ret);
    return 1;
  }
  // The operation has finished, safe to examine the fields of the 'fetch' pointer now.

  if (fetch->status != 200) {
    printf("Downloading %s failed, HTTP failure status code: %d.\n", fetch->url, fetch->status);    
    return 1;
  }

  printf("Finished downloading %llu bytes from URL %s.\n", fetch->numBytes, fetch->url);
  emscripten_fetch_close(fetch);
  return 0;
}
