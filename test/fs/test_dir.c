/*
 * Copyright 2023 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <assert.h>
#include <emscripten/emscripten.h>
#include <stdio.h>

int main() {
  EM_ASM(
    // Create multiple directories
    var err = FS.mkdir('/dir1');
    assert(!err);
    err = FS.mkdir('/dir2');
    assert(!err);

    // Remove the multiple directories
    err = FS.rmdir('/dir1');
    assert(!err);
    err = FS.rmdir('/dir2');
    assert(!err);
        
    // Create a directory with a file inside it
    var err = FS.mkdir('/test_dir');
    assert(!err);
    err = FS.writeFile('/test_dir/file.txt', 'Hello World!');
    assert(!err);

    // Attempt to remove the directory (should fail)
    err = FS.rmdir('/test_dir');
    assert(err);

    // Remove the file and then the directory
    err = FS.unlink('/test_dir/file.txt');
    assert(!err);
    err = FS.rmdir('/test_dir');
    assert(!err);

    // Attempt to remove a non-existent directory (should fail)
    var err = FS.rmdir('/non_existent_dir');
    assert(err);
  );
  puts("success");

  return 0;
}