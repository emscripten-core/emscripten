/*
 * Copyright 2014 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>

void throwhere(void) {
    throw(1);
}

void (*funptr)(void) = throwhere;

void nocatch(void) {
    try {
        funptr();
    }
    catch (...) {
        printf("ERROR\n");
    }
}

int main(int argc, char* argv[]) {
    try {
        nocatch();
    }
    catch (...) {
        printf("SUCCESS\n");
    }
    return 0;
}

