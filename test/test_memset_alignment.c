// Copyright 2017 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <memory.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

char ptr[1024*1024*2+64] = {};

#define GUARDSIZE 32
void test_memset(int copySize, int offset) {
  char *content = ptr + GUARDSIZE + offset;

  char *guardBefore = content - GUARDSIZE;
  char *guardAfter  = content + copySize;

  char guard = (char)rand();

  for (int i = 0; i < GUARDSIZE; ++i) {
    // Generate a guardband area around memory that should not change.
    guardBefore[i] = (char)(guard ^ i);
    guardAfter[i] = (char)(guard ^ i);
  }

  // Generate fill source data
  char s = (char)(rand() ^ 0xF0);

  memset(content, s, copySize);
  for (int i = 0; i < copySize; ++i) {
    if (content[i] != s) {
      printf("test_memset(copySize=%d, offset=%d failed!\n", copySize, offset);
      exit(1);
    }
  }

  // Verify guardband area
  for (int i = 0; i < GUARDSIZE; ++i) {
    // Generate a guardband area around memory that should not change.
    if (guardBefore[i] != (char)(guard ^ i)) {
      printf("test_memset(copySize=%d, offset=%d failed! Pre-guardband area at i=%d overwritten!\n", copySize, offset, i);
      exit(1);
    }
    if (guardAfter[i] != (char)(guard ^ i)) {
      printf("test_memset(copySize=%d, offset=%d failed! Post-guardband area at i=%d overwritten!\n", copySize, offset, i);
      exit(1);
    }
  }
}

void test_copysize(int copySize) {
  for (int offset = 0; offset < 31; ++offset) {
    test_memset(copySize, offset);
  }
}

int main() {
  srand(time(NULL));

  for (int copySize = 0; copySize < 128; ++copySize) {
    test_copysize(copySize);
  }

  for (int copySizeI = 128; copySizeI <= 1048576; copySizeI <<= 1) {
    for (int copySizeJ = 1; copySizeJ <= 16; copySizeJ <<= 1) {
      test_copysize(copySizeI | copySizeJ);
    }
  }

  printf("OK.\n");
}
