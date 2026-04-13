#include <assert.h>
#include <emscripten.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

EM_JS_DEPS(main, "$TTY");

// clang-format off
EM_JS(void, init, (void), {
  var major = 100;
  var tty_ops = {
    get_char: function (tty) {
      if (tty.input.length > 0) {
        return tty.input.shift();
      }
      return undefined;
    },
    put_char: function (tty, val) {
      if (val !== 0 && val !== 10) {
        tty.output.push(val);
      }
    },
    fsync: function (tty) {
      console.log('fsync called');
      tty.output = [];
    },
    ioctl_tcgets: function (tty) {
      return {
        c_iflag: 0,
        c_oflag: 0,
        c_cflag: 0,
        c_lflag: 0,
        c_cc: new Array(32).fill(0),
      };
    },
    ioctl_tcsets: function (tty, optional_actions, data) {
      return 0;
    },
    ioctl_tiocgwinsz: function (tty) {
      return [25, 80];
    },
  };
  var device = FS.makedev(major, 0);
  TTY.register(device, tty_ops);
  FS.mkdev('/custom_tty', device);
  // Populate the TTY input buffer with test data "ABCD"
  TTY.ttys[device].input = [65, 66, 67, 68];

  // TTY without get_char - should cause ENXIO on read
  var tty_no_getchar = {put_char: function (tty, val) {}};
  var device_no_getchar = FS.makedev(major + 1, 0);
  TTY.register(device_no_getchar, tty_no_getchar);
  FS.mkdev('/tty_no_getchar', device_no_getchar);

  // TTY without put_char - should cause ENXIO on write
  var tty_no_putchar = {
    get_char: function (tty) {
      return 0;
    },
  };
  var device_no_putchar = FS.makedev(major + 2, 0);
  TTY.register(device_no_putchar, tty_no_putchar);
  FS.mkdev('/tty_no_putchar', device_no_putchar);

  // TTY with throwing get_char - should cause EIO on read
  var tty_throw_getchar = {
    get_char: function (tty) {
      throw new Error('get_char error');
    },
    put_char: function (tty, val) {},
  };
  var device_throw_getchar = FS.makedev(major + 3, 0);
  TTY.register(device_throw_getchar, tty_throw_getchar);
  FS.mkdev('/tty_throw_getchar', device_throw_getchar);

  // TTY with throwing put_char - should cause EIO on write
  var tty_throw_putchar = {
    get_char: function (tty) {
      return 0;
    },
    put_char: function (tty, val) {
      throw new Error('put_char error');
    },
  };
  var device_throw_putchar = FS.makedev(major + 4, 0);
  TTY.register(device_throw_putchar, tty_throw_putchar);
  FS.mkdev('/tty_throw_putchar', device_throw_putchar);

  // TTY with empty input (returns undefined immediately) - should cause EAGAIN on
  // read
  var tty_empty = {
    get_char: function (tty) {
      return undefined;
    },
    put_char: function (tty, val) {},
  };
  var device_empty = FS.makedev(major + 5, 0);
  TTY.register(device_empty, tty_empty);
  FS.mkdev('/tty_empty', device_empty);
});
// clang-format on

int main() {
  init();
  char readBuffer[256] = {0};
  char writeBuffer[] = "Test";
  struct winsize ws;
  struct termios term;

  printf("\nTest 1: open custom TTY device and check isatty\n");
  int fd = open("/custom_tty", O_RDWR);
  assert(fd >= 0);
  printf("isatty: %d\n", isatty(fd));
  printf("errno after open: %s\n", strerror(errno));
  errno = 0;

  printf("\nTest 2: read from TTY with data\n");
  ssize_t bytesRead = read(fd, readBuffer, sizeof(readBuffer));
  printf("read bytes: %zd\n", bytesRead);
  printf("read data: %s\n", bytesRead > 0 ? readBuffer : "(none)");
  printf("errno after read: %s\n", strerror(errno));
  errno = 0;

  printf("\nTest 3: write to TTY\n");
  ssize_t bytesWritten = write(fd, writeBuffer, strlen(writeBuffer));
  printf("write bytes: %zd\n", bytesWritten);
  printf("errno after write: %s\n", strerror(errno));
  errno = 0;

  printf("\nTest 4: ioctl TIOCGWINSZ\n");
  int result = ioctl(fd, TIOCGWINSZ, &ws);
  printf("ioctl TIOCGWINSZ: %d\n", result);
  printf("ws_row: %d ws_col: %d\n", ws.ws_row, ws.ws_col);
  printf("errno after ioctl: %s\n", strerror(errno));
  errno = 0;

  printf("\nTest 5: ioctl TCGETS\n");
  result = ioctl(fd, TCGETS, &term);
  printf("ioctl TCGETS: %d\n", result);
  printf("errno after TCGETS: %s\n", strerror(errno));
  errno = 0;

  printf("\nTest 6: ioctl TCSETS\n");
  result = ioctl(fd, TCSETS, &term);
  printf("ioctl TCSETS: %d\n", result);
  printf("errno after TCSETS: %s\n", strerror(errno));
  errno = 0;

  printf("\nTest 7: fsync\n");
  result = fsync(fd);
  printf("fsync: %d\n", result);
  printf("errno after fsync: %s\n", strerror(errno));
  errno = 0;

  close(fd);

  printf("\nTest 8: no put_char\n");
  fd = open("/tty_no_putchar", O_WRONLY);
  assert(fd >= 0);
  bytesWritten = write(fd, writeBuffer, strlen(writeBuffer));
  printf("write: %zd\n", bytesWritten);
  printf("errno: %s\n", strerror(errno));
  errno = 0;
  close(fd);

  printf("\nTest 9: no get_char\n");
  fd = open("/tty_no_getchar", O_RDONLY);
  assert(fd >= 0);
  bytesRead = read(fd, readBuffer, sizeof(readBuffer));
  printf("read: %zd\n", bytesRead);
  printf("errno: %s\n", strerror(errno));
  errno = 0;
  close(fd);

  printf("\nTest 10: put_char throws\n");
  fd = open("/tty_throw_putchar", O_WRONLY);
  assert(fd >= 0);
  bytesWritten = write(fd, writeBuffer, strlen(writeBuffer));
  printf("write: %zd\n", bytesWritten);
  printf("errno: %s\n", strerror(errno));
  errno = 0;
  close(fd);

  printf("\nTest 11: get_char throws\n");
  fd = open("/tty_throw_getchar", O_RDONLY);
  assert(fd >= 0);
  bytesRead = read(fd, readBuffer, sizeof(readBuffer));
  printf("read: %zd\n", bytesRead);
  printf("errno: %s\n", strerror(errno));
  errno = 0;
  close(fd);

  printf("\nTest 12: get_char returns undefined\n");
  fd = open("/tty_empty", O_RDONLY);
  assert(fd >= 0);
  bytesRead = read(fd, readBuffer, sizeof(readBuffer));
  printf("read: %zd\n", bytesRead);
  printf("errno: %s\n", strerror(errno));
  errno = 0;
  close(fd);

  printf("\ndone\n");
  return 0;
}
