// Copyright 2015 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <assert.h>
#include <emscripten.h>

extern "C" {

void one(char *data, int size) {
  int *x = (int*)data;
  int num = size/sizeof(int);
  for (int i = 0; i < num; i++) {
    x[i] += 1234;
  }
  emscripten_sleep(1000);
  emscripten_worker_respond(data, size);
}

}

