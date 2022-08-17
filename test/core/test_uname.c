/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <sys/utsname.h>

int main() {
  struct utsname u;
  printf("ret: %d\n", uname(&u));
  printf("sysname: %s\n", u.sysname);
  printf("nodename: %s\n", u.nodename);
  printf("release: %s\n", u.release);
  printf("version: %s\n", u.version);
  printf("machine: %s\n", u.machine);
#ifdef __wasm64__
  assert(strcmp(u.machine, "wasm64") == 0);
#else
  assert(strcmp(u.machine, "wasm32") == 0);
#endif
  printf("invalid: %d\n", uname(0));

  return 0;
}
