// Copyright 2017 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <emscripten.h>
template <typename... Args>
int call(Args... args) {
  return(EM_ASM_INT(
    {
      out(Array.prototype.join.call(arguments, ','));
    },
    args...
  ));
}

int main(int argc, char **argv) {
  call(1);
  call(1, 2);
  call(1, 2, 3);
  return 0;
}
