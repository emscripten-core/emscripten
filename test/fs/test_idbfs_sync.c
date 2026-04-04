/*
 * Copyright 2013 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <emscripten.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>

EM_JS_DEPS(deps, "$callUserCallback");

bool test_complete = false;

EMSCRIPTEN_KEEPALIVE
void finish() {
  printf("finish\n");
  test_complete = true;
  emscripten_force_exit(0);
}

void cleanup() {
  // If the test failed, then delete test files from IndexedDB so that the test
  // runner will not leak test state to subsequent tests that reuse this same
  // file.
  printf("cleaning up test files\n");
  unlink("/working1/empty.txt");
  unlink("/working1/waka.txt");
  unlink("/working1/moar.txt");
  rmdir("/working1/dir");
  EM_ASM(FS.syncfs(function(){})); // And persist deleted changes
}

EMSCRIPTEN_KEEPALIVE
void test() {
  int fd, res;
  struct stat st;

#if FIRST
  printf("running test FIRST half ..\n");

  // Run cleanup first in case a previous test failed half way through.
  cleanup();

  // for each file, we first make sure it doesn't currently exist
  // (we delete it at the end of !FIRST).  We then test an empty
  // file plus two files each with a small amount of content

  // the empty file
  res = stat("/working1/empty.txt", &st);
  assert(res == -1 && errno == ENOENT);

  fd = open("/working1/empty.txt", O_RDWR | O_CREAT, 0666);
  assert(fd != -1);
  res = close(fd);
  assert(res == 0);

  // a file whose contents are just 'az'
  res = stat("/working1/waka.txt", &st);
  assert(res == -1 && errno == ENOENT);
  fd = open("/working1/waka.txt", O_RDWR | O_CREAT, 0666);
  assert(fd != -1);
  res = write(fd, "az", 2);
  assert(res == 2);
  res = close(fd);
  assert(res == 0);

  // a file whose contents are random-ish string set by the test_browser.py file
  res = stat("/working1/moar.txt", &st);
  assert(res == -1 && errno == ENOENT);
  fd = open("/working1/moar.txt", O_RDWR | O_CREAT, 0666);
  assert(fd != -1);
  res = write(fd, SECRET, strlen(SECRET));
  assert(res == strlen(SECRET));
  res = close(fd);
  assert(res == 0);

  // a directory
  res = stat("/working1/dir", &st);
  assert(res == -1 && errno == ENOENT);
  res = mkdir("/working1/dir", 0777);
  assert(res == 0);

#else
  printf("running test SECOND half ..\n");

  // does the empty file exist?
  fd = open("/working1/empty.txt", O_RDONLY);
  assert(fd != -1);
  res = close(fd);
  assert(res == 0);
  res = unlink("/working1/empty.txt");
  assert(res == 0);

  // does the 'az' file exist, and does it contain 'az'?
  fd = open("/working1/waka.txt", O_RDONLY);
  assert(fd != -1);
  {
    char bf[4];
    int bytes_read = read(fd,&bf[0],sizeof(bf));
    assert(bytes_read == 2);
    assert(bf[0] == 'a' && bf[1] == 'z');
  }
  res = close(fd);
  assert(res == 0);
  res = unlink("/working1/waka.txt");
  assert(res == 0);

  // does the random-ish file exist and does it contain SECRET?
  fd = open("/working1/moar.txt", O_RDONLY);
  assert(fd != -1);
  {
    char bf[256];
    int bytes_read = read(fd,&bf[0],sizeof(bf));
    assert(bytes_read == strlen(SECRET));
    bf[strlen(SECRET)] = 0;
    assert(strcmp(bf, SECRET) == 0);
  }
  res = close(fd);
  assert(res == 0);
  res = unlink("/working1/moar.txt");
  assert(res == 0);

  // does the directory exist?
  res = stat("/working1/dir", &st);
  assert(res == 0);
  assert(S_ISDIR(st.st_mode));
  res = rmdir("/working1/dir");
  assert(res == 0);

#endif

  printf("done test ..\n");

#if EXTRA_WORK && !FIRST
  EM_ASM(
    for (var i = 0; i < 100; i++) {
      FS.syncfs(function (err) {
        assert(!err);
        console.log('extra work');
      });
    }
  );
#endif

#ifdef IDBFS_AUTO_PERSIST
  finish();
#else
  // sync from memory state to persisted and then
  // run 'finish'
  EM_ASM({
    // Ensure IndexedDB is closed at exit.
    var orig = Module['onExit'];
    Module['onExit'] = (status) => {
      assert(Object.keys(IDBFS.dbs).length == 0);
      orig(status);
    };
    FS.syncfs((err) => {
      assert(!err);
      callUserCallback(_finish);
    });
    });
#endif
}

int main() {
  EM_ASM(
    FS.mkdir('/working1');
    FS.mount(IDBFS, {
#ifdef IDBFS_AUTO_PERSIST
      autoPersist: true
#endif
    }, '/working1');

#if !FIRST
    // syncfs(true, f) should not break on already-existing directories:
    FS.mkdir('/working1/dir');
#endif

    // sync from persisted state into memory and then
    // run the 'test' function
    FS.syncfs(true, function (err) {
      assert(!err);
      callUserCallback(_test);
    });
  );

  emscripten_exit_with_live_runtime();
  return 0;
}
