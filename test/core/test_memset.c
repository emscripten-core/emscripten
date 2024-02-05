/*
 * Copyright 2014 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>

#define TOTAL 10240

#define TEST(size, type) { \
  for (int i = 0; i < TOTAL; i++) { \
    buffer[i] = i*seed; \
  } \
  memset(buffer+size%17, 0xA5, size); \
  int v = 0; \
  for (int i = 0; i < TOTAL; i++) { \
    v += buffer[i]; \
  } \
  printf("final %d:%d\n", size, v); \
}

int main() {
  #define RUN(type) \
  { \
    static type buffer[TOTAL]; \
    volatile int seed = 123; \
    TEST(1, type); \
    TEST(2, type); \
    TEST(3, type); \
    TEST(4, type); \
    TEST(5, type); \
    TEST(6, type); \
    TEST(7, type); \
    TEST(8, type); \
    TEST(9, type); \
    TEST(10, type); \
    TEST(16, type); \
    TEST(32, type); \
    TEST(64, type); \
    TEST(128, type); \
    TEST(256, type); \
    TEST(512, type); \
    TEST(1024, type); \
    for (int x = 10; x < 100; x += 10) { TEST(x, type) }; \
  }
  printf("8\n");
  RUN(unsigned char);
  printf("16\n");
  RUN(unsigned short);
  printf("32\n");
  RUN(unsigned);
  return 0;
}

