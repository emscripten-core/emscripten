/*
 * Copyright 2019 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <cstdint>

extern void passBigInt(uint64_t);

void callPassBigInt(){
    passBigInt(1152921504606846975);
}