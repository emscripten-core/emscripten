/*
 * Copyright 2017 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <setjmp.h>
#include <stdio.h>

// TODO: Once signals are supported, improve this test to verify preservation of signals state

int main() {
    sigjmp_buf env;
    volatile int flag = 0;
    if (sigsetjmp(env, 1) == 0) {
        // Cannot print anything here, because siglongjmp will
        // print a warning in between but only with ASSERTIONS enabled
        flag = 1;
        siglongjmp(env, 1);
    } else {
        if (flag) {
            puts("Success");
        }
    }
    return 0;
}

