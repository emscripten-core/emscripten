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
    chunks[i].resize(CHUNK_SIZE);
  }

  puts("testing");

  for (int i = 0; i < NUM_CHUNKS; i++) {
    printf("test %d\n", i);
    chunks[i][i] = i;
    int fromJS = EM_ASM_INT({
      return HEAP8[$0];
    }, &chunks[i][i]);
    printf("wrote %d in C, read %d from JS\n", i, fromJS);
    EM_ASM_INT({
      HEAP8[$0] = 2 * $1;
    }, &chunks[i][i], i);
    int fromC = chunks[i][i];
    printf("wrote %d in JS, read %d from C\n", 2 * i, fromC);
  }

  puts("success");
}

