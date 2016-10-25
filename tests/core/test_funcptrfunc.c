#include <stdio.h>

typedef void (*funcptr)(int, int);
typedef funcptr (*funcptrfunc)(int);

funcptr __attribute__((noinline)) getIt(int x) { return (funcptr)x; }

int main(int argc, char **argv) {
  funcptrfunc fpf = argc < 100 ? getIt : NULL;
  printf("*%p*\n", fpf(argc));
  return 0;
}
