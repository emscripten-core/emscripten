#include <pthread.h>
#include "libc.h"
#include "pthread_impl.h"

#if !__EMSCRIPTEN_PTHREADS__
static struct pthread __main_pthread;
pthread_t pthread_self(void) {
  // Ensure the locale is set up here, avoid a global ctor as done below for
  // the pthreads case.
  __main_pthread.locale = &libc.global_locale;
  return &__main_pthread;
}
#endif // !__EMSCRIPTEN_PTHREADS__

#if __EMSCRIPTEN_PTHREADS__
// In pthreads, we must initialize the runtime at the proper time, which
// is after memory is initialized and before any userland global ctors.
// We must also keep this function alive so it is always called; without
// pthreads, if pthread_self is used then this file will be included,
// and if not then it's fine to not have this.
EM_JS(void, initPthreadsJS, (void), {
  PThread.initRuntime();
})

EMSCRIPTEN_KEEPALIVE
__attribute__((constructor(100))) // This must run before any userland ctors
void __emscripten_pthread_data_constructor(void) {
  initPthreadsJS();
  pthread_self()->locale = &libc.global_locale;
}
#endif
