/*
 * Boehm-compatible GC API
 */

#ifdef __cplusplus
extern "C" {
#endif

void __attribute__((used)) __GC_KEEPALIVE__() {
  // Force inclusion of necessary dlmalloc functions
  static times = 1;
  void *x = malloc(times);
  free(x);
  x = calloc(times);
  free(x);
  times++;
}

/* Initialize. */
void GC_INIT();

/* Allocate memory. Cleared to 0 to erase all pointers. */
void *GC_MALLOC(int bytes);

/* Allocate memory for an object that the user promises will not contain pointers. */
void *GC_MALLOC_ATOMIC(int bytes);

/* Explicitly deallocate an object. Dangerous as it forces a free and does not check if the object is reffed. */
void GC_FREE(void *ptr);

/* Register a finalizer. func(ptr, arg) will be called. The old values are saved in old_func, old_arg */
void GC_REGISTER_FINALIZER_NO_ORDER((void*)ptr, void (*func)(),      void *arg,
                                                void *(*old_func)(), void *old_arg);

/* Non-Boehm addition. Call this once per frame or such, it will collect if necessary */
void GC_MAYBE_COLLECT();

#ifdef __cplusplus
}
#endif

