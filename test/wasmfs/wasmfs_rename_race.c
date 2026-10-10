// Concurrent renames, moves between a directory and its parent, and path
// lookups in the same directory tree must not deadlock.
#include <assert.h>
#include <dirent.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define ITERATIONS 200

static void write_file(const char* path) {
  int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
  assert(fd >= 0);
  assert(write(fd, "x", 1) == 1);
  assert(close(fd) == 0);
}

// Publish files by writing a temporary and renaming it into place in the same
// directory, two levels below the root.
static void* publisher(void* arg) {
  int id = (int)(intptr_t)arg;
  char tmp[64], final[64];
  for (int i = 0; i < ITERATIONS; i++) {
    snprintf(tmp, sizeof(tmp), "/a/b/c/tmp%d_%d", id, i);
    snprintf(final, sizeof(final), "/a/b/c/file%d_%d", id, i);
    write_file(tmp);
    assert(rename(tmp, final) == 0);
    assert(unlink(final) == 0);
  }
  return NULL;
}

// Move files between a directory and its parent, so the two directories a
// rename locks are an ancestor and a descendant.
static void* mover(void* arg) {
  int id = (int)(intptr_t)arg;
  char lower[64], upper[64];
  for (int i = 0; i < ITERATIONS; i++) {
    snprintf(lower, sizeof(lower), "/a/b/c/move%d_%d", id, i);
    snprintf(upper, sizeof(upper), "/a/b/move%d_%d", id, i);
    write_file(lower);
    assert(rename(lower, upper) == 0);
    assert(rename(upper, lower) == 0);
    assert(unlink(lower) == 0);
  }
  return NULL;
}

// Resolve paths through the same tree, which locks each directory on the way
// down.
static void* walker(void* arg) {
  struct stat st;
  for (int i = 0; i < ITERATIONS * 4; i++) {
    stat("/a/b/c/absent", &st);
    stat("/a/b/c", &st);
    DIR* dir = opendir("/a/b/c");
    assert(dir);
    while (readdir(dir)) {
    }
    closedir(dir);
  }
  return NULL;
}

int main() {
  assert(mkdir("/a", 0777) == 0);
  assert(mkdir("/a/b", 0777) == 0);
  assert(mkdir("/a/b/c", 0777) == 0);

  pthread_t threads[8];
  int count = 0;
  for (int i = 0; i < 3; i++) {
    assert(pthread_create(&threads[count++], NULL, publisher, (void*)(intptr_t)i) == 0);
  }
  for (int i = 0; i < 2; i++) {
    assert(pthread_create(&threads[count++], NULL, mover, (void*)(intptr_t)i) == 0);
  }
  for (int i = 0; i < 3; i++) {
    assert(pthread_create(&threads[count++], NULL, walker, NULL) == 0);
  }
  for (int i = 0; i < count; i++) {
    assert(pthread_join(threads[i], NULL) == 0);
  }
  printf("ok\n");
  return 0;
}
