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
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

static void create_file(const char *path, const char *buffer, int mode) {
  printf("creating: %s\n", path);
  int fd = open(path, O_WRONLY | O_CREAT | O_EXCL, mode);
  assert(fd >= 0);

  int err = write(fd, buffer, sizeof(char) * strlen(buffer));
  assert(err ==  (sizeof(char) * strlen(buffer)));

  close(fd);
}

void setup() {
  mkdir("working", 0777);
#ifdef __EMSCRIPTEN__
  EM_ASM(
#if NODEFS
    FS.mount(NODEFS, { root: '.' }, 'working');
#endif
  );
#endif
  chdir("working");
  create_file("file", "test", 0777);
  create_file("file1", "test", 0777);
  // create the file as readable and then use chmod
  create_file("file-readonly", "test", 0777);
#ifndef NO_SYMLINK
  symlink("file1", "file1-link");
#endif
  mkdir("dir-empty", 0777);
#ifndef NO_SYMLINK
  symlink("dir-empty", "dir-empty-link");
#endif
  mkdir("dir-readonly", 0777);
  create_file("dir-readonly/anotherfile", "test", 0777);
  mkdir("dir-readonly/anotherdir", 0777);
  chmod("dir-readonly", 0555);
  chmod("file-readonly", 0555);
  mkdir("dir-full", 0777);
  create_file("dir-full/anotherfile", "test", 0777);
}

void cleanup() {
  unlink("file");
  unlink("file1");
#ifndef NO_SYMLINK
  unlink("file1-link");
#endif
  rmdir("dir-empty");
#ifndef NO_SYMLINK
  unlink("dir-empty-link");
#endif
  chmod("dir-readonly", 0777);
  chmod("file-readonly", 0777);
  unlink("file-readonly");
  unlink("dir-readonly/anotherfile");
  rmdir("dir-readonly/anotherdir");
  rmdir("dir-readonly");
  unlink("dir-full/anotherfile");
  rmdir("dir-full");
}

void test() {
  int err;
  char buffer[512];

  //
  // test unlink
  //
  err = unlink("noexist");
  assert(err == -1);
  assert(errno == ENOENT);

  // Test non-existent parent
  err = unlink("noexist/foo");
  assert(err == -1);
  assert(errno == ENOENT);

  // Test empty pathname
  err = unlink("");
  assert(err == -1);
  printf("%s\n", strerror(errno));
  assert(errno == ENOENT);

  err = unlink("dir-readonly");
  assert(err == -1);

  // emscripten uses 'musl' what is an implementation of the standard library for Linux-based systems
#if defined(__linux__) || defined(__EMSCRIPTEN__)
  // Here errno is supposed to be EISDIR, but it is EPERM for NODERAWFS on macOS.
  // See issue #6121.
  assert(errno == EISDIR || errno == EPERM);
#else
  assert(errno == EPERM);
#endif

#ifndef SKIP_ACCESS_TESTS
  err = unlink("dir-readonly/anotherfile");
  printf("err: %d %d\n", err, errno);
  assert(err == -1);
  assert(errno == EACCES);
#endif

#ifndef NO_SYMLINK
  // try unlinking the symlink first to make sure
  // we don't follow the link
  err = unlink("file1-link");
  assert(!err);
#endif
  err = access("file1", F_OK);
  assert(!err);
#if !defined(NO_SYMLINK)
  err = access("file1-link", F_OK);
  assert(err == -1);
#endif

  err = unlink("file");
  assert(!err);
  err = access("file", F_OK);
  assert(err == -1);

  // Should be able to delete a read-only file.
  err = unlink("file-readonly");
  assert(!err);

  //
  // test rmdir
  //
  err = rmdir("noexist");
  assert(err == -1);
  assert(errno == ENOENT);

  err = rmdir("file1");
  assert(err == -1);
  assert(errno == ENOTDIR);

#ifndef SKIP_ACCESS_TESTS
  err = rmdir("dir-readonly/anotherdir");
  assert(err == -1);
  assert(errno == EACCES);
#endif

  err = rmdir("dir-full");
  assert(err == -1);
  assert(errno == ENOTEMPTY);

  // test removing the cwd / root. The result isn't specified by
  // POSIX, but Linux seems to set EBUSY in both cases.
  // Update: Removing cwd on Linux does not return EBUSY.
  // WASMFS behaviour will match the native FS.
#ifndef __APPLE__
  getcwd(buffer, sizeof(buffer));
  err = rmdir(buffer);
  assert(err == -1);
#if defined(NODERAWFS) || defined(WASMFS)
  assert(errno == ENOTEMPTY);
#else
  assert(errno == EBUSY);
#endif
#endif
  err = rmdir("/");
  assert(err == -1);
#ifdef __APPLE__
  assert(errno == EISDIR);
#else
  // errno is EISDIR for NODERAWFS on macOS. See issue #6121.
  assert(errno == EBUSY || errno == EISDIR);
#endif

#ifndef NO_SYMLINK
  err = rmdir("dir-empty-link");
  assert(err == -1);
  assert(errno == ENOTDIR);
#endif

  err = rmdir("dir-empty");
  assert(!err);
  err = access("dir-empty", F_OK);
  assert(err == -1);

  puts("success");
}

int main() {
  atexit(cleanup);
  signal(SIGABRT, cleanup);
  setup();
  test();

  return EXIT_SUCCESS;
}
