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

int onerrors = 0;

void after_onerror(void*) {
  printf("reporting delayed result\n");
  result = 1;
  REPORT_RESULT(result);
}

// This test is run in two modes: if FILE_DOES_NOT_EXIST defined,
// then testing an XHR of a missing file.
// #define FILE_DOES_NOT_EXIST

int main()
{
  emscripten_fetch_attr_t attr;
  emscripten_fetch_attr_init(&attr);
  strcpy(attr.requestMethod, "GET");
  attr.userData = (void*)0x12345678;
  attr.attributes = EMSCRIPTEN_FETCH_REPLACE | EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;

  attr.onsuccess = [](emscripten_fetch_t *fetch) {
#if FILE_DOES_NOT_EXIST
    assert(false && "onsuccess handler called, but the file shouldn't exist"); // Shouldn't reach here if the file doesn't exist
#endif
#ifdef CLOSE_IMMEDIATELY
    assert(false && "onsuccess handler called, but the the fetch was closed");
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

#ifdef REPORT_RESULT
#ifndef FILE_DOES_NOT_EXIST
    result = 1;
#endif
    REPORT_RESULT(result);
#endif
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
    assert(false && "onprogress handler called, but the file should not exist"); // We should not receive progress reports if the file doesn't exist.
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
    onerrors++;
    assert(onerrors == 1 && "onerror should be called at most once");
#if !defined(FILE_DOES_NOT_EXIST) && !defined(CLOSE_IMMEDIATELY)
    assert(false && "onerror handler called, but the transfer should have succeeded!"); // The file exists, shouldn't reach here.
#endif
    assert(fetch);
    assert(fetch->id != 0);
    assert(!strcmp(fetch->url, "gears.png"));
    assert((uintptr_t)fetch->userData == 0x12345678);
#ifdef CLOSE_IMMEDIATELY
    // Wait to report the result - if we get an extra async onerror later, that is bad.
    // There is some risk of random timing here, but it should be random successes, not fails,
    // as we may not wait long enough to see the problem.
    emscripten_async_call(after_onerror, NULL, 1000);
    return;
#endif

    emscripten_fetch_close(fetch);

#ifdef REPORT_RESULT
#if defined(FILE_DOES_NOT_EXIST) || defined(CLOSE_IMMEDIATELY)
    result = 1;
#endif
    REPORT_RESULT(result);
#endif
  };

  emscripten_fetch_t *fetch = emscripten_fetch(&attr, "gears.png");
#ifdef CLOSE_IMMEDIATELY
  emscripten_fetch_close(fetch);
#endif
  assert(fetch != 0);
  memset(&attr, 0, sizeof(attr)); // emscripten_fetch() must be able to operate without referencing to this structure after the call.
}
