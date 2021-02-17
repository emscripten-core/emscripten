#include <pthread.h>
#include "libc.h"
#include "pthread_impl.h"
#include "stdio_impl.h"

#ifdef __EMSCRIPTEN_PTHREADS__
// In pthreads, we must initialize the runtime at the proper time, which
// is after memory is initialized and before any userland global ctors.
// We must also keep this function alive so it is always called; without
// pthreads, if pthread_self is used then this file will be included,
// and if not then it's fine to not have this.
EM_JS(void, initPthreadsJS, (void), {
  PThread.initRuntime();
})

static void init_file_lock(FILE *f)
{
  if (f && f->lock<0) f->lock = 0;
}

// std{in,out,err}.c will override this if linked
static FILE *volatile dummy_file = 0;
weak_alias(dummy_file, __stdin_used);
weak_alias(dummy_file, __stdout_used);
weak_alias(dummy_file, __stderr_used);

// This must run before any userland ctors
// Note that ASan constructor priority is 50, and we must be higher.
EMSCRIPTEN_KEEPALIVE
__attribute__((constructor(48)))
void __emscripten_pthread_data_constructor(void) {
  initPthreadsJS();
  pthread_self()->locale = &libc.global_locale;
  init_file_lock(__stdin_used);
  init_file_lock(__stdout_used);
  init_file_lock(__stderr_used);
  libc.threaded = 1;
}
#else // !defined(__EMSCRIPTEN_PTHREADS__)
static struct pthread __main_pthread;
uintptr_t __get_tp(void) {
  return (uintptr_t)&__main_pthread;
}

__attribute__((constructor))
void __emscripten_pthread_data_constructor(void) {
  __pthread_self()->locale = &libc.global_locale;
}
#endif // !defined(__EMSCRIPTEN_PTHREADS__)
