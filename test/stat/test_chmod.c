/*
 * Copyright 2013 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <utime.h>
#include <sys/stat.h>
#include <sys/types.h>

void create_file(const char *path, const char *buffer, int mode) {
  int fd = open(path, O_WRONLY | O_CREAT | O_EXCL, mode);
  assert(fd >= 0);

  int err = write(fd, buffer, sizeof(char) * strlen(buffer));
  assert(err ==  (sizeof(char) * strlen(buffer)));

  close(fd);
}

void setup() {
  create_file("file", "abcdef", 0777);
  create_file("otherfile", "abcdef", 0777);
  symlink("file", "file-link");
  mkdir("folder", 0777);
}

void cleanup() {
  unlink("file-link");
  unlink("file");
  unlink("otherfile");
  rmdir("folder");
}

void test() {
  int err;
  int lastctime;
  int lastmtime;
  struct stat s;

  //
  // chmod a file
  //
  // get the current ctime for the file
  memset(&s, 0, sizeof s);
  stat("file", &s);
  lastctime = s.st_ctime;
  lastmtime = s.st_mtime;
  sleep(1);

  // do the actual chmod
  err = chmod("file", S_IWUSR);
  assert(!err);

  memset(&s, 0, sizeof s);
  stat("file", &s);
  assert(s.st_mode == (S_IWUSR | S_IFREG));
  assert(s.st_ctime != lastctime);
  assert(s.st_mtime == lastmtime);

  //
  // fchmod a file
  //
  lastctime = s.st_ctime;
  lastmtime = s.st_mtime;
  sleep(1);

  err = fchmod(open("file", O_WRONLY), S_IXUSR);
  assert(!err);

  memset(&s, 0, sizeof s);
  stat("file", &s);
  assert(s.st_mode == (S_IXUSR | S_IFREG));
  assert(s.st_ctime != lastctime);
  assert(s.st_mtime == lastmtime);

  //
  // fchmodat a file
  //
  lastctime = s.st_ctime;
  lastmtime = s.st_mtime;
  sleep(1);
  err = fchmodat(AT_FDCWD, "otherfile", S_IXUSR, 0);
  assert(!err);

  assert(symlink("otherfile", "link") == 0);
  err = fchmodat(AT_FDCWD, "link", S_IXGRP, AT_SYMLINK_NOFOLLOW);
#if defined(NODEFS) || defined(NODERAWFS)
  assert(err == -1);
  assert(errno == ENOTSUP);
#else
  assert(err == 0);
#endif

  memset(&s, 0, sizeof s);
  stat("otherfile", &s);
  assert(s.st_mode == (S_IXUSR | S_IFREG));
  assert(s.st_ctime != lastctime);
  assert(s.st_mtime == lastmtime);

  //
  // chmod a folder
  //
  // get the current ctime for the folder
  memset(&s, 0, sizeof s);
  stat("folder", &s);
  lastctime = s.st_ctime;
  lastmtime = s.st_mtime;
  sleep(1);

  // do the actual chmod
  err = chmod("folder", S_IWUSR | S_IXUSR);
  assert(!err);
  memset(&s, 0, sizeof s);
  stat("folder", &s);
  assert(s.st_mode == (S_IWUSR | S_IXUSR | S_IFDIR));
  assert(s.st_ctime != lastctime);
  assert(s.st_mtime == lastmtime);

#ifndef WASMFS // TODO https://github.com/emscripten-core/emscripten/issues/15948
  lstat("file-link", &s);
  int link_mode = s.st_mode;
  assert((link_mode & 0777) != S_IRUSR);

  //
  // chmod a symlink's target
  //
  err = chmod("file-link", S_IRUSR);
  assert(!err);

  // make sure the file it references changed
  stat("file-link", &s);
  assert(s.st_mode == (S_IRUSR | S_IFREG));

  // but the link didn't
  lstat("file-link", &s);
  assert(s.st_mode == link_mode);

  // TODO: lchmod is not supported in NODEFS but it chmods the link target
  // instead of raising an error. Will fix in a follow up to #23058.
#ifndef NODEFS
  //
  // chmod the actual symlink
  //
  err = lchmod("file-link", S_IRUSR | S_IXUSR);
  // On linux lchmod is not supported so allow that here.
  assert(!err || errno == ENOTSUP);
  printf("lchmod -> %s\n", strerror(errno));

  // make sure the file it references didn't change
  stat("file-link", &s);
  assert(s.st_mode == (S_IRUSR | S_IFREG));
#endif
#endif // WASMFS

  assert(stat("", &s) == -1);
  assert(errno == ENOENT);
  assert(chmod("", 0777) == -1);
  assert(errno == ENOENT);
  assert(chown("", 1000, 1000) == -1);
  assert(errno == ENOENT);

  puts("success");
}

int main() {
  setup();
  test();
  return EXIT_SUCCESS;
}
