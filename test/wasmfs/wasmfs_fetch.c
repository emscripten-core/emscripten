/*
 * Copyright 2021 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <assert.h>
#include <dirent.h>
#include <emscripten/emscripten.h>
#include <emscripten/wasmfs.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

// NOTE: Each fetch backend runs in a separate thread. When not using
//       PROXY_TO_PTHREAD, that means we need a pool of at least one thread per
//       backend, so updating test_browser.py may be needed when adding more
//       here.

void getUrlOrigin(char* ptr, int len);
char url_orig[256] = {};

void check_file(int fd, const char* content) {
  assert(fd >= 0);
  struct stat st;
  assert(fstat(fd, &st) != -1);
  printf("file size: %lld\n", st.st_size);
  assert(st.st_size > 0);

  char buf[st.st_size];
  int bytes_read = read(fd, buf, st.st_size);
  printf("read size: %d\n", bytes_read);
  buf[bytes_read] = 0;
  printf("'%s'\n", buf);
  assert(strcmp(buf, content) == 0);
}

void test_url_relative() {
  printf("Running %s...\n", __FUNCTION__);

  backend_t backend2 = wasmfs_create_fetch_backend("test.txt");
  int fd = wasmfs_create_file("/file_rel", 0777, backend2);
  check_file(fd, "fetch 2");
  assert(close(fd) == 0);
}

void test_url_absolute() {
  printf("Running %s...\n", __FUNCTION__);

  assert(strlen(url_orig) != 0);
  const char* file_name = "/test.txt";
  char url[200];
  snprintf(url, sizeof(url), "%s%s", url_orig, file_name);

  backend_t backend = wasmfs_create_fetch_backend(url);
  int fd = wasmfs_create_file(file_name, 0777, backend);
  check_file(fd, "fetch 2");
  assert(close(fd) == 0);
}

void test_directory_abs() {
  printf("Running %s...\n", __FUNCTION__);

  const char* dir_path = "/subdir";
  char url[200];
  snprintf(url, sizeof(url), "%s%s", url_orig, dir_path);

  backend_t backend = wasmfs_create_fetch_backend(url);
  int res = wasmfs_create_directory(dir_path, 0777, backend);
  if (errno)
    perror("wasmfs_create_directory");
  assert(errno == 0);
  assert(res == 0);

  int fd = wasmfs_create_file("/subdir/backendfile", 0777, backend);
  int fd2 = wasmfs_create_file("/subdir/backendfile2", 0777, backend);

  printf("readdir: %s\n", dir_path);
  struct dirent* entry;
  DIR* dir = opendir(dir_path);
  assert(dir);
  int n = 0;
  while ((entry = readdir(dir)) != NULL) {
    printf("  %s\n", entry->d_name);
    ++n;
  }
  assert(n == 4);
  closedir(dir);

  check_file(fd, "file 1");
  check_file(fd2, "file 2");

  assert(close(fd) == 0);
  assert(close(fd2) == 0);
}

void test_default() {
  printf("Running %s...\n", __FUNCTION__);
  backend_t backend = wasmfs_create_fetch_backend("data.dat");

  // Create a file in that backend.
  int fd = wasmfs_create_file("/testfile", 0777, backend);

  // Get the size of the file. This will cause a transparent fetch of the data,
  // after which the size is > 0
  struct stat file;
  assert(fstat(fd, &file) != -1);
  printf("file size: %lld\n", file.st_size);
  assert(file.st_size > 0);

  // Create a second file.
  int fd2 = wasmfs_create_file("/testfile2", 0777, backend);

  // Read the data from the file. As before, when we need the data it is
  // transparently fetched for us.
  char buf[file.st_size];
  int bytes_read = read(fd, buf, file.st_size);
  printf("read size: %d\n", bytes_read);
  assert(errno == 0);
  buf[bytes_read] = 0;
  printf("%s\n", buf);
  assert(strcmp(buf, "hello, fetch") == 0);

  assert(close(fd) == 0);
  assert(close(fd2) == 0);
}

void test_small_reads() {
  // Read the file in small amounts.
  printf("Running %s...\n", __FUNCTION__);

  char expected[] = "hello";
  size_t size = 5;

  backend_t backend = wasmfs_create_fetch_backend("small.dat");
  int fd = wasmfs_create_file("/testfile3", 0777, backend);
  char buf[size + 1];
  for (size_t i = 0; i < size; i++) {
    int read_now = read(fd, buf + i, 1);
    assert(read_now == 1);
    printf("read one byte\n");
  }
  buf[size] = 0;
  assert(strcmp(buf, "hello") == 0);

  assert(close(fd) == 0);
}

void test_nonexistent() {
  printf("Running %s...\n", __FUNCTION__);

  const char* dir_path = "/subdir";
  char url[200];
  snprintf(url, sizeof(url), "%s%s", url_orig, dir_path);

  backend_t backend = wasmfs_get_backend_by_path(url);

  const char* file_name = "/subdir/nonexistent.txt";
  int fd = wasmfs_create_file(file_name, 0777, backend);

  struct stat file;
  assert(fstat(fd, &file) != -1);
  printf("file size: %lld\n", file.st_size);
  assert(file.st_size == 0);

  errno = 0;
  char buf[1];
  int bytes_read = read(fd, buf, sizeof(buf));
  printf("Bytes read: %d\n", bytes_read);
  assert(bytes_read == -1);
  printf("Errno: %s\n", strerror(errno));
  assert(errno == ENOENT);

  assert(close(fd) == 0);
}

int main() {
  getUrlOrigin(url_orig, sizeof(url_orig));
  test_default();
  test_url_relative();
  test_url_absolute();
  test_directory_abs();
  test_small_reads();
  test_nonexistent();

  return 0;
}
