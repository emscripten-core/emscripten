/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdlib.h>
#include <stdio.h>
#include <setjmp.h>
#include <emscripten.h>

jmp_buf abortframe;

void dostuff(int a) {
  printf("pre\n");
  if (a != 42)
    emscripten_run_script(
        "waka_waka()");  // this should fail, and never reach "never"
  printf("never\n");

  if (a == 100) {
    longjmp(abortframe, -1);
  }

  if (setjmp(abortframe)) {
    printf("got 100");
  }
}

int main(int argc, char **argv) {
  dostuff(argc);
  exit(1);
  return 1;
}
