#include <stdio.h>
#include <assert.h>
int main() {
  assert(1 == true);   // pass
  assert(1 == false);  // fail
  return 0;
}
