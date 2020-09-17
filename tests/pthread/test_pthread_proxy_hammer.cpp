#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <emscripten.h>
#include <stdio.h>

class random_device
{
    int __f_;
public:
    // constructors
    explicit random_device();
    ~random_device();

    // generating functions
    unsigned operator()();
};

random_device::random_device()
    : __f_(open("/dev/urandom", O_RDONLY))
{
    if (__f_ < 0) abort();
}

random_device::~random_device()
{
    close(__f_);
}

unsigned
random_device::operator()()
{
    unsigned r;
    size_t n = sizeof(r);
    char* p = reinterpret_cast<char*>(&r);
    while (n > 0)
    {
        ssize_t s = read(__f_, p, 1);
        if (s == 0) abort();
        if (s == -1)
        {
            if (errno != EINTR) abort();
            continue;
        }
        n -= static_cast<size_t>(s);
        p += static_cast<size_t>(s);
    }
    return r;
}

void* ptr;

int main() {
  int total = 0;
  for (int i = 0; i < 128; i++) {
    EM_ASM({ out($0, $1) }, i, total);
    for (int j = 0; j < 128; j++) {
      auto* rd = new random_device;
      total += (*rd)();
      ptr = (void*)rd; // make sure the optimizer doesn't remove the allocation
      delete rd;
    }
  }
  EM_ASM({ out("done") });
#ifdef REPORT_RESULT
	REPORT_RESULT(0);
#endif
  return 0;
}

