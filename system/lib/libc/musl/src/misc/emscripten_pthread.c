#include <pthread.h>
#include "libc.h"
#include "pthread_impl.h"

#if !__EMSCRIPTEN_PTHREADS__
static struct pthread __main_pthread;
pthread_t pthread_self(void) {
    return &__main_pthread;
}
#endif

__attribute__((constructor))
void __emscripten_pthread_data_constructor(void) {
    pthread_self()->locale = &libc.global_locale;
}
