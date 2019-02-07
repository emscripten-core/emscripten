// Copyright 2016 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <string.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <pthread.h>
#include <emscripten/fetch.h>

int result = 0;

void ThreadMainLoop()
{
  printf("TML!\n");
}

void *ThreadMain(void *arg)
{
  emscripten_fetch_attr_t attr;
  emscripten_fetch_attr_init(&attr);
  strcpy(attr.requestMethod, "GET");
  attr.userData = (void*)0x12345678;
  attr.attributes = EMSCRIPTEN_FETCH_REPLACE | EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;

  attr.onsuccess = [](emscripten_fetch_t *fetch) {
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
    assert(fetch);
    assert(fetch->id != 0);
    assert(!strcmp(fetch->url, "gears.png"));
    assert((uintptr_t)fetch->userData == 0x12345678);

#ifdef REPORT_RESULT
    REPORT_RESULT(result);
#endif
  };

  emscripten_fetch_t *fetch = emscripten_fetch(&attr, "gears.png");
  assert(fetch != 0);
  memset(&attr, 0, sizeof(attr)); // emscripten_fetch() must be able to operate without referencing to this structure after the call.
  emscripten_exit_with_live_runtime();
}

int main()
{
  if (!emscripten_has_threading_support())
  {
#ifdef REPORT_RESULT
    REPORT_RESULT(0);
#endif
    printf("Skipped: Threading is not supported.\n");
    return 0;
  }

  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
  pthread_t thread;
  int rc = pthread_create(&thread, &attr, ThreadMain, (void*)0);
  assert(rc == 0);
  pthread_attr_destroy(&attr);
  int status;
  rc = pthread_join(thread, (void**)&status);
  assert(rc == 0);
}
