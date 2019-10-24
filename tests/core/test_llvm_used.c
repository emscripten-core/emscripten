/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <emscripten.h>

__attribute__((annotate("hello attribute world")))
EMSCRIPTEN_KEEPALIVE void foobar(int x) {
    printf("Worked! %d\n", x);
}

int main() {
    emscripten_run_script("Module['_foobar'](10)");
    return 0;
}
