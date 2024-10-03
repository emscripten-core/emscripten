// Copyright 2016 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <emscripten/fetch.h>

void downloadSucceeded(emscripten_fetch_t *fetch) {
  printf("Finished downloading %llu bytes from URL %s.\n", fetch->numBytes, fetch->url);
  // The data is now available at fetch->data[0] through fetch->data[fetch->numBytes-1];
  emscripten_fetch_close(fetch); // Free data associated with the fetch.
}

void downloadFailed(emscripten_fetch_t *fetch) {
  printf("Downloading %s failed, HTTP failure status code: %d.\n", fetch->url, fetch->status);
  emscripten_fetch_close(fetch); // Also free data on failure.
  exit(1);
}

void downloadProgress(emscripten_fetch_t *fetch) {
  printf("Downloading %s.. %.2f%s complete. HTTP readyState: %d. HTTP status: %d.\n"
    "HTTP statusText: %s. Received chunk [%llu, %llu[\n",
    fetch->url, (fetch->totalBytes > 0) ? ((fetch->dataOffset + fetch->numBytes) * 100.0 / fetch->totalBytes) : (double)(fetch->dataOffset + fetch->numBytes),
    (fetch->totalBytes > 0) ? "%" : " bytes",
    fetch->readyState, fetch->status, fetch->statusText,
    fetch->dataOffset, fetch->dataOffset + fetch->numBytes);
}

int main() {
  printf("Fetching file\n");
  emscripten_fetch_attr_t attr;
  emscripten_fetch_attr_init(&attr);
  strcpy(attr.requestMethod, "GET");
  attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
  attr.onsuccess = downloadSucceeded;
  attr.onprogress = downloadProgress;
  attr.onerror = downloadFailed;
  emscripten_fetch(&attr, "myfile.dat");

  // Program won't actaully exit until fetch is complete (i.e. when
  // emscripten_fetch_close is called above).
  return 0;
}
