#include <emscripten.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

// Creates all ancestor directories of a given file, if they do not already
// exist.
static void mkdirs(const char* file) {
  char* copy = strdup(file);
  char* c = copy;
  while (*c) {
    // Create any non-trivial (not the root "/") directory.
    if (*c == '/' && c != copy) {
      *c = 0;
      int result = mkdir(copy, 0777);
      *c = '/';
      // Continue while we succeed in creating directories or while we see that
      // they already exist.
      if (result < 0 && errno != EEXIST) {
        free(copy);
        return;
      }
    }
    c++;
  }
  free(copy);
}

void emscripten_wget(const char* url, const char* file) {
  // Create the ancestor directories.
  mkdirs(file);

  // Fetch the data.
  void* buffer;
  int num;
  int error;
  emscripten_wget_data(url, &buffer, &num, &error);
  if (error) {
    return;
  }

  // Write the data.
  int fd = open(file, O_WRONLY | O_CREAT);
  if (fd >= 0) {
    write(fd, buffer, num);
    close(fd);
  }
  free(buffer);
}
