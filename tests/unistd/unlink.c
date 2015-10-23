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

  err = unlink("dir-readonly");
  assert(err == -1);
#ifdef __linux__
  assert(errno == EISDIR);
#else
  assert(errno == EPERM);
#endif

  err = unlink("dir-readonly/anotherfile");
  assert(err == -1);
  assert(errno == EACCES);

#ifndef NO_SYMLINK
  // try unlinking the symlink first to make sure
  // we don't follow the link
  err = unlink("file1-link");
  assert(!err);
#endif
  err = access("file1", F_OK);
  assert(!err);
#ifndef NO_SYMLINK
  err = access("file1-link", F_OK);
  assert(err == -1);
#endif

  err = unlink("file");
  assert(!err);
  err = access("file", F_OK);
  assert(err == -1);

  //
  // test rmdir
  //
  err = rmdir("noexist");
  assert(err == -1);
  assert(errno == ENOENT);

  err = rmdir("file1");
  assert(err == -1);
  assert(errno == ENOTDIR);

  err = rmdir("dir-readonly/anotherdir");
  assert(err == -1);
  assert(errno == EACCES);

  err = rmdir("dir-full");
  assert(err == -1);
  assert(errno == ENOTEMPTY);

  // test removing the cwd / root. The result isn't specified by
  // POSIX, but Linux seems to set EBUSY in both cases.
#ifndef __APPLE__
  getcwd(buffer, sizeof(buffer));
  err = rmdir(buffer);
  assert(err == -1);
  assert(errno == EBUSY);
#endif
  err = rmdir("/");
  assert(err == -1);
#ifdef __APPLE__
  assert(errno == EISDIR);
#else
  assert(errno == EBUSY);
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
