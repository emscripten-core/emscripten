#include "zlib.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>


// don't inline, to be friendly to js engine osr
void __attribute__ ((noinline)) doit(char *buffer, int size, int i) {
  static char *buffer2 = NULL;
  static char *buffer3 = NULL;

  unsigned long maxCompressedSize = compressBound(size);

  if (!buffer2) buffer2 = (char*)malloc(maxCompressedSize);
  if (!buffer3) buffer3 = (char*)malloc(size);

  unsigned long compressedSize = maxCompressedSize;
  compress(buffer2, &compressedSize, buffer, size);
  if (i == 0) printf("sizes: %d,%d\n", size, compressedSize);

  unsigned long decompressedSize = size;
  uncompress(buffer3, &decompressedSize, buffer2, compressedSize);
  assert(decompressedSize == size);
  if (i == 0) assert(strcmp(buffer, buffer3) == 0);
}

int main(int argc, char **argv) {
  int size = atoi(argv[1]);
  int iters = atoi(argv[2]);
  char *buffer = malloc(size);

  int i = 0;
  int run = 0;
  char runChar = 17;
  while (i < size) {
    if (run > 0) {
      run--;
    } else {
      if ((i & 7) == 0) {
        runChar = i & 7;
        run = i & 31;
      } else {
        runChar = (i*i) % 6714;
      }
    }
    buffer[i] = runChar;
    i++;
  }

  for (i = 0; i < iters; i++) {
    doit(buffer, size, i);
  }

  printf("ok.\n");

  return 0;
}

