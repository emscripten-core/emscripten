/*
 * Copyright 2013 The Emscripten Authors.  All rights reserved.
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
#include <sys/stat.h>
#include <unistd.h>
#if WASMFS
#include <emscripten/wasmfs.h>
#endif

void create_file(const char *path, const char *buffer, int mode) {
  int fd = open(path, O_WRONLY | O_CREAT | O_EXCL, mode);
  assert(fd >= 0);

  int err = write(fd, buffer, sizeof(char) * strlen(buffer));
  assert(err ==  (sizeof(char) * strlen(buffer)));

  close(fd);
}

void setup() {
  create_file("file", "abcdef", 0777);
  mkdir("dir", 0777);
  mkdir("new-dir", 0777);
  create_file("dir/file", "abcdef", 0777);
  mkdir("dir/subdir", 0777);
  mkdir("dir/subdir/subsubdir", 0777);
  mkdir("dir/rename-dir", 0777);
  mkdir("dir/rename-dir/subdir", 0777);
  mkdir("dir/rename-dir/subdir/subsubdir", 0777);
  mkdir("dir-readonly", 0555);
  // TODO: Remove when chmod is implemented in WasmFS.
  mkdir("dir-readonly2", 0777);
  mkdir("dir-readonly2/somename", 0777);
  chmod("dir-readonly2", 0555);
  mkdir("dir-nonempty", 0777);
  mkdir("dir/subdir3", 0777);
  mkdir("dir/subdir3/subdir3_1", 0777);
  mkdir("dir/subdir4/", 0777);
  mkdir("dir/a/", 0777);
  mkdir("dir/b/", 0777);
  mkdir("dir/b/c", 0777);
  create_file("dir-nonempty/file", "abcdef", 0777);
}

void cleanup() {
  // We're hulk-smashing and removing original + renamed files to
  // make sure we get it all regardless of anything failing
  unlink("file");
  unlink("dir/file");
  unlink("dir/file1");
  unlink("dir/file2");
  rmdir("dir/subdir/subsubdir");
  rmdir("dir/subdir");
  rmdir("dir/subdir1");
  rmdir("dir/subdir2");
  rmdir("dir/subdir3/subdir3_1/subdir1 renamed");
  rmdir("dir/subdir3/subdir3_1");
  rmdir("dir/subdir3");
  rmdir("dir/subdir4/");
  rmdir("dir/subdir5/");
  rmdir("dir/b/c");
  rmdir("dir/b");
  rmdir("dir/rename-dir/subdir/subsubdir");
  rmdir("dir/rename-dir/subdir");
  rmdir("dir/rename-dir");
  rmdir("dir");
#ifndef WASMFS
  chmod("dir-readonly2", 0777);
#endif
  rmdir("dir-readonly2/somename");
  rmdir("dir-readonly2");
  rmdir("new-dir");
  rmdir("dir-readonly");
  unlink("dir-nonempty/file");
  rmdir("dir-nonempty");
}

void test() {
  int err;

  // Can't rename something that doesn't exist
  err = rename("noexist", "dir");
  assert(err == -1);
  assert(errno == ENOENT);

  // Can't overwrite a folder with a file
  err = rename("file", "dir");
  assert(err == -1);
  assert(errno == EISDIR);

  // Can't overwrite a file with a folder
  err = rename("dir", "file");
  assert(err == -1);
  assert(errno == ENOTDIR);

  // Can't overwrite a non-empty folder
  err = rename("dir", "dir-nonempty");
  assert(err == -1);
  assert(errno == ENOTEMPTY);

  // Can't create anything in a read-only directory
  err = rename("dir", "dir-readonly/dir");
  assert(err == -1);
  assert(errno == EACCES);

  // Can't move from a read-only directory.
  err = rename("dir-readonly2/somename", "dir");
  assert(err == -1);
  assert(errno == EACCES);

  // Can't rename a file with a new path name that is longer than WASMFS_NAME_MAX.
#ifdef WASMFS
  errno = 0;
  rename("dir",
         "000000000100000000020000000003000000000400000000050000000006000000000"
         "700000000080000000009000000000000000000010000000002000000000300000000"
         "040000000005000000000600000000070000000008000000000900000000000000000"
         "0010000000002000000000300000000040000000005123456");

  assert(errno == ENAMETOOLONG);
#endif

  // Can't use an empty path for oldpath.
  err = rename("", "test");
  assert(err == -1);
  assert(errno == ENOENT);

  // Can't use an empty path for newpath.
  err = rename("dir", "");
  assert(err == -1);
  assert(errno == ENOENT);

  // Source should not be ancestor of target.
  err = rename("dir", "dir/somename");
  assert(err == -1);
  assert(errno == EINVAL);

  // Even if it is not a direct ancestor.
  err = rename("dir", "dir/subdir/noexist");
  assert(err == -1);
  assert(errno == EINVAL);

  // After changing the current working directory and using a relative path to
  // the target, we should still detect that the source is an ancestor of the
  // target.
  chdir("dir/rename-dir/subdir");
  err = rename("/dir/rename-dir", "subsubdir");
  assert(err == -1);
  assert(errno == EINVAL);
  chdir("/");

  // Target should not be an ancestor of source
  err = rename("dir/subdir", "dir");
  assert(err == -1);
  assert(errno == ENOTEMPTY);

  err = rename("dir/subdir/subsubdir", "dir");
  assert(err == -1);
  assert(errno == ENOTEMPTY);

  // Do some valid renaming
  err = rename("dir/file", "dir/file1");
  assert(!err);
  err = rename("dir/file1", "dir/file2");
  assert(!err);
  // TODO: Remove when WASMFS implements the access syscall.
#ifndef WASMFS
  err = access("dir/file2", F_OK);
#endif
  assert(!err);
  err = rename("dir/subdir", "dir/subdir1");
  assert(!err);
  err = rename("dir/subdir1", "dir/subdir2");
  assert(!err);
#ifndef WASMFS
  err = access("dir/subdir2", F_OK);
#endif
  assert(!err);

  err = rename("dir/subdir2", "dir/subdir3/subdir3_1/subdir1 renamed");
  assert(!err);
#ifndef WASMFS
  err = access("dir/subdir3/subdir3_1/subdir1 renamed", F_OK);
#endif
  assert(!err);

  // Test that non-existant parent during rename generates the correct error
  // code.
  err = rename("dir/hicsuntdracones/empty", "dir/hicsuntdracones/renamed");
  assert(err == -1);
  assert(errno == ENOENT);

  err = rename("dir/subdir4/", "dir/subdir5/");
  assert(!err);

  // Test renaming the same directory.
  err = rename("dir/file2", "dir/file2");
  assert(!err);

  // Test renaming a directory with a subdirectory with a common ancestor.
  err = rename("dir/a", "dir/b/c");
  assert(!err);

  // In Linux and WasmFS, renaming the root directory should return EBUSY.
  // In the JS file system it reports EINVAL.
  err = rename("/", "dir/file2");
  assert(err == -1);
#ifdef WASMFS
  assert(errno == EBUSY);
#else
  assert(errno == EINVAL);
#endif

  // Test renaming a directory with root.
  err = rename("dir/file2", "/");
  assert(err == -1);
#ifdef WASMFS
  assert(errno == EBUSY);
#else
  assert(errno == ENOTEMPTY);
#endif

#ifdef WASMFS
  // Files cannot be renamed between backends.

  // Create a new backend (and make sure it is different).
  backend_t js_backend = wasmfs_create_js_file_backend();
  assert(js_backend != wasmfs_get_backend_by_path("/"));

  // Create a directory and a file.
  err = wasmfs_create_directory("/js-dir", 0777, js_backend);
  assert(err == 0);
  int fd = wasmfs_create_file("/js-dir/js-file", 0777, js_backend);
  assert(fd >= 0);

  // Try to move it.
  err = rename("/js-dir/js-file", "/moved-js-file");
  assert(err == -1);
  assert(errno == EXDEV);
#endif

  puts("success");
}

int main() {
  atexit(cleanup);
  signal(SIGABRT, cleanup);
  setup();
  test();
  return EXIT_SUCCESS;
}
