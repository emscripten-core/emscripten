#include <assert.h>
#include <emscripten/console.h>
#include <emscripten/emmalloc.h>

int main() {
  // Verify we can call both malloc and emmalloc_malloc, and that those are
  // different functions, unless TEST_EMMALLOC_IS_MALLOC is set (in that case,
  // emmalloc is malloc because we let emmalloc define the standard exports like
  // malloc).

  // We have allocated nothing so far, but there may be some initial allocation
  // from startup.
  size_t initial = emmalloc_dynamic_heap_size();
  emscripten_console_logf("initial: %zu\n", initial);

  const size_t ONE_MB = 1024 * 1024;
  void* one = malloc(ONE_MB);
  assert(one);
#ifndef TEST_EMMALLOC_IS_MALLOC
  // We have allocated using malloc, but not emmalloc, so emmalloc reports no
  // change in usage.
  assert(emmalloc_dynamic_heap_size() == initial);
#else
  // malloc == emmalloc_malloc, so emmalloc will report additional usage (of the
  // size of the allocation, or perhaps more if it overallocated as an
  // optimization).
  assert(emmalloc_dynamic_heap_size() >= initial + ONE_MB);
#endif

  void* two = emmalloc_malloc(ONE_MB);
  assert(two);
  // We have allocated using emmalloc, so now emmalloc definitely reports usage.
  assert(emmalloc_dynamic_heap_size() >= initial + ONE_MB);

  emscripten_console_log("success");
}
