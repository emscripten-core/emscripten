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

void test() {

  int fd;
  
#if FIRST

  // for each file, we first make sure it doesn't currently exist
  // (we delete it at the end of !FIRST).  We then test an empty
  // file plus two files each with a small amount of content

  struct stat st;

  // the empty file
  if ((stat("/working1/empty.txt", &st) != -1) || (errno != ENOENT))
    result = -1000 - errno;
  fd = open("/working1/empty.txt", O_RDWR | O_CREAT, 0666);
  if (fd == -1)
    result = -2000 - errno;
  else if (close(fd) != 0)
    result = -3000 - errno;

  // a file whose contents are just 'az'
  if ((stat("/working1/waka.txt", &st) != -1) || (errno != ENOENT))
    result = -4000 - errno;
  fd = open("/working1/waka.txt", O_RDWR | O_CREAT, 0666);
  if (fd == -1)
    result = -5000 - errno;
  else
  {
    if (write(fd,"az",2) != 2)
      result = -6000 - errno;
    if (close(fd) != 0)
      result = -7000 - errno;
  }
  
  // a file whose contents are random-ish string set by the test_browser.py file
  if ((stat("/working1/moar.txt", &st) != -1) || (errno != ENOENT))
    result = -8000 - errno;
  fd = open("/working1/moar.txt", O_RDWR | O_CREAT, 0666);
  if (fd == -1)
    result = -9000 - errno;
  else
  {
    if (write(fd, SECRET, strlen(SECRET)) != strlen(SECRET))
      result = -10000 - errno;
    if (close(fd) != 0)
      result = -11000 - errno;
  }

#else

  // does the empty file exist?
  fd = open("/working1/empty.txt", O_RDONLY);
  if (fd == -1)
    result = -12000 - errno;
  else if (close(fd) != 0)
    result = -13000 - errno;
  if (unlink("/working1/empty.txt") != 0)
    result = -14000 - errno;

  // does the 'az' file exist, and does it contain 'az'?
  fd = open("/working1/waka.txt", O_RDONLY);
  if (fd == -1)
    result = -15000 - errno;
  else
  {
    char bf[4];
    int bytes_read = read(fd,&bf[0],sizeof(bf));
    if (bytes_read != 2)
      result = -16000;
    else if ((bf[0] != 'a') || (bf[1] != 'z'))
      result = -17000;
    if (close(fd) != 0)
      result = -18000 - errno;
    if (unlink("/working1/waka.txt") != 0)
      result = -19000 - errno;
  }
  
  // does the random-ish file exist and does it contain SECRET?
  fd = open("/working1/moar.txt", O_RDONLY);
  if (fd == -1)
    result = -20000 - errno;
  else
  {
    char bf[256];
    int bytes_read = read(fd,&bf[0],sizeof(bf));
    if (bytes_read != strlen(SECRET))
      result = -21000;
    else
    {
      bf[strlen(SECRET)] = 0;
      if (strcmp(bf,SECRET) != 0)
        result = -22000;
    }
    if (close(fd) != 0)
      result = -23000 - errno;
    if (unlink("/working1/moar.txt") != 0)
      result = -24000 - errno;
  }

#endif

#if EXTRA_WORK
  EM_ASM(
    for (var i = 0; i < 100; i++) {
      FS.syncfs(function (err) {
        assert(!err);
        console.log('extra work');
      });
    }
  );
#endif

  // sync from memory state to persisted and then
  // run 'success'
  EM_ASM(
    FS.syncfs(function (err) {
      assert(!err);
      ccall('success', 'v');
    });
  );

}

int main() {

  EM_ASM(
    FS.mkdir('/working1');
    FS.mount(IDBFS, {}, '/working1');
    
    // sync from persisted state into memory and then
    // run the 'test' function
    FS.syncfs(true, function (err) {
      assert(!err);
      ccall('test', 'v');
    });
  );

  emscripten_exit_with_live_runtime();

  return 0;
}
