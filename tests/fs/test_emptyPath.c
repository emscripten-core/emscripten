#include <stdio.h>

int main() {
  FILE* f1 = fopen("s", "r");
  if (f1 == NULL) {
    printf("file 's' not found!\n");
  }

  FILE* f2 = fopen("", "r");
  if (f2 == NULL) {
    printf("file '' not found!\n");
  }
  return 0;
}
