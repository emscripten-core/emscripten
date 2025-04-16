// Copyright 2011 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void dump(char *s) {
  printf("%s\n", s);
}

int main() {
  char *original = (char*)"h e l l o ,   w o r l d ! ";
  char copy[strlen(original)];
  for (int i = 0; i < strlen(original); i += 2) {
    copy[i/2] = original[i];
  }
  copy[strlen(copy)+1] = (long)&original; // force original to be on the stack
  dump(copy);
  return 0;
}
