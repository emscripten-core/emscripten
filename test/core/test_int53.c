#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include "test_int53.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Uncomment to compute the expected result:
//#define GENERATE_ANSWERS

EM_JS_DEPS(main, "$convertI32PairToI53,$convertU32PairToI53,$readI53FromU64,$readI53FromI64,$writeI53ToI64,$writeI53ToI64Clamped,$writeI53ToU64Clamped,$writeI53ToI64Signaling,$writeI53ToU64Signaling");

void writeI53ToI64_int64(int64_t *heapAddress, int64_t num) {
#ifdef GENERATE_ANSWERS
  *heapAddress = num;
#else
  EM_ASM(writeI53ToI64($0, $1), heapAddress, (double)num);
#endif
}

void writeI53ToI64_double(int64_t *heapAddress, double num) {
#ifdef GENERATE_ANSWERS
  if (num > 0 || num <= -9223372036854775808.0 /* underflow, garbage in-garbage out situation: just produce a value that matches current JS impl*/)
    *(uint64_t*)heapAddress = (uint64_t)num;
  else
    *heapAddress = (int64_t)num;
#else
  EM_ASM(writeI53ToI64($0, $1), heapAddress, num);
#endif
}

void writeI53ToI64Clamped_double(int64_t *heapAddress, double num) {
#ifdef GENERATE_ANSWERS
  if (num >= 9223372036854775808.0)
    *heapAddress = 0x7FFFFFFFFFFFFFFFLL;
  else if (num <= -9223372036854775808.0)
    *heapAddress = -0x8000000000000000LL;
  else if (num > 0)
    *(uint64_t*)heapAddress = (uint64_t)num;
  else
    *heapAddress = (int64_t)num;
#else
  EM_ASM(writeI53ToI64Clamped($0, $1), heapAddress, num);
#endif
}

void writeI53ToI64Signaling_double(int64_t *heapAddress, double num) {
#ifdef GENERATE_ANSWERS
  if (num <= -9223372036854775808.0 || num >= 9223372036854775808.0) {
    *heapAddress = 0x0102030401020304ULL;
  } else {
    *heapAddress = (int64_t)num;
  }
#else
  EM_ASM(try {
    writeI53ToI64Signaling($0, $1)
  } catch(e) {
    HEAPU32[($0) / 4] = 0x01020304;
    HEAPU32[($0+4) / 4] = 0x01020304;
  }, heapAddress, num);
#endif
}

void writeI53ToU64Clamped_double(uint64_t *heapAddress, double num) {
#ifdef GENERATE_ANSWERS
  if (num >= 18446744073709551616.0)
    *heapAddress = 0xFFFFFFFFFFFFFFFFULL;
  else if (num < 0)
    *heapAddress = 0;
  else
    *heapAddress = (uint64_t)num;
#else
  EM_ASM(writeI53ToU64Clamped($0, $1), heapAddress, num);
#endif
}

void writeI53ToU64Signaling_double(uint64_t *heapAddress, double num) {
#ifdef GENERATE_ANSWERS
  if (num < 0 || num >= 18446744073709551616.0) {
    *heapAddress = 0x0102030401020304ULL;
  } else {
    *heapAddress = (uint64_t)num;
  }
#else
  EM_ASM(try {
    writeI53ToU64Signaling($0, $1)
  } catch(e) {
    HEAPU32[$0 / 4] = 0x01020304;
    HEAPU32[($0+4) / 4] = 0x01020304;
  } , heapAddress, num);
#endif
}

int64_t readI53FromI64_toInt64(int64_t *heapAddress) {
#ifdef GENERATE_ANSWERS
  return *heapAddress;
#else
  return (int64_t)EM_ASM_DOUBLE(return readI53FromI64($0), heapAddress);
#endif
}

double readI53FromI64(int64_t *heapAddress)
{
#ifdef GENERATE_ANSWERS
  return (double)*heapAddress;
#else
  return EM_ASM_DOUBLE(return readI53FromI64($0), heapAddress);
#endif
}

int64_t readI53FromU64_toInt64(uint64_t *heapAddress) {
#ifdef GENERATE_ANSWERS
  return (int64_t)*heapAddress;
#else
  return (int64_t)EM_ASM_DOUBLE(return readI53FromU64($0), heapAddress);
#endif
}

double readI53FromU64(uint64_t *heapAddress) {
#ifdef GENERATE_ANSWERS
  return (double)*heapAddress;
#else
  return EM_ASM_DOUBLE(return readI53FromU64($0), heapAddress);
#endif
}

int64_t convertI32PairToI53(int32_t lo, int32_t hi) {
#ifdef GENERATE_ANSWERS
  uint64_t val = (uint32_t)lo;
  val |= ((uint64_t)(uint32_t)hi) << 32;
  return (int64_t)val;
#else
  return (int64_t)EM_ASM_DOUBLE(return convertI32PairToI53($0, $1), lo, hi);
#endif
}

int64_t convertU32PairToI53(uint32_t lo, uint32_t hi) {
#ifdef GENERATE_ANSWERS
  uint64_t val = (uint32_t)lo;
  val |= ((uint64_t)(uint32_t)hi) << 32;
  return val;
#else
  return (int64_t)EM_ASM_DOUBLE(return convertU32PairToI53($0, $1), lo, hi);
#endif
}

int64_t testconvertI32PairToI53(int64_t val) {
  uint64_t u = (uint64_t)val;
  int32_t lo = (uint32_t)u;
  int32_t hi = u >> 32;
  return convertI32PairToI53(lo, hi);
}

int64_t testconvertU32PairToI53(uint64_t val) {
  uint32_t lo = (uint32_t)val;
  uint32_t hi = val >> 32;
  return convertU32PairToI53(lo, hi);
}

int64_t testWriteI64AsI53(int64_t num) {
  int64_t addr = 0;
  writeI53ToI64_int64(&addr, num);
  return addr;
}

int64_t testReadWriteI64AsI53(int64_t num) {
  int64_t addr = 0;
  writeI53ToI64_int64(&addr, num);
  return readI53FromI64_toInt64(&addr);
}

uint64_t testReadWriteU64AsI53(uint64_t num) {
  uint64_t addr = 0;
  writeI53ToI64_int64((int64_t*)&addr, num);
  return readI53FromU64_toInt64(&addr);
}

int main() {
  printf("Testing positive losslessIntegers:\n");
  for (int i = 0; i < sizeof(losslessIntegers) / sizeof(losslessIntegers[0]); ++i) {
    uint64_t num = losslessIntegers[i];
    printf("%llu (0x%llx):\n", num, num);
    printf("  convertI32PairToI53: 0x%llx\n", testconvertI32PairToI53(num));
    if (num != testconvertI32PairToI53(num)) return 1;
    printf("  convertU32PairToI53: 0x%llx\n", testconvertU32PairToI53(num));
    if (num != testconvertU32PairToI53(num)) return 1;
    printf("  writeI53ToI64: 0x%llx\n", testWriteI64AsI53(num));
    if (num != testWriteI64AsI53(num)) return 1;
    printf("  readI53FromI64: 0x%llx\n", testReadWriteI64AsI53(num));
    if (num != testReadWriteI64AsI53(num)) return 1;
    printf("  readI53FromU64: 0x%llx\n", testReadWriteU64AsI53(num));
    if (num != testReadWriteU64AsI53(num)) return 1;
    printf("\n");
  }

  printf("Testing negative losslessIntegers:\n");
  for (int i = 0; i < sizeof(losslessIntegers) / sizeof(losslessIntegers[0]); ++i) {
    // Test negative:
    int64_t neg = -(int64_t)losslessIntegers[i];
    printf("%lld (0x%llx):\n", neg, neg);
    printf("  convertI32PairToI53: 0x%llx\n", testconvertI32PairToI53(neg));
    if (neg != testconvertI32PairToI53(neg)) return 1;
    printf("  writeI53ToI64: 0x%llx\n", testWriteI64AsI53(neg));
    if (neg != testWriteI64AsI53(neg)) return 1;
    printf("  readI53FromI64: 0x%llx\n", testReadWriteI64AsI53(neg));
    if (neg != testReadWriteI64AsI53(neg)) return 1;
    printf("\n");
  }

  printf("Testing preciseUnsignedIntegers:\n");
  for (int i = 0; i < sizeof(preciseUnsignedIntegers) / sizeof(preciseUnsignedIntegers[0]); ++i) {
    uint64_t num = preciseUnsignedIntegers[i];
    printf("%llu (0x%llx):\n", num, num);
    printf("  convertU32PairToI53: 0x%llx: error difference: %lld\n", testconvertU32PairToI53(num), testconvertU32PairToI53(num) - num);
    printf("  readI53FromU64: 0x%llx: error difference: %lld\n", testReadWriteU64AsI53(num), testReadWriteU64AsI53(num) - num);
    printf("\n");
  }

  printf("Testing preciseNegativeIntegers:\n");
  for (int i = 0; i < sizeof(preciseNegativeIntegers) / sizeof(preciseNegativeIntegers[0]); ++i) {
    uint64_t num = preciseNegativeIntegers[i];
    printf("%llu (0x%llx):\n", num, num);
    printf("  convertI32PairToI53: 0x%llx: error difference: %lld\n", testconvertI32PairToI53(num), testconvertI32PairToI53(num) - num);
    printf("  readI53FromI64: 0x%llx: error difference: %lld\n", testReadWriteI64AsI53(num), testReadWriteI64AsI53(num) - num);
    printf("\n");
  }

  printf("Testing impreciseUnsignedIntegers:\n");
  for (int i = 0; i < sizeof(impreciseUnsignedIntegers) / sizeof(impreciseUnsignedIntegers[0]); ++i) {
    double num = impreciseUnsignedIntegers[i];
    printf("%f:\n", num);
    uint64_t u;
    writeI53ToI64_double((int64_t*)&u, num);
    printf("  writeI53ToI64: 0x%llx (%llu): error difference: %g\n", u, u, u - num);
    printf("\n");
  }

  printf("Testing impreciseNegativeIntegers:\n");
  for (int i = 0; i < sizeof(impreciseNegativeIntegers) / sizeof(impreciseNegativeIntegers[0]); ++i) {
    double num = impreciseNegativeIntegers[i];
    printf("%f:\n", num);
    int64_t u;
    writeI53ToI64_double(&u, num);
    printf("  writeI53ToI64: 0x%llx (%lld): error difference: %g\n", u, u, u - num);
    printf("\n");
  }

  printf("Testing otherDoubles:\n");
  for (int i = 0; i < sizeof(otherDoubles) / sizeof(otherDoubles[0]); ++i) {
    for (int sign = 0; sign < 2; ++sign) {
      double num = otherDoubles[i];
      if (sign) {
        num = -num;
      }
      printf("%f:\n", num);
      int64_t u;
      writeI53ToI64_double(&u, num);
      printf("  writeI53ToI64: 0x%llx (%lld): error difference: %g\n", u, u, u - num);
      writeI53ToI64Clamped_double(&u, num);
      printf("  writeI53ToI64Clamped: 0x%llx (%lld): error difference: %g\n", u, u, u - num);
      writeI53ToI64Signaling_double(&u, num);
      if (u == 0x0102030401020304)
        printf("  writeI53ToI64Signaling: (RangeError)\n");
      else
        printf("  writeI53ToI64Signaling: 0x%llx (%lld): error difference: %g\n", u, u, u - num);
      writeI53ToU64Clamped_double((uint64_t*)&u, num);
      printf("  writeI53ToU64Clamped: 0x%llx (%lld): error difference: %g\n", u, u, u - num);
      writeI53ToU64Signaling_double((uint64_t*)&u, num);
      if (u == 0x0102030401020304)
        printf("  writeI53ToU64Signaling: (RangeError)\n");
      else
        printf("  writeI53ToU64Signaling: 0x%llx (%lld): error difference: %g\n", u, u, u - num);
      printf("\n");
    }
  }

  printf("All done!\n");
}
