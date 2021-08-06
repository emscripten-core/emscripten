/*
 * Copyright 2011 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <emscripten.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "pthreadfs.h"

int main() {
  emscripten_init_pthreadfs();
  EM_PTHREADFS_ASM(
    await PThreadFS.mkdir('working');
    await PThreadFS.chdir('working');
    await PThreadFS.writeFile('forbidden', ""); await PThreadFS.chmod('forbidden', 0o000);
    await PThreadFS.writeFile('readable',  ""); await PThreadFS.chmod('readable',  0o444);
    await PThreadFS.writeFile('writeable', ""); await PThreadFS.chmod('writeable', 0o222);
    await PThreadFS.writeFile('allaccess', ""); await PThreadFS.chmod('allaccess', 0o777);
  );
  // Empty path checks #9136 fix
  char* files[] = {"readable", "writeable",
                   "allaccess", "forbidden", "nonexistent", ""};
  for (int i = 0; i < sizeof files / sizeof files[0]; i++) {
    printf("F_OK(%s): %d\n", files[i], access(files[i], F_OK));
    printf("errno: %d\n", errno);
    errno = 0;
    printf("R_OK(%s): %d\n", files[i], access(files[i], R_OK));
    printf("errno: %d\n", errno);
    errno = 0;
    printf("X_OK(%s): %d\n", files[i], access(files[i], X_OK));
    printf("errno: %d\n", errno);
    errno = 0;
    printf("W_OK(%s): %d\n", files[i], access(files[i], W_OK));
    printf("errno: %d\n", errno);
    errno = 0;
    printf("\n");
  }

  EM_PTHREADFS_ASM(
    await PThreadFS.writeFile('filetorename',  'renametest');
  );
  
  rename("filetorename", "renamedfile");

  errno = 0;
  printf("F_OK(%s): %d\n", "filetorename", access("filetorename", F_OK));
  printf("errno: %d\n", errno);
  errno = 0;
  printf("F_OK(%s): %d\n", "renamedfile", access("renamedfile", F_OK));
  printf("errno: %d\n", errno);
  return 0;
}
