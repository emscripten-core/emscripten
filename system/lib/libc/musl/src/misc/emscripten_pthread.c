#include <pthread.h>
#include "libc.h"
#include "pthread_impl.h"

#if !__EMSCRIPTEN_PTHREADS__
static struct pthread __main_pthread;
pthread_t pthread_self(void) {
    return &__main_pthread;
}
#endif

// Needs to be called after PThread.initRuntime, which in turn needs to be
// called after constructors have run and memory is initialized.
EMSCRIPTEN_KEEPALIVE
void __emscripten_pthread_data_constructor(void) {
    pthread_self()->locale = &libc.global_locale;
}
