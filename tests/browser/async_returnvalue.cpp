// Copyright 2019 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <assert.h>
#include <stdio.h>

#include <emscripten.h>

// A "sync" tunnel that adds 1.
extern "C" int sync_tunnel(int);

int main() {
  int x;
  x = sync_tunnel(0);
  assert(x == 1);
  x = sync_tunnel(1);
  assert(x == 2);
  x = sync_tunnel(2);
  assert(x == 3);
  x = sync_tunnel(42);
  assert(x == 43);
  x = sync_tunnel(-1);
  assert(x == 0);
  x = sync_tunnel(-2);
  assert(x == -1);

  REPORT_RESULT(0);
  return 0;
}

