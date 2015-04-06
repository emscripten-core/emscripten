#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <emscripten.h>

int main() {
  EM_ASM(
    var dummy_device = FS.makedev(64, 0);
    FS.registerDevice(dummy_device, {});
    FS.mkdev('/device', dummy_device);

    FS.mkdir('/folder');
    FS.symlink('/folder', '/link');
    FS.writeFile('/file', '', { mode: 0777 });
  );

  char buffer[256];
  printf("getcwd: %s\n", getcwd(buffer, 256));
  printf("errno: %d\n", errno);
  errno = 0;
  printf("\n");

  printf("chdir(file): %d\n", chdir("/file"));
  printf("errno: %d\n", errno);
  if (!errno) {
    errno = 0;
    printf("getcwd: %s\n", getcwd(buffer, 256));
    printf("errno: %d\n", errno);
  }
  errno = 0;
  printf("\n");

  printf("chdir(device): %d\n", chdir("/device"));
  printf("errno: %d\n", errno);
  if (!errno) {
    errno = 0;
    printf("getcwd: %s\n", getcwd(buffer, 256));
    printf("errno: %d\n", errno);
  }
  errno = 0;
  printf("\n");

  printf("chdir(folder): %d\n", chdir("/folder"));
  printf("errno: %d\n", errno);
  if (!errno) {
    errno = 0;
    printf("getcwd: %s\n", getcwd(buffer, 256));
    printf("errno: %d\n", errno);
  }
  errno = 0;
  printf("\n");

  printf("chdir(nonexistent): %d\n", chdir("/nonexistent"));
  printf("errno: %d\n", errno);
  if (!errno) {
    errno = 0;
    printf("getcwd: %s\n", getcwd(buffer, 256));
    printf("errno: %d\n", errno);
  }
  errno = 0;
  printf("\n");

  printf("chdir(link): %d\n", chdir("/link"));
  printf("errno: %d\n", errno);
  if (!errno) {
    errno = 0;
    printf("getcwd: %s\n", getcwd(buffer, 256));
    printf("errno: %d\n", errno);
  }
  errno = 0;
  printf("\n");

  errno = 0;
  printf("fchdir(/): %d\n", fchdir(open("/", O_RDONLY, 0777)));
  printf("errno: %d\n", errno);
  if (!errno) {
    errno = 0;
    printf("getcwd: %s\n", getcwd(buffer, 256));
    printf("errno: %d\n", errno);
    errno = 0;
  }

  return 0;
}
