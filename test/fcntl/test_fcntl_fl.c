/*
 * Copyright 2011 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>

int main() {
  int f = open("test",
               O_RDWR
#if defined(STANDALONE_WASM)
                 | O_CREAT
#endif
               ,
               0777);
#if defined(STANDALONE_WASM)
  assert(f >= 0);
#else
  assert(f == 3);
#endif

  printf("F_GETFL: %d\n", !!(fcntl(f, F_GETFL) & O_RDWR));
  printf("errno: %d\n", errno);
  printf("\n");
  errno = 0;

  printf("F_SETFL: %d\n", fcntl(f, F_SETFL, O_APPEND));
  printf("errno: %d\n", errno);
  printf("\n");
  errno = 0;

  printf("F_GETFL/2: %d\n", !!(fcntl(f, F_GETFL) & (O_RDWR | O_APPEND)));
  printf("errno: %d\n", errno);
  printf("\n");
  errno = 0;

  return 0;
}
