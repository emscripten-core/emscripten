/*
 * Copyright 2011 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <sys/stat.h>

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <emscripten.h>

int main() {
#if !defined(STANDALONE_WASM)
  EM_ASM(
    var dummy_device = FS.makedev(64, 0);
    FS.registerDevice(dummy_device, {});
    FS.mkdev('/device', dummy_device);

    FS.mkdir('/folder');
    FS.symlink('/folder', '/link');
    FS.writeFile('/file', "", { mode: 0o777 });
  );
#else
  {
    int fd = open("device", O_CREAT, 0777);
    assert(fd >= 0);
    close(fd);
  }
  {
    int rv = mkdir("folder", 0777);
    assert(rv == 0);
  }
  {
    int rv = symlink("folder", "link");
    assert(rv == 0);
  }
  {
    int fd = open("file", O_CREAT, 0777);
    assert(fd >= 0);
    close(fd);
  }
#endif

  char buffer[256];

  printf("getcwd: %s\n", getcwd(buffer, 256));
  printf("errno: %s\n", strerror(errno));
  errno = 0;
  printf("\n");

  printf("chdir(file): %d\n", chdir("/file"));
  printf("errno: %s\n", strerror(errno));
  assert(errno != 0);
  errno = 0;
  printf("\n");

  printf("chdir(\"\"): %d\n", chdir(""));
  printf("errno: %s\n", strerror(errno));
  assert(errno != 0);
  errno = 0;
  printf("\n");

  printf("chdir(device): %d\n", chdir("/device"));
  printf("errno: %s\n", strerror(errno));
  assert(errno != 0);
  errno = 0;
  printf("\n");

  printf("chdir(folder): %d\n", chdir("/folder"));
  printf("errno: %s\n", strerror(errno));
  assert(errno == 0);
  printf("getcwd: %s\n", getcwd(buffer, 256));
  printf("\n");

  printf("chdir(nonexistent): %d\n", chdir("/nonexistent"));
  printf("errno: %s\n", strerror(errno));
  assert(errno != 0);
  errno = 0;
  printf("\n");

  printf("chdir(link): %d\n", chdir("/link"));
  printf("errno: %s\n", strerror(errno));
  assert(errno == 0);
  printf("getcwd: %s\n", getcwd(buffer, 256));
  printf("\n");

  errno = 0;
  printf("fchdir(/): %d\n", fchdir(open("/", O_RDONLY, 0777)));
  printf("errno: %s\n", strerror(errno));
  assert(errno == 0);
  printf("getcwd: %s\n", getcwd(buffer, 256));

  return 0;
}
