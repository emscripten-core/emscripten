/*
 * Copyright 2011 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <emscripten.h>
#include "pthreadfs.h"

void setup() {
  EM_PTHREADFS_ASM(
    await PThreadFS.mkdir('working');
    await PThreadFS.chdir('working');
    await PThreadFS.writeFile('towrite', 'abcdef');
    await PThreadFS.writeFile('toread', 'abcdef');
    await PThreadFS.chmod('toread', 0o444);
  );
}

int main() {
  emscripten_init_pthreadfs();
  setup();

  struct stat s;
  int f = open("towrite", O_WRONLY);
  int f2 = open("toread", O_RDONLY);
  printf("f2: %d\n", f2);

  fstat(f, &s);
  printf("st_size: %lld\n", s.st_size);
  memset(&s, 0, sizeof s);
  errno = 0;
  printf("\n");

  printf("ftruncate(10): %d\n", ftruncate(f, 10));
  printf("errno: %s\n", strerror(errno));
  fstat(f, &s);
  printf("st_size: %lld\n", s.st_size);
  memset(&s, 0, sizeof s);
  errno = 0;
  printf("\n");

  printf("ftruncate(4): %d\n", ftruncate(f, 4));
  printf("errno: %s\n", strerror(errno));
  fstat(f, &s);
  printf("st_size: %lld\n", s.st_size);
  memset(&s, 0, sizeof s);
  errno = 0;
  printf("\n");

  printf("ftruncate(-1): %d\n", ftruncate(f, -1));
  printf("errno: %s\n", strerror(errno));
  fstat(f, &s);
  printf("st_size: %lld\n", s.st_size);
  memset(&s, 0, sizeof s);
  errno = 0;
  printf("\n");

  printf("truncate(2): %d\n", truncate("towrite", 2));
  printf("errno: %s\n", strerror(errno));
  stat("towrite", &s);
  printf("st_size: %lld\n", s.st_size);
  memset(&s, 0, sizeof s);
  errno = 0;
  printf("\n");

  printf("truncate(readonly, 2): %d\n", truncate("toread", 2));
  printf("errno: %s\n", strerror(errno));
  stat("toread", &s);
  printf("st_size: %lld\n", s.st_size);
  memset(&s, 0, sizeof s);
  errno = 0;
  printf("\n");

  printf("ftruncate(readonly, 4): %d\n", ftruncate(f2, 4));
  printf("errno: %s\n", strerror(errno));
  fstat(f2, &s);
  printf("st_size: %lld\n", s.st_size);
  memset(&s, 0, sizeof s);
  errno = 0;

  // Restore full permissions on all created files so that python test runner rmtree
  // won't have problems on deleting the files. On Windows, calling shutil.rmtree()
  // will fail if any of the files are read-only.
  EM_PTHREADFS_ASM(
    await PThreadFS.chmod('toread', 0o777);
  );
  return 0;
}
