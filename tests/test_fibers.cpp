// Copyright 2019 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

// Based on test_coroutines.cpp

#include <stdio.h>
#include <stdlib.h>
#include <emscripten.h>

struct Fiber {
    emscripten_fiber context;
    void *stack;
    int result;

    static void bad(void *arg) {
        abort();
    }

    void init(em_arg_callback_func entry, void *arg) {
        const int stack_size = 1 << 13;
        stack = malloc(stack_size);
        context = emscripten_fiber_create(bad, nullptr, stack, stack_size);
        emscripten_fiber_recycle(context, entry, arg);
        result = 0;
    }

    ~Fiber() {
        free(stack);
        emscripten_fiber_destroy(context);
        printf("\ndestructor");
    }
};

static struct Globals {
    emscripten_fiber main;
    Fiber fibers[2];

    Globals() {
        main = emscripten_fiber_create_from_current_context();
    }

    ~Globals() {
        emscripten_fiber_destroy(main);
        printf("\ndestructor");
    }
} G;

static void leaf(void) {
    printf("leaf-");
}

static void fib(void * arg) {
    int *p = (int*)arg;
    int cur = 1;
    int next = 1;
    for(int i = 0; i < 9; ++i) {
        *p = cur;
        emscripten_fiber_swap(G.fibers[0].context, G.main);
        int next2 = cur + next;
        cur = next;
        next = next2;
    }
}

static void f(void *arg) {
    int *p = (int*)arg;
    *p = 0;
    leaf();
    emscripten_fiber_swap(G.fibers[0].context, G.main);
    fib(arg);

    G.fibers[0].result = 1;
    emscripten_fiber_swap(G.fibers[0].context, G.main);
    abort();
}

static void g(void *arg) {
    int *p = (int*)arg;
    for(int i = 0; i < 10; ++i) {
        *p = 100+i;
        emscripten_fiber_swap(G.fibers[1].context, G.main);
    }

    G.fibers[1].result = 1;
    emscripten_fiber_swap(G.fibers[1].context, G.main);
    abort();
}

int main(int argc, char **argv) {
    int i;
    G.fibers[0].init(f, &i);
    G.fibers[1].init(g, &i);

    printf("*");
    while(emscripten_fiber_swap(G.main, G.fibers[0].context), !G.fibers[0].result) {
        printf("%d-", i);
        emscripten_fiber_swap(G.main, G.fibers[1].context);
        printf("%d-", i);
    }
    printf("*");

    return 0;
}
