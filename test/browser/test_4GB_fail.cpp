#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void* allocation;

int main() {
  const int MB = 1024 * 1024;
  const int CHUNK = 512 * MB;
  long num_allocations;
  for (num_allocations = 0;; num_allocations++) {
    allocation = malloc(CHUNK);
    if (!allocation) {
      puts("failed to allocate any more");
      break;
    }
    printf("%ld: Allocated %d MB, total so far: %ld MB\n",
           num_allocations, CHUNK / MB,
           (num_allocations * CHUNK) / MB);
    printf("  (writing to make sure, to %p)\n", allocation);
    memset(allocation, 42, CHUNK);
  }
  // We should have allocated at least 2GB.
  assert(num_allocations >= 4);
  // We should have allocated less than 4GB (we can't get to exactly 4GB
  // since we started with some small amount, and then add 512MB chunks).
  assert(num_allocations < 8);
  puts("success");
}
