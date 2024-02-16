// Copyright 2023 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <cassert>
#include <cstdio>
#include <new>

int main() {
  char* data = new (std::nothrow) char[20 * 1024 * 1024];
  printf("data: %p\n", data);
  assert(data == nullptr);
  printf("success\n");
  return 0;
}
