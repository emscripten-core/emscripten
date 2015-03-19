#ifndef EMSCRIPTEN_MMAP_H_
#define EMSCRIPTEN_MMAP_H_

#include <math.h>

#ifndef EMSCRIPTEN
#define define_mmap_write(T, name) \
void mmap_write_##name(void* map, size_t index, T value) { \
  ((T*)map)[index] = value; \
}
 
#define define_mmap_read(T, name) \
T mmap_read_##name(void* map, size_t index) { \
  return ((T*)map)[index]; \
}
#else
#define define_mmap_write(T, name) \
void mmap_write_##name(void* map, size_t index, T value);
 
#define define_mmap_read(T, name) \
T mmap_read_##name(void* map, size_t index);
#endif // EMSCRIPTEN
 
define_mmap_write(char, char);
define_mmap_write(short, short);
define_mmap_write(long, long);
define_mmap_write(int8_t, int8_t);
define_mmap_write(int16_t, int16_t);
define_mmap_write(int32_t, int32_t);
define_mmap_write(unsigned char, unsigned_char);
define_mmap_write(unsigned short, unsigned_short);
define_mmap_write(unsigned long, unsigned_long);
define_mmap_write(uint8_t, uint8_t);
define_mmap_write(uint16_t, uint16_t);
define_mmap_write(uint32_t, uint32_t);
define_mmap_write(float, float);
define_mmap_write(double, double);
 
define_mmap_read(char, char);
define_mmap_read(short, short);
define_mmap_read(long, long);
define_mmap_read(int8_t, int8_t);
define_mmap_read(int16_t, int16_t);
define_mmap_read(int32_t, int32_t);
define_mmap_read(unsigned char, unsigned_char);
define_mmap_read(unsigned short, unsigned_short);
define_mmap_read(unsigned long, unsigned_long);
define_mmap_read(uint8_t, uint8_t);
define_mmap_read(uint16_t, uint16_t);
define_mmap_read(uint32_t, uint32_t);
define_mmap_read(float, float);
define_mmap_read(double, double);
 
#endif // EMSCRIPTEN_MMAP_H_
