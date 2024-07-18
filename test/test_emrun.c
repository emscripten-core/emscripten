/*
 * Copyright 2015 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <emscripten.h>

int main(int argc, char **argv) {
  printf("argc: %d\n", argc);
  for (int i = 0; i < argc; ++i) {
    printf("argv[%d]: %s\n", i, argv[i]);
  }
  assert(argc > 1);

  // Dump a file to local filesystem with emrun.
  EM_ASM(emrun_file_dump("test.dat", HEAPU8.subarray(0, 128)););
  EM_ASM(emrun_file_dump("heap.dat", HEAPU8));
  EM_ASM(emrun_file_dump("nested/with space.dat", HEAPU8.subarray(128, 256)););

  printf("hello, world!\n");
  fprintf(stderr, "hello, error stream!\n");

  printf("Testing ASCII characters: !\"$%%&'()*+,-./:;<=>?@[\\]^_`{|}~\n");
  printf("Testing char sequences: %%20%%21 &auml;\n");

  exit(100);
}
