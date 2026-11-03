// Copyright 2026 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <emscripten/eventloop.h>
#include <emscripten/fetch.h>

int onerror_count = 0;

void downloadSucceeded(emscripten_fetch_t *fetch) {
  printf("downloadSucceeded\n");
  assert(0 && "onsuccess should not be called for a failed request");
}

void downloadProgress(emscripten_fetch_t *fetch) {
  printf("downloadProgress\n");
  assert(0 && "onprogress should not be called for a failed request");
}

void checkDone(void *arg) {
  assert(onerror_count == 1);
}

void downloadFailed(emscripten_fetch_t *fetch) {
  printf("downloadFailed: readyState=%d status=%d statusText=%s\n",
         fetch->readyState, fetch->status, fetch->statusText);
  onerror_count++;
  assert(onerror_count == 1);
  assert(fetch->readyState == 4); // DONE
  assert(fetch->status == 0);
  assert(strlen(fetch->statusText) > 0);
  emscripten_fetch_close(fetch);
  // Defer exit by a turn of the event loop to ensure no duplicate callbacks
  // (e.g. onload/onerror) are fired after onerror returns.
  emscripten_set_immediate(checkDone, NULL);
}

int main() {
  emscripten_fetch_attr_t attr;
  emscripten_fetch_attr_init(&attr);
  strcpy(attr.requestMethod, "GET");
  attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY | EMSCRIPTEN_FETCH_STREAM_DATA;
  attr.onsuccess = downloadSucceeded;
  attr.onprogress = downloadProgress;
  attr.onerror = downloadFailed;
  emscripten_fetch(&attr, "unknownprotocol://example");
  return 0;
}
