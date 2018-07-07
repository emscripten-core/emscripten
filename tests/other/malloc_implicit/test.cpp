#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
int main() {
  const char *home = getenv("HOME");
  for(unsigned int i = 0; i < 5; ++i) {
    const char *curr = getenv("HOME");
    assert(curr == home);
  }
  printf("ok\n");
}

