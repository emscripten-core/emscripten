/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <limits>

int main()
{
    long long i,j,k;

    i = 0;
    j = -1,
    k = 1;

    printf( "*\n" );
    printf( "%s\n", i > j ? "Ok": "Fail" );
    printf( "%s\n", k > i ? "Ok": "Fail" );
    printf( "%s\n", k > j ? "Ok": "Fail" );
    printf( "%s\n", i < j ? "Fail": "Ok" );
    printf( "%s\n", k < i ? "Fail": "Ok" );
    printf( "%s\n", k < j ? "Fail": "Ok" );
    printf( "%s\n", (i-j) >= k ? "Ok": "Fail" );
    printf( "%s\n", (i-j) <= k ? "Ok": "Fail" );
    printf( "%s\n", i > std::numeric_limits<long long>::min() ? "Ok": "Fail" );
    printf( "%s\n", i < std::numeric_limits<long long>::max() ? "Ok": "Fail" );
    printf( "*\n" );
}
