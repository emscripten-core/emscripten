// Copyright 2013 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <stdio.h>

#if __cplusplus != 201103L
#error This file should be compiled with -std=c++11!
#endif

#ifdef _REENTRANT
#error Expected to be compiled single-threaded.
#endif

#ifdef __cpp_threadsafe_static_init
#error single-threaded builds should not define __cpp_threadsafe_static_init
#endif

int main( int argc, const char *argv[] ) {
  printf("Hello world!\\n");
  return 0;
}
