// Copyright 2015 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <emscripten/bind.h>

using namespace emscripten;

EMSCRIPTEN_BINDINGS(settings) {
    constant("isMemoryGrowthEnabled", true);
}
