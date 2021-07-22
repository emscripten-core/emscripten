/*
 * Copyright 2021 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <emscripten.h>
#include <assert.h>
#include <stdio.h>
#include <sys/stat.h>

int main() {
  EM_ASM(
    FS.mkdir('/working');
    FS.mount(NODEFS, { root: '.' }, '/working');
  );

  struct stat stats;
  assert(stat("/working/non-existing-file.txt", &stats) == -1);
  printf("success\n");
  return 0;
}
