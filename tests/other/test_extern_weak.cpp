#include <assert.h>
#include <stdio.h>

extern int __attribute__((weak)) externWeak();

int main() {
#ifdef LINKABLE
  printf("in linkable code, we don't handle this properly quite yet - we create a function pointer, but do not have a way to change that at runtime\n");
  assert(externWeak != nullptr);
#else
  printf("all linking is done, we should have set to null this thing that was not linked in\n");
  assert(externWeak == nullptr);
#endif
  printf("ok.\n");
}

