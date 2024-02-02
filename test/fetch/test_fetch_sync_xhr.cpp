// Copyright 2016 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <string.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <emscripten.h>
#include <emscripten/fetch.h>

int result = -1;

int main() {
  // If an exception is thrown from the user callback, it bubbles up to
  // self.onerror but is otherwise completely swallowed by xhr.send.
  EM_ASM({self.onerror = (e) => {
           out('Got error', e);
           HEAP32[$0 >> 2] = 2;
         };}, &result);
  emscripten_fetch_attr_t attr;
  emscripten_fetch_attr_init(&attr);
  strcpy(attr.requestMethod, "GET");
  attr.attributes = EMSCRIPTEN_FETCH_REPLACE | EMSCRIPTEN_FETCH_LOAD_TO_MEMORY | EMSCRIPTEN_FETCH_SYNCHRONOUS;

  attr.onsuccess = [](emscripten_fetch_t *fetch) {
    printf("Finished downloading %llu bytes\n", fetch->numBytes);
    // Compute rudimentary checksum of data
    uint8_t checksum = 0;
    for(int i = 0; i < fetch->numBytes; ++i)
      checksum ^= fetch->data[i];
    printf("Data checksum: %02X\n", checksum);
    assert(checksum == 0x08);
    emscripten_fetch_close(fetch);

    if (result == -1) result = 0;
  };

  attr.onprogress = [](emscripten_fetch_t *fetch) {
    if (fetch->totalBytes > 0) {
      printf("Downloading.. %.2f%% complete.\n", (fetch->dataOffset + fetch->numBytes) * 100.0 / fetch->totalBytes);
    } else {
      printf("Downloading.. %lld bytes complete.\n", fetch->dataOffset + fetch->numBytes);
    }
  };

  attr.onerror = [](emscripten_fetch_t *fetch) {
    printf("Download failed!\n");
    assert(false && "Shouldn't fail!");
  };

  emscripten_fetch_t *fetch = emscripten_fetch(&attr, "gears.png");
  if (result == -1) {
    printf("emscripten_fetch() failed to run synchronously!\n");
  }
  assert(result == 0);
  return result;
}
