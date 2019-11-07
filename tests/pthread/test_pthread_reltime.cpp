// Copyright 2019 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <iostream>
#include <pthread.h>
#include <emscripten.h>

typedef std::chrono::milliseconds::rep TimePoint;

TimePoint now() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
}

static TimePoint ping, pong;

static std::mutex mutex;
static std::condition_variable cond_var;

void *thread_main(void *arg) {
    std::cout << "running thread ..." << std::endl;

    std::unique_lock<std::mutex> lock(mutex);

    cond_var.wait(lock);

    // Measure time in the pthread.
    pong = now();

    std::cout << "pong - ping: " << (now() - ping) << std::endl;

    cond_var.notify_one();

    return NULL;
}

EMSCRIPTEN_KEEPALIVE
extern "C" int notify() {
    {
        std::unique_lock<std::mutex> lock(mutex);

        std::cout << "notifying ..." << std::endl;

        ping = now();

        cond_var.notify_one();
    }
    {
        std::unique_lock<std::mutex> lock(mutex);

        cond_var.wait(lock);

        TimePoint last = now();

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
