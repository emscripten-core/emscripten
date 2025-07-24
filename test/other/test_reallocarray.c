#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main(void) {
  char* p = calloc(4, sizeof(char));
  assert(p);

  strcpy(p, "foo");
  puts(p);

  p = reallocarray(p, 32, sizeof(char));
  assert(p);

  strcpy(p, "foobar");
  puts(p);

  free(p);
  return 0;
}
