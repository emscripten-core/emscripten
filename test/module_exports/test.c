/*
 * Copyright 2013 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>

/**
 * This is a simple test method to verify that the contents of a buffer created in JavaScript can be
 * passed into a C function.
 */
void bufferTest(const void* source, unsigned int sourceLen) {
    int i = 0;
    unsigned char* src = (unsigned char*)source;

    printf("in test method: size of source buffer = %d\n", sourceLen);

    for (i = 0; i < sourceLen; i++) {
        printf("source[%d] = %d\n", i, src[i]);
    }
}

