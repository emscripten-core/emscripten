#include <assert.h>
#include <emscripten/console.h>
#include <stdlib.h>
#include <unistd.h>

size_t sizeT(void* p) {
  return (size_t)p;
}

int main() {
  const size_t MB = 1024 * 1024;
  const size_t ALIGN = 4 * MB;
  const size_t SIZE = 32 * MB;

  // Allocate a very large chunk of memory (32MB) with very high alignment (4
  // MB). This is similar to what mimalloc does in practice.
  void* before = sbrk(0);
  void* p = aligned_alloc(ALIGN, SIZE);
  // aligned_alloc returns NULL on error; validate we actually allocated.
  assert(p);
  void* after = sbrk(0);
  emscripten_console_logf("before: %p  after: %p  p: %p\n", before, after, p);

  // The allocation must be properly aligned.
  assert(sizeT(p) % ALIGN == 0);

  // We should only have sbrk'd a reasonable amount (this is a regression test
  // for #20645 where we sbrk'd double the necessary amount). We expect at most
  // 36 MB (the size of the allocation plus the alignment) plus a few bytes of
  // general overhead.
  assert(sizeT(after) - sizeT(before) < ALIGN + SIZE + 100);

  emscripten_console_log("success");
}
