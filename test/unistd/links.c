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
  );
#else
  mkdir("working", 0777);
  chdir("working");
  symlink("../test/../there!", "link");
  int fd = open("file", O_RDWR);
  write(fd, "test", 5);
  close(fd);
  mkdir("folder", 0777);
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

  return 0;
}
