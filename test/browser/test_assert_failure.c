#include <assert.h>
#include <stdbool.h>

int main() {
  assert(false && "this is a test");
  return 0;
}
