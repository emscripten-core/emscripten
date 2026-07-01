// Copyright 2025 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <emscripten.h>
#include <emscripten/fetch.h>
#include <emscripten/eventloop.h>

bool fetch_abort_queued = false;

int main() {
  emscripten_fetch_attr_t attr;
  emscripten_fetch_attr_init(&attr);
  strcpy(attr.requestMethod, "GET");
  attr.onsuccess = [](emscripten_fetch_t *fetch) {
    printf("Finished downloading %llu bytes\n", fetch->totalBytes);
    emscripten_fetch_close(fetch);
  };
  attr.onerror = [](emscripten_fetch_t *fetch) {
    printf("Downloading failed with status code: %d.\n", fetch->status);
    if (fetch->status != (uint16_t)-1) { // if not aborted with emscripten_fetch_close()
      emscripten_fetch_close(fetch);
    }
  };
  attr.onprogress = [](emscripten_fetch_t *fetch) {
    printf("Downloading.. %.2f%s complete. Received chunk [%llu, %llu[\n", 
      (fetch->totalBytes > 0) ? ((fetch->dataOffset + fetch->numBytes) * 100.0 / fetch->totalBytes) : (double)(fetch->dataOffset + fetch->numBytes),
      (fetch->totalBytes > 0) ? "%" : " bytes",
      fetch->dataOffset,
      fetch->dataOffset + fetch->numBytes);

    if (fetch_abort_queued) return;
    fetch_abort_queued = true;
    emscripten_set_immediate([](void *arg) {
      emscripten_fetch_t *fetch = (emscripten_fetch_t *)arg;
      printf("Abort fetch when downloading\n");
      emscripten_fetch_close(fetch);
    }, fetch);
  };
  attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY | EMSCRIPTEN_FETCH_STREAM_DATA;
  emscripten_fetch(&attr, "largefile.txt");
}
