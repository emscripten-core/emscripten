#include <stdio.h>
#include <errno.h>
#include <unistd.h>

int main() {
  char* files[] = {"/device", "/file", "/file-forbidden", "/noexist"};
  char* folders[] = {"/empty", "/empty-forbidden", "/full"};
  int i;

  for (i = 0; i < sizeof files / sizeof files[0]; i++) {
    printf("access(%s) before: %d\n", files[i], access(files[i], F_OK));
    rmdir(files[i]);
    printf("errno: %d\n", errno);
    errno = 0;
    printf("access(%s) after rmdir: %d\n", files[i], access(files[i], F_OK));
    unlink(files[i]);
    printf("errno: %d\n", errno);
    errno = 0;
    printf("access(%s) after unlink: %d\n\n", files[i], access(files[i], F_OK));
  }

  for (i = 0; i < sizeof folders / sizeof folders[0]; i++) {
    printf("access(%s) before: %d\n", folders[i], access(folders[i], F_OK));
    unlink(folders[i]);
    printf("errno: %d\n", errno);
    errno = 0;
    printf("access(%s) after unlink: %d\n", folders[i], access(folders[i], F_OK));
    rmdir(folders[i]);
    printf("errno: %d\n", errno);
    errno = 0;
    printf("access(%s) after rmdir: %d\n\n", folders[i], access(folders[i], F_OK));
  }

  return 0;
}
