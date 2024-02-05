/*
 * Copyright 2011 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <assert.h>
#include <emscripten.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main() {
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

  char* files[] = {"link", "file", "folder"};
  char buffer[256] = {0};

  for (int i = 0; i < sizeof files / sizeof files[0]; i++) {
    printf("readlink(%s)\n", files[i]);
    int ret = readlink(files[i], buffer, 256);
    printf("errno: %d\n", errno);
    if (ret < 0) {
      printf("not a link\n\n");
      continue;
    }
    // WASMFS behaves the same as Linux (and as best as I can tell, the spec),
    // seeing the symlink as a string. The old JS FS instead normalizes it and
    // returns something modified.
    // The same happens in the assertions below.
#ifdef WASMFS
    assert(strcmp(buffer, "../test/../there!") == 0);
#else
    assert(strcmp(buffer, "/there!") == 0);
#endif
    assert(strlen(buffer) == ret);
    errno = 0;
    printf("\n");
  }

  printf("symlink/overwrite\n");
  printf("ret: %d\n", symlink("new-nonexistent-path", "link"));
  printf("errno: %d\n\n", errno);
  errno = 0;

  printf("symlink/normal\n");
  printf("ret: %d\n", symlink("new-nonexistent-path", "folder/link"));
  printf("errno: %d\n", errno);
  errno = 0;

  printf("readlink(created link)\n");
  int ret = readlink("folder/link", buffer, 256);
  printf("errno: %d\n", errno);
#ifdef WASMFS
  assert(strcmp(buffer, "new-nonexistent-path") == 0);
#else
  assert(strcmp(buffer, "/working/folder/new-nonexistent-path") == 0);
#endif
  assert(strlen(buffer) == ret);
  errno = 0;
  printf("\n");

  buffer[0] = buffer[1] = buffer[2] = buffer[3] = buffer[4] = buffer[5] = '*';
  printf("readlink(short buffer)\n");
  printf("ret: %zd\n", readlink("link", buffer, 4));
  printf("errno: %d\n", errno);
#ifdef WASMFS
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
  symlink("./linkX/inside","./linkX");
  int result = access("linkX", F_OK);
  assert(result == -1);
  printf("errno: %d\n", errno);
  assert(errno == ELOOP);
  errno = 0;

  return 0;
}
