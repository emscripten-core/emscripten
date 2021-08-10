// Copyright 2019 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <assert.h>
#include <stdio.h>

#include <emscripten.h>

// A "sync" tunnel that adds 1.
#if USE_EM_JS
EM_JS(int, sync_tunnel, (int value), {
  return Asyncify.handleSleep(function(wakeUp) {
    setTimeout(function() {
      wakeUp(value + 1);
    }, 1);
  });
})
EM_JS(int, sync_tunnel_bool, (bool value), {
  return Asyncify.handleSleep(function(wakeUp) {
    setTimeout(function() {
      wakeUp(!value);
    }, 1);
  });
})
#else
extern "C" int sync_tunnel(int);
extern "C" int sync_tunnel_bool(bool);
#endif

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

  bool y;
  y = sync_tunnel_bool(false);
  assert(y == true);
  y = sync_tunnel_bool(true);
  assert(y == false);

#ifdef BAD
  // We should not get here.
  printf("We should not get here\n");
  return 1;
#else
  // Success!
  REPORT_RESULT(0);
  return 0;
#endif
}
