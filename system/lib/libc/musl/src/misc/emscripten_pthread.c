#include <pthread.h>
#include "libc.h"
#include "pthread_impl.h"

#if !__EMSCRIPTEN_PTHREADS__
static struct pthread __main_pthread;
pthread_t pthread_self(void) {
    return &__main_pthread;
}
#endif // !__EMSCRIPTEN_PTHREADS__

#if __EMSCRIPTEN_PTHREADS__
EM_JS(void, initPthreadsJS, (void), {
  PThread.initRuntime();
})
#endif

EMSCRIPTEN_KEEPALIVE
__attribute__((constructor(100))) // This must run before any userland ctors
void __emscripten_pthread_data_constructor(void) {
#if __EMSCRIPTEN_PTHREADS__
  initPthreadsJS();
#endif
  pthread_self()->locale = &libc.global_locale;
}
