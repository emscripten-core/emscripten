#include <sys/types.h>

extern "C" ssize_t write(int fd, const void *buf, size_t count);

int main() {
  write(1, "hello, world!", 5);
  write(1, "\n", 1);
}

