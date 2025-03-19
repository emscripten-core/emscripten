#include <assert.h>
#include <stdio.h>
#include <sys/vfs.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>   // for strerror
#include <errno.h>    // for errno#include <filesystem>

#define DEFAULT_BLOCKS 1000000

int main() {
  struct statfs buf;
  const char *expected_blocks_str = getenv("EXPECTED_BLOCKS");
  long expected_blocks = expected_blocks_str ? atol(expected_blocks_str) : DEFAULT_BLOCKS;

  int fstatfs_rtn = statfs("/nodefs", &buf);
  printf("f_type: %ld\n", buf.f_type);
  assert(fstatfs_rtn == 0);
  printf("f_blocks: %d\n", buf.f_blocks);
  printf("expected_blocks: %ld\n", expected_blocks);
#if NODEFS || NODERAWFS
  assert(buf.f_blocks == expected_blocks);
#else
  assert(buf.f_blocks == DEFAULT_BLOCKS);
#endif

  fstatfs_rtn = fstatfs(STDOUT_FILENO, &buf);
  printf("f_type: %ld\n", buf.f_type);
  assert(fstatfs_rtn == 0);
  printf("f_blocks: %d\n", buf.f_blocks);
  assert(buf.f_blocks == DEFAULT_BLOCKS);

  int f = open("file", O_RDWR | O_CREAT);
  fstatfs_rtn = fstatfs(f, &buf);
  printf("f_type: %ld\n", buf.f_type);
  printf("f_blocks: %d\n", buf.f_blocks);
#if NODEFS || NODERAWFS
  assert(fstatfs_rtn == 0);
  assert(buf.f_blocks != 0);
#else
  assert(fstatfs_rtn == 0);
  assert(buf.f_blocks == DEFAULT_BLOCKS);
#endif

  assert(statfs("file", &buf) == 0);
  printf("f_type: %ld\n", buf.f_type);
  printf("f_blocks: %d\n", buf.f_blocks);
#if NODEFS || NODERAWFS
  assert(buf.f_blocks != 0);
#else
  assert(buf.f_blocks == DEFAULT_BLOCKS);
#endif

  return 0;
}
