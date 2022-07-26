/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <libgen.h>

int main() {
  char p1[16] = "/usr/lib", p1x[16] = "/usr/lib";
  printf("%s -> ", p1);
  printf("%s : %s\n", dirname(p1x), basename(p1));

  char p2[16] = "/usr", p2x[16] = "/usr";
  printf("%s -> ", p2);
  printf("%s : %s\n", dirname(p2x), basename(p2));

  char p3[16] = "/usr/", p3x[16] = "/usr/";
  printf("%s -> ", p3);
  printf("%s : %s\n", dirname(p3x), basename(p3));

  char p4[16] = "/usr/lib///", p4x[16] = "/usr/lib///";
  printf("%s -> ", p4);
  printf("%s : %s\n", dirname(p4x), basename(p4));

  char p5[16] = "/", p5x[16] = "/";
  printf("%s -> ", p5);
  printf("%s : %s\n", dirname(p5x), basename(p5));

  char p6[16] = "///", p6x[16] = "///";
  printf("%s -> ", p6);
  printf("%s : %s\n", dirname(p6x), basename(p6));

  char p7[16] = "/usr/../lib/..", p7x[16] = "/usr/../lib/..";
  printf("%s -> ", p7);
  printf("%s : %s\n", dirname(p7x), basename(p7));

  char p8[16] = "", p8x[16] = "";
  printf("(empty) -> %s : %s\n", dirname(p8x), basename(p8));

  printf("(null) -> %s : %s\n", dirname(0), basename(0));

  return 0;
}
