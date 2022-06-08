// Copyright 2018 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <string>
#include <emscripten/bind.h>
#include <emscripten/emscripten.h>

const std::string global_string = "global string";

EMSCRIPTEN_BINDINGS(constants) {
  emscripten::constant("global_string", global_string);
}

int main() {
    EM_ASM(
        console.log("global_string = " + Module['global_string']);
    );
}
