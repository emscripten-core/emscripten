/*
 * Copyright 2024 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <my_port.h>
#include <assert.h>
#include <stdio.h>
#include <SDL2/SDL_image.h>

#ifdef TEST_DEPENDENCY_MPG123
#include <mpg123.h>
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
  printf("value1=%d&value2=%d&value3=%s\n", TEST_VALUE_1, TEST_VALUE_2, TEST_VALUE_3);

  // external port declares deps = ['sdl2_image:formats=jpg'] as a dependency
  // this makes sure that the dependency + options gets processed properly
  assert(IMG_Init(IMG_INIT_JPG) == IMG_INIT_JPG);
  IMG_Quit();

#ifdef TEST_DEPENDENCY_MPG123
  printf("mpg123=%d\n", MPG123_API_VERSION);
#endif
  return 0;
}
