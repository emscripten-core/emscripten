#include <assert.h>
#include <emscripten/threading.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define OLD 42

int main() {
  char buffer[16];
  memset(&buffer[0], OLD, sizeof(buffer));

  uint8_t o8 = emscripten_atomic_exchange_u8(&buffer[0], 1);
  assert(o8 == OLD);
  assert(buffer[0] == 1);
  assert(buffer[1] == OLD);

  uint16_t o16 = emscripten_atomic_exchange_u16(&buffer[0], 2);
  assert((o16 & 0xff) == 1);
  assert((o16 >> 8) == OLD);
  assert(buffer[0] == 2);
  assert(buffer[1] == 0);
  assert(buffer[2] == OLD);

  uint16_t o32 = emscripten_atomic_exchange_u32(&buffer[0], 3);
  assert((o32 & 0xff) == 2);
  assert(((o32 >> 8) & 0xff) == 0);
  assert(((o32 >> 16) & 0xff) == 0);
  assert(((o32 >> 24) & 0xff) == 0);
  assert(buffer[0] == 3);
  assert(buffer[1] == 0);
  assert(buffer[2] == 0);
  assert(buffer[3] == 0);
  assert(buffer[4] == OLD);

  printf("OK\n");
}

