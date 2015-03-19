#include <stdio.h>
#include <sys/mman.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#ifdef EMSCRIPTEN
#include <emscripten.h>
#include <emscripten_mmap.h>
#else
#include "../../system/include/emscripten/emscripten_mmap.h"
#endif

#define define_writeread_test(T, name) \
void writeread_test_##name(char* path, char* print_format, size_t length, T data[]) { \
  { \
    int fd = open(path, O_RDWR | O_CREAT | O_TRUNC, (mode_t)0600); \
    assert(fd != -1); \
    assert(lseek(fd, length * sizeof(data[0]), SEEK_SET) != -1); \
    assert(write(fd, "", 1) != -1); \
    void* map = mmap(0, length * sizeof(data[0]), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0); \
    assert(map != MAP_FAILED); \
    for (size_t i = 0; i < length; i++) { \
      mmap_write_##name(map, i, data[i]); \
    } \
    assert(msync(map, length * sizeof(data[0]), MS_SYNC) != -1); \
    assert(munmap(map, length * sizeof(data[0])) != -1); \
    close(fd); \
  } \
  { \
    int fd = open(path, O_RDONLY); \
    assert(fd != -1); \
    \
    int filesize = length * sizeof(data[0]); \
    void* map = mmap(NULL, filesize, PROT_READ, MAP_PRIVATE, fd, 0); \
    assert(map != MAP_FAILED); \
    printf("%s read_from_mmap content=", path); \
    for (int i = 0; i < filesize / sizeof(data[0]); i++) { \
      printf(print_format, mmap_read_##name(map, i)); \
    } \
    printf("\n"); \
    int rc = munmap(map, filesize); \
    assert(rc == 0); \
    close(fd); \
  } \
  { \
    FILE* fd = fopen(path, "rb"); \
    T* buffer = (T*)malloc(sizeof(data[0]) * length); \
    fread(buffer, sizeof(data[0]), length, fd); \
    printf("%s read_from_fread content=", path); \
    for (int i = 0; i < length; i++) { \
      printf(print_format, buffer[i]); \
    } \
    printf("\n"); \
    fclose(fd); \
    free(buffer); \
  } \
}

define_writeread_test(char, char);
define_writeread_test(short, short);
define_writeread_test(long, long);
define_writeread_test(int8_t, int8_t);
define_writeread_test(int16_t, int16_t);
define_writeread_test(int32_t, int32_t);
define_writeread_test(unsigned char, unsigned_char);
define_writeread_test(unsigned short, unsigned_short);
define_writeread_test(unsigned long, unsigned_long);
define_writeread_test(uint8_t, uint8_t);
define_writeread_test(uint16_t, uint16_t);
define_writeread_test(uint32_t, uint32_t);
define_writeread_test(float, float);
define_writeread_test(double, double);

int main() {
#ifdef EMSCRIPTEN
  EM_ASM(
    FS.mkdir('yolo');
#if NODEFS
    FS.mount(NODEFS, { root: '.' }, 'yolo');
#endif
  );
#endif
  
  writeread_test_char("yolo/char.txt", "%x ", 3, (char[]){ 0x4a, 0x61, 0x6e });
  writeread_test_short("yolo/short.txt", "%x ", 4, (short[]){ 0x7e3a, 0x0302, 0x7FFF, 0x0af2 });
  writeread_test_long("yolo/long.txt", "%x ", 2, (long[]){ 0x8acd78be, 0x33ff21ee });
  writeread_test_int8_t("yolo/int8_t.txt", "%x ", 3, (int8_t[]){ 0x4a, 0x61, 0x6e });
  writeread_test_int16_t("yolo/int16_t.txt", "%x ", 4, (int16_t[]){ 0x7e3a, 0x512a, 0x8321, 0x0af2 });
  writeread_test_int32_t("yolo/int32_t.txt", "%x ", 2, (int32_t[]){ 0xe3a278be, 0x00002eee });
  writeread_test_unsigned_char("yolo/uchar.txt", "%x ", 7, (unsigned char[]){ 0x54, 0x65, 0x6c, 0x65, 0x6e, 0x6f, 0x72 });
  writeread_test_unsigned_short("yolo/ushort.txt", "%x ", 3, (unsigned short[]){ 0x5465, 0xffff, 0x6e6f });
  writeread_test_unsigned_long("yolo/ulong.txt", "%x ", 3, (unsigned long[]){ 0xaabbccdd, 0x98765432, 0xa });
  writeread_test_uint8_t("yolo/uint8_t.txt", "%x ", 3, (uint8_t[]){ -0x4a, 0x61, 0x6e });
  writeread_test_uint16_t("yolo/uint16_t.txt", "%x ", 4, (uint16_t[]){ 0x7e3a, 0x512a, 0x8321, 0x0af2 });
  writeread_test_uint32_t("yolo/uint32_t.txt", "%x ", 2, (uint32_t[]){ 0xe3a278be, 0x00002eee });
  writeread_test_float("yolo/float.txt", "%.6f ", 2, (float[]){ 133.721, -31.081988 });
  writeread_test_double("yolo/double.txt", "%.6f ", 3, (double[]){ 511.387382, 123.2139982, -31.081988  });
  
  return 0;
}
