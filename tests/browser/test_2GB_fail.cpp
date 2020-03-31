#include <assert.h>
#include <emscripten.h>
#include <stdio.h>
#include <vector>

int main() {
  const int CHUNK_SIZE = 100 * 1024 * 1024;
  const int NUM_CHUNKS = 31; // total allocation will be over 3GB

  std::vector<std::vector<char>> chunks;
  chunks.resize(NUM_CHUNKS);

  puts("allocating");

  for (int i = 0; i < NUM_CHUNKS; i++) {
    printf("alloc %d\n", i);
    try {
      chunks[i].resize(CHUNK_SIZE);
    } catch (const std::bad_alloc& e) {
      assert(i <= 20); // can't get to 2GB
      puts("expected allocation failure");
      return 0;
    }
  }

  puts("UNEXPECTED");
}

