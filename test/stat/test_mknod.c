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
#include <string.h>
#include <unistd.h>
#include <utime.h>
#include <sys/stat.h>
#include <sys/types.h>

void setup() {
  mkdir("folder-readonly", 0555);
}

void test() {
  int err;
  struct stat s;

  //
  // mknod
  // mknod is _extremely_ unportable for anything other
  // than a FIFO. so, the tests are disabled when running
  // natively as they'd be utterly inconsistent.
  //
#ifdef __EMSCRIPTEN__

  // mknod a folder
  err = mknod("mknod-folder", S_IFDIR | 0777, 0);
  assert(err);
  assert(errno == EINVAL);

  // mknod fifo
  err = mknod("mknod-fifo", S_IFIFO | 0777, 0);
  assert(err);
  assert(errno == EPERM);

  // mknod a file
  err = mknod("mknod-file", S_IFREG | 0777, 0);
  assert(!err);
  memset(&s, 0, sizeof s);
  stat("mknod-file", &s);
  assert(S_ISREG(s.st_mode));

#ifndef WASMFS // TODO: wasmfs support for special devices
  // mknod a character device
  err = mknod("mknod-device", S_IFCHR | 0777, 123);
  assert(!err);
  memset(&s, 0, sizeof s);
  stat("mknod-device", &s);
  assert(S_ISCHR(s.st_mode));
#endif // WASMFS

  assert(mknod("", 0777, 0) == -1);
  assert(errno == ENOENT);
#endif

  //
  // mkdir
  //
  // can't mkdir in a readonly dir
  err = mkdir("folder-readonly/subfolder", 0777);
  assert(err);
  assert(errno == EACCES);

  // regular creation
  err = mkdir("folder", 0777);
  assert(!err);
  memset(&s, 0, sizeof s);
  stat("folder", &s);
  assert(S_ISDIR(s.st_mode));

  // try to re-create the same folder
  err = mkdir("folder", 0777);
  assert(err);
  assert(errno == EEXIST);

  puts("success");
}

int main() {
  setup();
  test();
  return 0;
}
