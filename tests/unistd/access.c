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

int main() {
#ifdef __EMSCRIPTEN_ASMFS__
  mkdir("working", 0777);
  chdir("working");
  close(open("forbidden", O_WRONLY|O_CREAT|O_TRUNC|O_EXCL, 0000));
  close(open("readable", O_WRONLY|O_CREAT|O_TRUNC|O_EXCL, 0444));
  close(open("writeable", O_WRONLY|O_CREAT|O_TRUNC|O_EXCL, 0222));
  close(open("allaccess", O_WRONLY|O_CREAT|O_TRUNC|O_EXCL, 0777));
#else
  EM_ASM(
    FS.mkdir('working');
#if NODEFS
    FS.mount(NODEFS, { root: '.' }, 'working');
#endif
    FS.chdir('working');
    FS.writeFile('forbidden', ''); FS.chmod('forbidden', 0000);
    FS.writeFile('readable',  ''); FS.chmod('readable',  0444);
    FS.writeFile('writeable', ''); FS.chmod('writeable', 0222);
    FS.writeFile('allaccess', ''); FS.chmod('allaccess', 0777);
  );
#endif
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

#ifndef __EMSCRIPTEN_ASMFS__
  // Restore full permissions on all created files so that python test runner rmtree
  // won't have problems on deleting the files. On Windows, calling shutil.rmtree()
  // will fail if any of the files are read-only.
  EM_ASM(
    FS.chmod('forbidden', 0777);
    FS.chmod('readable',  0777);
    FS.chmod('writeable', 0777);
    FS.chmod('allaccess', 0777);
  );
#endif

#ifdef REPORT_RESULT
  REPORT_RESULT(0);
#endif

  return 0;
}
