#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

void* allocation;

int main() {
  const int MB = 1024 * 1024;
  const int CHUNK = 512 * MB;
  int i;
  for (i = 0;; i++) {
    allocation = malloc(CHUNK);
    if (!allocation) {
      puts("failed to allocate any more");
      break;
    }
    printf("%ld: Allocated %d MB, total so far: %ld MB\n",
           i, CHUNK / MB,
           (i * CHUNK) / MB);
    printf("  (writing to make sure, to %p)\n", allocation);
    memset(allocation, 42, CHUNK);
  }
  // We should have allocated at least 2GB.
  assert(((i * CHUNK) / MB) >= (2 * 1024));
  // We should have allocated less than 4GB (we can't get to exactly 4GB
  // since we started with some small amount, and then add 512MB chunks).
  assert(((i * CHUNK) / MB) < (4 * 1024));
  puts("success");
}
