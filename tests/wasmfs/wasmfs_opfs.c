#include <assert.h>
#include <dirent.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <emscripten/console.h>
#include <emscripten/wasmfs.h>

// Define WASMFS_SETUP then WASMFS_RESUME to run the test as two separate
// programs to test persistence. Alternatively, define neither and run the full
// test as a single program.

void cleanup(void);

int main(int argc, char* argv[]) {
  int err, fd;
  const char* msg = "Hello, OPFS!";

  emscripten_console_log("starting main");

  backend_t opfs = wasmfs_create_opfs_backend();
  emscripten_console_log("created OPFS backend");

  err = wasmfs_create_directory("/opfs", 0777, opfs);
  assert(err == 0);
  emscripten_console_log("mounted OPFS root directory");

#ifdef WASMFS_RESUME

  fd = open("/opfs/working/foo.txt", O_RDWR);
  assert(fd > 0);
  emscripten_console_log("opened existing OPFS file");

#else // !WASMFS_RESUME

  // Remove old files if they exist.
  cleanup();

  err = mkdir("/opfs/working", 0777);
  assert(err == 0);
  emscripten_console_log("created OPFS directory");

  fd = open("/opfs/working/foo.txt", O_RDWR | O_CREAT | O_EXCL, 0777);
  assert(fd > 0);
  emscripten_console_log("created OPFS file");

  int nwritten = write(fd, msg, strlen(msg));
  assert(nwritten == strlen(msg));
  emscripten_console_logf("wrote message: %s (%d)", msg, nwritten);

  int off = lseek(fd, 0, SEEK_SET);
  assert(off == 0);
  emscripten_console_log("seeked");

#endif // !WASMFS_RESUME

  char buf[100] = {};
  int nread = read(fd, buf, 100);
  assert(nread == strlen(msg));
  assert(strcmp(buf, msg) == 0);
  emscripten_console_logf("read message: %s (%d)", buf, nread);

  fdatasync(fd);
  emscripten_console_log("flushed");

  struct stat stat_buf;
  err = fstat(fd, &stat_buf);
  assert(err == 0);
  assert(stat_buf.st_size == strlen(msg));
  emscripten_console_log("statted");

#ifndef WASMFS_SETUP

  err = ftruncate(fd, 100);
  assert(err == 0);
  err = fstat(fd, &stat_buf);
  assert(err == 0);
  assert(stat_buf.st_size == 100);
  emscripten_console_log("truncated to 100");

  err = ftruncate(fd, 1);
  assert(err == 0);
  err = fstat(fd, &stat_buf);
  assert(err == 0);
  assert(stat_buf.st_size == 1);
  emscripten_console_log("truncated to 1");

  struct dirent** entries;
  int nentries = scandir("/opfs/working", &entries, NULL, alphasort);
  assert(nentries == 3);
  assert(strcmp(entries[0]->d_name, ".") == 0);
  assert(strcmp(entries[1]->d_name, "..") == 0);
  assert(strcmp(entries[2]->d_name, "foo.txt") == 0);
  assert(entries[2]->d_type == DT_REG);
  for (int i = 0; i < nentries; i++) {
    free(entries[i]);
  }
  free(entries);
  emscripten_console_log("read /opfs/working entries");

  nentries = scandir("/opfs", &entries, NULL, alphasort);
  assert(nentries == 3);
  assert(strcmp(entries[2]->d_name, "working") == 0);
  assert(entries[2]->d_type == DT_DIR);
  for (int i = 0; i < nentries; i++) {
    free(entries[i]);
  }
  free(entries);
  emscripten_console_log("read /opfs entries");

  err = close(fd);
  assert(err == 0);
  emscripten_console_log("closed file");

  err = stat("/opfs/working/foo.txt", &stat_buf);
  assert(err == 0);
  assert(stat_buf.st_size == 1);
  emscripten_console_log("statted");

  err = rename("/opfs/working/foo.txt", "/opfs/foo.txt");
  assert(err == 0);
  err = access("/opfs/working/foo.txt", F_OK);
  assert(err == -1);
  err = access("/opfs/foo.txt", F_OK);
  assert(err == 0);
  emscripten_console_log("moved file");

  err = unlink("/opfs/foo.txt");
  assert(err == 0);
  err = access("/opfs/foo.txt", F_OK);
  assert(err == -1);
  emscripten_console_log("removed OPFS file");

  err = rmdir("/opfs/working");
  assert(err == 0);
  err = access("/opfs/working", F_OK);
  assert(err == -1);
  emscripten_console_log("removed OPFS directory");

  emscripten_console_log("done");

#endif // !WASMFS_SETUP
}

void cleanup(void) {
  printf("cleaning up\n");

  unlink("/opfs/working/foo.txt");
  rmdir("/opfs/working");
  unlink("/opfs/foo.txt");

  assert(access("/opfs/working/foo.txt", F_OK) != 0);
  assert(access("/opfs/working", F_OK) != 0);
  assert(access("/opfs/foo.txt", F_OK) != 0);
}
