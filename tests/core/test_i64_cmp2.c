#include <inttypes.h>
#include <stdio.h>

typedef int32_t INT32;
typedef int64_t INT64;
typedef uint8_t UINT8;

void interface_clock_changed() {
  UINT8 m_divshift;
  INT32 m_divisor;

  // INT64 attos = m_attoseconds_per_cycle;
  INT64 attos = 279365114840;
  m_divshift = 0;
  while (attos >= (1UL << 31)) {
    m_divshift++;
    printf("m_divshift is %i, on %lld >?= %lu\n", m_divshift, attos, 1UL << 31);
    attos >>= 1;
  }
  m_divisor = attos;

  printf("m_divisor is %i\n", m_divisor);
}

int main() {
  interface_clock_changed();
  return 0;
}
