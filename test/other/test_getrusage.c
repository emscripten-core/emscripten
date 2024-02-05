#include <sys/time.h>
#include <sys/resource.h>

int main() {
  struct rusage u;
  getrusage(0, &u);
  return 0;
}
