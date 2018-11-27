/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <strings.h>
int sign(int x) {
  if (x < 0) return -1;
  if (x > 0) return 1;
  return 0;
}
int main() {
  printf("*\n");

  printf("%d\n", sign(strcasecmp("hello", "hello")));
  printf("%d\n", sign(strcasecmp("hello1", "hello")));
  printf("%d\n", sign(strcasecmp("hello", "hello1")));
  printf("%d\n", sign(strcasecmp("hello1", "hello1")));
  printf("%d\n", sign(strcasecmp("iello", "hello")));
  printf("%d\n", sign(strcasecmp("hello", "iello")));
  printf("%d\n", sign(strcasecmp("A", "hello")));
  printf("%d\n", sign(strcasecmp("Z", "hello")));
  printf("%d\n", sign(strcasecmp("a", "hello")));
  printf("%d\n", sign(strcasecmp("z", "hello")));
  printf("%d\n", sign(strcasecmp("hello", "a")));
  printf("%d\n", sign(strcasecmp("hello", "z")));

  printf("%d\n", sign(strcasecmp("Hello", "hello")));
  printf("%d\n", sign(strcasecmp("Hello1", "hello")));
  printf("%d\n", sign(strcasecmp("Hello", "hello1")));
  printf("%d\n", sign(strcasecmp("Hello1", "hello1")));
  printf("%d\n", sign(strcasecmp("Iello", "hello")));
  printf("%d\n", sign(strcasecmp("Hello", "iello")));
  printf("%d\n", sign(strcasecmp("A", "hello")));
  printf("%d\n", sign(strcasecmp("Z", "hello")));
  printf("%d\n", sign(strcasecmp("a", "hello")));
  printf("%d\n", sign(strcasecmp("z", "hello")));
  printf("%d\n", sign(strcasecmp("Hello", "a")));
  printf("%d\n", sign(strcasecmp("Hello", "z")));

  printf("%d\n", sign(strcasecmp("hello", "Hello")));
  printf("%d\n", sign(strcasecmp("hello1", "Hello")));
  printf("%d\n", sign(strcasecmp("hello", "Hello1")));
  printf("%d\n", sign(strcasecmp("hello1", "Hello1")));
  printf("%d\n", sign(strcasecmp("iello", "Hello")));
  printf("%d\n", sign(strcasecmp("hello", "Iello")));
  printf("%d\n", sign(strcasecmp("A", "Hello")));
  printf("%d\n", sign(strcasecmp("Z", "Hello")));
  printf("%d\n", sign(strcasecmp("a", "Hello")));
  printf("%d\n", sign(strcasecmp("z", "Hello")));
  printf("%d\n", sign(strcasecmp("hello", "a")));
  printf("%d\n", sign(strcasecmp("hello", "z")));

  printf("%d\n", sign(strcasecmp("Hello", "Hello")));
  printf("%d\n", sign(strcasecmp("Hello1", "Hello")));
  printf("%d\n", sign(strcasecmp("Hello", "Hello1")));
  printf("%d\n", sign(strcasecmp("Hello1", "Hello1")));
  printf("%d\n", sign(strcasecmp("Iello", "Hello")));
  printf("%d\n", sign(strcasecmp("Hello", "Iello")));
  printf("%d\n", sign(strcasecmp("A", "Hello")));
  printf("%d\n", sign(strcasecmp("Z", "Hello")));
  printf("%d\n", sign(strcasecmp("a", "Hello")));
  printf("%d\n", sign(strcasecmp("z", "Hello")));
  printf("%d\n", sign(strcasecmp("Hello", "a")));
  printf("%d\n", sign(strcasecmp("Hello", "z")));

  printf("%d\n", sign(strncasecmp("hello", "hello", 3)));
  printf("%d\n", sign(strncasecmp("hello1", "hello", 3)));
  printf("%d\n", sign(strncasecmp("hello", "hello1", 3)));
  printf("%d\n", sign(strncasecmp("hello1", "hello1", 3)));
  printf("%d\n", sign(strncasecmp("iello", "hello", 3)));
  printf("%d\n", sign(strncasecmp("hello", "iello", 3)));
  printf("%d\n", sign(strncasecmp("A", "hello", 3)));
  printf("%d\n", sign(strncasecmp("Z", "hello", 3)));
  printf("%d\n", sign(strncasecmp("a", "hello", 3)));
  printf("%d\n", sign(strncasecmp("z", "hello", 3)));
  printf("%d\n", sign(strncasecmp("hello", "a", 3)));
  printf("%d\n", sign(strncasecmp("hello", "z", 3)));

  printf("*\n");

  return 0;
}
