/*
 * Copyright 2021 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <assert.h>
#include <dirent.h>
#include <emscripten/emscripten.h>
#include <emscripten/wasmfs.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <unistd.h>

#include "get_backend.h"

void print_one(int fd) {
  struct dirent d;
  int nread = getdents(fd, &d, sizeof(d));
  assert(nread != -1);
  if (nread == 0) {
    return;
  }
  printf("d.d_name = %s\n", d.d_name);
  printf("d.d_reclen = %hu\n", d.d_reclen);
  printf("d.d_type = %s\n\n",
         (d.d_type == DT_REG)   ? "regular"
         : (d.d_type == DT_DIR) ? "directory"
                                : "???");
}

void print(const char* dir) {
  struct dirent** entries;
  int nentries = scandir(dir, &entries, NULL, alphasort);
  assert(nentries != -1);
  for (int i = 0; i < nentries; i++) {
    printf("d.d_name = %s\n", entries[i]->d_name);
    printf("d.d_reclen = %hu\n", entries[i]->d_reclen);
    printf("d.d_type = %s\n\n",
           (entries[i]->d_type == DT_REG)   ? "regular"
           : (entries[i]->d_type == DT_DIR) ? "directory"
                                            : "???");
    free(entries[i]);
  }
  free(entries);
}

int main() {
  int err = wasmfs_create_directory("/root", 0777, get_backend());

  // Set up test directories.
  err = mkdir("/root/working", 0777);
  assert(err != -1);
  err = mkdir("/root/working/test", 0777);
  assert(err != -1);

  struct dirent d;

  // Try opening the directory that was just created.
  printf("------------- Reading from /root/working Directory -------------\n");
  print("/root/working");

  int fd = open("/root/working", O_RDONLY | O_DIRECTORY);

  // Try reading an invalid fd.
  errno = 0;
  getdents(-1, &d, sizeof(d));
  printf("Errno: %s\n", strerror(errno));
  assert(errno == EBADF);

  // Try passing in a size that is too small.
  // The JS file system doesn't catch this error.
  // https://man7.org/linux/man-pages/man2/getdents.2.html#ERRORS
  errno = 0;
  getdents(fd, &d, sizeof(d) - 1);
  printf("Errno: %s\n", strerror(errno));
  assert(errno == EINVAL);

  // Try to read from a file.
  int fileFd = open("/dev/stdin", O_RDONLY);
  getdents(fileFd, &d, sizeof(d));
  printf("Errno: %s\n\n", strerror(errno));
  assert(errno == ENOTDIR);
  close(fileFd);

  close(fd);

  // Try opening the dev directory and read its contents.
  printf("------------- Reading from /dev Directory -------------\n");
  print("/dev");

  // The same, but via the JS API.
  printf("------------- Reading from /dev Directory via JS -------------\n");
  EM_ASM({
    var entries = FS.readdir("/dev");
    for (var i = 0; i < entries.length; i++) {
      console.log(entries[i]);
    }
    console.log();
  });

  // Try to advance the offset of the directory.
  // Expect that '.' will be skipped.
  fd = open("/root/working", O_RDONLY | O_DIRECTORY);
  printf("/root/working file position is: %lli\n", lseek(fd, 1, SEEK_SET));
  printf(
    "------------- Reading one from /root/working Directory -------------\n");
  print_one(fd);
  close(fd);

  // Try to add a file to the /working directory.
  fd = open("/root/working/foobar", O_CREAT, S_IRGRP);
  assert(fd != -1);
  close(fd);
  printf("------------- Reading from /root/working Directory -------------\n");
  print("/root/working");
}
