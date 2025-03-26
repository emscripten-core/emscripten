// Copyright 2021 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <string.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <emscripten/fetch.h>
#include <pthread.h>

int result = 0;
void* thread_main(void* arg) {
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
    result = 42;
#ifdef DO_PTHREAD_EXIT
    pthread_exit(&result);
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
    assert(false && "onerror handler called, but the transfer should have succeeded!");
    result = 43;
#ifdef DO_PTHREAD_EXIT
    pthread_exit(&result);
#endif
  };

  emscripten_fetch_t *fetch = emscripten_fetch(&attr, "gears.png");
  assert(fetch);
  static int my_result = 99;
  return (void*)&my_result;
}


int main() {
  pthread_t thread;
  pthread_create(&thread, NULL, thread_main, NULL);
  int* res;
  printf("joining fetch thread\n");
  pthread_join(thread, (void**)&res);
  printf("join done\n");
  printf("fetch thread complete: result=%d\n", *res);
#if DO_PTHREAD_EXIT
  // if we don't use pthread_exit then exit status should be &result
  assert(res == &result);
#else
  // if we don't use pthread_exit then we expect the exit status to
  // be what was returned from thread_main.
  assert(*res == 99);
#endif
  return 0;
}
