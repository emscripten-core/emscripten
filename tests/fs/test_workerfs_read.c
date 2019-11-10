/*
 * Copyright 2015 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <emscripten.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>

#define SECRET_LEN 10

int result = 1;

int main() {
  int fd;
  int fd2;
  struct stat st;
  struct stat st2;
  char buf[100];
  char secret2[] = SECRET2;
  int len2 = SECRET_LEN / 2;

  if (stat("/work/notexist.txt", &st) != -1 || errno != ENOENT) {
    result = -1000 - errno;
    goto exit;
  }

  if (stat("/work/blob.txt", &st) != 0) {
    result = -2000 - errno;
    goto exit;
  }

  fd = open("/work/blob.txt", O_RDWR, 0666);
  if (fd == -1) {
    result = -3000 - errno;
    goto exit;
  }

  if (read(fd, buf, 1000) != SECRET_LEN ||
      strncmp(buf, SECRET, SECRET_LEN) != 0) {
    result = -4000 - errno;
    goto exit;
  }

  fd2 = open("/work/file.txt", O_RDONLY, 0666);
  if (fd2 == -1) {
    result = -5000 - errno;
    goto exit;
  }

  if (lseek(fd2, len2, SEEK_SET) != len2) {
    result = -6000 - errno;
    goto exit;
  }

  if (read(fd2, buf, len2) != len2 ||
      strncmp(buf, secret2 + len2, len2) != 0) {
    result = -7000 - errno;
    goto exit;
  }

  stat("/work/file.txt", &st);
  chmod("/work/file.txt", 0640);
  stat("/work/file.txt", &st2);
  if (st.st_mode != (0777 | S_IFREG) || st2.st_mode != (0640 | S_IFREG)) {
    result = -8000 - errno;
    goto exit;
  }
 
  DIR *pDir = opendir("/work/");
  if (pDir == NULL) {
    result = -9000 - errno;
    goto exit;
  }
  
  int blobFileExists = 0;
  int fileTxtExists = 0;
  struct dirent *pDirent;
  while ((pDirent = readdir(pDir)) != NULL) {
    if (strcmp(pDirent->d_name, "blob.txt") == 0) {
       blobFileExists = 1;
    }   
    if (strcmp(pDirent->d_name, "file.txt") == 0) {
       fileTxtExists = 1;
    }
 
  }
  
  if (blobFileExists == 0) {
    result = -10000-errno;
    goto exit;
  }
  
  if (fileTxtExists == 0) {
    result = -20000-errno;
    goto exit;
  }


exit:
  REPORT_RESULT(result);
}
