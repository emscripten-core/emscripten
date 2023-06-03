/*
 * Copyright 2023 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <assert.h>
#include <emscripten/emscripten.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>

int main() {
  EM_ASM(
    // Create multiple directories
    FS.mkdir('/dir1');
    FS.mkdir('/dir2');
  );

  struct stat s;
  stat("/dir1", &s);
  assert(S_ISDIR(s.st_mode));
  stat("/dir2", &s);
  assert(S_ISDIR(s.st_mode));


  EM_ASM(
    // Remove the multiple directories
    FS.rmdir('/dir1');
    FS.rmdir('/dir2');
  );

  int err = open("/dir1", O_RDWR);
  assert(err);
  err = open("/dir2", O_RDWR);
  assert(err);

  EM_ASM(    
    // Create a directory with a file inside it
    FS.mkdir('/test_dir');
    FS.writeFile('/test_dir/file.txt', 'Hello World!');

    // Attempt to remove the directory (should fail)
    var ex;
    try {
      FS.rmdir('/test_dir');
    } catch (err) {
      ex = err;
    }
    assert(ex.name === "ErrnoError" && ex.errno === 55 /* ENOTEMPTY */);

    // Remove the file and then the directory
    FS.unlink('/test_dir/file.txt');
    FS.rmdir('/test_dir');

    // Attempt to remove a non-existent directory (should fail)
    try {
      FS.rmdir('/non_existent_dir');
    } catch (err) {
      ex = err;
    }
    assert(ex.name === "ErrnoError" && ex.errno === 44 /* ENOEN */);
  );
  puts("success");

  return 0;
}
