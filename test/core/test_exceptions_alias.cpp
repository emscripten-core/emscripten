/*
 * Copyright 2014 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#define _POSIX_SOURCE
#include <locale.h>
#include <ctype.h>
#include <stdio.h>

int main(void) {
    try {
        printf("*%i*\n", isdigit('0'));
        printf("*%i*\n", isdigit_l('0', LC_GLOBAL_LOCALE));
    }
    catch (...) {
        printf("EXCEPTION!\n");
    }
}

