#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <emscripten.h>
#include <stdio.h>

class random_device {
  int __f_;

public:
  // constructors
  explicit random_device() {
    __f_ = open("/dev/urandom", O_RDONLY);
    assert(__f_ >= 0);
  }

  ~random_device() {
    close(__f_);
  }

  // generating functions
  unsigned operator()();
};

unsigned random_device::operator()() {
  unsigned r;
  size_t n = sizeof(r);
  char* p = reinterpret_cast<char*>(&r);
  while (n > 0) {
    ssize_t s = read(__f_, p, 1);
    assert(s > 0);
    if (s == -1) {
      assert(errno == EINTR);
      continue;
    }
    n -= static_cast<size_t>(s);
    p += static_cast<size_t>(s);
  }
  return r;
}

int main() {
  int total = 0;
  for (int i = 0; i < ITERATIONS; i++) {
    // printf causes proxying
    printf("%d %d\n", i, total);
    for (int j = 0; j < 1024; j++) {
      // allocation uses a mutex
      auto* rd = new random_device();
      // reading data causes proxying
      for (int k = 0; k < 4; k++) {
        total += (*rd)();
      }
      // make sure the optimizer doesn't remove the allocation
      EM_ASM({ out("iter") }, rd);
      delete rd;
    }
  }
  printf("done: %d", total);
  return 0;
}

