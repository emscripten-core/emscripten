/*
 * Copyright 2011 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

void makedir(const char *dir) {
  int rtn = mkdir(dir, 0777);
  assert(rtn == 0);
}

void makefile(const char *file, const char *content) {
  int fd = open(file, O_RDWR | O_CREAT, 0777);
  assert(fd >= 0);
  int rtn = write(fd, content, strlen(content));
  assert(rtn == strlen(content));
  close(fd);
}

void makelink(const char *link, const char *path) {
  int rtn = symlink(link, path);
  assert(rtn == 0);
}

void changedir(const char *dir) {
  int rtn = chdir(dir);
  assert(rtn == 0);
}

void setup() {
  makedir("working");
#if defined(__EMSCRIPTEN__) && defined(NODEFS)
  EM_ASM(FS.mount(NODEFS, { root: '.' }, 'working'));
#endif
  changedir("working");
  makelink("../test/../there!", "link");
  makefile("file", "test");
  makedir("directory");
  makedir("directory/subdirectory");
  makefile("directory/subdirectory/file", "Subdirectory");

  makedir("relative");
  makedir("relative/subrelative");
  makefile("relative/file", "Relative");
  makefile("relative/subrelative/file", "Subrelative");
  makelink("../relative/file", "directory/relative");
  makelink("../../relative/subrelative/file", "directory/subdirectory/subrelative");
  makelink("directory/subdirectory/file", "subdirectoryrelative");

  makedir("absolute");
  makedir("absolute/subabsolute");
  makefile("absolute/file", "Absolute");
  makefile("absolute/subabsolute/file", "Subabsolute");
  makelink("/working/absolute/file", "/working/directory/absolute");
  makelink("/working/absolute/subabsolute/file", "/working/directory/subdirectory/subabsolute");
  makelink("/working/directory/subdirectory/file", "/working/subdirectoryabsolute");
}

void test_reading_existing_symlinks() {
  char* files[] = {"link", "file", "directory"};

  for (int i = 0; i < sizeof files / sizeof files[0]; i++) {
    char buffer[256] = {0};
    int rtn = readlink(files[i], buffer, 256);
    printf("readlink: '%s'\n", files[i]);
    printf("errno: %s\n\n", strerror(errno));
    if (rtn < 0) {
      continue;
    }

    // WASMFS behaves the same as Linux (and as best as I can tell, the spec),
    // seeing the symlink as a string. The old JS FS instead normalizes it and
    // returns something modified.
    // The same happens in the assertions below.
#if !defined(__EMSCRIPTEN__) || defined(WASMFS)
    assert(strcmp(buffer, "../test/../there!") == 0);
#else
    assert(strcmp(buffer, "/there!") == 0);
#endif
    assert(strlen(buffer) == rtn);
    errno = 0;
  }
}

void test_overwriting_symlink() {
  int rtn = symlink("new-nonexistent-path", "link");
  assert(rtn == -1);
  assert(errno == EEXIST);
  errno = 0;
}

void test_creating_symlink() {
  int rtn = symlink("new-nonexistent-path", "directory/link");
  assert(rtn == 0);
  assert(errno == 0);
  errno = 0;

  char buffer[256] = {0};
  rtn = readlink("directory/link", buffer, 256);
  assert(errno == 0);
#if !defined(__EMSCRIPTEN__) || defined(WASMFS)
  assert(strcmp(buffer, "new-nonexistent-path") == 0);
#else
  assert(strcmp(buffer, "/working/directory/new-nonexistent-path") == 0);
#endif
  assert(strlen(buffer) == rtn);
  errno = 0;
}

void test_reading_shortened_symlink() {
  char buffer[256] = {0};
  readlink("directory/link", buffer, 256);
  buffer[0] = buffer[1] = buffer[2] = buffer[3] = buffer[4] = buffer[5] = '*';
  int rtn = readlink("link", buffer, 4);
  assert(errno == 0);
  assert(rtn == 4);
#if !defined(__EMSCRIPTEN__) || defined(WASMFS)
  assert(strcmp(buffer, "../t**nexistent-path") == 0);
#else
  assert(strcmp(buffer, "/the**ng/directory/new-nonexistent-path") == 0);
#endif
  errno = 0;
}

void test_noticing_loop_in_symlink() {
  // FS.lookupPath should notice the symlink loop and return ELOOP, not go into
  // an infinite recurse.
  //
  // This test doesn't work in wasmfs -- probably because access sees the
  // symlink and returns true without bothering to chase the symlink
  symlink("./loop-link/inside", "./loop-link");
  int rtn = access("loop-link", F_OK);
  assert(rtn == -1);
  assert(errno == ELOOP);
  errno = 0;
}


void test_relative_path_symlinks() {
  char* parents[] = {
    "/working/directory/",
    "/working/directory/subdirectory/",
    "/working/"
  };

  char* links[] = {
    "relative",
    "subrelative",
    "subdirectoryrelative",
  };

  for (int i = 0; i < sizeof links / sizeof links[0]; i++) {
    int rtn = chdir(parents[i]);
    assert(rtn == 0);
    char symlink[256] = {0};
    strcat(strcpy(symlink, parents[i]), links[i]);
    printf("symlink: '%s'\n", symlink);
    char buf[256] = {0};
    rtn = readlink(links[i], buf, 256);
    FILE *fd = fopen(buf, "r");
    assert(fd);
    char buffer[13] = {0};
    rtn = fread(buffer, 1, 13, fd);
    assert(rtn <= 13);
    printf("buffer: '%s'\n\n", buffer);
    fclose(fd);
  }
}

void test_absolute_path_symlinks() {
  char* links[] = {
    "/working/directory/absolute",
    "/working/directory/subdirectory/subabsolute",
    "/working/subdirectoryabsolute"
  };

  for (int i = 0; i < sizeof links / sizeof links[0]; i++) {
    printf("symlink: '%s'\n", links[i]);
    char buf[256] = {0};
    readlink(links[i], buf, 256);
    FILE *fd = fopen(buf, "r");
    assert(fd);
    char buffer[13] = {0};
    int rtn = fread(buffer, 1, 13, fd);
    assert(rtn <= 13);
    printf("buffer: '%s'\n\n", buffer);
    fclose(fd);
  }
}

int main() {
  setup();
  test_reading_existing_symlinks();
  test_overwriting_symlink();
  test_creating_symlink();
  test_reading_shortened_symlink();
  test_noticing_loop_in_symlink();
  test_relative_path_symlinks();
  test_absolute_path_symlinks();
  return 0;
}
