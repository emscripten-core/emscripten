/*
 * Copyright 2011 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

char nonexistent_name[] = "noexist-##";

void create_file(const char *path, const char *buffer, int mode) {
  int fd = open(path, O_WRONLY | O_CREAT | O_EXCL, mode);
  assert(fd >= 0);
  assert(!errno);

  int err = write(fd, buffer, sizeof(char) * strlen(buffer));
  assert(err ==  (sizeof(char) * strlen(buffer)));
  assert(!errno);

  close(fd);
  assert(!errno);
}

void setup() {
  create_file("test-file", "abcdef", 0777);
  mkdir("test-folder", 0777);
  assert(!errno);
}

void cleanup() {
  unlink("test-file");
  rmdir("test-folder");
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 16; j++) {
      nonexistent_name[8] = 'a' + i;
      nonexistent_name[9] = 'a' + j;
      unlink(nonexistent_name);
    }
  }
  errno = 0;
  unlink("creat-me");
  assert(!errno);
}

void test() {
  struct stat s;
  int modes[] = {O_RDONLY, O_WRONLY, O_RDWR};

  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 16; j++) {
      int flags = modes[i];
      if (j & 0x1) flags |= O_CREAT;
      if (j & 0x2) flags |= O_EXCL;
      if (j & 0x4) flags |= O_TRUNC;
      if (j & 0x8) flags |= O_APPEND;

      printf("EXISTING FILE %d,%d\n", i, j);
      int success = open("test-file", flags, 0777) != -1;
      printf("success: %d\n", success);
      printf("errno: %d\n", errno);
      if ((flags & O_CREAT) && (flags & O_EXCL)) {
        assert(!success);
        assert(errno == EEXIST);
      } else {
        assert(success);
        assert(errno == 0);
      }
      errno = 0;

      int ret = stat("test-file", &s);
      assert(ret == 0);
      assert(errno == 0);
      printf("st_mode: 0%o\n", s.st_mode & 037777777000);
      assert((s.st_mode & 037777777000) == 0100000);
      memset(&s, 0, sizeof s);
      printf("\n");
      errno = 0;

      printf("EXISTING FOLDER %d,%d\n", i, j);
      success = open("test-folder", flags, 0777) != -1;
      printf("success: %d\n", success);
      printf("errno: %d\n", errno);
      if ((flags & O_CREAT) && (flags & O_EXCL)) {
        assert(!success);
        assert(errno == EEXIST);
      } else if ((flags & O_TRUNC) || i != 0 /*mode != O_RDONLY*/) {
        assert(!success);
        assert(errno == EISDIR);
      } else {
        assert(success);
        assert(errno == 0);
      }
      errno = 0;

      ret = stat("test-folder", &s);
      assert(ret == 0);
      assert(errno == 0);
      printf("st_mode: 0%o\n", s.st_mode & 037777777000);
      assert((s.st_mode & 037777777000) == 040000);
      memset(&s, 0, sizeof s);
      printf("\n");
      errno = 0;

      nonexistent_name[8] = 'a' + i;
      nonexistent_name[9] = 'a' + j;
      printf("NON-EXISTING %d,%d\n", i, j);
      success = open(nonexistent_name, flags, 0777) != -1;
      printf("success: %d\n", success);
      printf("errno: %d\n", errno);
      if ((flags & O_CREAT)) {
        assert(success);
        assert(errno == 0);
      } else {
        assert(!success);
        assert(errno == ENOENT);
      }

      ret = stat(nonexistent_name, &s);
      printf("st_mode: 0%o\n", s.st_mode & 037777777000);

      if ((flags & O_CREAT)) {
        assert(ret == 0);
        assert((s.st_mode & 037777777000) == 0100000);
      } else {
        assert(ret != 0);
        assert((s.st_mode & 037777777000) == 0);
      }

      memset(&s, 0, sizeof s);
      printf("\n");
      errno = 0;
    }
  }

  printf("CREAT\n");
  printf("success: %d\n", creat("creat-me", 0777) != -1);
  printf("errno: %d\n", errno);
  errno = 0;
}

int main() {
  atexit(cleanup);
  signal(SIGABRT, cleanup);
  setup();
  test();
#ifdef REPORT_RESULT
  REPORT_RESULT(0);
#endif
  return EXIT_SUCCESS;
}
