#include <assert.h>
#include <stdio.h>
#include <sys/vfs.h>
#include <unistd.h>
#include <fcntl.h>

int main() {
  struct statfs buf;

  int rtn;
  int fstatfs_rtn = fstatfs(STDOUT_FILENO, &buf);
  printf("f_type: %ld\n", buf.f_type);
  assert(fstatfs_rtn == 0);
  printf("f_blocks: %d\n", buf.f_blocks);
  assert(buf.f_blocks == 1000000);

  int f = open("file", O_RDWR | O_CREAT);
  fstatfs_rtn = fstatfs(f, &buf);
  printf("f_type: %ld\n", buf.f_type);
  printf("f_blocks: %d\n", buf.f_blocks);
#if NODEFS
  assert(fstatfs_rtn == 0);
  assert(buf.f_blocks != 0);
  // 2048000000 is a hardcoded value for NODEFS blocks.
  assert(buf.f_blocks != 2048000000);
#else
  assert(fstatfs_rtn == 0);
  assert(buf.f_blocks == 1000000);
#endif

  assert(statfs("file", &buf) == 0);
  printf("f_type: %ld\n", buf.f_type);
  printf("f_blocks: %d\n", buf.f_blocks);
#if NODEFS
  assert(buf.f_blocks != 0);
  assert(buf.f_blocks != 2048000000);
#else
  assert(buf.f_blocks == 1000000);
#endif

  return 0;
}
