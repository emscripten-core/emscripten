// Copyright 2019 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <iostream>
#include <ctime>
#include <condition_variable>
#include <pthread.h>
#include <emscripten.h>

static long now() {
    struct timespec time;
    clock_gettime(CLOCK_MONOTONIC, &time);
    return time.tv_sec * 1000 + time.tv_nsec / 1000 / 1000;
}

static long ping, pong;

static std::mutex mutex;
static std::condition_variable cond_var;
static bool pong_requested = false;

void *thread_main(void *arg) {
    std::cout << "running thread ..." << std::endl;

    std::unique_lock<std::mutex> lock(mutex);
    while (!pong_requested)
        cond_var.wait(lock);
    pong = now(); // Measure time in the pthread
    std::cout << "pong - ping: " << (pong - ping) << std::endl;
    pong_requested = false;
    cond_var.notify_one();

    return NULL;
}

EMSCRIPTEN_KEEPALIVE
extern "C" int notify() {
    {
        std::unique_lock<std::mutex> lock(mutex);
        std::cout << "notifying ..." << std::endl;
        ping = now();
        pong_requested = true;
        cond_var.notify_one();
    }

    {
        std::unique_lock<std::mutex> lock(mutex);
        while (pong_requested)
            cond_var.wait(lock);
        long last = now();
        std::cout << "last - pong: " << (last - ping) << std::endl;

        // Time measured on a worker should be relative to the main thread,
        // so that things are basically monotonic.
        REPORT_RESULT(int(pong >= ping) + 2 * int(last >= pong));
    }

    return 0;
}

int main() {
    std::cout << "running main ..." << std::endl;

    EM_ASM({
        setTimeout(function() {
            Module._notify();
        });
    });

    pthread_attr_t attr;
    pthread_attr_init(&attr);

    pthread_t thread;
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    int error = pthread_create(&thread, &attr, thread_main, NULL);
    if (error)
        abort();

    return 0;
}
