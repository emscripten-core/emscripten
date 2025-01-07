// Copyright 2016 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <string.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <emscripten/fetch.h>

int result = 1;

// This test is run in two modes: if FILE_DOES_NOT_EXIST defined,
// then testing an XHR of a missing file.
// #define FILE_DOES_NOT_EXIST

int main() {
  emscripten_fetch_attr_t attr;
  emscripten_fetch_attr_init(&attr);
  strcpy(attr.requestMethod, "GET");
  attr.userData = (void*)0x12345678;
  attr.attributes = EMSCRIPTEN_FETCH_REPLACE | EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;

  attr.onsuccess = [](emscripten_fetch_t *fetch) {
#if FILE_DOES_NOT_EXIST
    assert(false && "onsuccess handler called, but the file shouldn't exist"); // Shouldn't reach here if the file doesn't exist
#endif
    assert(fetch);
    printf("Finished downloading %llu bytes\n", fetch->numBytes);
    assert(fetch->url);
    assert(!strcmp(fetch->url, "gears.png"));
    assert(fetch->id != 0);
    assert((uintptr_t)fetch->userData == 0x12345678);
    assert(fetch->totalBytes == 6407);
    assert(fetch->numBytes == fetch->totalBytes);
    assert(fetch->data != 0);
    // Compute rudimentary checksum of data
    uint8_t checksum = 0;
    for(int i = 0; i < fetch->numBytes; ++i)
      checksum ^= fetch->data[i];
    printf("Data checksum: %02X\n", checksum);
    assert(checksum == 0x08);
    emscripten_fetch_close(fetch);

#ifndef FILE_DOES_NOT_EXIST
    result = 0;
#endif
    exit(result);
  };

  attr.onprogress = [](emscripten_fetch_t *fetch) {
    assert(fetch);
    if (fetch->status != 200) return;
    printf("onprogress: dataOffset: %llu, numBytes: %llu, totalBytes: %llu\n", fetch->dataOffset, fetch->numBytes, fetch->totalBytes);
    if (fetch->totalBytes > 0) {
      printf("Downloading.. %.2f%% complete.\n", (fetch->dataOffset + fetch->numBytes) * 100.0 / fetch->totalBytes);
    } else {
      printf("Downloading.. %lld bytes complete.\n", fetch->dataOffset + fetch->numBytes);
    }
#ifdef FILE_DOES_NOT_EXIST
    // We should not receive progress reports if the file doesn't exist.
    assert(false && "onprogress handler called, but the file should not exist");
#endif
    // We must receive a call to the onprogress handler with 100% completion.
    if (fetch->dataOffset + fetch->numBytes == fetch->totalBytes) ++result;
    assert(fetch->dataOffset + fetch->numBytes <= fetch->totalBytes);
    assert(fetch->url);
    assert(!strcmp(fetch->url, "gears.png"));
    assert(fetch->id != 0);
    assert((uintptr_t)fetch->userData == 0x12345678);
  };

  attr.onerror = [](emscripten_fetch_t *fetch) {
    printf("Download failed!\n");
#ifndef FILE_DOES_NOT_EXIST
    // The file exists, shouldn't reach here.
    assert(false && "onerror handler called, but the transfer should have succeeded!");
#endif
    assert(fetch);
    assert(fetch->id != 0);
    assert(!strcmp(fetch->url, "gears.png"));
    assert((uintptr_t)fetch->userData == 0x12345678);

#ifdef FILE_DOES_NOT_EXIST
    result = 0;
#endif
    exit(result);
  };

  emscripten_fetch_t *fetch = emscripten_fetch(&attr, "gears.png");
  assert(fetch != 0);
  // emscripten_fetch() must be able to operate without referencing to this
  // structure after the call.
  memset(&attr, 0, sizeof(attr));
  return 99;
}
