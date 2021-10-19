/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <stdlib.h>

int main() {
  int size = snprintf(NULL, 0, "%s %d %.2f\n", "me and myself", 25, 1.345);
  char buf[size];
  snprintf(buf, size, "%s %d %.2f\n", "me and myself", 25, 1.345);
  printf("%d : %s\n", size, buf);
  char *buff = NULL;
  asprintf(&buff, "%d waka %d\n", 21, 95);
  puts(buff);
  free(buff);
  // test buffering, write more than a musl buffer at once
  #define X 1026
  char c[X];
  for (int i=0;i<X;i++) c[i] ='A';
  c[X-1] = '\0';
  printf("%s\n", c);  /// if X > 1025 this line doesn't print if we don't handle buffering properly
  return 0;
}
