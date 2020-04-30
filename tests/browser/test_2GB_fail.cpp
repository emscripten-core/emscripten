#include <assert.h>
#include <emscripten.h>
#include <stdio.h>
#include <stdlib.h>

void* allocation;

int main() {
  const int CHUNK_SIZE = 100 * 1024 * 1024;
  const int NUM_CHUNKS = 31; // total allocation will be over 3GB

  puts("allocating");

  for (int i = 0; i < NUM_CHUNKS; i++) {
    printf("alloc %d\n", i);
    allocation = malloc(CHUNK_SIZE);
    if (!allocation) {
      assert(i <= 20); // can't get to 2GB
      puts("expected allocation failure");
#ifdef REPORT_RESULT
      REPORT_RESULT(0);
#endif
      return 0;
    }
  }

  puts("UNEXPECTED");

#ifdef REPORT_RESULT
  REPORT_RESULT(1);
#endif
}
