// Copyright 2016 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <string.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <emscripten/fetch.h>

int main() {
  emscripten_fetch_attr_t attr;
  emscripten_fetch_attr_init(&attr);
  strcpy(attr.requestMethod, "GET");
  attr.userData = (void*)0x12345678;
  attr.attributes = EMSCRIPTEN_FETCH_REPLACE | EMSCRIPTEN_FETCH_PERSIST_FILE;

  attr.onsuccess = [](emscripten_fetch_t *fetch) {
    assert(fetch);
    printf("Finished downloading %llu bytes\n", fetch->totalBytes);
    assert(fetch->url);
    assert(!strcmp(fetch->url, "gears.png"));
    assert(fetch->id != 0);
    assert((uintptr_t)fetch->userData == 0x12345678);
    assert(fetch->totalBytes == 6407);
    emscripten_fetch_close(fetch);
    exit(0);
  };

  attr.onprogress = [](emscripten_fetch_t *fetch) {
    assert(fetch);
    if (fetch->status != 200) return;
    if (fetch->totalBytes > 0) {
      printf("Downloading.. %.2f%% complete.\n", (fetch->dataOffset + fetch->numBytes) * 100.0 / fetch->totalBytes);
    } else {
      printf("Downloading.. %lld bytes complete.\n", fetch->dataOffset + fetch->numBytes);
    }
  };

  attr.onerror = [](emscripten_fetch_t *fetch) {
    printf("Download failed!\n");
  };

  emscripten_fetch_t *fetch = emscripten_fetch(&attr, "gears.png");
  assert(fetch != 0);
  // emscripten_fetch() must be able to operate without referencing to this
  // structure after the call.
  memset(&attr, 0, sizeof(attr));
  return 99;
}
