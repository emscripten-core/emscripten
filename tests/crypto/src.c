#include <emscripten.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

int main() {
  int rv;
  unsigned char buffer[1024] = { 0, };

  // Test the random number interface
  if ((rv = emscripten_crypto_random(buffer, 10)) < 0)
    printf("emscripten_crypto_random failed: %s\n", strerror(errno));
  if (!rv && (rv = emscripten_crypto_random(buffer+20, 10)) < 0)
    printf("emscripten_crypto_random failed: %s\n", strerror(errno));
  int sum;
  for (int i = 10; i < 20; ++i) sum += buffer[i];
  for (int i = 30; i < 40; ++i) sum += buffer[i];
  if (sum) { rv = -1; printf("random overran buffer\n"); }
  for (int i = 0; i < 10; ++i) sum += buffer[i];
  for (int i = 20; i < 30; ++i) sum += buffer[i];
  if (!sum) { rv = -1; printf("random generated zeros (bad)\n"); }
  sum = 0;
  for (int i = 0; i < 10; ++i) sum |= buffer[i] ^ buffer[i+20];
  if (!sum) {rv = -1; printf("random generated same output on two runs\n"); }
  printf("Random tests: %s\n", (rv < 0) ? "failed" : "passed");

  // Test the hash interface: basic SHA1 tests
  if ((rv = emscripten_crypto_open(42)) < 0 && errno == EINVAL)
    printf("emscripten_crypto_open(42) failed with EINVAL (OK)\n");
  if ((rv = emscripten_crypto_close(0)) < 0 && errno == EBADF)
    printf("emscripten_crypto_close(0) failed with EBADF (OK)\n");
  if ((rv = emscripten_crypto_open(EMSCRIPTEN_ALGORITHM_SHA1)) < 0)
    printf("emscripten_crypto_open(SHA1) failed\n");
  int cd1 = rv;
  const char* abc = "abc";
  unsigned char abcSha1[] = { 0xa9, 0x99, 0x3e, 0x36, 0x47, 0x06, 0x81, 0x6a,
                              0xba, 0x3e, 0x25, 0x71, 0x78, 0x50, 0xc2, 0x6c,
                              0x9c, 0xd0, 0xd8, 0x9d };
  if ((rv = emscripten_crypto_hash_update(cd1, (unsigned char*)abc, 3)) < 0)
    printf("emscripten_crypto_hash_update failed\n");
  if ((rv = emscripten_crypto_hash_final(cd1, buffer, sizeof(buffer))) != 20 ||
      memcmp(buffer, abcSha1, sizeof(abcSha1)))
    printf("emscripten_crypto_hash_final failed\n");
  else
    printf("SHA1 hash tests passed\n");

  // SHA-256 tests check incremental calls to update(), and final() to return a
  // truncated digest.
  if ((rv = emscripten_crypto_open(EMSCRIPTEN_ALGORITHM_SHA256)) < 0)
    printf("emscripten_crypto_open(SHA256) failed\n");
  int cd2 = rv;
  const char* big = "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq";
  unsigned char bigSha256[] = { 0x24,0x8d,0x6a,0x61,0xd2,0x06,0x38,0xb8,0xe5,
                                0xc0,0x26,0x93,0x0c,0x3e,0x60,0x39,0xa3,0x3c,
                                0xe4,0x59,0x64,0xff,0x21,0x67,0xf6,0xec,0xed,
                                0xd4,0x19,0xdb,0x06,0xc1, };
  if ((rv = emscripten_crypto_hash_update(cd2, (unsigned char*)big, 10)) < 0)
    printf("emscripten_crypto_hash_update failed\n");
  if ((rv = emscripten_crypto_hash_update(cd2, (unsigned char*)big+10, strlen(big)-10)) < 0)
    printf("emscripten_crypto_hash_update failed\n");
  if ((rv = emscripten_crypto_hash_final(cd2, buffer, 24)) != 24 ||
      memcmp(buffer, bigSha256, 24))
    printf("emscripten_crypto_hash_final failed\n");
  else
    printf("SHA256 hash tests passed\n");

  if ((rv = emscripten_crypto_close(cd1)) < 0)
    printf("emscripten_crypto_close failed\n");
  if ((rv = emscripten_crypto_close(cd2)) < 0)
    printf("emscripten_crypto_close failed\n");

  printf("tests done\n");

  return 0;
}
