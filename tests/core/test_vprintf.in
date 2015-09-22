#include <stdio.h>
#include <stdarg.h>

void print(const char* format, ...) {
  va_list args;
  va_start(args, format);
  vprintf(format, args);
  va_end(args);
}

int main() {
  print("Call with %d variable argument.\n", 1);
  print("Call with %d variable %s.\n", 2, "arguments");

  return 0;
}
