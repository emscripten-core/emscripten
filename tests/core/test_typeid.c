#include <stdio.h>
#include <string.h>
#include <typeinfo>
int main() {
  printf("*\n");
#define MAX 100
  int ptrs[MAX];
  int groups[MAX];
  memset(ptrs, 0, MAX * sizeof(int));
  memset(groups, 0, MAX * sizeof(int));
  int next_group = 1;
#define TEST(X)                         \
  {                                     \
    int ptr = (int)&typeid(X);          \
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
