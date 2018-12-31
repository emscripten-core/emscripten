#include <stdio.h>
#include <sys/mman.h>
#include <emscripten.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/io.h>
#include <sys/mman.h>

int main() {
  EM_ASM(
    FS.mkdir('yolo');
    FS.writeFile('/yolo/in.txt', 'mmap ftw!');
  );

  // Use mmap to read in.txt
  {
    const char* path = "/yolo/in.txt";
    int fd = open(path, O_RDONLY);
    assert(fd != -1);

    int filesize = 9;
    char* map = (char*)mmap(NULL, filesize, PROT_READ, MAP_PRIVATE, fd, 0);
    assert(map != MAP_FAILED);

    printf("/yolo/in.txt content=");
    for (int i = 0; i < filesize; i++) {
        printf("%c", map[i]);
    }
    printf("\n");

    int rc = munmap(map, filesize);
    assert(rc == 0);

    close(fd);
  }

  // Use mmap to write out.txt
  {
    const char* text = "written mmap";
    const char* path = "/yolo/out.txt";

    int fd = open(path, O_RDWR | O_CREAT | O_TRUNC, (mode_t)0600);
    assert(fd != -1);

    size_t textsize = strlen(text) + 1; // + \0 null character
    assert(lseek(fd, textsize - 1, SEEK_SET) != -1);

    // need to write something first to allow us to mmap
    assert(write(fd, "", 1) != -1);

    char *map = (char*)mmap(0, textsize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    assert(map != MAP_FAILED);

    for (size_t i = 0; i < textsize; i++) {
      map[i] = text[i];
    }

    assert(msync(map, textsize, MS_SYNC) != -1);

    assert(munmap(map, textsize) != -1);

    close(fd);
  }

  {
    FILE* fd = fopen("/yolo/out.txt", "r");
    if (fd == NULL) {
      printf("failed to open /yolo/out.txt\n");
      return 1;
    }
    char buffer[13];
    fread(buffer, 1, 14, fd);
    printf("/yolo/out.txt content=%s\n", buffer);
    fclose(fd);
  }

  // MAP_PRIVATE
  {
    const char* text = "written mmap";
    const char* path = "/yolo/private.txt";

    int fd = open(path, O_RDWR | O_CREAT | O_TRUNC, (mode_t)0600);
    assert(fd != -1);

    size_t textsize = strlen(text) + 1; // + \0 null character
    assert(lseek(fd, textsize - 1, SEEK_SET) != -1);

    // need to write something first to allow us to mmap
    assert(write(fd, "", 1) != -1);

    char *map = (char*)mmap(0, textsize, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
    assert(map != MAP_FAILED);

    for (size_t i = 0; i < textsize; i++) {
      map[i] = text[i];
    }

    assert(msync(map, textsize, MS_SYNC) != -1);

    assert(munmap(map, textsize) != -1);

    close(fd);
  }

  {
    FILE* fd = fopen("/yolo/private.txt", "r");
    if (fd == NULL) {
      printf("failed to open /yolo/private.txt\n");
      return 1;
    }
    char buffer[13];
    fread(buffer, 1, 14, fd);
    printf("/yolo/private.txt content=%s\n", buffer);
    fclose(fd);
  }

  return 0;
}
