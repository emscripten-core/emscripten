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
#include <limits.h>

void cleanup();

#define ASSERT_WITH_CLEANUP(cond) if (!(cond)) { cleanup(); assert(cond); }

#define CHECK(cond) if (!(cond)) { printf("errno: %s\n", strerror(errno)); ASSERT_WITH_CLEANUP(cond); }

static void create_file(const char *path, const char *buffer, int mode) {
  int fd = open(path, O_WRONLY | O_CREAT | O_EXCL, mode);
  CHECK(fd >= 0);

  int err = write(fd, buffer, sizeof(char) * strlen(buffer));
  ASSERT_WITH_CLEANUP(err ==  (sizeof(char) * strlen(buffer)));

  close(fd);
}

char windir[PATH_MAX] = {0};
char tempdir[PATH_MAX] = {0};
char testtmp[PATH_MAX] = {0};

void setup() {
  sprintf(testtmp, "%s\\testtmp", tempdir);
  int err;
  err = mkdir(testtmp, 0777);  // can't call it tmp, that already exists
  CHECK(!err);
  chdir(testtmp);
  err = mkdir("nocanread", 0111);
  CHECK(!err);
  err = mkdir("foobar", 0777);
  CHECK(!err);
  create_file("foobar\\file.txt", "ride into the danger zone", 0666);
}

void cleanup() {
  rmdir("nocanread");
  unlink("foobar\\file.txt");
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
  ASSERT_WITH_CLEANUP(!dir);
  ASSERT_WITH_CLEANUP(errno == ENOENT);
  char filepath[PATH_MAX] = {0};
  sprintf(filepath, "%s\\foobar\\file.txt", testtmp);
  dir = opendir(filepath);
  ASSERT_WITH_CLEANUP(!dir);

  //
  // do a normal read with readdir
  //
  char dirpath[PATH_MAX] = {0};
  sprintf(dirpath, "%s\\foobar", testtmp);
  dir = opendir(dirpath);
  ASSERT_WITH_CLEANUP(dir);
  int seen[3] = { 0, 0, 0 };
  for (i = 0; i < 3; i++) {
    errno = 0;
    ent = readdir(dir);
    if (ent) {
      fprintf(stderr, "%d file: %s (%d : %lu)\n", i, ent->d_name, ent->d_reclen, sizeof(*ent));
    } else {
      fprintf(stderr, "ent: %p, errno: %d\n", ent, errno);
      ASSERT_WITH_CLEANUP(ent);
    }
    ASSERT_WITH_CLEANUP(ent->d_reclen == sizeof(*ent));
    if (!seen[0] && !strcmp(ent->d_name, ".")) {
      ASSERT_WITH_CLEANUP(ent->d_type & DT_DIR);
      seen[0] = 1;
      continue;
    }
    if (!seen[1] && !strcmp(ent->d_name, "..")) {
      ASSERT_WITH_CLEANUP(ent->d_type & DT_DIR);
      seen[1] = 1;
      continue;
    }
    if (!seen[2] && !strcmp(ent->d_name, "file.txt")) {
      ASSERT_WITH_CLEANUP(ent->d_type & DT_REG);
      seen[2] = 1;
      continue;
    }
    ASSERT_WITH_CLEANUP(0 && "odd filename");
  }
  ent = readdir(dir);
  if (ent) printf("surprising ent: %p : %s\n", ent, ent->d_name);
  ASSERT_WITH_CLEANUP(!ent);

  // test rewinddir
  rewinddir(dir);
  ent = readdir(dir);
  ASSERT_WITH_CLEANUP(ent && ent->d_ino);
  ASSERT_WITH_CLEANUP(!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, "..") || !strcmp(ent->d_name, "file.txt"));

  // test seek / tell
  rewinddir(dir);
  ent = readdir(dir);
  ASSERT_WITH_CLEANUP(ent && ent->d_ino);
  ASSERT_WITH_CLEANUP(!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, "..") || !strcmp(ent->d_name, "file.txt"));
  loc = telldir(dir);
  ASSERT_WITH_CLEANUP(loc >= 0);
  //printf("loc=%d\n", loc);
  loc2 = ent->d_off;
  ent = readdir(dir);
  ASSERT_WITH_CLEANUP(ent && ent->d_ino);
  char name_at_loc[1024];
  strcpy(name_at_loc, ent->d_name);
  //printf("name_at_loc: %s\n", name_at_loc);
  ASSERT_WITH_CLEANUP(!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, "..") || !strcmp(ent->d_name, "file.txt"));
  ent = readdir(dir);
  ASSERT_WITH_CLEANUP(ent && ent->d_ino);
  ASSERT_WITH_CLEANUP(!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, "..") || !strcmp(ent->d_name, "file.txt"));
  seekdir(dir, loc);
  ent = readdir(dir);
  ASSERT_WITH_CLEANUP(ent && ent->d_ino);
  //printf("check: %s / %s\n", ent->d_name, name_at_loc);
  ASSERT_WITH_CLEANUP(!strcmp(ent->d_name, name_at_loc));

  seekdir(dir, loc2);
  ent = readdir(dir);
  ASSERT_WITH_CLEANUP(ent && ent->d_ino);
  //printf("check: %s / %s\n", ent->d_name, name_at_loc);
  ASSERT_WITH_CLEANUP(!strcmp(ent->d_name, name_at_loc));

  //
  // do a normal read with readdir_r
  //
  rewinddir(dir);
  err = readdir_r(dir, &ent_r, &result);
  ASSERT_WITH_CLEANUP(!err);
  ASSERT_WITH_CLEANUP(&ent_r == result);
  ASSERT_WITH_CLEANUP(!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, "..") || !strcmp(ent->d_name, "file.txt"));
  err = readdir_r(dir, &ent_r, &result);
  ASSERT_WITH_CLEANUP(!err);
  ASSERT_WITH_CLEANUP(&ent_r == result);
  ASSERT_WITH_CLEANUP(!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, "..") || !strcmp(ent->d_name, "file.txt"));
  err = readdir_r(dir, &ent_r, &result);
  ASSERT_WITH_CLEANUP(!err);
  ASSERT_WITH_CLEANUP(&ent_r == result);
  ASSERT_WITH_CLEANUP(!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, "..") || !strcmp(ent->d_name, "file.txt"));
  err = readdir_r(dir, &ent_r, &result);
  ASSERT_WITH_CLEANUP(!err);
  ASSERT_WITH_CLEANUP(!result);

  err = closedir(dir);
  ASSERT_WITH_CLEANUP(!err);

  // Read the windir and verify
  dir = opendir(windir);
  ASSERT_WITH_CLEANUP(dir);
  err = closedir(dir);
  ASSERT_WITH_CLEANUP(!err);

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

int main(int argc, char * argv[]) {
  if (argc < 3) {
    printf("test_readdir_windows args: %%WINDIR%% %%TEMP%%\n");
    return -1;
  }
  strcpy(windir, argv[1]);
  strcpy(tempdir, argv[2]);
  // atexit(cleanup); Doesn't work on windows
  signal(SIGABRT, cleanup);
  setup();
  test();
  test_scandir();
  cleanup();

  return EXIT_SUCCESS;
}
