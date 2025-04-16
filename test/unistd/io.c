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

EM_JS_DEPS(main, "$ERRNO_CODES");

int main() {
  EM_ASM(
    FS.mkdir('/working');
#if NODEFS
    FS.mount(NODEFS, { root: '.' }, '/working');
#endif

    var major = 80;

    var device = FS.makedev(major++, 0);
    FS.registerDevice(device, {
      read: function(stream, buffer, offset, length, pos) {
        if (!stream.payload) {
          stream.payload = [65, 66, 67, 68];
        }
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

  // This part of the test has moved to wasmfs/wasmfs_seek.c

  return 0;
}
