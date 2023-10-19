#include <assert.h>
#include <emscripten/console.h>
#include <emscripten/emmalloc.h>

int main() {
#ifdef TEST_EMMALLOC_IS_MALLOC
  // In this mode emmalloc defines malloc, so the two functions are the same.
  assert(&emmalloc_malloc == &malloc);
#else
  // In this mode we told emmalloc *not* to define malloc etc., and so malloc is
  // the system malloc, which is different.
  assert(&emmalloc_malloc != &malloc);
#endif

  // Verify we can call both malloc and emmalloc_malloc, and that those are
  // different functions as the above checks imply, unless
  // TEST_EMMALLOC_IS_MALLOC is set. This adds extra testing on top of just
  // comparing the function pointers (that comparison could be wrong if there
  // were a trampoline, etc.; from here we test actual functionality, to be
  // sure).

  // We have allocated nothing so far, but there may be some initial allocation
  // from startup.
  size_t initial = emmalloc_dynamic_heap_size();
  emscripten_console_logf("initial: %zu\n", initial);

  const size_t ONE_MB = 1024 * 1024;
  void* one = malloc(ONE_MB);
  assert(one);
#ifdef TEST_EMMALLOC_IS_MALLOC
  // malloc == emmalloc_malloc, so emmalloc will report additional usage (of the
  // size of the allocation, or perhaps more if it overallocated as an
  // optimization).
  assert(emmalloc_dynamic_heap_size() >= initial + ONE_MB);
#else
  // We have allocated using malloc, but not emmalloc, so emmalloc reports no
  // change in usage.
  assert(emmalloc_dynamic_heap_size() == initial);
#endif

  void* two = emmalloc_malloc(ONE_MB);
  assert(two);
  // We have allocated using emmalloc, so now emmalloc definitely reports usage.
  assert(emmalloc_dynamic_heap_size() >= initial + ONE_MB);

  emscripten_console_log("success");
}
