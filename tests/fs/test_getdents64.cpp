// Copyright 2016 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/syscall.h>

#define BUF_SIZE (sizeof(dirent)*2)

int main(int argc, char *argv[])
{
  int fd = open(".", O_RDONLY | O_DIRECTORY);
  assert(fd > 0);

  printf("sizeof(dirent): %zu, sizeof(buffer): %zu\n", sizeof(dirent), BUF_SIZE);

  bool first = true;

  for(;;)
  {
    char buf[BUF_SIZE];
    int nread = getdents(fd, (dirent*)buf, BUF_SIZE);
    assert(nread != -1);
    if (nread == 0)
      break;

    // In this test, we provide enough space to read two dirent entries at a time.
    // Test that on the first iteration of the loop we get exactly two entries. (there's at least "." and ".." in each directory)
    assert(nread == BUF_SIZE || !first);
    first = false;

    printf("--------------- nread=%d ---------------\n", nread);
    printf("i-node#  file type  d_reclen  d_off   d_name\n");
    int bpos = 0;
    while(bpos < nread)
    {
      dirent *d = (dirent *)(buf + bpos);
      printf("%8ld  ", (long)d->d_ino);
      char d_type = *(buf + bpos + d->d_reclen - 1);
      printf("%-10s ", (d_type == DT_REG) ?  "regular" :
                       (d_type == DT_DIR) ?  "directory" :
                       (d_type == DT_FIFO) ? "FIFO" :
                       (d_type == DT_SOCK) ? "socket" :
                       (d_type == DT_LNK) ?  "symlink" :
                       (d_type == DT_BLK) ?  "block dev" :
                       (d_type == DT_CHR) ?  "char dev" : "???");
      printf("%4d %10lld  %s\n", d->d_reclen, (long long) d->d_off, d->d_name);
      bpos += d->d_reclen;
    }
  }
}
