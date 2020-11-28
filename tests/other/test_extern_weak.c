#include <assert.h>
#include <stdio.h>

extern int __attribute__((weak)) externFunc();
extern int __attribute__((weak)) externData;

int main() {
  printf("externFunc: %p\n", externFunc);
  printf("externData: %p\n", &externData); 
  return 0;
}

