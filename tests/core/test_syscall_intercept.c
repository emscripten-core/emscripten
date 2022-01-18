#include <assert.h>
#include <string.h>
#include <stdio.h>

// Import the syscall under a separate name
__attribute__((import_module("env"), import_name("__syscall_getcwd")))
long __orig_getcwd(long buf, long size);


long __syscall_getcwd(long buf, long size) {
  printf("__syscall_getcwd intercepted\n");
  return __orig_getcwd(buf, size);
}

int main() {
  char cwd[1024];
  int rtn = __syscall_getcwd((long)cwd, sizeof(cwd));
  assert(rtn > 0);
  printf("cwd = %s\n", cwd);
  return 0;
}
