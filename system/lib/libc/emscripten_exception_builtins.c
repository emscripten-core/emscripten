/*
 * Copyright 2018 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

/*
  References to these longjmp- and exceptions-supporting things are generated
  in the llvm backend.

  Note that these might make more sense in compiler-rt, but they need to be
  built with multithreading support when relevant (to avoid races between
  threads that throw at the same time etc.), and compiler-rt is not built
  that way atm.
*/

#include <threads.h>

thread_local int __THREW__ = 0;
thread_local int __threwValue = 0;

void setThrew(int threw, int value) {
  if (__THREW__ == 0) {
    __THREW__ = threw;
    __threwValue = value;
  }
}
