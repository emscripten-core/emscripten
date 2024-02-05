/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <string.h>

int main() {
  printf("%d\n", !!strstr("\\n", "\\n"));
  printf("%d\n", !!strstr("cheezy", "ez"));
  printf("%d\n", !!strstr("cheeezy", "ez"));
  printf("%d\n", !!strstr("cheeeeeeeeeezy", "ez"));
  printf("%d\n", !!strstr("cheeeeeeeeee1zy", "ez"));
  printf("%d\n", !!strstr("che1ezy", "ez"));
  printf("%d\n", !!strstr("che1ezy", "che"));
  printf("%d\n", !!strstr("ce1ezy", "che"));
  printf("%d\n", !!strstr("ce1ezy", "ezy"));
  printf("%d\n", !!strstr("ce1ezyt", "ezy"));
  printf("%d\n", !!strstr("ce1ez1y", "ezy"));
  printf("%d\n", !!strstr("cheezy", "a"));
  printf("%d\n", !!strstr("cheezy", "b"));
  printf("%d\n", !!strstr("cheezy", "c"));
  printf("%d\n", !!strstr("cheezy", "d"));
  printf("%d\n", !!strstr("cheezy", "g"));
  printf("%d\n", !!strstr("cheezy", "h"));
  printf("%d\n", !!strstr("cheezy", "i"));
  printf("%d\n", !!strstr("cheezy", "e"));
  printf("%d\n", !!strstr("cheezy", "x"));
  printf("%d\n", !!strstr("cheezy", "y"));
  printf("%d\n", !!strstr("cheezy", "z"));
  printf("%d\n", !!strstr("cheezy", "_"));

  const char *str = "a big string";
  printf("%zu\n", strstr(str, "big") - str);
  return 0;
}
