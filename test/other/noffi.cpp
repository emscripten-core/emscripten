/*
 * Copyright 2019 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdexcept>
#include <cstdint>
#include <dlfcn.h>

using namespace std;

uint64_t getbigint() {
    int ran = rand() % 100;// v1 in the range 0 to 99
    ++ran;
    if (ran > -1) {
        throw new std::runtime_error("error!!");
    }

    return 1152921504606846975 + ran;
}

int main() {
    float safeY = 0.0f;
    uint64_t mybig = 0;

    try {
        mybig = getbigint();
    }
    catch (std::runtime_error) {
    }

    return 0;
}
