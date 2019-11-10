// Copyright 2017 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include<emscripten.h>

int main() {
  emscripten_log(EM_LOG_CONSOLE, "hello, world!");
  return 0;
}

