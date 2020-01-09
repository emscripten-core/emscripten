/*
 * Copyright 2019 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <assert.h>
#include <stdalign.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    for(int i = 0; i < 16; ++i) {
        void *p = malloc(i);
        assert(((uintptr_t)p & (alignof(max_align_t) - 1)) == 0);
    }

    return 0;
}
