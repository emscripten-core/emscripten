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

    FS.mkdir('working');
    FS.mount(NODEFS, { root: '.' }, 'working');

    FS.mkdir('mount-link');
    FS.mount(NODEFS, { root: 'inside-symlink' }, 'mount-link');
  );
}

void test_inside_symlink()
{
  char buf[256] = {0};
  readlink("/working/inside-symlink", buf, 256);
  printf("readlink: '%s'\n", buf);
  FILE* fd = fopen("/working/inside-symlink/test", "r");
  assert(fd);
  char buffer[8] = {0};
  int rtn = fread(buffer, 1, 7, fd);
  assert(rtn == 7);
  printf("buffer: '%s'\n", buffer);
  fclose(fd);
}

void test_outside_symlink()
{
  // outside-symlink is link to an absolute path which is not part of the emscripten VFS
  // and so we should be able to open it.
  FILE* fd = fopen("/working/outside-symlink/test", "r");
  assert(fd == NULL);
  assert(errno == ENOENT);
}

void test_mount_link()
{
  char buf[256] = {0};
  readlink("/mount-link", buf, 256);
  printf("\nreadlink: '%s'\n", buf);
  FILE* fd = fopen("/mount-link/test", "r");
  assert(fd);
  char buffer[8] = {0};
  int rtn = fread(buffer, 1, 7, fd);
  assert(rtn == 7);
  printf("buffer: '%s'\n", buffer);
  fclose(fd);
}

int main() {
  setup();
  test_inside_symlink();
  test_outside_symlink();
  test_mount_link();
  return 0;
}
