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
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

void setup() {
#ifdef __EMSCRIPTEN__
  EM_ASM(
    FS.mkdir('working');
#if NODEFS
    FS.mount(NODEFS, { root: '.' }, 'working');
#endif
    FS.chdir('working');
    FS.writeFile('towrite', 'abcdef');
    FS.writeFile('toread', 'abcdef');
    FS.chmod('toread', 0o444);
  );
#else
  FILE* f = fopen("towrite", "w");
  fwrite("abcdef", 6, 1, f);
  fclose(f);
  f = fopen("toread", "w");
  fwrite("abcdef", 6, 1, f);
  fclose(f);

  chmod("toread", 0444);
#endif
}

int main() {
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
  fstat(f, &s);
  printf("st_size: %lld\n", s.st_size);
  memset(&s, 0, sizeof s);
  errno = 0;
  printf("\n");

  printf("open(O_TRUNC)\n");
  open("towrite", O_WRONLY | O_TRUNC);
  printf("errno: %s\n", strerror(errno));
  fstat(f, &s);
  printf("st_size: %lld\n", s.st_size);
  memset(&s, 0, sizeof s);
  errno = 0;
  printf("\n");

  printf("truncate(readonly, 2): %d\n", truncate("toread", 2));
  printf("errno: %s\n", strerror(errno));
  fstat(f2, &s);
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
  printf("\n");

  printf("open(readonly, O_TRUNC)\n");
  open("toread", O_RDONLY | O_TRUNC);
  printf("errno: %s\n", strerror(errno));
  fstat(f2, &s);
  printf("st_size: %lld\n", s.st_size);
  memset(&s, 0, sizeof s);
  errno = 0;

#ifdef __EMSCRIPTEN__
  // Restore full permissions on all created files so that python test runner rmtree
  // won't have problems on deleting the files. On Windows, calling shutil.rmtree()
  // will fail if any of the files are read-only.
  EM_ASM(
    FS.chmod('toread', 0o777);
  );
#else
  chmod("toread", 0777);
#endif
  return 0;
}
