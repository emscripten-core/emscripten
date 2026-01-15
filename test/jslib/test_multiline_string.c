#include <stdio.h>
#include <stdlib.h>

char *test_multiline_string(void);

int main() {
  char *str = test_multiline_string();
  printf("%s\n", str);
  free(str);
}
