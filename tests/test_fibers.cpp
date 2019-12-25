// Copyright 2019 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

// Based on test_coroutines.cpp

#include <stdio.h>
#include <stdlib.h>
#include <emscripten/fiber.h>

struct Fiber {
    emscripten_fiber_t context;
    alignas(16) char stack[1 << 13];
    int result = 0;

    void init_with_api(em_arg_callback_func entry, void *arg) {
        emscripten_fiber_init(&context, sizeof(context), entry, arg, stack, sizeof(stack));
    }

    void init_manually(em_arg_callback_func entry, void *arg) {
        context.asyncify_fiber.stack_base = stack + sizeof(stack);
        context.asyncify_fiber.stack_limit = stack;
        context.asyncify_fiber.stack_ptr = context.asyncify_fiber.stack_base;
        context.asyncify_fiber.entry = entry;
        context.asyncify_fiber.user_data = arg;
        context.asyncify_data.stack_ptr = context.asyncify_stack;
        context.asyncify_data.stack_limit = reinterpret_cast<char*>(&context) + sizeof(context);
    }

    void swap(emscripten_fiber_t *fiber) {
        emscripten_fiber_swap(&context, fiber);
    }

    ~Fiber() {
        printf("\ndestructor");
    }
};

static struct Globals {
    emscripten_fiber_t main;
    Fiber fibers[2];

    Globals() {
        emscripten_fiber_init_from_current_context(&main, sizeof(main));
    }

    ~Globals() {
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
        G.fibers[0].swap(&G.main);
        int next2 = cur + next;
        cur = next;
        next = next2;
    }
}

[[noreturn]]
static void f(void *arg) {
    int *p = (int*)arg;
    *p = 0;
    leaf();
    G.fibers[0].swap(&G.main);
    fib(arg);

    G.fibers[0].result = 1;
    G.fibers[0].swap(&G.main);
    abort();
}

[[noreturn]]
static void g(void *arg) {
    int *p = (int*)arg;
    for(int i = 0; i < 10; ++i) {
        *p = 100+i;
        G.fibers[1].swap(&G.main);
    }

    G.fibers[1].result = 1;
    G.fibers[1].swap(&G.main);
    abort();
}

int main(int argc, char **argv) {
    int i;
    G.fibers[0].init_with_api(f, &i);
    G.fibers[1].init_manually(g, &i);

    printf("*");
    while(emscripten_fiber_swap(&G.main, &G.fibers[0].context), !G.fibers[0].result) {
        printf("%d-", i);
        emscripten_fiber_swap(&G.main, &G.fibers[1].context);
        printf("%d-", i);
    }
    printf("*");

    return 0;
}
