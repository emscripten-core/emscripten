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
#include <string.h>
#include <sys/uio.h>
#include <emscripten.h>

int main() {
  EM_ASM(
    FS.mkdir('/working');
#if NODEFS
    FS.mount(NODEFS, { root: '.' }, '/working');
#endif

    var major = 80;

    var device = FS.makedev(major++, 0);
    FS.registerDevice(device, {
      open: function(stream) {
        stream.payload = [65, 66, 67, 68];
      },
      read: function(stream, buffer, offset, length, pos) {
        var bytesRead = 0;
        for (var i = 0; i < length; i++) {
          if (stream.payload.length) {
            bytesRead++;
            buffer[offset+i] = stream.payload.shift();
          } else {
            break;
          }
        }
        return bytesRead;
      },
      write: function(stream, buffer, offset, length, pos) {
        for (var i = 0; i < length; i++) {
          out('TO DEVICE: ' + buffer[offset+i]);
        }
        return i;
      }
    });
    FS.mkdev('/device', device);

    var broken_device = FS.makedev(major++, 0);
    FS.registerDevice(broken_device, {
      read: function(stream, buffer, offset, length, pos) {
        throw new FS.ErrnoError(ERRNO_CODES.EIO);
      },
      write: function(stream, buffer, offset, length, pos) {
        throw new FS.ErrnoError(ERRNO_CODES.EIO);
      }
    });
    FS.mkdev('/broken-device', broken_device);

    // NB: These are meant to test FS.createDevice specifically,
    //     and as such do not use registerDevice/mkdev
    FS.createDevice('/', 'createDevice-read-only', function() {});
    FS.createDevice('/', 'createDevice-write-only', null, function() {});

    FS.mkdir('/working/folder');
    FS.writeFile('/working/file', '1234567890');
  );

  char readBuffer[256] = {0};
  char writeBuffer[] = "writeme";

  int fl = open("/working/folder", O_RDWR);
  printf("read from folder: %zd\n", read(fl, readBuffer, sizeof readBuffer));
  printf("errno: %d\n", errno);
  errno = 0;
  printf("write to folder: %zd\n", write(fl, writeBuffer, sizeof writeBuffer));
  printf("errno: %d\n\n", errno);
  errno = 0;

  int bd = open("/broken-device", O_RDWR);
  printf("read from broken device: %zd\n", read(bd, readBuffer, sizeof readBuffer));
  printf("errno: %d\n", errno);
  errno = 0;
  printf("write to broken device: %zd\n", write(bd, writeBuffer, sizeof writeBuffer));
  printf("errno: %d\n\n", errno);
  errno = 0;

  int d = open("/device", O_RDWR);
  printf("read from device: %zd\n", read(d, readBuffer, sizeof readBuffer));
  printf("data: %s\n", readBuffer);
  memset(readBuffer, 0, sizeof readBuffer);
  printf("errno: %d\n", errno);
  errno = 0;
  printf("write to device: %zd\n", write(d, writeBuffer, sizeof writeBuffer));
  printf("errno: %d\n\n", errno);
  errno = 0;

  int cd_ro_r = open("/createDevice-read-only", O_RDONLY);
  printf("open read-only device from createDevice for read, errno: %d\n", errno);
  errno = 0;
  int cd_ro_w = open("/createDevice-read-only", O_WRONLY);
  printf("open read-only device from createDevice for write, errno: %d\n", errno);
  errno = 0;
  int cd_wo_r = open("/createDevice-write-only", O_RDONLY);
  printf("open write-only device from createDevice for read, errno: %d\n", errno);
  errno = 0;
  int cd_wo_w = open("/createDevice-write-only", O_WRONLY);
  printf("open write-only device from createDevice for write, errno: %d\n\n", errno);
  errno = 0;

  int f = open("/working/file", O_RDWR);
  printf("read from file: %zd\n", read(f, readBuffer, sizeof readBuffer));
  printf("data: %s\n", readBuffer);
  memset(readBuffer, 0, sizeof readBuffer);
  printf("errno: %d\n\n", errno);
  errno = 0;

  printf("pread past end of file: %zd\n", pread(f, readBuffer, sizeof readBuffer, 999999999));
  printf("data: %s\n", readBuffer);
  memset(readBuffer, 0, sizeof readBuffer);
  printf("errno: %d\n\n", errno);
  errno = 0;

  printf("seek: %lld\n", lseek(f, 3, SEEK_SET));
  printf("errno: %d\n\n", errno);
  printf("partial read from file: %zd\n", read(f, readBuffer, 3));
  printf("data: %s\n", readBuffer);
  memset(readBuffer, 0, sizeof readBuffer);
  printf("errno: %d\n\n", errno);
  errno = 0;

  printf("seek: %lld\n", lseek(f, -2, SEEK_END));
  printf("errno: %d\n", errno);
  errno = 0;
  printf("partial read from end of file: %zd\n", read(f, readBuffer, 3));
  printf("data: %s\n", readBuffer);
  memset(readBuffer, 0, sizeof readBuffer);
  printf("errno: %d\n\n", errno);
  errno = 0;

  printf("seek: %lld\n", lseek(f, -15, SEEK_CUR));
  printf("errno: %d\n", errno);
  errno = 0;
  printf("partial read from before start of file: %zd\n", read(f, readBuffer, 3));
  printf("data: %s\n", readBuffer);
  memset(readBuffer, 0, sizeof readBuffer);
  printf("errno: %d\n\n", errno);
  errno = 0;

  printf("seek: %lld\n", lseek(f, 0, SEEK_SET));
  printf("write to start of file: %zd\n", write(f, writeBuffer, 3));
  printf("errno: %d\n\n", errno);
  errno = 0;

  printf("seek: %lld\n", lseek(f, 0, SEEK_END));
  printf("write to end of file: %zd\n", write(f, writeBuffer, 3));
  printf("errno: %d\n\n", errno);
  errno = 0;

  printf("seek: %lld\n", lseek(f, 10, SEEK_END));
  printf("write after end of file: %zd\n", write(f, writeBuffer, sizeof writeBuffer));
  printf("errno: %d\n\n", errno);
  errno = 0;

  printf("pwrite to the middle of file: %zd\n", pwrite(f, writeBuffer + 2, 3, 17));
  printf("errno: %d\n", errno);
  printf("seek: %lld\n\n", lseek(f, 0, SEEK_CUR));
  errno = 0;

  printf("pwrite past end of file: %zd\n", pwrite(f, writeBuffer, 5, 32));
  printf("errno: %d\n", errno);
  printf("seek: %lld\n\n", lseek(f, 0, SEEK_CUR));
  errno = 0;

  ssize_t bytesRead;
  printf("seek: %lld\n", lseek(f, 0, SEEK_SET));
  printf("read after write: %zd\n", bytesRead = read(f, readBuffer, sizeof readBuffer));
  printf("errno: %d\n", errno);
  errno = 0;
  printf("final: ");
  for (ssize_t i = 0; i < bytesRead; i++) {
    if (readBuffer[i] == 0) {
      printf("\\0");
    } else {
      printf("%c", readBuffer[i]);
    }
  }
  printf("\n");

  // readv
  printf("\n");
  memset(readBuffer, 0, sizeof readBuffer);
  struct iovec iov;
  iov.iov_base = readBuffer;
  iov.iov_len = sizeof readBuffer;
  printf("seek: %lld\n", lseek(f, 0, SEEK_SET));
  printf("read after write: %zd\n", bytesRead = readv(f, &iov, 1));
  printf("errno: %d\n", errno);
  errno = 0;
  printf("final: ");
  for (ssize_t i = 0; i < bytesRead; i++) {
    if (readBuffer[i] == 0) {
      printf("\\0");
    } else {
      printf("%c", readBuffer[i]);
    }
  }
  printf("\n");

#ifdef REPORT_RESULT
  REPORT_RESULT(0);
#endif

  return 0;
}
