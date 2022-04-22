#include <assert.h>
#include <stdio.h>

int jscall(uint64_t arg);

int main() {
  int rtn = jscall(42);
  printf("%d\n", rtn);
  assert(rtn == 84);

  // TODO(sbc): Test edge cases such as i64 values that overflow int32 and
  // int53
  return 0;
}
