// Copyright 2017 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <stdio.h>

extern "C" {
float floaty(float f32, int i32, double f64);
float floatyAlone(float f32, int i32, double f64);
}

int main()
{
  // call it once normally. the asm.js ffi will force it to be an f64
  printf("|%f|\n", floaty(12.34f, 1, 100.32));
  {
    // call it using a function pointer, which will need to be f32
    auto* fp = floaty;
    volatile auto vfp = fp;
    printf("|%f|\n", vfp(12.34f, 1, 100.32));
  }
  {
    // no other call for this one
    auto* fp = floatyAlone;
    volatile auto vfp = fp;
    printf("|%f|\n", vfp(12.34f, 1, 100.32));
  }
  return 0;
}

