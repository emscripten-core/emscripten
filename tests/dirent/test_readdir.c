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

static void create_file(const char *path, const char *buffer, int mode) {
  int fd = open(path, O_WRONLY | O_CREAT | O_EXCL, mode);
  assert(fd >= 0);

  int err = write(fd, buffer, sizeof(char) * strlen(buffer));
  assert(err ==  (sizeof(char) * strlen(buffer)));

  close(fd);
}

void setup() {
  mkdir("nocanread", 0111);
  mkdir("foobar", 0777);
  create_file("foobar/file.txt", "ride into the danger zone", 0666);
}

void cleanup() {
  rmdir("nocanread");
  unlink("foobar/file.txt");
  rmdir("foobar");
}

void test() {
  int err;
  int loc;
  DIR *dir;
  struct dirent *ent;
  struct dirent ent_r;
  struct dirent *result;

  // check bad opendir input
  dir = opendir("noexist");
  assert(!dir);
  assert(errno == ENOENT);
  dir = opendir("nocanread");
  assert(!dir);
  assert(errno == EACCES);
  dir = opendir("foobar/file.txt");
  assert(!dir);
  assert(errno == ENOTDIR);

  // check bad readdir input
  dir = opendir("foobar");
  closedir(dir);
  ent = readdir(dir);
  assert(!ent);
  assert(errno == EBADF);

  // check bad readdir_r input
  dir = opendir("foobar");
  closedir(dir);
  err = readdir_r(dir, NULL, &result);
  assert(err == EBADF);
  
  //
  // do a normal read with readdir
  //
  dir = opendir("foobar");
  assert(dir);
  ent = readdir(dir);
  assert(!strcmp(ent->d_name, "."));
  assert(ent->d_type & DT_DIR);
  ent = readdir(dir);
  assert(!strcmp(ent->d_name, ".."));
  assert(ent->d_type & DT_DIR);
  ent = readdir(dir);
  assert(!strcmp(ent->d_name, "file.txt"));
  assert(ent->d_type & DT_REG);
  ent = readdir(dir);
  assert(!ent);

  // test rewinddir
  rewinddir(dir);
  ent = readdir(dir);
  assert(!strcmp(ent->d_name, "."));

  // test seek / tell
  rewinddir(dir);
  ent = readdir(dir);
  assert(!strcmp(ent->d_name, "."));
  loc = telldir(dir);
  ent = readdir(dir);
  assert(!strcmp(ent->d_name, ".."));
  ent = readdir(dir);
  assert(!strcmp(ent->d_name, "file.txt"));
  seekdir(dir, loc);
  ent = readdir(dir);
  assert(!strcmp(ent->d_name, ".."));

  //
  // do a normal read with readdir_r
  //
  rewinddir(dir);
  err = readdir_r(dir, &ent_r, &result);
  assert(!err);
  assert(&ent_r == result);
  assert(!strcmp(ent_r.d_name, "."));
  err = readdir_r(dir, &ent_r, &result);
  assert(!err);
  assert(&ent_r == result);
  assert(!strcmp(ent_r.d_name, ".."));
  err = readdir_r(dir, &ent_r, &result);
  assert(!err);
  assert(&ent_r == result);
  assert(!strcmp(ent_r.d_name, "file.txt"));
  err = readdir_r(dir, &ent_r, &result);
  assert(!err);
  assert(!result);

  err = closedir(dir);
  assert(!err);

  puts("success");
}

int main() {
  atexit(cleanup);
  signal(SIGABRT, cleanup);
  setup();
  test();
  return EXIT_SUCCESS;
}
