#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include "test_int53.h"

#include <stdio.h>
#include <stdlib.h>

// Uncomment to compute the expected results without testing:
//#define GENERATE_ANSWERS

EM_JS_DEPS(test, "$convertI32PairToI53Checked");

double test(int64_t val) {
  int32_t lo = (uint32_t)val;
  int32_t hi = (uint64_t)val >> 32;
  printf("input=%lld / 0x%llx: convertI32PairToI53Checked(lo=%d hi=%d)=", val, val, lo, hi);
#ifdef GENERATE_ANSWERS
  int64_t v = (uint64_t)(uint32_t)lo;
  v |= ((uint64_t)(uint32_t)hi) << 32;
  double out = v > 9007199254740992ll || v < -9007199254740992ll ? (double)NAN : v;
#else
  double out = EM_ASM_DOUBLE(return convertI32PairToI53Checked($0, $1), lo, hi);
#endif
  printf("%f\n", out);
  return out;
}

int main() {
  printf("Testing library_int53.js function $convertI32PairToI53Checked():\n\n");

  printf("Testing positive losslessIntegers:\n");
  for (uint64_t num : losslessIntegers) {
    double ret = test(num);
    if (ret != num) return 1;
  }

  printf("\nTesting negative losslessIntegers:\n");
  for (uint64_t num : losslessIntegers) {
    int64_t neg = -(int64_t)num;
    double ret = test(neg);
    if (ret != neg) return 1;
  }

  printf("\nTesting preciseUnsignedIntegers:\n");
  for (uint64_t num : preciseUnsignedIntegers) {
    double ret = test(num);
    if (!isnan(ret)) return 1;
  }

  printf("\nTesting preciseNegativeIntegers:\n");
  for (uint64_t num : preciseNegativeIntegers) {
    double ret = test(num);
    if (!isnan(ret)) return 1;
  }

  printf("\nTesting impreciseUnsignedIntegers:\n");
  for (uint64_t num : impreciseUnsignedIntegers) {
    test(num);
  }

  printf("\nTesting impreciseNegativeIntegers:\n");
  for (uint64_t num : impreciseNegativeIntegers) {
    double ret = test(num);
    if (!isnan(ret)) return 1;
  }

  printf("\nTesting otherDoubles:\n");
  for (int sign = 0; sign < 2; ++sign) {
    for (double d : otherDoubles) {
      double num = sign ? -d : d;
      uint64_t u = *(uint64_t*)&num;
      printf("Double: %f -> ", num);
      test(u);
    }
  }
  printf("All done!\n");
}
