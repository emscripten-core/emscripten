/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <string.h>
#include <typeinfo>
#include <stdint.h>
int main() {
  printf("*\n");
#define MAX 100
  long ptrs[MAX];
  int groups[MAX];
  memset(ptrs, 0, MAX * sizeof(long));
  memset(groups, 0, MAX * sizeof(int));
  int next_group = 1;
#define TEST(X)                         \
  {                                     \
    intptr_t ptr = (intptr_t)&typeid(X);          \
    int group = 0;                      \
    int i;                              \
    for (i = 0; i < MAX; i++) {         \
      if (!groups[i]) break;            \
      if (ptrs[i] == ptr) {             \
        group = groups[i];              \
        break;                          \
      }                                 \
    }                                   \
    if (!group) {                       \
      groups[i] = group = next_group++; \
      ptrs[i] = ptr;                    \
    }                                   \
    printf("%s:%d\n", #X, group);       \
  }
  TEST(int);
  TEST(unsigned int);
  TEST(unsigned);
  TEST(signed int);
  TEST(long);
  TEST(unsigned long);
  TEST(signed long);
  TEST(long long);
  TEST(unsigned long long);
  TEST(signed long long);
  TEST(short);
  TEST(unsigned short);
  TEST(signed short);
  TEST(char);
  TEST(unsigned char);
  TEST(signed char);
  TEST(float);
  TEST(double);
  TEST(long double);
  TEST(void);
  TEST(void*);
  printf("*\n");
}
