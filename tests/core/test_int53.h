#pragma once

// test_int53.h: Contains bit patterns of all kinds of interesting doubles.

#include <stdint.h>
#include <float.h>
#include <math.h>

#define NUMELEMS(x) (sizeof(x)/sizeof(x[0]))

// We can subdivide the set of all possible double precision floating point
// numbers + 64-bit (u)int numbers to eight categories:
// 1. Lossless integers: numbers that are precisely representable by both a
//    double and 64-bit signed integer, and all numbers smaller in abs value are
//    also precisely representable.
//    I.e. numbers [-2^53, 2^53] (inclusive)
const uint64_t losslessIntegers[] = {
  0,
  1,
  2,
  3,
  0x0FFFFFFu,
  0x1000000u, // == 16777216, largest consecutive single-precision floating point number
  0x1000001u,
  0x01020304u,
  0x7FFFFFFFu,
  0x80000000u,
  0x90000000u,
  0xFFFFFFFFu,
  0x100000000ull,
  0x100000001ull,
  0x17FFFFFFFull,
  0x180000000ull,
  0xFFFFFFFFFull,
  0x10203000000000ull,
  0x1FFFFF00000000ull,
  0x1FFFFF00000001ull,
  0x1FFFFFFFFFFFFEull,
  0x1FFFFFFFFFFFFFull,
  0x20000000000000ull, // 9,007,199,254,740,992, largest consecutive double-precision floating point number
};

// 2. Precise integers: numbers that are precisely representable by both a
//    double and 64-bit unsigned integer, but their neighboring numbers are
//    not.  E.g.
//    E.g. 9223372036854775808 == 0x8000000000000000ull and
//    18,446,744,073,709,549,568 == 0xfffffffffffff800ull are integer numbers
//    representable as both double and 64-bit uint.
const uint64_t preciseUnsignedIntegers[] = {
  0x20000000000002ull, // 9,007,199,254,740,994, largest consecutive double-precision floating point number plus two
  0x8000000000000000ull, // 9,223,372,036,854,775,808, a number around the sign point of int64_t, representable as double
  // Disabled for now, this is not converting consistently in different build modes.
  //0x8000000000000800ull, // 9,223,372,036,854,777,856, a number around the sign point of int64_t, representable as double (however conversion to this is not possible due to precision issues)
  0x25F5BDA103AA08ull, // 10684768937290248
  0x3F3837D5442494ull, // 17794735985140884
  0x55B4ACAE7DC2A0ull, // 24124026775257760
  0x72BDFA99BF28A8ull, // 32297031363930280
  0xA4055CD86A9F40ull, // 46167792506543936
  0x125AFCA30078D10ull, // 82665451100278032
  0x1268C844FE925C0ull, // 82908143057184192
  0x12A1DB1454D02A0ull, // 83912190268867232
  0x13E8D61ECEA80C0ull, // 89664494320124096
  0x1881B7DBD49D3D0ull, // 110368417731171280
  0xFE73E98A5E93F00ull, // 1145953455528558336
  0x2A44DB9E56754000ull, // 3045800721111138304
  0x7FFFFFFFFFFFFC00ull, // 9,223,372,036,854,774,784, a number around the sign point of int64_t, representable as double
  // Disabled for now, the following do not convert consistently in different build modes.
  // 0x9C04E99FFB426800ull, // 11242367443148105728
  // 0xB1BEDE55F1E6B000ull, // 12807918851000283136
  // 0xE762A64DFB28E800ull, // 16673071624335452160
  // 0xFFFFFFFFFFFFF800ull, // 18,446,744,073,709,549,568, largest integer that is representable as both a double and a uint64_t. (however conversion to this is not possible due to precision issues) (-2048 as int64)
};

// 3. Precise negative integers: numbers that are precisely representable by
//    both a double and 64-bit signed integer, but their neighboring numbers are
//    not.
const int64_t preciseNegativeIntegers[] = {
  -9007199254740994ll,            // -9007199254740992 minus two: smallest negative consecutive double-precision floating point number minus two
  (int64_t)0xFFD32C4AC85FB1AEull, // -12617674251062866
  (int64_t)0xFF3A4C372D2373A8ull, // -55648245524499544
  (int64_t)0xFF15853220D118D0ull, // -66000169181570864
  (int64_t)0xFE555489B4E3E2D0ull, // -120096864633363760
  (int64_t)0xFAFD5B94D7646780ull, // -361031700292802688
  (int64_t)0xF838033421CB9A40ull, // -560694631167452608
  (int64_t)0xD310CE1F89FC2200ull, // -3237861497225076224
  (int64_t)0xCA6ACDC11C161C00ull, // -3861047501233185792
  (int64_t)0xAF6178DCAFF5A800ull, // -5809229155090978816
  (int64_t)0x9AFE3153D877A400ull, // -7278325711600376832
  (int64_t)0x8B7B357A4C942C00ull, // -8396058280915096576
};

// 4. Imprecise unsigned integers: Numbers representable by a 64-bit uint, but
//    not representable in a double, so a rounding error occurs with uint64_t
//    -> double -> uint64_t conversion.
//    I.e. numbers [2^53+1, 2^64-1] for uint64 that are not representable as a
//    double. E.g. 0xffffffffffffffffull == 18,446,744,073,709,551,615 cannot
//    be stored in a double.
const uint64_t impreciseUnsignedIntegers[] = {
  0x20000000000001ull, // 9,007,199,254,740,993, error 1, largest consecutive double-precision floating point number plus one
  0x1C4FD83EC4ABAEEull, // 127505432520080110 error -2
  0x6A89C715876E7CCull, // 479805370944382924 error 12
  0xA4ABE649588F1E0ull, // 741614427870654944 error -32
  0xC980AC53EFE9DFCull, // 907487167196863996 error -4
  0x1860F52F16D4DA11ull, // 1756673437269809681 error 17
  0x198BF5B92CEFC68Dull, // 1840835048382645901 error -115
  0x2A02A453407B9F26ull, // 3027162577017478950 error -218
  0x36B24960B42D38E5ull, // 3941293303591155941 error 229
  0x702767EAC6668103ull, // 8081542314388259075 error 259
  0x80D1029A15D7ADAEull, // 9282203167801978286 error -594
  0x91604C04718E9B6Dull, // 10475456315232525165 error 877
  0xB4B9C5F0621A4DD9ull, // 13022657433747213785 error -551
  0xC138364191D593EBull, // 13922937903263355883 error 1003
  0xC7B40DCF0DBA8958ull, // 14390141892295297368 error 344
  0xF297E30AE976BAE9ull, // 17480690114667920105 error 745
  0xFAEA007C9BD7F40Dull, // 18080264189222843405 error -1011
  0xFBB8A15D8F62FC95ull, // 18138424922444332181 error -875
  0xFFFFFFFFFFFFFFFFull  // 18,446,744,073,709,551,615, largest uint64 integer. (-1 as int64)
};

// 5. Imprecise negative integers: Numbers representable by a 64-bit int, but
//    not representable in a double, so a rounding error occurs with int64_t ->
//    double -> int64_t conversion.
//    I.e. numbers [-2^63, -2^53-1] for int64 that are not representable as a double.
const int64_t impreciseNegativeIntegers[] = {
  -9007199254740993ll,            // -9007199254740992 minus one: smallest negative consecutive double-precision floating point number minus two
  (int64_t)0xFE23F334576C950Eull, // -133996157760400114 error -2
  (int64_t)0xFCD4520C047DF684ull, // -228467469520603516 error 4
  (int64_t)0xFC33BD80FF0149B0ull, // -273666790607730256 error -16
  (int64_t)0xEF76BD16E384CA04ull, // -1191557145388856828 error 4
  (int64_t)0xEDFB655E2E82185Full, // -1298332612384647073 error 95
  (int64_t)0xE4643C51E609E373ull, // -1989398812941491341 error 115
  (int64_t)0xD121A4BE92C4A969ull, // -3377237107138057879 error -151
  (int64_t)0xB53BA491A205DFDAull, // -5387531583823159334 error -38
  (int64_t)0x9592DF0A9FA4AEE9ull, // -7668821978737496343 error -279
  (int64_t)0x9027EEEFCC17CE38ull, // -8059210294467506632 error -456
  (int64_t)0x8818DC364AF41065ull, // -8639913759366442907 error 101
};

const double otherDoubles[] = {
  // 6. Rational numbers within range: double precision fractional numbers
  //    that are within [-2^63, 2^63-1] for int64 and [0, 2^64-1] for uint64, but
  //    not integers.
  DBL_TRUE_MIN, // smallest positive double (unnormalized)
  DBL_MIN, // smallest normalized positive double
  DBL_EPSILON, // smallest positive double so that 1+e != e
  0.1,
  0.25,
  0.5,
  0.75,
  1.912606627916564328,
  2.7463697084735994025,
  150655528000.36105347,
  679247267523850.5,
  967873430891084.25,
  1913278962515964.5,

  // 7. Out of range numbers: Double precision numbers >= 2^64 and < -2^63,
  //    i.e. they don't fit within an int64/uint64 range.
  DBL_MAX, // largest noninfinite double
  INFINITY, // +inf

  // 8. NaNs:
  // NAN
  // Ignoring payloaded NaNs for now.
};
