#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include <sys/user.h>

void create_file(const char* filename, const char* content) {
  int fd = open(filename, O_RDWR|O_CREAT, S_IRUSR|S_IWUSR); // create an empty file
  assert(fd >= 0);
  write(fd, content, strlen(content));
  close(fd);
}

int main() {
  struct stat st;

  create_file("empty_file", "");
  create_file("nonempty_file", "some content");

  int fd = open("empty_file", O_RDONLY);
  int fd2 = open("nonempty_file", O_RDONLY);

  // "fd" should be an empty file, "fd2" non-empty.
  assert(fstat(fd, &st) != -1);
  int filesize = st.st_size;
  assert(filesize == 0);

  assert(fstat(fd2, &st) != -1);
  filesize = st.st_size;
  assert(filesize != 0);

  // mapping a length of zero bytes should always fail
  char *mapped = (char*) mmap(NULL, 0, PROT_READ, MAP_PRIVATE, fd, 0);
  printf("mmap -> %p\n", mapped);
  assert(mapped == MAP_FAILED);
  printf("mmap failed with: %s\n", strerror(errno));
  assert(errno == EINVAL);

  mapped = (char*) mmap(NULL, 0, PROT_READ, MAP_PRIVATE, fd2, 0);
  printf("mmap -> %p\n", mapped);
  assert(mapped == MAP_FAILED);
  printf("mmap failed with: %s\n", strerror(errno));
  assert(errno == EINVAL);

  // mapping a non-zero number of pages should succeed, even for empty files.
  // On native systems this results in pages beyond the length of the file
  // being inaccessable, but we don't have any real memory protection on
  // emscripten so we cannot test that.
  mapped = (char*) mmap(NULL, 2*PAGE_SIZE, PROT_READ, MAP_PRIVATE, fd, 0);
  assert(mapped != MAP_FAILED);
  munmap(mapped, 2*PAGE_SIZE);

  close(fd);
  close(fd2);

  // Now check that mapping zero bytes fails
  printf("done\n");
  return 0;
}
