// Copyright 2020 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <string.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <emscripten/fetch.h>

void downloadSucceeded(emscripten_fetch_t *fetch)
{
  char *userData = (char*)fetch->userData;
  printf("Fetch is successed!\n");
  printf("Passed userData is \"%s\"\n", userData);
  assert(strcmp(userData, "User Data") == 0);
}

int main()
{
  char *userData = (char*)"User Data";
  emscripten_fetch_attr_t attr;
  emscripten_fetch_attr_init(&attr);
  strcpy(attr.requestMethod, "GET");
  attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
  attr.userData = userData;
  attr.onsuccess = downloadSucceeded;
  printf("Fetch is started!\n");
  emscripten_fetch_t *fetch = emscripten_fetch(&attr, "gears.png");
  #ifdef REPORT_RESULT
    REPORT_RESULT(fetch->status);
  #endif
}
