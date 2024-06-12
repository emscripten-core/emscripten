// Copyright 2016 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <emscripten/fetch.h>

int main() {
  emscripten_fetch_attr_t attr;
  emscripten_fetch_attr_init(&attr);
  strcpy(attr.requestMethod, "GET");
  attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY | EMSCRIPTEN_FETCH_SYNCHRONOUS;
  emscripten_fetch_t *fetch = emscripten_fetch(&attr, "gears.png");
  assert(fetch);
  printf("Fetch finished with status %d\n", fetch->status);
  assert(fetch->status == 200);
  printf("Downloaded %llu bytes", fetch->numBytes);
  return 0;
}
