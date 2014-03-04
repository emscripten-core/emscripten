/*
 * Boehm-compatible GC API
 *
 * WARNING: this is deprecated. You should just build Boehm from source, it is much faster than the toy version written in emscripten
 */
#ifndef _GC_H_INCLUDED
#define _GC_H_INCLUDED

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

static void __attribute__((used)) __GC_KEEPALIVE__() {
  // Force inclusion of necessary dlmalloc functions
  static int times = 1;
  void *x = malloc(times);
  free(x);
  x = calloc(1, times);
  free(x);
  x = calloc(times, 1);
  free(x);
  times++;
}

/* Initialize. */
void GC_INIT();

/* Allocate memory. Cleared to 0 to erase all pointers. */
void *GC_MALLOC(int bytes);

/* Allocate memory for an object that the user promises will not contain pointers. */
void *GC_MALLOC_ATOMIC(int bytes);

/* Allocate memory that might container pointers but that can't be collected. */
void *GC_MALLOC_UNCOLLECTABLE(int bytes);

/* Reallocate a GC managed memory block to a new size. */
void *GC_REALLOC(void *ptr, int newBytes);

/* Explicitly deallocate an object. Dangerous as it forces a free and does not check if the object is reffed. */
void GC_FREE(void *ptr);

/* Register a finalizer. func(ptr, arg) will be called. The old values are saved in old_func, old_arg */
void GC_REGISTER_FINALIZER_NO_ORDER(void *ptr, void (*func)(void *, void *),      void *arg,
                                               void *(*old_func)(void *, void *), void *old_arg);

/* Gets the bytes allocated and managed by the GC */
int GC_get_heap_size();

/* Non-Boehm additions */

/* Call this once per frame or such, it will collect if necessary */
void GC_MAYBE_COLLECT();

/* Forces a GC. Mainly useful for testing, but call it if you know a good time to GC in your app. */
void GC_FORCE_COLLECT();

typedef void (*GC_finalization_proc)(void *func, void *arg);
extern void (*GC_finalizer_notifier)();

extern int GC_finalize_on_demand;
extern int GC_java_finalization;

void GC_enable_incremental();

#ifdef __cplusplus
}
#endif

#endif
