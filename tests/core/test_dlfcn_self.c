/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <dlfcn.h>
#include <emscripten.h>

int EMSCRIPTEN_KEEPALIVE global = 123;

extern "C" EMSCRIPTEN_KEEPALIVE void foo(int x) {
        printf("%d\n", x);
}

extern "C" EMSCRIPTEN_KEEPALIVE void repeatable() {
        void* self = dlopen(NULL, RTLD_LAZY);
        int* global_ptr = (int*)dlsym(self, "global");
        void (*foo_ptr)(int) = (void (*)(int))dlsym(self, "foo");
        foo_ptr(*global_ptr);
        dlclose(self);
}

int main() {
        repeatable();
        repeatable();
        return 0;
}
