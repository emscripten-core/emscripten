#include <assert.h>
#include <emscripten/threading.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define OLD 42

int main() {
  uint8_t buffer[16];
  memset(&buffer[0], OLD, sizeof(buffer));

  uint8_t o8 = emscripten_atomic_exchange_u8(&buffer[0], 0x12);
  assert(o8 == OLD);
  assert(buffer[0] == 0x12);
  assert(buffer[1] == OLD);

  uint16_t o16 = emscripten_atomic_exchange_u16(&buffer[0], 0x3456);
  assert((o16 & 0xff) == 0x12);
  assert((o16 >> 8) == OLD);
  assert(buffer[0] == 0x56);
  assert(buffer[1] == 0x34);
  assert(buffer[2] == OLD);

  uint32_t o32 = emscripten_atomic_exchange_u32(&buffer[0], 0xabcdef91);
  assert((o32 & 0xff) == 0x56);
  assert(((o32 >> 8) & 0xff) == 0x34);
  assert(((o32 >> 16) & 0xff) == OLD);
  assert(((o32 >> 24) & 0xff) == OLD);
  assert(buffer[0] == 0x91);
  assert(buffer[1] == 0xef);
  assert(buffer[2] == 0xcd);
  assert(buffer[3] == 0xab);
  assert(buffer[4] == OLD);

  printf("OK\n");
}

