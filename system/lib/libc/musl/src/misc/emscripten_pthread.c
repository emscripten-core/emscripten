#include "libc.h"
#include "pthread_impl.h"

#if !__EMSCRIPTEN_PTHREADS__
static struct pthread __main_pthread;
pthread_t pthread_self(void) {
    return &__main_pthread;
}
#endif

void __emscripten_setup_pthread_data(void*);

__attribute__((constructor))
void __emscripten_pthread_data_constructor(void) {
    __emscripten_setup_pthread_data(&libc.global_locale);
    pthread_self()->locale = &libc.global_locale;
}
