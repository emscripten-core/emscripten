/*
 * Copyright 2018 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

/*
  References to these longjmp- and exceptions-supporting things are generated
  in the llvm backend.
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
