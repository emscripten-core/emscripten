/*
 * Copyright 2025 The Emscripten Authors.  All rights reserved.
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

EM_JS_DEPS(deps, "$callUserCallback");

enum {
  TEST_CASE_OPEN,
  TEST_CASE_CLOSE,
  TEST_CASE_SYMLINK,
  TEST_CASE_UNLINK,
  TEST_CASE_RENAME,
  TEST_CASE_MKDIR,
};

static void test_case_open(void) {
  switch (TEST_PHASE) {
    case 1: {
      int fd = open("/working1/file", O_RDWR | O_CREAT | O_EXCL, 0777);
      assert(fd != -1);
      break;
    }
    case 2: {
      struct stat st;
      int res = lstat("/working1/file", &st);
      assert(res == 0);
      assert(st.st_size == 0);
      break;
    }
    default:
      assert(false);
  }
}

static void test_case_close(void) {
  switch (TEST_PHASE) {
    case 1: {
      int fd = open("/working1/file", O_RDWR | O_CREAT | O_EXCL, 0777);
      assert(fd != -1);
      break;
    }
    case 2: {
      int fd = open("/working1/file", O_RDWR | O_CREAT, 0777);
      assert(fd != -1);
      ssize_t bytes_written = write(fd, "foo", 3);
      assert(bytes_written == 3);
      int res = close(fd);
      assert(res == 0);
      break;
    }
    case 3: {
      struct stat st;
      int res = lstat("/working1/file", &st);
      assert(res == 0);
      assert(st.st_size == 3);
      break;
    }
    default:
      assert(false);
  }
}

static void test_case_symlink(void) {
  switch (TEST_PHASE) {
    case 1: {
      int fd = open("/working1/file", O_RDWR | O_CREAT | O_EXCL, 0777);
      assert(fd != -1);
      break;
    }
    case 2: {
      int res = symlink("/working1/file", "/working1/symlink");
      assert(res == 0);
      break;
    }
    case 3: {
      struct stat st;
      int res = lstat("/working1/symlink", &st);
      assert(res == 0);
      break;
    }
    default:
      assert(false);
  }
}

static void test_case_unlink(void) {
  switch (TEST_PHASE) {
    case 1: {
      int fd = open("/working1/file", O_RDWR | O_CREAT | O_EXCL, 0777);
      assert(fd != -1);
      break;
    }
    case 2: {
      int res = unlink("/working1/file");
      assert(res == 0);
      break;
    }
    case 3: {
      struct stat st;
      int res = lstat("/working1/file", &st);
      assert(res == -1);
      assert(errno == ENOENT);
      break;
    }
    default:
      assert(false);
  }
}

static void test_case_rename(void) {
  switch (TEST_PHASE) {
    case 1: {
      int fd = open("/working1/file", O_RDWR | O_CREAT | O_EXCL, 0777);
      assert(fd != -1);
      break;
    }
    case 2: {
      int res = rename("/working1/file", "/working1/file_renamed");
      assert(res == 0);
      break;
    }
    case 3: {
      struct stat st;
      int res = lstat("/working1/file_renamed", &st);
      assert(res == 0);
      res = lstat("/working1/file", &st);
      assert(res == -1);
      assert(errno == ENOENT);
      break;
    }
    default:
      assert(false);
  }
}

static void test_case_mkdir(void) {
  switch (TEST_PHASE) {
    case 1: {
      int res = mkdir("/working1/dir", 0777);
      assert(res == 0);
      break;
    }
    case 2: {
      struct stat st;
      int res = lstat("/working1/dir", &st);
      assert(res == 0);
      break;
    }
    default:
      assert(false);
  }
}

EMSCRIPTEN_KEEPALIVE
void finish(void) {
  emscripten_force_exit(0);
}

EMSCRIPTEN_KEEPALIVE
void test(void) {
  switch (TEST_CASE) {
    case TEST_CASE_OPEN: test_case_open(); break;
    case TEST_CASE_CLOSE: test_case_close(); break;
    case TEST_CASE_SYMLINK: test_case_symlink(); break;
    case TEST_CASE_UNLINK: test_case_unlink(); break;
    case TEST_CASE_RENAME: test_case_rename(); break;
    case TEST_CASE_MKDIR: test_case_mkdir(); break;
    default: assert(false);
  }

  EM_ASM({
    // Wait until IDBFS has persisted before exiting
    runOnceIDBFSIdle(() => {
      callUserCallback(_finish);
    });
  });
}

int main(void) {
  EM_ASM({
    globalThis.runOnceIDBFSIdle = (callback) => {
      const { mount } = FS.lookupPath('/working1').node;
      assert('idbPersistState' in mount, 'mount object must have idbPersistState');
      if (mount.idbPersistState !== 0) {
        // IDBFS hasn't finished persisting. Check again after all pending tasks have executed
        setTimeout(() => runOnceIDBFSIdle(callback), 0);
        return;
      }
      callback();
    };

    FS.mkdir('/working1');
    FS.mount(IDBFS, {
      autoPersist: true
    }, '/working1');
  });

  if (TEST_PHASE == 1) {
    EM_ASM({
      // The first phase of a test case must start from an empty filesystem.
      // Erase persisted state by overwriting the contents of IndexedDB
      // with our empty in-memory filesystem.
      FS.syncfs(false, (err) => {
        assert(!err);
        callUserCallback(_test);
      });
    });
  } else if (TEST_PHASE > 1) {
    EM_ASM({
      // All subsequent phases rely on the effects of phases before them.
      // Load the persisted filesystem from IndexedDB into memory.
      FS.syncfs(true, (err) => {
        assert(!err);

        // FS.syncfs() may run operations on the in-memory filesystem which
        // might trigger IDBFS.queuePersist() calls. These queued calls will
        // also persist modifications made by the test. We want to verify that
        // each operation we test calls IDBFS.queuePersist() on its own, so
        // the interference from FS.syncfs() is unwanted.
        // Wait until the IDBFS mount has been persisted.
        runOnceIDBFSIdle(() => {
          callUserCallback(_test);
        });
      });
    });
  } else {
    assert(false);
  }

  emscripten_exit_with_live_runtime();
  return 0;
}
