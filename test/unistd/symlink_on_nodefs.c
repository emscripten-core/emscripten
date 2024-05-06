/*
 * Copyright 2015 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <assert.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <emscripten.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <limits.h>
#include <stdlib.h>

void setup() {
  EM_ASM(
    fs.mkdirSync('directory', '0777');
    fs.writeFileSync('directory/test', 'Link it');
    fs.symlinkSync('/working/directory', 'inside-symlink');
    fs.symlinkSync(fs.realpathSync('directory'), 'outside-symlink');
    fs.mkdirSync('directory/subdirectory', '0777');
    fs.writeFileSync('directory/subdirectory/file', 'Subdirectory');

    fs.mkdirSync('relative', '0777');
    fs.writeFileSync('relative/file', 'Relative');
    fs.mkdirSync('relative/subrelative', '0777');
    fs.writeFileSync('relative/subrelative/file', 'Subrelative');
    fs.symlinkSync("../relative/file", "directory/relative");
    fs.symlinkSync("../../relative/subrelative/file", "directory/subdirectory/subrelative");
    fs.symlinkSync("./directory/subdirectory/file", "subdirectoryrelative");

    fs.mkdirSync('absolute', '0777');
    fs.writeFileSync('absolute/file', 'Absolute');
    fs.mkdirSync('absolute/subabsolute', '0777');
    fs.writeFileSync('absolute/subabsolute/file', 'Subabsolute');
    fs.symlinkSync("/working/absolute/file", "directory/absolute");
    fs.symlinkSync("/working/absolute/subabsolute/file", "directory/subdirectory/subabsolute");
    fs.symlinkSync("/working/directory/subdirectory/file", "subdirectoryabsolute");

    FS.mkdir('working');
    FS.mount(NODEFS, { root: '.' }, 'working');

    FS.mkdir('mount-inside-link');
    FS.mount(NODEFS, { root: 'inside-symlink' }, 'mount-inside-link');

    FS.mkdir('mount-outside-link');
    FS.mount(NODEFS, { root: 'outside-symlink' }, 'mount-outside-link');
  );
}

void test_inside_symlink()
{
  char buf[256] = {0};
  readlink("/working/inside-symlink", buf, 256);
  printf("readlink: %s\n", buf);
  FILE* fd = fopen("/working/inside-symlink/test", "r");
  assert(fd);
  char buffer[8] = {0};
  int rtn = fread(buffer, 1, 7, fd);
  assert(rtn == 7);
  printf("buffer is '%s'\n", buffer);
  fclose(fd);
}

void test_outside_symlink()
{
  FILE* fd = fopen("/working/outside-symlink/test", "r");
  assert(fd == NULL);
}

void test_mount_inside_link()
{
  char buf[256] = {0};
  readlink("/mount-inside-link", buf, 256);
  printf("\nreadlink: %s\n", buf);
  FILE* fd = fopen("/mount-inside-link/test", "r");
  assert(fd);
  char buffer[8] = {0};
  int rtn = fread(buffer, 1, 7, fd);
  assert(rtn == 7);
  printf("buffer is '%s'\n", buffer);
  fclose(fd);
}

void test_mount_outside_link()
{
  FILE* fd = fopen("/mount-outside-link/test", "r");
  assert(fd == NULL);
}

void test_relative_path_symlinks()
{
  char* paths[] = {
    "/working/directory/relative",
    "/working/directory/subdirectory/subrelative",
    "/working/subdirectoryrelative",
  };

  for (int i = 0; i < sizeof paths / sizeof paths[0]; i++) {
    char buf[256] = {0};
    readlink(paths[i], buf, 256);
    printf("\nreadlink: %s\n", buf);
    FILE *fd = fopen(buf, "r");
    assert(fd);
    char buffer[13] = {0};
    int rtn = fread(buffer, 1, 13, fd);
    assert(rtn <= 13);
    printf("buffer is '%s'\n", buffer);
    fclose(fd);
  }
}

void test_absolute_path_symlinks()
{
  char* paths[] = {
    "/working/directory/absolute",
    "/working/directory/subdirectory/subabsolute",
    "/working/subdirectoryabsolute"
  };

  for (int i = 0; i < sizeof paths / sizeof paths[0]; i++) {
    char buf[1024] = {0};
    readlink(paths[i], buf, 256);
    printf("\nreadlink: %s\n", buf);
    FILE *fd = fopen(buf, "r");
    assert(fd);
    char buffer[13] = {0};
    int rtn = fread(buffer, 1, 13, fd);
    assert(rtn <= 13);
    printf("buffer is '%s'\n", buffer);
    fclose(fd);
  }
}

int main() {
  setup();
  test_inside_symlink();
  test_outside_symlink();
  test_mount_inside_link();
  test_mount_outside_link();
  test_relative_path_symlinks();
  test_absolute_path_symlinks();
  return 0;
}
