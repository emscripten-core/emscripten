#include <assert.h>
#include <emscripten/console.h>
#include <emscripten/emmalloc.h>

int main() {
  // Verify we can call both malloc and emmalloc_malloc, and that those are
  // different functions, unless _TEST_WITH_STD_EXP is set (in that case, we
  // let emmalloc define the standard exports like malloc, which overrode the
  // system defaults, and so malloc == emmalloc_malloc).

  // We have allocated nothing so far, but there may be some initial allocation
  // from startup.
  size_t initial = emmalloc_dynamic_heap_size();
  emscripten_console_logf("initial: %zu\n", initial);

  const size_t ONE_MB = 1024 * 1024;
  void* one = malloc(ONE_MB);
  assert(one);
#ifndef _TEST_WITH_STD_EXP
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
