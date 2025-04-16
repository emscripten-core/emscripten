/*
 * Copyright 2015 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <emscripten.h>

int main(void) {
  EM_ASM(
    FS.close(FS.open('test.txt', 'w'));
  );
  assert(open("test.txt", O_RDONLY | O_CLOEXEC) != -1);
  printf("success\n");
  return 0;
}
