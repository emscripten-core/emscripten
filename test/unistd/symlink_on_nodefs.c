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
    fs.mkdirSync('new-directory', '0777');
    fs.writeFileSync('new-directory/test', 'Link it');
    fs.symlinkSync(fs.realpathSync('new-directory'), 'symlink');

    FS.mkdir('working');
    FS.mount(NODEFS, { root: '.' }, 'working');

    FS.mkdir('direct-link');
    FS.mount(NODEFS, { root: 'symlink' }, 'direct-link');
  );
}

int main() {
  setup();

  char buf[1024];
  readlink("/working/symlink", buf, 1024);
  printf("readlink: %s\n", buf);

  FILE* fd = fopen("/working/symlink/test", "r");
  assert(fd);
  char buffer[8] = {0};
  int rtn = fread(buffer, 1, 7, fd);
  assert(rtn == 7);
  printf("buffer is '%s'\n", buffer);
  fclose(fd);

  // This should fail, since it resolves to ../new-directory which is not
  // mounted
  fd = fopen("/direct-link/test", "r");
  assert(fd == NULL);

  return 0;
}
