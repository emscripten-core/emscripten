/*
 * Copyright 2019 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdexcept>
#include <emscripten.h>


EMSCRIPTEN_KEEPALIVE void destructorWithAlias(){
    {
        std::length_error err("random");
        wprintf(L"success\n");
    }
}