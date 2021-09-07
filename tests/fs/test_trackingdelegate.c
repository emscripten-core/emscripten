/*
 * Copyright 2014 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <stdlib.h>
#include <emscripten.h>

int main() {

  EM_ASM(
    FS.trackingDelegate['willMovePath'] = function(oldpath, newpath) {
      out('About to move "' + oldpath + '" to "' + newpath + '"');
    };
    FS.trackingDelegate['onMovePath'] = function(oldpath, newpath) {
      out('Moved "' + oldpath + '" to "' + newpath + '"');
    };
    FS.trackingDelegate['willDeletePath'] = function(path) {
      out('About to delete "' + path + '"');
    };
    FS.trackingDelegate['onDeletePath'] = function(path) {
      out('Deleted "' + path + '"');
    };
    FS.trackingDelegate['onOpenFile'] = function(path, flags, fileSize) {
      out('Opened "' + path + '" with flags ' + flags + ' and size ' + fileSize);
    };
    FS.trackingDelegate['onReadFile'] = function(path, bytesRead) {
      out('Read ' + bytesRead + ' bytes from "' + path + '"');
    };
    FS.trackingDelegate['onWriteToFile'] = function(path, bytesWritten) {
      out('Wrote to file "' + path + '" with ' + bytesWritten + ' bytes written');
    };
    FS.trackingDelegate['onSeekFile'] = function(path, position) {
      out('Seek on "' + path + '" with position ' + position);
    };
  );

  FILE *file;
  char buffer[100];

  file = fopen("/file.txt", "w+");
  fputs("hello", file);
  fwrite(" world", 7, 1, file);
  fseek(file, 0, SEEK_SET);
  fread(buffer, 13, 1, file);
  printf("%s\n", buffer);
  fclose(file);
  rename("/file.txt", "/renamed.txt");
  file = fopen("/renamed.txt", "r");
  char str[256] = {};
  fgets(str, 255, file);
  printf("File read returned '%s'\n", str);
  fclose(file);
  remove("/renamed.txt");
}
