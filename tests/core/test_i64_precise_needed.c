/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>

int main( int argc, char ** argv )
{
    unsigned long a = 0x60DD1695U;
    unsigned long b = 0xCA8C4E7BU;
    unsigned long long c = (unsigned long long)a * b;
    printf( "c = %016llx\n", c );

    return 0;
}
