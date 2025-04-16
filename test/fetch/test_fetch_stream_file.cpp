// Copyright 2016 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <string.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <emscripten/fetch.h>

// Compute rudimentary checksum of data
uint32_t checksum = 0;

int main() {
  emscripten_fetch_attr_t attr;
  emscripten_fetch_attr_init(&attr);
  strcpy(attr.requestMethod, "GET");
  attr.onsuccess = [](emscripten_fetch_t *fetch) {
    printf("Finished downloading %llu bytes\n", fetch->totalBytes);
    printf("Data checksum: %08X\n", checksum);
    assert(fetch->data == 0); // The data was streamed via onprogress, no bytes available here.
    assert(fetch->numBytes == 0);
    assert(fetch->totalBytes == 134217728);
    assert(checksum == 0xA7F8E858U);
    emscripten_fetch_close(fetch);

    exit(0);
  };
  attr.onprogress = [](emscripten_fetch_t *fetch) {
    printf("Downloading.. %.2f%s complete. Received chunk [%llu, %llu[\n", 
      (fetch->totalBytes > 0) ? ((fetch->dataOffset + fetch->numBytes) * 100.0 / fetch->totalBytes) : (double)(fetch->dataOffset + fetch->numBytes),
      (fetch->totalBytes > 0) ? "%" : " bytes",
      fetch->dataOffset,
      fetch->dataOffset + fetch->numBytes);
    assert(fetch->data != 0);
    assert(fetch->numBytes > 0);
    assert(fetch->dataOffset + fetch->numBytes <= fetch->totalBytes);
    assert(fetch->totalBytes <= 134217728);

    for(int i = 0; i < fetch->numBytes; ++i)
      checksum = ((checksum << 8) | (checksum >> 24)) * fetch->data[i] + fetch->data[i];
  };
  attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY | EMSCRIPTEN_FETCH_APPEND | EMSCRIPTEN_FETCH_STREAM_DATA;
  emscripten_fetch_t *fetch = emscripten_fetch(&attr, "largefile.txt");
  return 99;
}
