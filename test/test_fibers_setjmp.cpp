// Copyright 2025 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <emscripten/fiber.h>

struct Fiber {
    emscripten_fiber_t context;
    char asyncify_stack[1024];
    alignas(16) char c_stack[4096];

    void init_current() {
        emscripten_fiber_init_from_current_context(&context, asyncify_stack, sizeof(asyncify_stack));
    }

    void init(em_arg_callback_func entry, void *arg) {
        emscripten_fiber_init(&context, entry, arg, c_stack, sizeof(c_stack), asyncify_stack, sizeof(asyncify_stack));
    }

    void swap(Fiber *fiber) {
        emscripten_fiber_swap(&context, &fiber->context);
    }
};

static struct Globals {
    Fiber main;
    Fiber f1;
    Fiber f2;

    jmp_buf buffer;
    jmp_buf buffer2;
} G;

[[noreturn]]
static void f1(void *arg) {
    printf("f1: start arg:%d\n", *(int*)arg);

    G.f1.swap(&G.main);

    printf("f1: cont1\n");

    G.f1.swap(&G.f2);

    printf("f1: cont2\n");

    G.f1.swap(&G.f2);

    printf("f1: cont3\n");

    G.f1.swap(&G.main);
    abort();
}

[[noreturn]]
static void f2(void *arg) {
    printf("f2: start arg:%d\n", *(int*)arg);

    int exc;
    if ((exc = setjmp(G.buffer2)) == 0) {
        G.f2.swap(&G.f1);

        printf("f2: cont1\n");

        longjmp(G.buffer2, 11);

        printf("f2: not visible\n");
    } else {
        printf("f2: exc %d\n", exc);
    }

    G.f2.swap(&G.main);

    printf("f2: cont2\n");

    G.f2.swap(&G.f1);
    abort();
}

int main(int argc, char **argv) {
    G.main.init_current();

    int test = 50;
    G.f1.init(f1, &test);

    int test2 = 60;
    G.f2.init(f2, &test2);

    int exc;
    if ((exc = setjmp(G.buffer)) == 0) {
        printf("main: prejump\n");

        G.main.swap(&G.f1);

        printf("main: continuing1\n");

        G.main.swap(&G.f1);

        printf("main: continuing2\n");

        G.main.swap(&G.f2);

        printf("main: continuing3\n");

        longjmp(G.buffer, 1);

        printf("main: not visible\n");

    } else {
        printf("main: got exc %d\n", exc);
    }

    return 0;
}
