// Copyright 2017 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <memory.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

char dst[1024*1024*2+64] = {};
char src[1024*1024*2+64] = {};

#define GUARDSIZE 32
void test_memcpy(int copySize, int srcOffset, int dstOffset) {
  char *dstContent = dst + GUARDSIZE + dstOffset;
  char *srcContent = src + GUARDSIZE + srcOffset;

  char *dstGuardBefore = dstContent - GUARDSIZE;
  char *dstGuardAfter  = dstContent + copySize;

  char *srcGuardBefore = srcContent - GUARDSIZE;
  char *srcGuardAfter  = srcContent + copySize;

  char guard = (char)rand();

  for (int i = 0; i < GUARDSIZE; ++i) {
    // Generate a guardband area around dst memory that should not change.
    dstGuardBefore[i] = (char)(guard ^ i);
    dstGuardAfter[i] = (char)(guard ^ i);
  }

  // Generate copy source data
  char s = (char)rand();
  for (int i = 0; i < copySize + srcOffset + 2*GUARDSIZE; ++i) {
    src[i] = (char)(s - i);
  }

  memcpy(dstContent, srcContent, copySize);
  if (!!memcmp(dstContent, srcContent, copySize)) {
    printf("test_memcpy(copySize=%d, srcOffset=%d, dstOffset=%d failed!\n", copySize, srcOffset, dstOffset);
    exit(1);
  }

  // Verify guardband area
  for (int i = 0; i < GUARDSIZE; ++i) {
    // Generate a guardband area around dst memory that should not change.
    if (dstGuardBefore[i] != (char)(guard ^ i)) {
      printf("test_memcpy(copySize=%d, srcOffset=%d, dstOffset=%d failed! Pre-guardband area at i=%d overwritten!\n", copySize, srcOffset, dstOffset, i);
      exit(1);
    }
    if (dstGuardAfter[i] != (char)(guard ^ i)) {
      printf("test_memcpy(copySize=%d, srcOffset=%d, dstOffset=%d failed! Post-guardband area at i=%d overwritten!\n", copySize, srcOffset, dstOffset, i);
      exit(1);
    }
  }
}

void test_copysize(int copySize) {
  int offsets[6] = { 0, 3, 4, 5, 8, 11 };

  for (int srcOffset = 0; srcOffset < 6; ++srcOffset) {
    for (int dstOffset = 0; dstOffset < 6; ++dstOffset) {
      test_memcpy(copySize, offsets[srcOffset], offsets[dstOffset]);
    }
  }
}

int main() {
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
