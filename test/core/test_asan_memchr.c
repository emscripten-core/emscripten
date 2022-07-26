#include <string.h>
#include <stdint.h>

int main() {
  return (int)(intptr_t)memchr("hello", 'z', 7);
}
