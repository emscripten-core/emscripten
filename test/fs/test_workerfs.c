/*
 * Copyright 2015 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <assert.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>


void test_no_exist() {
  struct stat st;
  int rtn = stat("/work/notexist.txt", &st);
  assert(rtn == -1 && errno == ENOENT);
}

void test_blob_txt() {
  // Check that /work/blob.txt contains the expected data.
  const char content[] = "hello blob";
  size_t file_len = sizeof(content) - 1;

  struct stat st;
  char buf[100];
  int rtn = stat("/work/blob.txt", &st);
  assert(rtn == 0);

  int fd = open("/work/blob.txt", O_RDWR, 0666);
  assert(fd >= 0);

  ssize_t cnt = read(fd, buf, 1000);
  printf("read blob.txt: %ld, expecting: %lu\n", cnt, file_len);
  assert(cnt == file_len);
  assert(strcmp(buf, content) == 0);
}

void test_file_txt() {
  // Seek half way through /work/file.txt then verify the data read from that point on.
  const char content[] = "hello file";
  size_t file_len = sizeof(content) - 1;
  char buf[100];
  off_t offset = file_len / 2;

  int fd = open("/work/file.txt", O_RDONLY, 0666);
  assert(fd != -1);

  off_t rtn = lseek(fd, offset, SEEK_SET);
  assert(rtn == offset);

  ssize_t cnt = read(fd, buf, 1000);
  printf("read file.txt: %ld, expecting: %llu\n", cnt, file_len - offset);
  assert(cnt == file_len - offset);
  assert(strncmp(buf, content + offset, file_len - offset) == 0);
}

void test_chmod() {
  struct stat st, st2;
  stat("/work/file.txt", &st);
  chmod("/work/file.txt", 0640);
  stat("/work/file.txt", &st2);
  assert(st.st_mode == (0777 | S_IFREG) && st2.st_mode == (0640 | S_IFREG));
}

void test_readdir() {
  DIR *pDir = opendir("/work/");
  assert(pDir);

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

  assert(blobFileExists);
  assert(fileTxtExists);
}

void test_readlink() {
  // attemping to read a worker node as a link should result in EINVAL
  char buf[100];
  buf[0] = '\0';
  assert(readlink("/work/blob.txt", buf, sizeof(buf)) == -1);
  assert(buf[0] == '\0');
  assert(errno == EINVAL);
}

int main() {
  printf("in main\n");
  test_no_exist();
  test_blob_txt();
  test_file_txt();
  test_chmod();
  test_readdir();
  test_readlink();
  printf("done\n");
  return 0;
}
