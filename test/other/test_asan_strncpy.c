#include <string.h>

int main() {
  char from[] = "x";
  char to[4];
  strncpy(to, from, sizeof(to));
  return 0;
}
