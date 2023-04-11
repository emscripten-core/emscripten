#include <assert.h>
#include <string.h>

const char *hello = "hello";
const char *world = "world";

int main() {
  char buffer[100];
  memset(buffer, 'a', 100);
  memcpy(buffer, hello, strlen(hello) + 1);
  assert(strcmp(buffer, hello) == 0);
  return 0;
}
