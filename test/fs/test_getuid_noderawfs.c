/*
 * Copyright 2024 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <emscripten.h>

int main() {
  // NODERAWFS reports the real host process credentials. Other backends report
  // 0 and are covered by test/unistd/misc.c.
  assert(getuid() == EM_ASM_INT({ return process.getuid(); }));
  assert(geteuid() == EM_ASM_INT({ return process.geteuid(); }));
  assert(getgid() == EM_ASM_INT({ return process.getgid(); }));
  assert(getegid() == EM_ASM_INT({ return process.getegid(); }));

  printf("done\n");
  return 0;
}
