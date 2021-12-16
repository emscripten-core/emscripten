#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>

int main() {
  // Opening either the target or source should work
  assert(open("filename", O_RDONLY) >= 0);
  assert(open("linkname", O_RDONLY) >= 0);

  // But adding O_NOFOLLOW should cause the opening on the link to
  // fail with ELOOP
  assert(open("linkname", O_NOFOLLOW | O_RDONLY) < 0);
  assert(errno == ELOOP);

  assert(open("filename", O_NOFOLLOW | O_RDONLY) >= 0);

  printf("success\n");
  return 0;
}
