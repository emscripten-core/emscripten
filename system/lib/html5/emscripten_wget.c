#include <emscripten.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

// Creates all ancestor directories of a given file, if they do not already
// exist. Returns 0 on success or 1 on error.
static int mkdirs(const char* file) {
  char* copy = strdup(file);
  char* c = copy;
  while (*c) {
    // Create any non-trivial (not the root "/") directory.
    if (*c == '/' && c != copy) {
      *c = 0;
      int result = mkdir(copy, S_IRWXU);
      *c = '/';
      // Continue while we succeed in creating directories or while we see that
      // they already exist.
      if (result < 0 && errno != EEXIST) {
        free(copy);
        return 1;
      }
    }
    c++;
  }
  free(copy);
  return 0;
}

int emscripten_wget(const char* url, const char* file) {
  // Create the ancestor directories.
  if (mkdirs(file)) {
    return 1;
  }

  // Fetch the data.
  void* buffer;
  int num;
  int error;
  emscripten_wget_data(url, &buffer, &num, &error);
  if (error) {
    return 1;
  }

  // Write the data.
  int fd = open(file, O_WRONLY | O_CREAT, S_IRWXU);
  if (fd >= 0) {
    write(fd, buffer, num);
    close(fd);
  }
  free(buffer);
  return fd < 0;
}
