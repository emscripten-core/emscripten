#include <stdio.h>
#include <stdlib.h>
#include <emscripten.h>

int main(int argc, char **argv) {
  printf("argc: %d\n", argc);
  for(int i = 0; i < argc; ++i) {
    printf("argv[%d]: %s\n", i, argv[i]);
  }

  // Dump a file to local filesystem with emrun.
  EM_ASM(emrun_file_dump("test.dat", HEAPU8.subarray(0, 128)););
  EM_ASM(emrun_file_dump("heap.dat", HEAPU8));

  if (argc <= 1)
    exit(1);
  printf("hello, world!\n");
  fprintf(stderr, "hello, error stream!\n");

  printf("Testing ASCII characters: !\"$%%&'()*+,-./:;<=>?@[\\]^_`{|}~\n");
  printf("Testing char sequences: %%20%%21 &auml;\n");

  exit(100);
}

