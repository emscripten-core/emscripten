// Copyright 2016 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <emscripten/fetch.h>

void readyStateChange(emscripten_fetch_t *fetch) {
  if (fetch->readyState != 2) return;

  size_t headersLengthBytes = emscripten_fetch_get_response_headers_length(fetch) + 1;
  char *headerString = malloc(headersLengthBytes);

  assert(headerString);
  emscripten_fetch_get_response_headers(fetch, headerString, headersLengthBytes);
  printf("Got headers: %s\n", headerString);

  char **responseHeaders = emscripten_fetch_unpack_response_headers(headerString);
  assert(responseHeaders);

  free(headerString);

  int numHeaders = 0;
  for (; responseHeaders[numHeaders * 2]; ++numHeaders) {
    // Check both the header and its value are present.
    assert(responseHeaders[(numHeaders * 2) + 1]);
    printf("Got response header: %s:%s\n", responseHeaders[numHeaders * 2], responseHeaders[(numHeaders * 2) + 1]);
  }

  printf("Finished receiving %d headers from URL %s.\n", numHeaders, fetch->url);

  emscripten_fetch_free_unpacked_response_headers(responseHeaders);
}

void success(emscripten_fetch_t *fetch) {
  printf("Finished downloading %llu bytes from URL %s.\n", fetch->numBytes, fetch->url);
  // The data is now available at fetch->data[0] through fetch->data[fetch->numBytes-1];
  emscripten_fetch_close(fetch); // Free data associated with the fetch.
}

void onerror(emscripten_fetch_t *fetch) {
  printf("onerror: %d '%s'\n", fetch->status, fetch->statusText);
  abort();
}

int main() {
  emscripten_fetch_attr_t attr;
  emscripten_fetch_attr_init(&attr);
  strcpy(attr.requestMethod, "GET");
  attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY | EMSCRIPTEN_FETCH_REPLACE;
  attr.onsuccess = success;
  attr.onerror = onerror;
  attr.onreadystatechange = readyStateChange;
  attr.timeoutMSecs = 2*60;
  printf("Calling emscripten_fetch\n");
  emscripten_fetch(&attr, "myfile.dat");
  return 0;
}
