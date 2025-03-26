// Copyright 2017 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <stdio.h>
#include <thread>

int main() {
  int x = std::this_thread::get_id() == std::this_thread::get_id();
  printf("is this thread the same as this thread? %d\n", x);
}
