/*
 * Copyright 2013 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <sys/mman.h>
#include <emscripten.h>
#include <string.h>
#include <assert.h>

int main() {
    printf("*\n");
    FILE *f = fopen("data.dat", "r");
    char *m;
    m = (char*)mmap(NULL, 9000, PROT_READ, MAP_PRIVATE, fileno(f), 0);
    for (int i = 0; i < 20; i++) putchar(m[i]);
    munmap(m, 9000);
    printf("\n");
    m = (char*)mmap(NULL, 9000, PROT_READ, MAP_PRIVATE, fileno(f), 4096);
    for (int i = 0; i < 20; i++) putchar(m[i]);
    munmap(m, 9000);
    printf("\n*\n");

#ifdef REPORT_RESULT
    REPORT_RESULT(1);
#endif
    return 0;
}
