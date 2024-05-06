/*
 * Copyright 2015 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <emscripten.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <limits.h>
#include <stdlib.h>

int main() {
  EM_ASM(
    fs.mkdirSync('directory', '0777');
    fs.writeFileSync('directory/test', 'Link it');
    fs.symlinkSync('/working/directory', 'inside-symlink');
    fs.symlinkSync(fs.realpathSync('./directory'), 'outside-symlink');
    fs.mkdirSync('directory/subdirectory', '0777');
    fs.writeFileSync('directory/subdirectory/file', 'subdirectory');

    fs.mkdirSync('relative', '0777');
    fs.writeFileSync('relative/file', 'relative');
    fs.mkdirSync('relative/subrelative', '0777');
    fs.writeFileSync('relative/subrelative/file', 'subrelative');
    fs.symlinkSync("../relative/file", "directory/relative");
    fs.symlinkSync("../../relative/subrelative/file", "directory/subdirectory/subrelative");
    fs.symlinkSync("./directory/subdirectory/file", "subdirectoryrelative");

    fs.mkdirSync('absolute', '0777');
    fs.writeFileSync('absolute/file', 'absolute');
    fs.mkdirSync('absolute/subabsolute', '0777');
    fs.writeFileSync('absolute/subabsolute/file', 'subabsolute');
    fs.symlinkSync("/working/absolute/file", "directory/absolute");
    fs.symlinkSync("/working/absolute/subabsolute/file", "directory/subdirectory/subabsolute");
    fs.symlinkSync("/working/directory/subdirectory/file", "subdirectoryabsolute");

    FS.mkdir('working');
    FS.mount(NODEFS, { root: '.' }, 'working');

    FS.mkdir('direct-inside-link');
    FS.mount(NODEFS, { root: './inside-symlink' }, 'direct-inside-link');

    FS.mkdir('direct-outside-link');
    FS.mount(NODEFS, { root: './outside-symlink' }, 'direct-outside-link');
  );

  {
    const char* path = "/working/inside-symlink/test";
    printf("reading %s\n", path);

    FILE* fd = fopen(path, "r");
    if (fd == NULL) {
      printf("failed to open file %s\n", path);
    }
    else {
      char buffer[8];
      fread(buffer, 1, 7, fd);
      buffer[7] = 0;
      printf("buffer is %s\n", buffer);
      fclose(fd);
    }
  }

  printf("\n");

  {
    const char* path = "/working/outside-symlink/test";
    printf("reading %s\n", path);

    FILE* fd = fopen(path, "r");
    if (fd == NULL) {
      printf("failed to open file %s\n", path);
    }
    else {
      char buffer[8];
      fread(buffer, 1, 7, fd);
      buffer[7] = 0;
      printf("buffer is %s\n", buffer);
      fclose(fd);
    }
  }

  printf("\n");

  {
    const char* path = "/direct-inside-link/test";
    printf("reading %s\n", path);

    FILE* fd = fopen(path, "r");
    if (fd != NULL) {
      printf("opened file %s\n", path);
      fclose(fd);
    }
    else {
      printf("failed to open file %s\n", path);
    }
  }

  printf("\n");

  {
    const char* path = "/direct-outside-link/test";
    printf("reading %s\n", path);

    FILE* fd = fopen(path, "r");
    if (fd != NULL) {
      printf("opened file %s\n", path);
      fclose(fd);
    }
    else {
      printf("failed to open file %s\n", path);
    }
  }

  printf("\n");

  {
    char* paths[] = {
      "/working/directory/relative",
      "/working/directory/subdirectory/subrelative",
      "/working/subdirectoryrelative",
      "/working/directory/absolute",
      "/working/directory/subdirectory/subabsolute",
      "/working/subdirectoryabsolute"
      };

    for (int i = 0; i < sizeof paths / sizeof paths[0]; i++) {
      printf("reading %s\n", paths[i]);
      char link[256] = {0};
      readlink(paths[i], link, 256);
      FILE *fd = fopen(link, "r");
      if (fd == NULL) {
        printf("failed to open file %s\n", link);
      }
      else {
        char buffer[256] = {0};
        fread(buffer, 1, 256, fd);
        printf("buffer is %s\n", buffer);
        fclose(fd);
      }

      printf("\n");
    }
  }

  return 0;
}
