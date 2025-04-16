#include <stdio.h>
#include <assert.h>

void side();

__attribute__((weak)) int foo();

int main(int argc, char const *argv[]) {
  printf("main &foo: %p\n", &foo);
  assert(&foo == NULL);
  side();
  return 0;
}
