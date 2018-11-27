// Copyright 2018 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <stdio.h>
#include <emscripten.h>

// Using extern "C" so we can pass not mangled names to EMTERPRETIFY_WHITELIST

extern "C" void leaf(void) {
  printf("leaf-");
}

extern "C" void fib(void * arg) {
    int * p = (int*)arg;
    int cur = 1;
    int next = 1;
    for(int i = 0; i < 9; ++i) {
        *p = cur;
        emscripten_yield();
        int next2 = cur + next;
        cur = next;
        next = next2;
    }
}

extern "C" void f(void * arg) {
    int * p = (int*)arg;
    *p = 0;
    leaf();
    emscripten_yield();
    fib(arg); // emscripten_yield in fib() can `pass through` f() back to main(), and then we can assume inside fib()
}

extern "C" void g(void * arg) {
    int * p = (int*)arg;
    for(int i = 0; i < 10; ++i) {
        *p = 100+i;
        emscripten_yield();
    }
}

int main(int argc, char **argv) {
    int i;
    emscripten_coroutine co = emscripten_coroutine_create(f, (void*)&i, 0);
    emscripten_coroutine co2 = emscripten_coroutine_create(g, (void*)&i, 0);
    printf("*");
    while(emscripten_coroutine_next(co)) {
        printf("%d-", i);
        emscripten_coroutine_next(co2);
        printf("%d-", i);
    }
    printf("*");
    return 0;
}
