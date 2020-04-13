// Copyright 2016 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <string.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <emscripten/fetch.h>

int result = 0;

int main()
{
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

    if (result == 0) result = 1;
#ifdef REPORT_RESULT
    // Fetch API appears to sometimes call the handlers more than once, see https://github.com/emscripten-core/emscripten/pull/8191
    MAYBE_REPORT_RESULT(result);
#endif
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
  if (result == 0) {
    result = 2;
    printf("emscripten_fetch() failed to run synchronously!\n");
  }
#ifdef REPORT_RESULT
  // Fetch API appears to sometimes call the handlers more than once, see https://github.com/emscripten-core/emscripten/pull/8191
  MAYBE_REPORT_RESULT(result);
#endif
}
