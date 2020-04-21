#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

std::vector<void*> allocations;

int main() {
  const int MB = 1024 * 1024;
  const int CHUNK = 512 * MB;
  while (1) {
    void* allocation = malloc(CHUNK);
    if (!allocation) {
      puts("failed to allocate any more");
      break;
    }
    allocations.push_back(allocation);
    printf("%ld: Allocated %d MB, total so far: %ld MB\n",
           allocations.size(), CHUNK / MB,
           (allocations.size() * CHUNK) / MB);
    printf("  (writing to make sure, to %p)\n", allocation);
    memset(allocation, 42, CHUNK);
  }
  // We should have allocated at least 2GB.
  assert(((allocations.size() * CHUNK) / MB) >= (2 * 1024));
  // We should have allocated less than 4GB.
  assert(((allocations.size() * CHUNK) / MB) < (4 * 1024));
  puts("success");
}

