// Copyright 2016 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main() {
  unsigned int a, b;
  sscanf("0x12AB 12AB", "%x %x", &a, &b);
  printf("%d %d\n", a, b);

  const char* hexstr = "0102037F00FF";
  int len = strlen(hexstr) / 2;
  char* tmp_data = malloc(len);
  for (int i = 0; i < len; i++) {
    sscanf(hexstr, "%2hhx", &tmp_data[i]);
    hexstr += 2 * sizeof(char);
  }

  for (int j = 0; j < len; j++) {
    printf("%i, ", tmp_data[j]);
  }
  printf("\n");
  free(tmp_data);
}
