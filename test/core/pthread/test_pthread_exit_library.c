#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <emscripten.h>
#include <emscripten/threading.h>

void* worker(void* arg) {
    printf("worker starting\n");
    fflush(stdout);
    emscripten_thread_sleep(100);

    // proxy back to the main thread
    MAIN_THREAD_ASYNC_EM_ASM({
        resolve();
    });
    return NULL;
}

EMSCRIPTEN_KEEPALIVE
void create_thread_async() {
    pthread_t thread;
    int rc = pthread_create(&thread, NULL, worker, NULL);
    assert(rc == 0);
    pthread_detach(thread);
}
