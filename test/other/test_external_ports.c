/*
 * Copyright 2024 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <my_port.h>
#include <assert.h>
#include <stdio.h>

#ifdef TEST_DEPENDENCY_SDL2
#include <SDL2/SDL.h>
#endif

// TEST_VALUE_1 and TEST_VALUE_2 are defined via port options
#ifndef TEST_VALUE_1
#define TEST_VALUE_1 0
#endif
#ifndef TEST_VALUE_2
#define TEST_VALUE_2 0
#endif

int main() {
  assert(my_port_fn(99) == 99); // check that we can call a function from my_port.h
  printf("value1=%d&value2=%d\n", TEST_VALUE_1, TEST_VALUE_2);
#ifdef TEST_DEPENDENCY_SDL2
  SDL_version version;
  SDL_VERSION(&version);
  printf("sdl2=%d\n", version.major);
#endif
  return 0;
}
