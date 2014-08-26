#include <stdio.h>

#define ALIGN(num_bytes) __attribute__((aligned(num_bytes)))

struct Aligned {
  char ALIGN(4) a4;
  char ALIGN(8) a8;
  char ALIGN(16) a16;
};

void Test(const void* p, int size) {
  printf("align %d: %d\n", size, reinterpret_cast<size_t>(p) % size);
}

int main() {
  Aligned a;
  Test(&a.a4, 4);
  Test(&a.a8, 8);
  Test(&a.a16, 16);

  int p = reinterpret_cast<size_t>(&a);
  printf("base align: %d, %d, %d, %d\n", p%4, p%8, p%16);

  return 0;
}

