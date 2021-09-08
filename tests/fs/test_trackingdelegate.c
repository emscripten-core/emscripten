/*
 * Copyright 2014 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <stdlib.h>
#include <emscripten.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int main() {

  EM_ASM(
    var O_RDONLY = 0;
    var O_WRONLY = 1;
    var O_RDWR = 2;
    var O_CREAT = 64;
    var O_EXCL = 128;
    var O_TRUNC = 512;
    var O_APPEND = 1024;

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
    FS.trackingDelegate['onOpenFile'] = function(path, flags) {
      var stat = FS.stat(path);
      var fileSize = stat.size;
      if (flags & O_CREAT) {
        flags &= O_CREAT;
      } else if (flags & O_TRUNC) {
        flags &= O_TRUNC;
      } else if (flags & O_APPEND) {
        flags &= O_APPEND;
      } else if (flags & O_EXCL) {
        flags &= O_EXCL;
      } else if (flags & O_WRONLY) {
        flags &= O_WRONLY;
      } else if (flags & O_RDWR) {
        flags &= O_RDWR;
      // O_RDONLY = 32768
      } else if (flags === 32768) {
        flags &= O_RDONLY;
      }
      out('Opened "' + path + '" with flags ' + flags + ' and file size ' + fileSize);
    };
    FS.trackingDelegate['onReadFile'] = function(path, bytesRead) {
      out('Read ' + bytesRead + ' bytes from "' + path + '"');
    };
    FS.trackingDelegate['onWriteToFile'] = function(path, bytesWritten) {
      out('Wrote to file "' + path + '" with ' + bytesWritten + ' bytes written');
    };
    FS.trackingDelegate['onSeekFile'] = function(path, position, whence) {
      out('Seek on "' + path + '" with position ' + position + ' and whence ' + whence);
    };
    FS.trackingDelegate['onCloseFile'] = function(path) {
      out('Closed ' + path);
    };
    FS.trackingDelegate['onMakeDirectory'] = function(path, mode) {
      out('Created directory ' + path + ' with mode ' + mode);
    };
    FS.trackingDelegate['onMakeSymlink'] = function(oldpath, newpath) {
      out('Created symlink from ' + oldpath + ' to ' + newpath);
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

  // Test unistd.h functions
  int fd = open("/renamed.txt", O_RDONLY);
  close(fd);
  fd = open("/renamed.txt", O_WRONLY);
  close(fd);
  fd = open("/renamed.txt", O_RDWR);
  char test[] = "test";
  write(fd, test, 10);
  lseek(fd, 0, SEEK_SET);
  char output[100];
  read(fd, output, 100);
  printf("read returned %s\n", output);
  close(fd);
  fd = open("/renamed.txt", O_CREAT);
  close(fd);
  fd = open("/renamed.txt", O_EXCL);
  close(fd);
  fd = open("/renamed.txt", O_TRUNC);
  close(fd);
  fd = open("/renamed.txt", O_APPEND);
  close(fd);
  remove("/renamed.txt");
  mkdir("/home/test", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
  symlink("/renamed.txt", "/file.txt");
}
