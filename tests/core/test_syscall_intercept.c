#include <assert.h>
#include <string.h>
#include <stdio.h>

// Import the syscall under a separate name
__attribute__((import_module("env"), import_name("__syscall_getcwd")))
int __orig_getcwd(intptr_t buf, size_t size);


int __syscall_getcwd(intptr_t buf, size_t size) {
  printf("__syscall_getcwd intercepted\n");
  return __orig_getcwd(buf, size);
}

int main() {
  char cwd[1024];
  int rtn = __syscall_getcwd((intptr_t)cwd, sizeof(cwd));
  assert(rtn > 0);
  printf("cwd = %s\n", cwd);
  return 0;
}
