/*
 * Copyright 2020 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>

int main(void) {
#ifdef ENV_NODE
#ifndef _W_EXITCODE
# define _W_EXITCODE(ret, sig) ((ret) << 8 | (sig))
#endif
  assert(system(NULL) == 1);
  assert(system("") == 0);
  assert(system("'something like this does not exist (hopefully)'") == _W_EXITCODE(127, 0));
  assert(system("false") == _W_EXITCODE(1, 0));
  assert(system("true") == _W_EXITCODE(0, 0));
  assert(system("kill $$") == _W_EXITCODE(0, 15));
  system("echo Hello from echo");
#else
  assert(system(NULL) == 0);
  assert(system("") == -1);
  assert(errno == ENOSYS);
  assert(system("true") == -1);
  assert(errno == ENOSYS);
#endif
  return 0;
}
