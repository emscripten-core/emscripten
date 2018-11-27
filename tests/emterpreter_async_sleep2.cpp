// Copyright 2015 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <stdio.h>
#include <emscripten.h>

int main(void) {
    printf("No yield:\n");
    emscripten_sleep(500);
    printf("With yield:\n");
    emscripten_sleep_with_yield(500);
    printf("Again no yield:\n");
    emscripten_sleep(500);
    printf("Done!\n");
    REPORT_RESULT(1);
    return 0;
}

