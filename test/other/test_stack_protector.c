#include <alloca.h>
#include <memory.h>
#include <string.h>
#include <stdio.h>

char mystring[] = "Hello world";

int main() {
  char buf[10];
  // This is not valid since mystring is larger than `buf`
  strcpy(buf, mystring);
  printf("buf: %s\n", buf);
  return 0;
}
