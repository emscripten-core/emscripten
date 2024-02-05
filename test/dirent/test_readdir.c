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
#include <sys/stat.h>

#define CHECK(cond) if (!(cond)) { printf("errno: %s\n", strerror(errno)); assert(cond); }

static void create_file(const char *path, const char *buffer, int mode) {
  int fd = open(path, O_WRONLY | O_CREAT | O_EXCL, mode);
  CHECK(fd >= 0);

  int err = write(fd, buffer, sizeof(char) * strlen(buffer));
  assert(err ==  (sizeof(char) * strlen(buffer)));

  close(fd);
}

void setup() {
  int err;
  err = mkdir("testtmp", 0777);  // can't call it tmp, that already exists
  CHECK(!err);
  chdir("testtmp");
  err = mkdir("nocanread", 0111);
  CHECK(!err);
  err = mkdir("foobar", 0777);
  CHECK(!err);
  create_file("foobar/file.txt", "ride into the danger zone", 0666);
}

void cleanup() {
  rmdir("nocanread");
  unlink("foobar/file.txt");
  rmdir("foobar");
  chdir("..");
  rmdir("testtmp");
}

void test() {
  int err;
  long loc, loc2;
  DIR *dir;
  struct dirent *ent;
  struct dirent ent_r;
  struct dirent *result;
  int i;

  // check bad opendir input
  dir = opendir("noexist");
  assert(!dir);
  assert(errno == ENOENT);
// NODERAWFS tests run as root, and the root user can opendir any directory
#ifndef NODERAWFS
  dir = opendir("nocanread");
  assert(!dir);
  assert(errno == EACCES);
#endif
  dir = opendir("foobar/file.txt");
  assert(!dir);
  assert(errno == ENOTDIR);

  // check bad readdir input
  //dir = opendir("foobar");
  //closedir(dir);
  //ent = readdir(dir);
  //assert(!ent);
  // XXX musl doesn't have enough error handling for this: assert(errno == EBADF);

  // check bad readdir_r input
  //dir = opendir("foobar");
  //closedir(dir);
  //err = readdir_r(dir, NULL, &result);
  // XXX musl doesn't have enough error handling for this: assert(err == EBADF);

  //
  // do a normal read with readdir
  //
  dir = opendir("foobar");
  assert(dir);
  int seen[3] = { 0, 0, 0 };
  for (i = 0; i < 3; i++) {
    errno = 0;
    ent = readdir(dir);
    if (ent) {
      fprintf(stderr, "%d file: %s (%d : %lu)\n", i, ent->d_name, ent->d_reclen, sizeof(*ent));
    } else {
      fprintf(stderr, "ent: %p, errno: %d\n", ent, errno);
      assert(ent);
    }
    assert(ent->d_reclen == sizeof(*ent));
    if (!seen[0] && !strcmp(ent->d_name, ".")) {
      assert(ent->d_type & DT_DIR);
      seen[0] = 1;
      continue;
    }
    if (!seen[1] && !strcmp(ent->d_name, "..")) {
      assert(ent->d_type & DT_DIR);
      seen[1] = 1;
      continue;
    }
    if (!seen[2] && !strcmp(ent->d_name, "file.txt")) {
      assert(ent->d_type & DT_REG);
      seen[2] = 1;
      continue;
    }
    assert(0 && "odd filename");
  }
  ent = readdir(dir);
  if (ent) printf("surprising ent: %p : %s\n", ent, ent->d_name);
  assert(!ent);

  // test rewinddir
  rewinddir(dir);
  ent = readdir(dir);
  assert(ent && ent->d_ino);
  assert(!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, "..") || !strcmp(ent->d_name, "file.txt"));

  // test seek / tell
  rewinddir(dir);
  ent = readdir(dir);
  assert(ent && ent->d_ino);
  assert(!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, "..") || !strcmp(ent->d_name, "file.txt"));
  loc = telldir(dir);
  assert(loc >= 0);
  //printf("loc=%d\n", loc);
  loc2 = ent->d_off;
  ent = readdir(dir);
  assert(ent && ent->d_ino);
  char name_at_loc[1024];
  strcpy(name_at_loc, ent->d_name);
  //printf("name_at_loc: %s\n", name_at_loc);
  assert(!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, "..") || !strcmp(ent->d_name, "file.txt"));
  ent = readdir(dir);
#ifndef WASMFS_NODERAWFS
  // WasmFS + NODERAWFS lacks ino numbers in directory listings, see
  // https://github.com/emscripten-core/emscripten/issues/19418
  // This is not an issue for "." and ".." (the other checks before and after
  // us) since "." and ".." are added by WasmFS code itself; only "file.txt" is
  // added from the node JS API, and as a result it has inode 0.
  assert(ent && ent->d_ino);
#endif
  assert(!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, "..") || !strcmp(ent->d_name, "file.txt"));
  seekdir(dir, loc);
  ent = readdir(dir);
  assert(ent && ent->d_ino);
  //printf("check: %s / %s\n", ent->d_name, name_at_loc);
  assert(!strcmp(ent->d_name, name_at_loc));

  seekdir(dir, loc2);
  ent = readdir(dir);
  assert(ent && ent->d_ino);
  //printf("check: %s / %s\n", ent->d_name, name_at_loc);
  assert(!strcmp(ent->d_name, name_at_loc));

  //
  // do a normal read with readdir_r
  //
  rewinddir(dir);
  err = readdir_r(dir, &ent_r, &result);
  assert(!err);
  assert(&ent_r == result);
  assert(!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, "..") || !strcmp(ent->d_name, "file.txt"));
  err = readdir_r(dir, &ent_r, &result);
  assert(!err);
  assert(&ent_r == result);
  assert(!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, "..") || !strcmp(ent->d_name, "file.txt"));
  err = readdir_r(dir, &ent_r, &result);
  assert(!err);
  assert(&ent_r == result);
  assert(!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, "..") || !strcmp(ent->d_name, "file.txt"));
  err = readdir_r(dir, &ent_r, &result);
  assert(!err);
  assert(!result);

  err = closedir(dir);
  assert(!err);

  puts("success");
}

void test_scandir() {
  struct dirent **namelist;
  int n;

  n = scandir(".", &namelist, NULL, alphasort);
  printf("n: %d\n", n);
  if (n < 0)
    return;
  else {
    while (n--) {
      printf("name: %s\n", namelist[n]->d_name);
      free(namelist[n]);
    }
    free(namelist);
  }
}

int main() {
  atexit(cleanup);
  signal(SIGABRT, cleanup);
  setup();
  test();
  test_scandir();

  return EXIT_SUCCESS;
}
