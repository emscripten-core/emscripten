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
  int err;
  err = mkdir("nocanread", 0111);
  assert(!err);
  err = mkdir("foobar", 0777);
  assert(!err);
  create_file("foobar/file.txt", "ride into the danger zone", 0666);
}

void cleanup() {
  rmdir("nocanread");
  unlink("foobar/file.txt");
  rmdir("foobar");
}

void test() {
  int err;
  long loc;
  DIR *dir;
  struct dirent *ent;
  struct dirent ent_r;
  struct dirent *result;
  int i;

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
    //printf("ent, errno: %p, %d\n", ent, errno);
    assert(ent);
    //printf("%d file: %s (%d : %d)\n", i, ent->d_name, ent->d_reclen, sizeof(*ent));
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
  assert(!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, "..") || !strcmp(ent->d_name, "file.txt"));

  // test seek / tell
  rewinddir(dir);
  ent = readdir(dir);
  assert(!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, "..") || !strcmp(ent->d_name, "file.txt"));
  char first[1024];
  //printf("first: %s\n", ent->d_name);
  strcpy(first, ent->d_name);
  loc = telldir(dir);
  assert(loc >= 0);
  ent = readdir(dir);
  assert(!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, "..") || !strcmp(ent->d_name, "file.txt"));
  ent = readdir(dir);
  assert(!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, "..") || !strcmp(ent->d_name, "file.txt"));
  seekdir(dir, loc);
  ent = readdir(dir);
  assert(ent);
  //printf("check: %s / %s\n", ent->d_name, first);
  assert(!strcmp(ent->d_name, first));

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
  printf("SIGILL: %s\n", strsignal(SIGILL));
  atexit(cleanup);
  signal(SIGABRT, cleanup);
  setup();
  test();
  test_scandir();
  return EXIT_SUCCESS;
}
