#include <stdio.h>
#include <string.h>

char* get_string(char* input) {
  static char buffer[256];
  strcpy(buffer, "hello ");
  strcat(buffer, input);
  return buffer;
}

void test_ptr_handling(char* (*fnptr)(char*), char* arg);

int main() {
  test_ptr_handling(&get_string, "world");
  return 0;
}
