// Copyright 2012 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

FILE *file;
bool mygetline(char *str, int len) { return fgets(str, len, file)!=NULL; }

int main() {
  file = fopen("test_cube2md5.txt", "r");
  char buf[1024];
  int tmp;
  mygetline(buf, sizeof(buf));
  if (sscanf(buf, " frame %d", &tmp)==1) {
    printf("frame %d\n", tmp);
    for (int numdata = 0; mygetline(buf, sizeof(buf)) && buf[0]!='}';) {
      printf("frameline\n");
      for (char *src = buf, *next = src; numdata < 198; numdata++, src = next) {
        double x = strtod(src, &next);
        printf("animdata[%d] = %.8f\n", numdata, x);
        if (next <= src) break;
      }
    }
  }
  return 0;
}
