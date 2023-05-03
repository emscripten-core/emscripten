/*
 * Copyright 2015 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

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
#include <errno.h>

void test_mmap_read() {
  // Use mmap to read in.txt
  EM_ASM(FS.writeFile('yolo/in.txt', 'mmap ftw!'));

  int fd = open("yolo/in.txt", O_RDONLY);
  assert(fd >= 0);

  int filesize = 9;
  char* map = (char*)mmap(NULL, filesize, PROT_READ, MAP_PRIVATE, fd, 0);
  assert(map != MAP_FAILED);

  printf("yolo/in.txt content=");
  for (int i = 0; i < filesize; i++) {
      printf("%c", map[i]);
  }
  printf("\n");

  int rc = munmap(map, filesize);
  assert(rc == 0);

  close(fd);
}

void test_mmap_write() {
  // Use mmap to write out.txt
  int fd = open("yolo/out.txt", O_RDWR | O_CREAT | O_TRUNC, (mode_t)0600);
  assert(fd >= 0);

  const char* text = "written mmap";
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

  {
    FILE* fd = fopen("yolo/out.txt", "r");
    assert(fd >= 0);
    char buffer[15];
    memset(buffer, 0, 15);
    fread(buffer, 1, 14, fd);
    printf("yolo/out.txt content=%s\n", buffer);
    fclose(fd);
  }
}

void test_mmap_readonly() {
  // Use mmap to read out.txt and modify the contents in memory,
  // but make sure it's not overwritten on munmap
  const char* readonlytext = "readonly mmap\0";
  const char* text = "write mmap\0";
  const char* path = "yolo/outreadonly.txt";
  size_t readonlytextsize = strlen(readonlytext);
  size_t textsize = strlen(text);

  int fd = open(path, O_RDWR | O_CREAT, (mode_t)0600);
  assert(fd >= 0);
  // write contents to the file ( we don't want this to be overwritten on munmap )
  assert(write(fd, readonlytext, readonlytextsize) != -1);
  close(fd);

  fd = open(path, O_RDWR);
  assert(fd >= 0);
  char *map = (char*)mmap(0, textsize, PROT_READ, MAP_SHARED, fd, 0);
  assert(map != MAP_FAILED);

  for (size_t i = 0; i < textsize; i++) {
    map[i] = text[i];
  }

  assert(munmap(map, textsize) != -1);
  close(fd);

  {
    FILE* fd = fopen("yolo/outreadonly.txt", "r");
    assert(fd >= 0);
    char buffer[16];
    memset(buffer, 0, 16);
    fread(buffer, 1, 15, fd);
    printf("yolo/outreadonly.txt content=%s\n", buffer);
    fclose(fd);
  }
}

void test_mmap_private() {
  int fd = open("yolo/private.txt", O_RDWR | O_CREAT | O_TRUNC, (mode_t)0600);
  assert(fd >= 0);

  const char* text = "written mmap";
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

  {
    FILE* fd = fopen("yolo/private.txt", "r");
    assert(fd >= 0);
    char buffer[15];
    memset(buffer, 0, 15);
    fread(buffer, 1, 14, fd);
    printf("yolo/private.txt content=%s\n", buffer);
    fclose(fd);
  }
}

void test_mmap_shared_with_offset() {
  int fd = open("yolo/sharedoffset.txt", O_RDWR | O_CREAT | O_TRUNC, (mode_t)0600);
  assert(fd > 0);

  const char* text = "written shared mmap with offset";
  size_t textsize = strlen(text) + 1; // + \0 null character
  // offset must be a multiple of the page size as returned by sysconf(_SC_PAGE_SIZE).
  size_t offset = sysconf(_SC_PAGE_SIZE) * 2;

  assert(lseek(fd, textsize + offset - 1, SEEK_SET) != -1);

  // need to write something first to allow us to mmap
  assert(write(fd, "", 1) != -1);

  char *map;
  map = (char*)mmap(0, textsize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, offset - 1);
  // assert failure if offset is not a multiple of page size
  assert(map == MAP_FAILED);

  map = (char*)mmap(0, textsize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, offset);
  assert(map != MAP_FAILED);

  for (size_t i = 0; i < textsize; i++) {
    map[i] = text[i];
  }

  assert(msync(map, textsize, MS_SYNC) != -1);
  assert(munmap(map, textsize) != -1);
  close(fd);

  {
    FILE* fd = fopen("yolo/sharedoffset.txt", "r");
    assert(fd >= 0);
    size_t offset = sysconf(_SC_PAGE_SIZE) * 2;

    char buffer[33];
    memset(buffer, 0, 33);
    fseek(fd, offset, SEEK_SET);
    fread(buffer, 1, 32, fd);
    // expect text written from mmap operation to appear at offset in the file
    printf("yolo/sharedoffset.txt content=%s %zu\n", buffer, offset);
    fclose(fd);
  }
}

void test_mmap_hint() {
  // mmap with a address is expected to fail
  int fd = open("yolo/private.txt", O_RDWR | O_CREAT | O_TRUNC, (mode_t)0600);
  assert(fd != -1);

  size_t map_size = 1 << 16;

  // Reserve some address space in which to perform the experiment
  char *alloc = (char*)mmap(NULL, map_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  assert(alloc != MAP_FAILED);

  char *addr = (char*)mmap((void*)alloc, map_size, PROT_READ, MAP_PRIVATE | MAP_FIXED, fd, 0);
#ifdef __EMSCRIPTEN__
  assert(addr == MAP_FAILED && errno == EINVAL); // Emscripten
#else
  assert(addr == alloc); // Native environments
  assert(munmap(addr, map_size) != -1);
#endif

  assert(munmap(alloc, map_size) != -1);
  close(fd);
}

/**
 * MMAP to an 'over-allocated' file
 *
 * When appending to a file, the buffer size is increased in chunks, and so the
 * actual length of the file could be less than the buffer size.
 *
 * When using mmap for an over-allocated file, we have to make sure that content
 * from the buffer is not written beyond the allocated memory area for the mmap
 * operation.
 */
void test_mmap_overallocate() {
#if !defined(NODEFS) && !defined(NODERAWFS) && !defined(WASMFS)
  int fd = open("yolo/overallocatedfile.txt", O_RDWR | O_CREAT, (mode_t)0600);
  assert(fd != -1);

  const size_t textsize = 33;

  // multiple calls to write so that the file will be over-allocated
  for (int n = 0; n < textsize; n++) {
    assert(write(fd, "a", 1) != -1);
  }

  EM_ASM({
    const stream = FS.streams.find(stream => stream.path.indexOf('yolo/overallocatedfile.txt') >= 0);
    assert(stream.node.usedBytes === Number($0),
      'Used bytes on the over-allocated file (' + stream.node.usedBytes + ') ' +
      'should be ' + $0
    );
    assert(stream.node.contents.length > stream.node.usedBytes,
      'Used bytes on the over-allocated file (' + stream.node.usedBytes + ') ' +
      'should be less than the length of the content buffer (' + stream.node.contents.length + ')'
    );
    stream.node.contents[stream.node.usedBytes] = 98; // 'b', we don't want to see this in the mmap area
  }, textsize);

  char *map = (char*)mmap(NULL, textsize, PROT_READ, 0, fd, 0);

  assert(map[textsize-1] == 'a');

  // Assert that content from the over-allocated file buffer is not written
  // beyond the allocated memory for the map
#if !__has_feature(address_sanitizer) // the following is invalid, and asan complains rightfully
  assert(map[textsize] != 'b');
#endif
  munmap(map, textsize);

  close(fd);
#endif
}

int main() {
  EM_ASM(
    FS.mkdir('yolo');
#if NODEFS
    FS.mount(NODEFS, { root: '.' }, 'yolo');
#endif
  );
  test_mmap_read();
  test_mmap_write();
  test_mmap_readonly();
  test_mmap_private();
  test_mmap_shared_with_offset();
  test_mmap_hint();
  test_mmap_overallocate();
  return 0;
}
