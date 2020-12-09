/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <errno.h>
#include <sys/statvfs.h>

int main() {
  struct statvfs s;

  printf("result: %d\n", statvfs("/test", &s));
  printf("errno: %d\n", errno);

  printf("f_bsize: %lu\n", s.f_bsize);
  printf("f_frsize: %lu\n", s.f_frsize);
  printf("f_blocks: %u\n", s.f_blocks);
  printf("f_bfree: %u\n", s.f_bfree);
  printf("f_bavail: %u\n", s.f_bavail);
  printf("f_files: %d\n", s.f_files > 5);
  printf("f_ffree: %u\n", s.f_ffree);
  printf("f_favail: %u\n", s.f_favail);
  printf("f_fsid: %lu\n", s.f_fsid);
  printf("f_flag: %lu\n", s.f_flag);
  printf("f_namemax: %lu\n", s.f_namemax);

  return 0;
}
