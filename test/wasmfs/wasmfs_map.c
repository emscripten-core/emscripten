/*
 * Copyright 2025 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <assert.h>
#include <fcntl.h>
#include <dirent.h>
#include <emscripten/emscripten.h>
#include <emscripten/wasmfs.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

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

void test_manifest() {
  printf("Running %s...\n", __FUNCTION__);
  void *manifest = wasmfs_map_create_manifest();
  wasmfs_map_add_to_manifest(manifest, "/file", "/test.txt");
  wasmfs_map_add_to_manifest(manifest, "/sub/file", "/subdir/test2.txt");
  backend_t backend = wasmfs_create_map_backend(manifest);
  assert(wasmfs_create_directory("/dat", 0777, backend) >= 0);
  int fd = open("/dat/file", O_RDONLY);
  printf("Loaded %d from %s\n",fd,"/dat/file");
  check_file(fd, "mapfs");
  assert(close(fd) == 0);
  int fd2 = open("/dat/sub/file", O_RDONLY);
  check_file(fd2, "mapfs 2");
  assert(close(fd2) == 0);
}

void test_manifest_js() {
  EM_ASM({
      var contents;
      FS.mount(MAPFS, {"manifest":{"/file":"/test.txt", "/sub/file":"/subdir/test2.txt"}}, "/dat2");
      contents = FS.readFile("/dat2/file", {encoding:'utf8'});
      if(contents != "mapfs") {
        throw "Wrong file contents "+contents;
      }
      contents = FS.readFile("/dat2/sub/file", {encoding:'utf8'});
      if(contents != "mapfs 2") {
        throw "Wrong file contents "+contents;
      }
    });
}

void test_nonexistent() {
  printf("Running %s...\n", __FUNCTION__);
  backend_t backend = wasmfs_get_backend_by_path("/dat/sub");
  const char* file_name = "test.txt";
  int fd = open("/dat/sub/test.txt", O_RDONLY);
  assert(fd < 0);
}

int main() {
  FILE *test_txt = fopen("/test.txt", "w");
  fputs("mapfs", test_txt);
  fclose(test_txt);
  mkdir("/subdir", 0777);
  FILE *test2_txt = fopen("/subdir/test2.txt", "w");
  fputs("mapfs 2", test2_txt);
  fclose(test2_txt);

  test_manifest();
  test_manifest_js();
  test_nonexistent();

  return 0;
}
