/*
 * Copyright 2011 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#else
#include <sys/stat.h>
#include <fcntl.h>
#endif

void setup() {
#ifdef __EMSCRIPTEN__
  EM_ASM(
    FS.mkdir('working');
#if NODEFS
    FS.mount(NODEFS, { root: '.' }, 'working');
#endif
    FS.chdir('working');
    FS.symlink('../test/../there!', 'link');
    FS.writeFile('file', 'test');
    FS.mkdir('folder');
    FS.mkdir('folder/subfolder');
    FS.writeFile('folder/subfolder/file', 'subfolder');

    FS.mkdir('relative');
    FS.writeFile('relative/file', 'relative');
    FS.mkdir('relative/subrelative');
    FS.writeFile('relative/subrelative/file', 'subrelative');
    FS.symlink("../relative/file", "./folder/relative");
    FS.symlink("../../relative/subrelative/file", "./folder/subfolder/subrelative");
    FS.symlink("./folder/subfolder/file", "./subfolderrelative");

    FS.mkdir('absolute');
    FS.writeFile('absolute/file', 'absolute');
    FS.mkdir('absolute/subabsolute');
    FS.writeFile('absolute/subabsolute/file', 'subabsolute');
    FS.symlink(fs.realpathSync("absolute/file"), "./folder/absolute");
    FS.symlink(fs.realpathSync("absolute/subabsolute/file"), "./folder/subfolder/subabsolute");
    FS.symlink(fs.realpathSync("folder/subfolder/file"), "./subfolderabsolute");
  );
#else
  int fd;
  mkdir("working", 0777);
  chdir("working");
  symlink("../test/../there!", "link");
  fd = open("file", O_RDWR);
  write(fd, "test", 5);
  close(fd);
  mkdir("folder", 0777);

  mkdir("folder/subfolder", 0777);
  fd = open("folder/subfolder/file", O_RDWR);
  write(fd, "subfolder", 10);
  close(fd);

  mkdir("relative", 0777);
  fd = open("relative/file", O_RDWR);
  write(fd, "relative", 10);
  close(fd);
  mkdir("relative/subrelative", 0777);
  fd = open("relative/subrelative/file", O_RDWR);
  write(fd, "subrelative", 10);
  close(fd);

  mkdir("absolute", 0777);
  fd = open("absolute/file", O_RDWR);
  write(fd, "absolute", 10);
  close(fd);
  mkdir("absolute/subabsolute", 0777);
  fd = open("absolute/subabsolute/file", O_RDWR);
  write(fd, "subabsolute", 10);
  close(fd);
#endif
}

int main() {
  setup();

  char* files[] = {"link", "file", "folder"};
  char buffer[256] = {0};
  int ret;

  for (int i = 0; i < sizeof files / sizeof files[0]; i++) {
    printf("readlink(%s)\n", files[i]);
    ret = readlink(files[i], buffer, 256);
    printf("errno: %s\n", strerror(errno));
    if (ret < 0) {
      printf("not a link\n\n");
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
    assert(strlen(buffer) == ret);
    errno = 0;
    printf("\n");
  }

  printf("symlink/overwrite\n");
  ret = symlink("new-nonexistent-path", "link");
  assert(ret == -1);
  assert(errno == EEXIST);
  errno = 0;

  printf("\nsymlink/normal\n");
  ret = symlink("new-nonexistent-path", "folder/link");
  assert(ret == 0);
  assert(errno == 0);
  errno = 0;

  printf("\nreadlink(created link)\n");
  ret = readlink("folder/link", buffer, 256);
  assert(errno == 0);
#if !defined(__EMSCRIPTEN__) || defined(WASMFS)
  assert(strcmp(buffer, "new-nonexistent-path") == 0);
#else
  assert(strcmp(buffer, "/working/folder/new-nonexistent-path") == 0);
#endif
  assert(strlen(buffer) == ret);
  errno = 0;

  buffer[0] = buffer[1] = buffer[2] = buffer[3] = buffer[4] = buffer[5] = '*';
  printf("\nreadlink(short buffer)\n");
  ret = readlink("link", buffer, 4);
  assert(errno == 0);
  assert(ret == 4);
#if !defined(__EMSCRIPTEN__) || defined(WASMFS)
  assert(strcmp(buffer, "../t**nexistent-path") == 0);
#else
  assert(strcmp(buffer, "/the**ng/folder/new-nonexistent-path") == 0);
#endif
  errno = 0;

  // FS.lookupPath should notice the symlink loop and return ELOOP, not go into
  // an infinite recurse.
  //
  // This test doesn't work in wasmfs -- probably because access sees the
  // symlink and returns true without bothering to chase the symlink
  symlink("./linkX/inside", "./linkX");
  ret = access("linkX", F_OK);
  assert(ret == -1);
  assert(errno == ELOOP);
  errno = 0;


  char* paths[] = {
    "./folder/relative",
    "./folder/subfolder/subrelative",
    "./subfolderrelative",
    "./folder/absolute",
    "./folder/subfolder/subabsolute",
    "./subfolderabsolute"
    };

  for (int i = 0; i < sizeof paths / sizeof paths[0]; i++) {
    char path[256] = {0};
    readlink(paths[i], path, 256);
    FILE *fd = fopen(path, "r");
    if (fd == NULL) {
      printf("\nfailed to open file %s\n", path);
    }
    else {
      char target[12] = {0};
      fread(target, 1, 12, fd);
      printf("\nsymlink/%s\n", target);
      fclose(fd);
    }
  }

  return 0;
}
