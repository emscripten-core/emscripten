#include <stdio.h>
#include <emscripten.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>

int result = 1;

void success()
{
  REPORT_RESULT();
}

void test_open_close() {
  // In this test, open and close an empty file will invoke mknod() indirectly
  // and close() directly, both should trigger deferredSyncFs but we should only
  // ended up calling syncfs once.
  struct stat st;
  int fd;

  if ((stat("/working1/empty.txt", &st) != -1) || (errno != ENOENT))
    result = -1000 - errno;
  fd = open("/working1/empty.txt", O_RDWR | O_CREAT, 0666);
  if (fd == -1)
    result = -2000 - errno;
  else if (close(fd) != 0)
    result = -3000 - errno;
}

void test_open_close2() {
  // In this test, open and close an existing empty file will invoke close()
  // directly but not mknod(), therefore assert close() alone will eventually
  // trigger syncfs.
  struct stat st;
  int fd;

  fd = open("/working1/empty.txt", O_RDWR | O_CREAT, 0666);
  if (fd == -1)
    result = -4000 - errno;
  else if (close(fd) != 0)
    result = -5000 - errno;
}

void test_fopen_fclose() {
  // In this test, fopen and fclose an empty file will invoke mknod() indirectly
  // and close() indirectly, and again both should trigger deferredSyncFs but
  // we should only ended up calling syncfs once.
  struct stat st;

  if ((stat("/working1/empty2.txt", &st) != -1) || (errno != ENOENT))
    result = -6000 - errno;

  FILE *fp;
  int res;

  fp = fopen("/working1/empty2.txt", "wb+");
  fclose(fp);
}

void test_rename() {
  // We would also like to make sure rename() will trigger syncfs.
  if (rename("/working1/empty2.txt", "/working1/empty3.txt") != 0)
    result = -7000 - errno;
}

void test_unlink() {
  // In this test, unlink() two files will invoke deferredSyncFs twice but
  // again, we should only ended up calling syncfs once.

  if (unlink("/working1/empty.txt") != 0)
    result = -8000 - errno;

  if (unlink("/working1/empty3.txt") != 0)
    result = -9000 - errno;
}

int main() {

  EM_ASM(
    var i = 0;

    // Set the timer to 0 to trigger syncfs at next tick.
    var timeout = IDBFS.DEFER_WRITE_TIMEOUT;
    IDBFS.DEFER_WRITE_TIMEOUT = 0;

    // Wrap IDBFS.syncfs so we could tell when it is invoked (and completes).
    IDBFS.realSyncFS = IDBFS.syncfs;
    IDBFS.syncfs = function(mount, populate, callback) {
      IDBFS.realSyncFS(mount, populate, function(err) {
        callback(err);

        assert(!err);
        switch (i) {
          case 0:
            ccall('test_open_close2', 'v');
            break;

          case 1:
            ccall('test_fopen_fclose', 'v');
            break;

          case 2:
            ccall('test_rename', 'v');
            break;

          case 3:
            ccall('test_unlink', 'v');
            break;

          case 4:
            IDBFS.syncfs = IDBFS.realSyncFS;
            delete IDBFS.realSyncFS;
            IDBFS.AUTOMATIC_SYNC_TIMEOUT = timeout;

            ccall('success', 'v');
            break;

          default:
            assert(false, 'Should not reach here.');
        }
        i++;

      });
    };

    FS.mkdir('/working1');
    FS.mount(IDBFS, {}, '/working1');
  );

  // Start the first test.
  test_open_close();

  emscripten_exit_with_live_runtime();

  return 0;
}
