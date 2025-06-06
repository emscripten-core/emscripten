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
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

void setup() {
  EM_ASM(
    FS.writeFile('forbidden', ""); FS.chmod('forbidden', 0o000);
    FS.writeFile('readable',  ""); FS.chmod('readable',  0o444);
    FS.writeFile('writeable', ""); FS.chmod('writeable', 0o222);
    FS.writeFile('allaccess', ""); FS.chmod('allaccess', 0o777);
  );
}

void test_rename() {
  EM_ASM({FS.writeFile('filetorename', 'renametest');});

  int rename_ret = rename("filetorename", "renamedfile");
  assert(rename_ret == 0);

  printf("F_OK('%s'): %d\n", "filetorename", access("filetorename", F_OK));
  printf("F_OK('%s'): %d\n", "renamedfile", access("renamedfile", F_OK));

  // Same againt with faccessat
  printf("F_OK('%s'): %d\n", "filetorename", faccessat(AT_FDCWD, "filetorename", F_OK, 0));
  printf("F_OK('%s'): %d\n", "renamedfile", faccessat(AT_FDCWD, "renamedfile", F_OK, 0));
}

void test_fchmod() {
  EM_ASM({FS.writeFile('fchmodtest', "");});
  chmod("fchmodtest", S_IRUGO | S_IWUGO);
  struct stat fileStats;
  stat("fchmodtest", &fileStats);
  int mode = fileStats.st_mode & 0o777;
  // Allow S_IXUGO in addtion to S_IWUGO because on windows
  // we always report the execute bit.
  assert(mode == (S_IRUGO | S_IWUGO) || mode == (S_IRUGO | S_IWUGO | S_IXUGO));

  EM_ASM(
    var fchmodstream = FS.open("fchmodtest", "r");
    FS.fchmod(fchmodstream.fd, 0o777);
  );
  stat("fchmodtest", &fileStats);
  assert((fileStats.st_mode & 0o777) == 0o777);
}

void test_lchmod() {
#if !defined(NODEFS) && !defined(NODERAWFS)
  // Node (and indeed linux) does not support lchmod
  // so skip this part of the test.
  EM_ASM(
    FS.symlink('writeable', 'symlinkfile');
    FS.lchmod('symlinkfile', 0o777);
  );

  struct stat symlinkStats;

  lstat("symlinkfile", &symlinkStats);
  assert((symlinkStats.st_mode & 0777) == 0777);

  struct stat fileStats;
  stat("writeable", &fileStats);
  int mode = fileStats.st_mode & 0777;
  assert(mode == S_IWUGO || mode == (S_IWUGO | S_IXUGO));
#endif
}

void test_chmod_errors() {
  EM_ASM(
    var ex;
    try {
      FS.chmod("nonexistent", 0o777);
    } catch (err) {
      ex = err;
    }
    assert(ex.name === "ErrnoError" && ex.errno === 44 /* ENOENT */);

    try {
      FS.fchmod(99, 0o777);
    } catch (err) {
      ex = err;
    }
    assert(ex.name === "ErrnoError" && ex.errno === 8 /* EBADF */);

    try {
      FS.lchmod("nonexistent", 0o777);
    } catch (err) {
      ex = err;
    }
    assert(ex.name === "ErrnoError" && ex.errno === 44 /* ENOENT */);
  );
}

int main() {
  setup();

  // Empty path checks #9136 fix
  char* files[] = {"readable", "writeable",
                   "allaccess", "forbidden", "nonexistent", ""};
  for (int i = 0; i < sizeof files / sizeof files[0]; i++) {
    printf("F_OK('%s'): %s\n", files[i], access(files[i], F_OK) < 0 ? strerror(errno) : "OK");
    printf("R_OK('%s'): %s\n", files[i], access(files[i], R_OK) < 0 ? strerror(errno) : "OK");
    printf("X_OK('%s'): %s\n", files[i], access(files[i], X_OK) < 0 ? strerror(errno) : "OK");
    printf("W_OK('%s'): %s\n", files[i], access(files[i], W_OK) < 0 ? strerror(errno) : "OK");
    printf("\n");
  }

  test_rename();
  test_fchmod();
  test_lchmod();
  test_chmod_errors();

  return 0;
}
