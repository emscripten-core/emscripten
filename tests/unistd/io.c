#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

int main() {
  char readBuffer[256] = {0};
  char writeBuffer[] = "writeme";

  int fl = open("/folder", O_RDWR);
  printf("read from folder: %d\n", read(fl, readBuffer, sizeof readBuffer));
  printf("errno: %d\n", errno);
  errno = 0;
  printf("write to folder: %d\n", write(fl, writeBuffer, sizeof writeBuffer));
  printf("errno: %d\n\n", errno);
  errno = 0;

  int bd = open("/broken-device", O_RDWR);
  printf("read from broken device: %d\n", read(bd, readBuffer, sizeof readBuffer));
  printf("errno: %d\n", errno);
  errno = 0;
  printf("write to broken device: %d\n", write(bd, writeBuffer, sizeof writeBuffer));
  printf("errno: %d\n\n", errno);
  errno = 0;

  int d = open("/device", O_RDWR);
  printf("read from device: %d\n", read(d, readBuffer, sizeof readBuffer));
  printf("data: %s\n", readBuffer);
  memset(readBuffer, 0, sizeof readBuffer);
  printf("errno: %d\n", errno);
  errno = 0;
  printf("write to device: %d\n", write(d, writeBuffer, sizeof writeBuffer));
  printf("errno: %d\n\n", errno);
  errno = 0;

  int f = open("/file", O_RDWR);
  printf("read from file: %d\n", read(f, readBuffer, sizeof readBuffer));
  printf("data: %s\n", readBuffer);
  memset(readBuffer, 0, sizeof readBuffer);
  printf("errno: %d\n\n", errno);
  errno = 0;

  printf("seek: %d\n", lseek(f, 3, SEEK_SET));
  printf("errno: %d\n\n", errno);
  printf("partial read from file: %d\n", read(f, readBuffer, 3));
  printf("data: %s\n", readBuffer);
  memset(readBuffer, 0, sizeof readBuffer);
  printf("errno: %d\n\n", errno);
  errno = 0;

  printf("seek: %d\n", lseek(f, -2, SEEK_END));
  printf("errno: %d\n", errno);
  errno = 0;
  printf("partial read from end of file: %d\n", read(f, readBuffer, 3));
  printf("data: %s\n", readBuffer);
  memset(readBuffer, 0, sizeof readBuffer);
  printf("errno: %d\n\n", errno);
  errno = 0;

  printf("seek: %d\n", lseek(f, -15, SEEK_CUR));
  printf("errno: %d\n", errno);
  errno = 0;
  printf("partial read from before start of file: %d\n", read(f, readBuffer, 3));
  printf("data: %s\n", readBuffer);
  memset(readBuffer, 0, sizeof readBuffer);
  printf("errno: %d\n\n", errno);
  errno = 0;

  printf("seek: %d\n", lseek(f, 0, SEEK_SET));
  printf("write to start of file: %d\n", write(f, writeBuffer, 3));
  printf("errno: %d\n\n", errno);
  errno = 0;

  printf("seek: %d\n", lseek(f, 0, SEEK_END));
  printf("write to end of file: %d\n", write(f, writeBuffer, 3));
  printf("errno: %d\n\n", errno);
  errno = 0;

  printf("seek: %d\n", lseek(f, 10, SEEK_END));
  printf("write after end of file: %d\n", write(f, writeBuffer, sizeof writeBuffer));
  printf("errno: %d\n\n", errno);
  errno = 0;

  int bytesRead;
  printf("seek: %d\n", lseek(f, 0, SEEK_SET));
  printf("read after write: %d\n", bytesRead = read(f, readBuffer, sizeof readBuffer));
  printf("errno: %d\n", errno);
  errno = 0;
  printf("final: ");
  for (int i = 0; i < bytesRead; i++) {
    if (readBuffer[i] == 0) {
      printf("\\0");
    } else {
      printf("%c", readBuffer[i]);
    }
  }
  printf("\n");

  return 0;
}
