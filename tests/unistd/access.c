/*
 * Copyright 2011 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <assert.h>
#include <emscripten.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

int main() {
  EM_ASM(
    FS.mkdir('working');
#if NODEFS
    FS.mount(NODEFS, { root: '.' }, 'working');
#endif
    FS.chdir('working');
    FS.writeFile('forbidden', ""); FS.chmod('forbidden', 0o000);
    FS.writeFile('readable',  ""); FS.chmod('readable',  0o444);
    FS.writeFile('writeable', ""); FS.chmod('writeable', 0o222);
    FS.writeFile('allaccess', ""); FS.chmod('allaccess', 0o777);
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

  EM_ASM({FS.writeFile('filetorename',  'renametest');});

  int rename_ret = rename("filetorename", "renamedfile");
  assert(rename_ret == 0);

  errno = 0;
  printf("F_OK(%s): %d\n", "filetorename", access("filetorename", F_OK));
  printf("errno: %d\n", errno);
  errno = 0;
  printf("F_OK(%s): %d\n", "renamedfile", access("renamedfile", F_OK));
  printf("errno: %d\n", errno);

  // Same againt with faccessat
  errno = 0;
  printf("F_OK(%s): %d\n", "filetorename", faccessat(AT_FDCWD, "filetorename", F_OK, 0));
  printf("errno: %d\n", errno);
  errno = 0;
  printf("F_OK(%s): %d\n", "renamedfile", faccessat(AT_FDCWD, "renamedfile", F_OK, 0));
  printf("errno: %d\n", errno);

  // Restore full permissions on all created files so that python test runner rmtree
  // won't have problems on deleting the files. On Windows, calling shutil.rmtree()
  // will fail if any of the files are read-only.
  EM_ASM(
    FS.chmod('forbidden', 0o777);
    FS.chmod('readable',  0o777);
    FS.chmod('writeable', 0o777);
    FS.chmod('allaccess', 0o777);
  );

  return 0;
}
