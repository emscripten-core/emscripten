// Copyright 2020 The Emscripten Authors.  All rights reserved.
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
  int number = 721;
  int *userData = &number;
  emscripten_fetch_attr_t attr;
  emscripten_fetch_attr_init(&attr);
  strcpy(attr.requestMethod, "GET");
  attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY | EMSCRIPTEN_FETCH_SYNCHRONOUS;
  attr.userData = userData;
  attr.onsuccess = [](emscripten_fetch_t *fetch) {
    int *passedUserData = (int*)fetch->userData;
    printf("Fetch is successed!\n");
    printf("Passed userData is \"%d\"\n", *passedUserData);
    assert(*passedUserData == 721);
  };
  printf("Fetch is started!\n");
  emscripten_fetch_t *fetch = emscripten_fetch(&attr, "gears.png");
  #ifdef REPORT_RESULT
    REPORT_RESULT(fetch->status);
  #endif
}
