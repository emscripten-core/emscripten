#include <iostream>
#include <emmintrin.h>
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <bitset>

using namespace std;

void testSetPs() {
  float __attribute__((__aligned__(16))) ar[4];
  __m128 v = _mm_set_ps(1.0, 2.0, 3.0, 4.0);
  _mm_store_ps(ar, v);
  assert(ar[0] == 4.0);
  assert(ar[1] == 3.0);
  assert(ar[2] == 2.0);
  assert(ar[3] == 1.0);
}

void testSet1Ps() {
  float __attribute__((__aligned__(16))) ar[4];
  __m128 v = _mm_set1_ps(5.5);
  _mm_store_ps(ar, v);
  assert(ar[0] == 5.5);
  assert(ar[1] == 5.5);
  assert(ar[2] == 5.5);
  assert(ar[3] == 5.5);
}

void testSetZeroPs() {
  float __attribute__((__aligned__(16))) ar[4];
  __m128 v = _mm_setzero_ps();
  _mm_store_ps(ar, v);
  assert(ar[0] == 0);
  assert(ar[1] == 0);
  assert(ar[2] == 0);
  assert(ar[3] == 0);
}

void testSetEpi32() {
  int32_t __attribute__((__aligned__(16))) ar[4];
  __m128i v = _mm_set_epi32(5, 7, 126, 381);
  _mm_store_si128((__m128i *)ar, v);
  assert(ar[0] == 381);
  assert(ar[1] == 126);
  assert(ar[2] == 7);
  assert(ar[3] == 5);
  v = _mm_set_epi32(0x55555555, 0xaaaaaaaa, 0xffffffff, 0x12345678);
  _mm_store_si128((__m128i *)ar, v);
  assert(ar[0] == 0x12345678);
  assert(ar[1] == 0xffffffff);
  assert(ar[2] == 0xaaaaaaaa);
  assert(ar[3] == 0x55555555);
}

void testSet1Epi32() {
  int32_t __attribute__((__aligned__(16))) ar[4];
  __m128i v = _mm_set1_epi32(-5);
  _mm_store_si128((__m128i *)ar, v);
  assert(ar[0] == -5);
  assert(ar[1] == -5);
  assert(ar[2] == -5);
  assert(ar[3] == -5);
}

void testSetZeroSi128() {
  int32_t __attribute__((__aligned__(16))) ar[4];
  __m128i v = _mm_setzero_si128();
  _mm_store_si128((__m128i *)ar, v);
  assert(ar[0] == 0);
  assert(ar[1] == 0);
  assert(ar[2] == 0);
  assert(ar[3] == 0);
}

void testBitCasts() {
  int32_t __attribute__((__aligned__(16))) ar1[4];
  float __attribute__((__aligned__(16))) ar2[4];
  __m128i v1 = _mm_set_epi32(0x3f800000, 0x40000000, 0x40400000, 0x40800000);
  __m128 v2 = _mm_castsi128_ps(v1);
  _mm_store_ps(ar2, v2);
  assert(ar2[0] == 4.0);
  assert(ar2[1] == 3.0);
  assert(ar2[2] == 2.0);
  assert(ar2[3] == 1.0);
  v2 = _mm_set_ps(5.0, 6.0, 7.0, 8.0);
  v1 = _mm_castps_si128(v2);
  _mm_store_si128((__m128i *)ar1, v1);
  assert(ar1[0] == 0x41000000);
  assert(ar1[1] == 0x40e00000);
  assert(ar1[2] == 0x40c00000);
  assert(ar1[3] == 0x40a00000);
  float w = 0;
  float z = -278.3;
  float y = 5.2;
  float x = -987654321;
  v1 = _mm_castps_si128(_mm_set_ps(w, z, y, x));
  _mm_store_ps(ar2, _mm_castsi128_ps(v1));
  assert(ar2[0] == x);
  assert(ar2[1] == y);
  assert(ar2[2] == z);
  assert(ar2[3] == w);
  /*
  std::bitset<sizeof(float)*CHAR_BIT> bits1x(*reinterpret_cast<unsigned
  long*>(&(ar2[0])));
  std::bitset<sizeof(float)*CHAR_BIT> bits1y(*reinterpret_cast<unsigned
  long*>(&(ar2[1])));
  std::bitset<sizeof(float)*CHAR_BIT> bits1z(*reinterpret_cast<unsigned
  long*>(&(ar2[2])));
  std::bitset<sizeof(float)*CHAR_BIT> bits1w(*reinterpret_cast<unsigned
  long*>(&(ar2[3])));
  std::bitset<sizeof(float)*CHAR_BIT> bits2x(*reinterpret_cast<unsigned
  long*>(&x));
  std::bitset<sizeof(float)*CHAR_BIT> bits2y(*reinterpret_cast<unsigned
  long*>(&y));
  std::bitset<sizeof(float)*CHAR_BIT> bits2z(*reinterpret_cast<unsigned
  long*>(&z));
  std::bitset<sizeof(float)*CHAR_BIT> bits2w(*reinterpret_cast<unsigned
  long*>(&w));
  assert(bits1x == bits2x);
  assert(bits1y == bits2y);
  assert(bits1z == bits2z);
  assert(bits1w == bits2w);
  */
  v2 = _mm_castsi128_ps(_mm_set_epi32(0xffffffff, 0, 0x5555cccc, 0xaaaaaaaa));
  _mm_store_si128((__m128i *)ar1, _mm_castps_si128(v2));
  assert(ar1[0] == 0xaaaaaaaa);
  assert(ar1[1] == 0x5555cccc);
  assert(ar1[2] == 0);
  assert(ar1[3] == 0xffffffff);
}

void testConversions() {
  int32_t __attribute__((__aligned__(16))) ar1[4];
  float __attribute__((__aligned__(16))) ar2[4];
  __m128i v1 = _mm_set_epi32(0, -3, -517, 256);
  __m128 v2 = _mm_cvtepi32_ps(v1);
  _mm_store_ps(ar2, v2);
  assert(ar2[0] == 256.0);
  assert(ar2[1] == -517.0);
  assert(ar2[2] == -3.0);
  assert(ar2[3] == 0);
  v2 = _mm_set_ps(5.0, 6.0, 7.45, -8.0);
  v1 = _mm_cvtps_epi32(v2);
  _mm_store_si128((__m128i *)ar1, v1);
  assert(ar1[0] == -8);
  assert(ar1[1] == 7);
  assert(ar1[2] == 6);
  assert(ar1[3] == 5);
}

void testMoveMaskPs() {
  __m128 v =
      _mm_castsi128_ps(_mm_set_epi32(0xffffffff, 0xffffffff, 0, 0xffffffff));
  int mask = _mm_movemask_ps(v);
  assert(mask == 13);
}

void testAddPs() {
  float __attribute__((__aligned__(16))) ar[4];
  __m128 v1 = _mm_set_ps(4.0, 3.0, 2.0, 1.0);
  __m128 v2 = _mm_set_ps(10.0, 20.0, 30.0, 40.0);
  __m128 v = _mm_add_ps(v1, v2);
  _mm_store_ps(ar, v);
  assert(ar[0] == 41.0);
  assert(ar[1] == 32.0);
  assert(ar[2] == 23.0);
  assert(ar[3] == 14.0);
}

void testSubPs() {
  float __attribute__((__aligned__(16))) ar[4];
  __m128 v1 = _mm_set_ps(4.0, 3.0, 2.0, 1.0);
  __m128 v2 = _mm_set_ps(10.0, 20.0, 30.0, 40.0);
  __m128 v = _mm_sub_ps(v1, v2);
  _mm_store_ps(ar, v);
  assert(ar[0] == -39.0);
  assert(ar[1] == -28.0);
  assert(ar[2] == -17.0);
  assert(ar[3] == -6.0);
}

void testMulPs() {
  float __attribute__((__aligned__(16))) ar[4];
  __m128 v1 = _mm_set_ps(4.0, 3.0, 2.0, 1.0);
  __m128 v2 = _mm_set_ps(10.0, 20.0, 30.0, 40.0);
  __m128 v = _mm_mul_ps(v1, v2);
  _mm_store_ps(ar, v);
  assert(ar[0] == 40.0);
  assert(ar[1] == 60.0);
  assert(ar[2] == 60.0);
  assert(ar[3] == 40.0);
}

void testDivPs() {
  float __attribute__((__aligned__(16))) ar[4];
  __m128 v1 = _mm_set_ps(4.0, 9.0, 8.0, 1.0);
  __m128 v2 = _mm_set_ps(2.0, 3.0, 1.0, 0.5);
  __m128 v = _mm_div_ps(v1, v2);
  _mm_store_ps(ar, v);
  assert(ar[0] == 2.0);
  assert(ar[1] == 8.0);
  assert(ar[2] == 3.0);
  assert(ar[3] == 2.0);
}

void testMinPs() {
  float __attribute__((__aligned__(16))) ar[4];
  __m128 v1 = _mm_set_ps(-20.0, 10.0, 30.0, 0.5);
  __m128 v2 = _mm_set_ps(2.0, 1.0, 50.0, 0.0);
  __m128 v = _mm_min_ps(v1, v2);
  _mm_store_ps(ar, v);
  assert(ar[0] == 0.0);
  assert(ar[1] == 30.0);
  assert(ar[2] == 1.0);
  assert(ar[3] == -20.0);
}

void testMaxPs() {
  float __attribute__((__aligned__(16))) ar[4];
  __m128 v1 = _mm_set_ps(-20.0, 10.0, 30.0, 0.5);
  __m128 v2 = _mm_set_ps(2.5, 5.0, 55.0, 1.0);
  __m128 v = _mm_max_ps(v1, v2);
  _mm_store_ps(ar, v);
  assert(ar[0] == 1.0);
  assert(ar[1] == 55.0);
  assert(ar[2] == 10.0);
  assert(ar[3] == 2.5);
}

void testSqrtPs() {
  float __attribute__((__aligned__(16))) ar[4];
  __m128 v1 = _mm_set_ps(16.0, 9.0, 4.0, 1.0);
  __m128 v = _mm_sqrt_ps(v1);
  _mm_store_ps(ar, v);
  assert(ar[0] == 1.0);
  assert(ar[1] == 2.0);
  assert(ar[2] == 3.0);
  assert(ar[3] == 4.0);
}

void testCmpLtPs() {
  int32_t __attribute__((__aligned__(16))) ar[4];
  __m128 v1 = _mm_set_ps(1.0, 2.0, 0.1, 0.001);
  __m128 v2 = _mm_set_ps(2.0, 2.0, 0.001, 0.1);
  __m128 v = _mm_cmplt_ps(v1, v2);
  _mm_store_si128((__m128i *)ar, _mm_castps_si128(v));
  assert(ar[0] == 0xffffffff);
  assert(ar[1] == 0);
  assert(ar[2] == 0);
  assert(ar[3] == 0xffffffff);
  assert(_mm_movemask_ps(v) == 9);
}

void testCmpLePs() {
  int32_t __attribute__((__aligned__(16))) ar[4];
  __m128 v1 = _mm_set_ps(1.0, 2.0, 0.1, 0.001);
  __m128 v2 = _mm_set_ps(2.0, 2.0, 0.001, 0.1);
  __m128 v = _mm_cmple_ps(v1, v2);
  _mm_store_si128((__m128i *)ar, _mm_castps_si128(v));
  assert(ar[0] == 0xffffffff);
  assert(ar[1] == 0);
  assert(ar[2] == 0xffffffff);
  assert(ar[3] == 0xffffffff);
  assert(_mm_movemask_ps(v) == 13);
}

void testCmpEqPs() {
  int32_t __attribute__((__aligned__(16))) ar[4];
  __m128 v1 = _mm_set_ps(1.0, 2.0, 0.1, 0.001);
  __m128 v2 = _mm_set_ps(2.0, 2.0, 0.001, 0.1);
  __m128 v = _mm_cmpeq_ps(v1, v2);
  _mm_store_si128((__m128i *)ar, _mm_castps_si128(v));
  assert(ar[0] == 0);
  assert(ar[1] == 0);
  assert(ar[2] == 0xffffffff);
  assert(ar[3] == 0);
  assert(_mm_movemask_ps(v) == 4);
}

void testCmpGePs() {
  int32_t __attribute__((__aligned__(16))) ar[4];
  __m128 v1 = _mm_set_ps(1.0, 2.0, 0.1, 0.001);
  __m128 v2 = _mm_set_ps(2.0, 2.0, 0.001, 0.1);
  __m128 v = _mm_cmpge_ps(v1, v2);
  _mm_store_si128((__m128i *)ar, _mm_castps_si128(v));
  assert(ar[0] == 0);
  assert(ar[1] == 0xffffffff);
  assert(ar[2] == 0xffffffff);
  assert(ar[3] == 0);
  assert(_mm_movemask_ps(v) == 6);
}

void testCmpGtPs() {
  int32_t __attribute__((__aligned__(16))) ar[4];
  __m128 v1 = _mm_set_ps(1.0, 2.0, 0.1, 0.001);
  __m128 v2 = _mm_set_ps(2.0, 2.0, 0.001, 0.1);
  __m128 v = _mm_cmpgt_ps(v1, v2);
  _mm_store_si128((__m128i *)ar, _mm_castps_si128(v));
  assert(ar[0] == 0);
  assert(ar[1] == 0xffffffff);
  assert(ar[2] == 0);
  assert(ar[3] == 0);
  assert(_mm_movemask_ps(v) == 2);
}

void testAndPs() {
  float __attribute__((__aligned__(16))) ar[4];
  __m128 v1 = _mm_set_ps(425, -501, -32, 68);
  __m128 v2 =
      _mm_castsi128_ps(_mm_set_epi32(0xffffffff, 0xffffffff, 0, 0xffffffff));
  __m128 v = _mm_and_ps(v1, v2);
  _mm_store_ps(ar, v);
  assert(ar[0] == 68);
  assert(ar[1] == 0);
  assert(ar[2] == -501);
  assert(ar[3] == 425);
  int32_t __attribute__((__aligned__(16))) ar2[4];
  v1 = _mm_castsi128_ps(
      _mm_set_epi32(0xaaaaaaaa, 0xaaaaaaaa, -1431655766, 0xaaaaaaaa));
  v2 = _mm_castsi128_ps(
      _mm_set_epi32(0x55555555, 0x55555555, 0x55555555, 0x55555555));
  v = _mm_and_ps(v1, v2);
  _mm_store_si128((__m128i *)ar2, _mm_castps_si128(v));
  assert(ar2[0] == 0);
  assert(ar2[1] == 0);
  assert(ar2[2] == 0);
  assert(ar2[3] == 0);
}

void testAndNotPs() {
  float __attribute__((__aligned__(16))) ar[4];
  __m128 v1 = _mm_set_ps(425, -501, -32, 68);
  __m128 v2 =
      _mm_castsi128_ps(_mm_set_epi32(0xffffffff, 0xffffffff, 0, 0xffffffff));
  __m128 v = _mm_andnot_ps(v2, v1);
  _mm_store_ps(ar, v);
  assert(ar[0] == 0);
  assert(ar[1] == -32);
  assert(ar[2] == 0);
  assert(ar[3] == 0);
  int32_t __attribute__((__aligned__(16))) ar2[4];
  v1 = _mm_castsi128_ps(
      _mm_set_epi32(0xaaaaaaaa, 0xaaaaaaaa, -1431655766, 0xaaaaaaaa));
  v2 = _mm_castsi128_ps(
      _mm_set_epi32(0x55555555, 0x55555555, 0x55555555, 0x55555555));
  v = _mm_andnot_ps(v1, v2);
  _mm_store_si128((__m128i *)ar2, _mm_castps_si128(v));
  assert(ar2[0] == 0x55555555);
  assert(ar2[1] == 0x55555555);
  assert(ar2[2] == 0x55555555);
  assert(ar2[3] == 0x55555555);
}

void testOrPs() {
  int32_t __attribute__((__aligned__(16))) ar[4];
  __m128 v1 =
      _mm_castsi128_ps(_mm_set_epi32(0xaaaaaaaa, 0xaaaaaaaa, 0xffffffff, 0));
  __m128 v2 = _mm_castsi128_ps(
      _mm_set_epi32(0x55555555, 0x55555555, 0x55555555, 0x55555555));
  __m128 v = _mm_or_ps(v1, v2);
  _mm_store_si128((__m128i *)ar, _mm_castps_si128(v));
  assert(ar[0] == 0x55555555);
  assert(ar[1] == 0xffffffff);
  assert(ar[2] == 0xffffffff);
  assert(ar[3] == 0xffffffff);
}

void testXorPs() {
  int32_t __attribute__((__aligned__(16))) ar[4];
  __m128 v1 =
      _mm_castsi128_ps(_mm_set_epi32(0xaaaaaaaa, 0xaaaaaaaa, 0xffffffff, 0));
  __m128 v2 = _mm_castsi128_ps(
      _mm_set_epi32(0x55555555, 0x55555555, 0x55555555, 0x55555555));
  __m128 v = _mm_xor_ps(v1, v2);
  _mm_store_si128((__m128i *)ar, _mm_castps_si128(v));
  assert(ar[0] == 0x55555555);
  assert(ar[1] == 0xaaaaaaaa);
  assert(ar[2] == 0xffffffff);
  assert(ar[3] == 0xffffffff);
}

void testAndSi128() {
  int32_t __attribute__((__aligned__(16))) ar[4];
  __m128i v1 = _mm_set_epi32(0xaaaaaaaa, 0xaaaaaaaa, -1431655766, 0xaaaaaaaa);
  __m128i v2 = _mm_set_epi32(0x55555555, 0x55555555, 0x55555555, 0x55555555);
  __m128i v = _mm_and_si128(v1, v2);
  _mm_store_si128((__m128i *)ar, v);
  assert(ar[0] == 0);
  assert(ar[1] == 0);
  assert(ar[2] == 0);
  assert(ar[3] == 0);
}

void testAndNotSi128() {
  int32_t __attribute__((__aligned__(16))) ar[4];
  __m128i v1 = _mm_set_epi32(0xaaaaaaaa, 0xaaaaaaaa, -1431655766, 0xaaaaaaaa);
  __m128i v2 = _mm_set_epi32(0x55555555, 0x55555555, 0x55555555, 0x55555555);
  __m128i v = _mm_andnot_si128(v1, v2);
  _mm_store_si128((__m128i *)ar, v);
  assert(ar[0] == 0x55555555);
  assert(ar[1] == 0x55555555);
  assert(ar[2] == 0x55555555);
  assert(ar[3] == 0x55555555);
}

void testOrSi128() {
  int32_t __attribute__((__aligned__(16))) ar[4];
  __m128i v1 = _mm_set_epi32(0xaaaaaaaa, 0xaaaaaaaa, 0xffffffff, 0);
  __m128i v2 = _mm_set_epi32(0x55555555, 0x55555555, 0x55555555, 0x55555555);
  __m128i v = _mm_or_si128(v1, v2);
  _mm_store_si128((__m128i *)ar, v);
  assert(ar[0] == 0x55555555);
  assert(ar[1] == 0xffffffff);
  assert(ar[2] == 0xffffffff);
  assert(ar[3] == 0xffffffff);
}

void testXorSi128() {
  int32_t __attribute__((__aligned__(16))) ar[4];
  __m128i v1 = _mm_set_epi32(0xaaaaaaaa, 0xaaaaaaaa, 0xffffffff, 0);
  __m128i v2 = _mm_set_epi32(0x55555555, 0x55555555, 0x55555555, 0x55555555);
  __m128i v = _mm_xor_si128(v1, v2);
  _mm_store_si128((__m128i *)ar, v);
  assert(ar[0] == 0x55555555);
  assert(ar[1] == 0xaaaaaaaa);
  assert(ar[2] == 0xffffffff);
  assert(ar[3] == 0xffffffff);
}

void testAddEpi32() {
  int32_t __attribute__((__aligned__(16))) ar[4];
  __m128i v1 = _mm_set_epi32(4, 3, 2, 1);
  __m128i v2 = _mm_set_epi32(10, 20, 30, 40);
  __m128i v = _mm_add_epi32(v1, v2);
  _mm_store_si128((__m128i *)ar, v);
  assert(ar[0] == 41);
  assert(ar[1] == 32);
  assert(ar[2] == 23);
  assert(ar[3] == 14);
}

void testSubEpi32() {
  int32_t __attribute__((__aligned__(16))) ar[4];
  __m128i v1 = _mm_set_epi32(4, 3, 2, 1);
  __m128i v2 = _mm_set_epi32(10, 20, 30, 40);
  __m128i v = _mm_sub_epi32(v1, v2);
  _mm_store_si128((__m128i *)ar, v);
  assert(ar[0] == -39);
  assert(ar[1] == -28);
  assert(ar[2] == -17);
  assert(ar[3] == -6);
}

int main(int argc, char **argv) {
  testSetPs();
  testSet1Ps();
  testSetZeroPs();
  testSetEpi32();
  testSet1Epi32();
  testSetZeroSi128();
  testBitCasts();
  testConversions();
  testMoveMaskPs();
  testAddPs();
  testSubPs();
  testMulPs();
  testDivPs();
  testMaxPs();
  testMinPs();
  testSqrtPs();
  testCmpLtPs();
  testCmpLePs();
  testCmpEqPs();
  testCmpGePs();
  testCmpGtPs();
  testAndPs();
  testAndNotPs();
  testOrPs();
  testXorPs();
  testAndSi128();
  testAndNotSi128();
  testOrSi128();
  testXorSi128();
  testAddEpi32();
  testSubEpi32();
  printf("DONE");
  return 0;
}
