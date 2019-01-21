// Copyright 2016 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <string.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <emscripten/fetch.h>

int main()
{
  emscripten_fetch_attr_t attr;
  emscripten_fetch_attr_init(&attr);
  attr.attributes = EMSCRIPTEN_FETCH_REPLACE | EMSCRIPTEN_FETCH_LOAD_TO_MEMORY | EMSCRIPTEN_FETCH_WAITABLE;
  emscripten_fetch_t *fetch = emscripten_fetch(&attr, "gears.png");
  assert(fetch != 0);
  memset(&attr, 0, sizeof(attr)); // emscripten_fetch() must be able to operate without referencing to this structure after the call.
  printf("Main thread waiting for fetch to finish...\n");
  emscripten_fetch_wait(fetch, INFINITY);
  printf("Main thread waiting for fetch to finish done...\n");
  assert(fetch->data != 0);
  assert(fetch->numBytes > 0);
  assert(fetch->totalBytes == fetch->numBytes);
  assert(fetch->readyState == 4/*DONE*/);
  assert(fetch->status == 200);

  uint8_t checksum = 0;
  for(int i = 0; i < fetch->numBytes; ++i)
    checksum ^= fetch->data[i];
  printf("Data checksum: %02X\n", checksum);
  assert(checksum == 0x08);
  emscripten_fetch_close(fetch);

#ifdef REPORT_RESULT
  REPORT_RESULT(0);
#endif
}
