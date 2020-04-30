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
      return 2 * HEAP8[$0];
    }, &chunks[i][i]);
    assert(2 * i == fromJS);
    EM_ASM_INT({
      HEAP8[$0] = 3 * $1;
    }, &chunks[i][i], i);
    int fromC = chunks[i][i];
    assert(3 * i == fromC);
  }

  puts("success");

#ifdef REPORT_RESULT
  REPORT_RESULT(0);
#endif
}
