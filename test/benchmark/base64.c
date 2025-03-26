// Copyright 2017 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

// https://github.com/kostya/benchmarks/blob/master/base64/test.c

#include "stdlib.h"
#include "stdio.h"
#include "time.h"
#include <stdint.h>

typedef unsigned int uint;
const char* chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static char decode_table[256];

int encode_size(int size) {
  return (int)(size * 4 / 3.0) + 6;
}

int decode_size(int size) {
  return (int)(size * 3 / 4.0) + 6;
}

void init_decode_table() {
  for (int i = 0; i < 256; i++) {
    char ch = (char)i;
    char code = -1;
    if (ch >= 'A' && ch <= 'Z') code = ch - 0x41;
    if (ch >= 'a' && ch <= 'z') code = ch - 0x47;
    if (ch >= '0' && ch <= '9') code = ch + 0x04; 
    if (ch == '+' || ch == '-') code = 0x3E;
    if (ch == '/' || ch == '_') code = 0x3F;
    decode_table[i] = code;
  }
}

#define next_char(x) char x = decode_table[(unsigned char)*str++]; if (x < 0) return 1;

int decode(int size, const char* str, int* out_size, char** output) {
  *output = (char*) malloc( decode_size(size) );
  char *out = *output;
  while (size > 0 && (str[size - 1] == '\n' || str[size - 1] == '\r' || str[size - 1] == '=')) size--;
  const char* ends = str + size - 4;
  while (1) {
    if (str > ends) break;
    while (*str == '\n' || *str == '\r') str++;

    if (str > ends) break;
    next_char(a); next_char(b); next_char(c); next_char(d);

    *out++ = (char)(a << 2 | b >> 4);
    *out++ = (char)(b << 4 | c >> 2);
    *out++ = (char)(c << 6 | d >> 0);
  }

  int mod = (str - ends) % 4;
  if (mod == 2) {
    next_char(a); next_char(b);
    *out++ = (char)(a << 2 | b >> 4);
  } else if (mod == 3) {
    next_char(a); next_char(b); next_char(c);
    *out++ = (char)(a << 2 | b >> 4);
    *out++ = (char)(b << 4 | c >> 2);
  }

  *out = '\0';
  *out_size = out - *output;
  return 0;
}

void encode(int size, const char* str, int* out_size, char** output) {
  *output = (char*) malloc( encode_size(size) );
  char *out = *output;
  const char* ends = str + (size - size % 3);
  uint n;
  while (str != ends) {
    uint32_t n = __builtin_bswap32(*(uint32_t*)str);
    *out++ = chars[(n >> 26) & 63];
    *out++ = chars[(n >> 20) & 63];
    *out++ = chars[(n >> 14) & 63];
    *out++ = chars[(n >> 8) & 63];
    str += 3;
  }
  int pd = size % 3;
  if  (pd == 1) {
    n = (uint)*str << 16;
    *out++ = chars[(n >> 18) & 63];
    *out++ = chars[(n >> 12) & 63];
    *out++ = '=';
    *out++ = '=';
  } else if (pd == 2) {
    n = (uint)*str++ << 16;
    n |= (uint)*str << 8;
    *out++ = chars[(n >> 18) & 63];
    *out++ = chars[(n >> 12) & 63];
    *out++ = chars[(n >> 6) & 63];
    *out++ = '=';
  }
  *out = '\0';
  *out_size = out - *output;
}

int main(int argc, char **argv) {
  int TRIES;

  int arg = argc > 1 ? argv[1][0] - '0' : 3;
  switch(arg) {
    case 0: return 0; break;
    case 1: TRIES = 3; break;
    case 2: TRIES = 15; break;
    case 3: TRIES = 25; break;
    case 4: TRIES = 50; break;
    case 5: TRIES = 100; break;
    default: printf("error: %d\\n", arg); return -1;
  }

  init_decode_table();

  const int STR_SIZE = 10000000;

  char *str = (char*) malloc(STR_SIZE + 1);
  for (int i = 0; i < STR_SIZE; i++) { str[i] = 'a'; }
  str[STR_SIZE] = '\0';

  int s = 0;
  clock_t t = clock();
  for (int i = 0; i < TRIES; i++) { 
    char *str2; 
    int str2_size;
    encode(STR_SIZE, str, &str2_size, &str2); 
    s += str2_size;
    free(str2); 
  }
  printf("encode: %d, %.2f\n", s, (float)(clock() - t)/CLOCKS_PER_SEC);

  char *str2;
  int str2_size;
  encode(STR_SIZE, str, &str2_size, &str2);

  s = 0;
  t = clock();
  for (int i = 0; i < TRIES; i++) {
    char *str3;
    int str3_size;
    if (decode(str2_size, str2, &str3_size, &str3) != 0) {
      printf("error when decoding");
    }
    s += str3_size;
    free(str3);
  }
  printf("decode: %d, %.2f\n", s, (float)(clock() - t)/CLOCKS_PER_SEC);
  return 0;
}
