#include <assert.h>
#include <emscripten.h>
#include <stdio.h>
#include <vector>

int main() {
  const int CHUNK_SIZE = 100 * 1024 * 1024;
  const int NUM_CHUNKS = 31; // total allocation will be over 3GB

  std::vector<std::vector<char>> chunks;
  chunks.resize(NUM_CHUNKS);

  for (int i = 0; i < NUM_CHUNKS; i++) {
    printf("alloc %d\n", i);
    chunks[i].resize(CHUNK_SIZE);
  }

  for (int i = 0; i < NUM_CHUNKS; i++) {
    printf("test %d\n", i);
    // write in C
    chunks[i][i] = i;
    // read in JS
    int fromJS = EM_ASM_INT({
      return HEAP8[$0];
    }, &chunks[i][i]);
    assert(fromJS == i);
    // write in JS
    EM_ASM_INT({
      HEAP8[$0] = $1 * 2;
    }, &chunks[i][i], i);
    // read in C
    int fromC = chunks[i][i];
    assert(fromC == 2 * i);
  }

  puts("success");
}

