// Copyright 2014 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <stdio.h>
#include <iostream>

int main()
{
    std::cout << "Print in " << "two parts." << '\n';
    std::cerr << "std::cerr in " << "two parts." << '\n';

    printf("hello!");
    fflush(stdout);
    fprintf(stderr, "hello from stderr too!");
    fflush(stderr);
    return 0;
}
