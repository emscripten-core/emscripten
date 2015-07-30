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

int main() {
  int fd;

#if FIRST

  // We first make sure the file doesn't currently exist
  // (we delete it at the end of !FIRST). We then write a file,
  // call fsync, and close the file.

  struct stat st;

  // a file whose contents are just 'az'
  if ((stat("/working1/wakaka.txt", &st) != -1) || (errno != ENOENT))
    result = -1000 - errno;
  fd = open("/working1/wakaka.txt", O_RDWR | O_CREAT, 0666);
  if (fd == -1)
    result = -2000 - errno;
  else
  {
    if (write(fd,"az",2) != 2)
      result = -3000 - errno;

    if (fsync(fd) != 0)
      result = -4000 - errno;

    if (close(fd) != 0)
      result = -5000 - errno;
  }

  REPORT_RESULT();

#else

  // does the 'az' file exist, and does it contain 'az'?
  fd = open("/working1/wakaka.txt", O_RDONLY);
  if (fd == -1)
    result = -6000 - errno;
  else
  {
    char bf[4];
    int bytes_read = read(fd,&bf[0],sizeof(bf));
    if (bytes_read != 2)
      result = -7000;
    else if ((bf[0] != 'a') || (bf[1] != 'z'))
      result = -8000;
    if (close(fd) != 0)
      result = -9000 - errno;
    if (unlink("/working1/wakaka.txt") != 0)
      result = -10000 - errno;
  }

  // sync from memory state to persisted and then
  // run 'success'
  EM_ASM(
    FS.syncfs(function (err) {
      assert(!err);
      ccall('success', 'v');
    });
  );

  emscripten_exit_with_live_runtime();

  return 0;

#endif
}
