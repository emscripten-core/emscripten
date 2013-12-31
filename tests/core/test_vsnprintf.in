#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>

void printy(const char *f, ...) {
  char buffer[256];
  va_list args;
  va_start(args, f);
  vsnprintf(buffer, 256, f, args);
  puts(buffer);
  va_end(args);
}

int main(int argc, char **argv) {
  int64_t x = argc - 1;
  int64_t y = argc - 1 + 0x400000;
  if (x % 3 == 2) y *= 2;

  printy("0x%llx_0x%llx", x, y);
  printy("0x%llx_0x%llx", x, x);
  printy("0x%llx_0x%llx", y, x);
  printy("0x%llx_0x%llx", y, y);

  {
    uint64_t A = 0x800000;
    uint64_t B = 0x800000000000ULL;
    printy("0x%llx_0x%llx", A, B);
  }
  {
    uint64_t A = 0x800;
    uint64_t B = 0x12340000000000ULL;
    printy("0x%llx_0x%llx", A, B);
  }
  {
    uint64_t A = 0x000009182746756;
    uint64_t B = 0x192837465631ACBDULL;
    printy("0x%llx_0x%llx", A, B);
  }

  return 0;
}
